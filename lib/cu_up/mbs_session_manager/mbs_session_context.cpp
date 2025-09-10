/*
 * 5G-MAG Reference Tools
 * Copyright (C) 2025 iTEAM UPV <borieher@iteam.upv.es>
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include "srsran/support/rate_limiting/token_bucket_config.h"
#include "srsran/sdap/sdap_factory.h"
#include "srsran/pdcp/pdcp_factory.h"
#include "srsran/f1u/cu_up/f1u_bearer_factory.h"
#include "srsran/e1ap/cu_up/e1ap_config_converters.h"
#include "mbs_session_context.h"

using namespace srsran;
using namespace srs_cu_up;

mbs_session_context::mbs_session_context(mbs_index_t                         mbs_index_,
                                         mbs_session_context_cfg             cfg_,
                                         mbs_session_id_t                    mbs_session_id_,
                                         std::optional<area_session_id_t>    area_session_id_,
                                         std::unique_ptr<ue_executor_mapper> mbs_session_exec_mapper_,
                                         ngu_session_manager&                ngu_session_mngr_,
                                         f1u_cu_up_gateway&                  f1u_gw_,
                                         gtpu_demux_ctrl&                    gtpu_rx_demux_,
                                         gtpu_teid_pool&                     f1u_teid_allocator_,
                                         timer_factory                       mbs_session_dl_timer_factory_,
                                         timer_factory                       mbs_session_ul_timer_factory_,
                                         timer_factory                       mbs_session_ctrl_timer_factory_,
                                         dlt_pcap&                           gtpu_pcap_) :
  mbs_index(mbs_index_),
  cfg(cfg_),
  mbs_session_id(mbs_session_id_),
  area_session_id(area_session_id_),
  mbs_session_exec_mapper(std::move(mbs_session_exec_mapper_)),
  ngu_session_mngr(ngu_session_mngr_),
  f1u_gw(f1u_gw_),
  gtpu_rx_demux(gtpu_rx_demux_),
  f1u_teid_allocator(f1u_teid_allocator_),
  mbs_session_dl_exec(mbs_session_exec_mapper->dl_pdu_executor()),
  mbs_session_ul_exec(mbs_session_exec_mapper->ul_pdu_executor()),
  mbs_session_ctrl_exec(mbs_session_exec_mapper->ctrl_executor()),
  crypto_exec(mbs_session_exec_mapper->crypto_executor()),
  mbs_session_dl_timer_factory(mbs_session_dl_timer_factory_),
  mbs_session_ul_timer_factory(mbs_session_ul_timer_factory_),
  mbs_session_ctrl_timer_factory(mbs_session_ctrl_timer_factory_),
  gtpu_pcap(gtpu_pcap_)
{
  // Get the configured MBS Session AMBR
  mbs_session_dl_ambr = cfg.mbs_session_dl_ambr;

  token_bucket_config mbs_session_ambr_config =
      generate_token_bucket_config(mbs_session_dl_ambr, mbs_session_dl_ambr, timer_duration(100), mbs_session_dl_timer_factory);
  mbs_session_ambr_limiter = std::make_unique<token_bucket>(mbs_session_ambr_config);

  // NOTE (borieher): It is needed but it won't be used
  mbs_session_inactivity_timer = mbs_session_ctrl_timer_factory.create_timer();
  // mbs_session_inactivity_timer.set(*cfg.ue_inactivity_timeout,
  //     [this](timer_id_t /*tid*/) { on_ue_inactivity_timer_expired(); });
  // mbs_session_inactivity_timer.run();
}

mbs_broadcast_session_setup_result
mbs_session_context::setup_mbs_broadcast_session(e1ap_bc_bearer_context_to_setup& bc_bearer_context_to_setup)
{
  mbs_broadcast_session_setup_result broadcast_session_setup_result = {};
  broadcast_session_setup_result.success                  = false;
  broadcast_session_setup_result.cause                    = e1ap_cause_radio_network_t::unspecified;

  // NOTE (borieher): MAX_NUM_OF_MRBS_PER_MBS_SESSION?
  // if (pdu_sessions.size() >= MAX_NUM_PDU_SESSIONS_PER_UE) {
  //   logger.log_error("PDU Session for {} cannot be created. Max number of PDU sessions reached",
  //                    session.pdu_session_id);
  //   return pdu_session_result;
  // }

  // NOTE (borieher): local N3 TEID comes pre-allocated from 5G-Core side (C-TEID)
  // Extract C-TEID and N3mb address from BC Bearer Context NG-U TNL Info at 5GC (O)
  expected<gtpu_teid_t> local_teid = make_unexpected(default_error_t{});
  std::string n3mb_addr;
  if (bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_info_at_5gc.has_value()) {
    // Parse location dependent
    if (bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_info_at_5gc.value().is_locationdependent()) {
      const auto& locationdependent = bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_info_at_5gc.value().get_locationdependent();

      for (const auto& locationdependent_item : locationdependent.location_dependent_mbs_ngu_info_at_5gc) {
        // Only check for the configured MBS Area Session ID
        if (area_session_id.has_value()) {
          if (locationdependent_item.mbs_area_session_id == area_session_id.value()) {
            // Grab C-TEID as local TEID
            local_teid = locationdependent_item.mbs_ngu_information_at_5gc.gtp_dl_teid;
            // Grab multicast address as N3mb address
            n3mb_addr = locationdependent_item.mbs_ngu_information_at_5gc.ip_multicast_address.to_string();
          }
        }
      }
    // Parse location independent
    } else {
      const auto& locationindependent = bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_info_at_5gc.value().get_locationindependent();
      // Grab C-TEID as local TEID
      local_teid = locationindependent.mbs_ngu_information_at_5gc.gtp_dl_teid;
      // Grab multicast address as N3mb address
      n3mb_addr = locationindependent.mbs_ngu_information_at_5gc.ip_multicast_address.to_string();
    }
  }

  if (!local_teid.has_value()) {
    //logger.log_warning("Failed to create PDU session. Cause: could not allocate local TEID. {}",
                       //session.pdu_session_id);
    return broadcast_session_setup_result;
  } else {
    c_teid = local_teid.value();
    // NOTE (borieher): Not sure if this should be done here or in the 5G-Core prior to send
    c_teid = int_to_gtpu_teid(htonl(c_teid.value()));
  }

  // TODO select correct GW based on slice or UE info.
  std::string           n3_addr;
  // NOTE (borieher): Having pdu_session in the name here is a bit misleading, it is an MBS Session
  gtpu_tnl_pdu_session& n3_gw = ngu_session_mngr.get_next_ngu_gateway();
  if (not n3_gw.get_bind_address(n3_addr)) {
    report_error("Could not get NG-U bind address to report to core.");
  }

  // Join N3mb multicast group for this MBS Session
  n3_gw.join_multicast_group(n3mb_addr);

  broadcast_session_setup_result.gtp_tunnel =
      up_transport_layer_info(transport_layer_address::create_from_string(n3mb_addr), int_to_gtpu_teid(c_teid.value()));

  // Create SDAP entity
  // TODO (borieher): Make this function more flexible by using ue_index or mbs_index, pdu_session_id_t or mbs_session_id_t and DRBs or MRBs.
  sdap_entity_creation_message sdap_msg = {(uint32_t)mbs_index, (pdu_session_id_t)0, &sdap_to_gtpu_adapter};
  sdap = create_sdap(sdap_msg);

  // TODO (borieher): Create GTPU entity
  gtpu_tunnel_ngu_creation_message gtpu_msg = {};
  gtpu_msg.ue_index                         = (ue_index_t) mbs_index;
  gtpu_msg.cfg.tx.peer_teid                 = int_to_gtpu_teid(9999); // NOTE (borieher): Dummy value, because we do not care about UL TEID in MBS
  gtpu_msg.cfg.tx.peer_addr                 = "1.1.1.1";              // NOTE (borieher): Dummy value, because we do not care about UL address in MBS
  gtpu_msg.cfg.tx.peer_port                 = cfg.n3_config.upf_port;
  gtpu_msg.cfg.rx.local_teid                = c_teid;
  gtpu_msg.cfg.rx.ignore_ue_ambr            = cfg.n3_config.gtpu_ignore_ue_ambr;
  gtpu_msg.cfg.rx.ue_ambr_limiter           = mbs_session_ambr_limiter.get();
  gtpu_msg.cfg.rx.t_reordering              = cfg.n3_config.gtpu_reordering_timer;
  gtpu_msg.cfg.rx.warn_on_drop              = cfg.n3_config.warn_on_drop;
  gtpu_msg.cfg.rx.test_mode                 = cfg.test_mode_cfg.enabled;
  gtpu_msg.rx_lower                         = &gtpu_to_sdap_adapter;
  gtpu_msg.tx_upper                         = &gtpu_to_udp_adapter;
  gtpu_msg.gtpu_pcap                        = &gtpu_pcap;
  gtpu_msg.ue_ctrl_timer_factory            = mbs_session_ctrl_timer_factory;
  gtpu = create_gtpu_tunnel_ngu(gtpu_msg);

  sdap_to_gtpu_adapter.connect_gtpu(*gtpu->get_tx_lower_layer_interface());
  gtpu_to_sdap_adapter.connect_sdap(sdap->get_sdap_tx_sdu_handler());
  gtpu_to_udp_adapter.connect_network_gateway(n3_gw);

  // Register tunnel at demux
  expected<std::unique_ptr<gtpu_demux_dispatch_queue>> expected_dispatch_queue =
      gtpu_rx_demux.add_tunnel(c_teid, mbs_session_dl_exec, gtpu->get_rx_upper_layer_interface());
  if (!expected_dispatch_queue) {
    logger.error(
        "MBS Session cannot be created. C-TEID {} already exists", c_teid);
    return broadcast_session_setup_result;
  }
  dispatch_queue = std::move(expected_dispatch_queue.value());

  // TODO (borieher): Handle MRB setup
  for (const auto& mrb_to_setup_item : bc_bearer_context_to_setup.bc_mrb_to_setup_list) {
    mrb_setup_result mrb_result = handle_mrb_to_setup_item(mrb_to_setup_item);
    broadcast_session_setup_result.mrb_setup_results.push_back(mrb_result);
  }

  return broadcast_session_setup_result;
}

mrb_setup_result
mbs_session_context::handle_mrb_to_setup_item(const e1ap_bc_mrb_setup_config& mrb_to_setup_item)
{
  auto&    cpu_desc  = cpu_architecture_info::get();
  uint32_t nof_cores = cpu_desc.get_host_nof_available_cpus();

  // Prepare MRB creation result
  mrb_setup_result mrb_result = {};
  mrb_result.success          = false;
  mrb_result.cause            = e1ap_cause_radio_network_t::unspecified;
  mrb_result.mrb_id           = mrb_to_setup_item.mrb_id;

  // Check 5QI exists before creating MRB
  if (mrb_to_setup_item.mbs_qos_flow_info_to_be_setup.empty()) {
    return mrb_result;
  }
  five_qi_t five_qi = mrb_to_setup_item.mbs_qos_flow_info_to_be_setup.begin()->qos_flow_level_qos_params.qos_desc.get_5qi();
  if (cfg.qos.find(five_qi) == cfg.qos.end()) {
    mrb_result.cause = e1ap_cause_radio_network_t::not_supported_5qi_value;
    return mrb_result;
  }

  // Get MRB from list and create context
  mrbs.emplace(mrb_to_setup_item.mrb_id, std::make_unique<mrb_context>(mrb_to_setup_item.mrb_id));
  mrb_context* new_mrb = mrbs.at(mrb_to_setup_item.mrb_id).get();

  // Create QoS Flows
  uint32_t nof_flow_success = 0;
  for (const auto& mbs_qos_flow_info : mrb_to_setup_item.mbs_qos_flow_info_to_be_setup) {
    // Prepare MBS QoS flow creation result
    mbs_qos_flow_setup_result flow_result = {};
    flow_result.success               = false;
    flow_result.cause                 = e1ap_cause_radio_network_t::unspecified;
    flow_result.qos_flow_id           = mbs_qos_flow_info.qos_flow_id;

    if (!sdap->is_mapped(mbs_qos_flow_info.qos_flow_id) &&
         mbs_qos_flow_info.qos_flow_level_qos_params.qos_desc.get_5qi() == five_qi) {
      // Create MBS QoS flow context
      const auto& qos_flow                     = mbs_qos_flow_info;
      new_mrb->qos_flows[qos_flow.qos_flow_id] = std::make_unique<qos_flow_context>(qos_flow);
      auto& new_qos_flow                       = new_mrb->qos_flows[qos_flow.qos_flow_id];
      logger.debug("Created QoS flow with {} and {}", new_qos_flow->qos_flow_id, new_qos_flow->five_qi);

      // NOTE (borieher): Seems like it is not being used
      //sdap_config sdap_cfg = make_sdap_drb_config(drb_to_setup.sdap_cfg);
      sdap_config sdap_cfg = {};
      sdap->add_mapping(
          qos_flow.qos_flow_id, mrb_to_setup_item.mrb_id, sdap_cfg, new_qos_flow->sdap_to_pdcp_adapter);
      flow_result.success = true;
      nof_flow_success++;
    } else {
      // Fail if mapping already exists
      flow_result.success = false;
      flow_result.cause   = sdap->is_mapped(mbs_qos_flow_info.qos_flow_id)
                                ? e1ap_cause_radio_network_t::multiple_qos_flow_id_instances
                                : e1ap_cause_radio_network_t::not_supported_5qi_value;
      logger.error("Cannot overwrite existing mapping for {}", mbs_qos_flow_info.qos_flow_id);
    }

    // Add QoS flow creation result
    mrb_result.mbs_qos_flow_results.push_back(flow_result);
  }

  // If no QoS flow could be created, remove the rest of the dangling MRB
  if (nof_flow_success == 0) {
    logger.error("Failed to create {}: Could not map any QoS flow", mrb_to_setup_item.mrb_id);
    mrbs.erase(mrb_to_setup_item.mrb_id);
    mrb_result.cause   = e1ap_cause_radio_network_t::unspecified;
    mrb_result.success = false;
    return mrb_result;
  }

  // If 5QI is not configured in CU-UP, remove the rest of the dangling MRB
  if (cfg.qos.find(five_qi) == cfg.qos.end()) {
    logger.error("Failed to create {}: Could not find 5QI. {}", mrb_to_setup_item.mrb_id, five_qi);
    mrbs.erase(mrb_to_setup_item.mrb_id);
    mrb_result.cause   = e1ap_cause_radio_network_t::not_supported_5qi_value;
    mrb_result.success = false;
    return mrb_result;
  }

  // Create PDCP entity
  srsran::pdcp_entity_creation_message pdcp_msg = {};
  pdcp_msg.ue_index                             = (ue_index_t) mbs_index;
  pdcp_msg.rb_id                                = mrb_to_setup_item.mrb_id;
  pdcp_msg.config                               = make_pdcp_mrb_config(mrb_to_setup_item.mbs_pdcp_cfg);
  pdcp_msg.config.custom                        = cfg.qos.at(five_qi).pdcp_custom_cfg;
  pdcp_msg.tx_lower                             = &new_mrb->pdcp_to_f1u_adapter;
  pdcp_msg.tx_upper_cn                          = &new_mrb->pdcp_tx_to_e1ap_adapter;
  pdcp_msg.rx_upper_dn                          = &new_mrb->pdcp_to_sdap_adapter;
  pdcp_msg.rx_upper_cn                          = &new_mrb->pdcp_rx_to_e1ap_adapter;
  pdcp_msg.ue_dl_timer_factory                  = mbs_session_dl_timer_factory;
  pdcp_msg.ue_ul_timer_factory                  = mbs_session_ul_timer_factory;
  pdcp_msg.ue_ctrl_timer_factory                = mbs_session_ctrl_timer_factory;
  pdcp_msg.ue_dl_executor                       = &mbs_session_dl_exec;
  pdcp_msg.ue_ul_executor                       = &mbs_session_ul_exec;
  pdcp_msg.ue_ctrl_executor                     = &mbs_session_ctrl_exec;
  pdcp_msg.crypto_executor                      = &crypto_exec;
  pdcp_msg.max_nof_crypto_workers               = nof_cores;
  new_mrb->pdcp                                 = srsran::create_pdcp_entity(pdcp_msg);

  // NOTE (borieher): Disabling PDCP integrity and ciphering
  // security::sec_128_as_config sec_128 = security::truncate_config(security_info);
  security::sec_128_as_config sec_128 = {};
  sec_128.domain = security::sec_domain::up;
  // NOTE (borieher): MBS does not use PDCP integrity nor ciphering
  auto integrity_enabled = security::integrity_enabled::off;
  auto ciphering_enabled = security::ciphering_enabled::off;

  // configure tx security
  auto& pdcp_tx_ctrl = new_mrb->pdcp->get_tx_upper_control_interface();
  pdcp_tx_ctrl.configure_security(sec_128, integrity_enabled, ciphering_enabled);

  // configure rx security
  auto& pdcp_rx_ctrl = new_mrb->pdcp->get_rx_upper_control_interface();
  pdcp_rx_ctrl.configure_security(sec_128, integrity_enabled, ciphering_enabled);

  // Connect "PDCP-E1AP" adapter to E1AP
  new_mrb->pdcp_tx_to_e1ap_adapter.connect_e1ap(); // TODO: pass actual E1AP handler
  new_mrb->pdcp_rx_to_e1ap_adapter.connect_e1ap(); // TODO: pass actual E1AP handler

  // Create F1-U bearer
  new_mrb->f1u_cfg = cfg.qos.at(five_qi).f1u_cfg;

  expected<gtpu_teid_t> ret = f1u_teid_allocator.request_teid();
  if (not ret.has_value()) {
    logger.error("Could not allocate ul_teid");
    return mrb_result;
  }
  gtpu_teid_t f1u_ul_teid = ret.value();

  new_mrb->f1u_gw_bearer = f1u_gw.create_cu_bearer((ue_index_t) mbs_index,
                                                 mrb_to_setup_item.mrb_id,
                                                 five_qi,
                                                 new_mrb->f1u_cfg,
                                                 f1u_ul_teid,
                                                 new_mrb->f1u_gateway_rx_to_nru_adapter,
                                                 mbs_session_ul_exec);

  // Create UL UP TNL address.
  expected<std::string> bind_addr = new_mrb->f1u_gw_bearer->get_bind_address();
  if (not bind_addr.has_value()) {
    logger.error("Could not get bind address for F1-U tunnel");
    return mrb_result;
  }
  up_transport_layer_info f1u_ul_tunnel_addr(transport_layer_address::create_from_string(bind_addr.value()),
                                             f1u_ul_teid);

  new_mrb->f1u = srs_cu_up::create_f1u_bearer((uint32_t) mbs_index,
                                              new_mrb->mrb_id,
                                              f1u_ul_tunnel_addr,
                                              new_mrb->f1u_cfg,
                                              *new_mrb->f1u_gw_bearer,
                                              new_mrb->f1u_to_pdcp_adapter,
                                              new_mrb->f1u_to_pdcp_adapter,
                                              mbs_session_ctrl_timer_factory,
                                              mbs_session_inactivity_timer,
                                              mbs_session_dl_exec,
                                              mbs_session_ul_exec);

  new_mrb->f1u_ul_teid = f1u_ul_teid;

  mrb_result.gtp_tunnel = f1u_ul_tunnel_addr;

  // Connect F1-U GW bearer RX adapter to NR-U bearer
  new_mrb->f1u_gateway_rx_to_nru_adapter.connect_nru_bearer(new_mrb->f1u->get_rx_pdu_handler());

  // Connect F1-U's "F1-U->PDCP adapter" directly to PDCP
  new_mrb->f1u_to_pdcp_adapter.connect_pdcp(new_mrb->pdcp->get_rx_lower_interface(),
                                            new_mrb->pdcp->get_tx_lower_interface());
  new_mrb->pdcp_to_f1u_adapter.connect_f1u(new_mrb->f1u->get_tx_sdu_handler());

  // Connect QoS flows to DRB
  for (auto& new_qos_flow : new_mrb->qos_flows) {
    new_qos_flow.second->sdap_to_pdcp_adapter.connect_pdcp(new_mrb->pdcp->get_tx_upper_data_interface());
    new_mrb->pdcp_to_sdap_adapter.connect_sdap(sdap->get_sdap_rx_pdu_handler(mrb_to_setup_item.mrb_id));
  }

  // Add result
  mrb_result.success = true;

  return mrb_result;
}
