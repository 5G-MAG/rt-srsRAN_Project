/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
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

#pragma once

#include "../common/e1ap_asn1_converters.h"
#include "srsran/asn1/asn1_utils.h"
#include "srsran/asn1/e1ap/e1ap_ies.h"
#include "srsran/asn1/e1ap/e1ap_pdu_contents.h"
#include "srsran/ran/qos/qos_prio_level.h"

namespace srsran {
namespace srs_cu_cp {

inline void fill_asn1_qos_flow_level_qos_params(asn1::e1ap::qos_flow_level_qos_params_s& asn1_qos_flow_level_qos_params,
                                                const e1ap_qos_flow_level_qos_params&    qos_flow_level_qos_params)
{
  // Fill QoS Characteristics (M).
  // Fill dynamic 5QI.
  if (qos_flow_level_qos_params.qos_desc.is_dyn_5qi()) {
    auto& dynamic_5qi = qos_flow_level_qos_params.qos_desc.get_dyn_5qi();
    asn1_qos_flow_level_qos_params.qos_characteristics.set_dyn_5qi();
    auto& asn1_dynamic_5qi = asn1_qos_flow_level_qos_params.qos_characteristics.dyn_5qi();

    asn1_dynamic_5qi.qos_prio_level                 = dynamic_5qi.qos_prio_level.value();
    asn1_dynamic_5qi.packet_delay_budget            = dynamic_5qi.packet_delay_budget;
    asn1_dynamic_5qi.packet_error_rate.per_scalar   = dynamic_5qi.per.scalar;
    asn1_dynamic_5qi.packet_error_rate.per_exponent = dynamic_5qi.per.exponent;
    if (dynamic_5qi.five_qi.has_value()) {
      asn1_dynamic_5qi.five_qi_present = true;
      asn1_dynamic_5qi.five_qi         = five_qi_to_uint(dynamic_5qi.five_qi.value());
    }
    if (dynamic_5qi.is_delay_critical.has_value()) {
      asn1_dynamic_5qi.delay_crit_present = true;
      asn1_dynamic_5qi.delay_crit.value   = dynamic_5qi.is_delay_critical.value()
                                                ? asn1::e1ap::dyn_5qi_descriptor_s::delay_crit_opts::delay_crit
                                                : asn1::e1ap::dyn_5qi_descriptor_s::delay_crit_opts::non_delay_crit;
    }
    if (dynamic_5qi.averaging_win.has_value()) {
      asn1_dynamic_5qi.averaging_win_present = true;
      asn1_dynamic_5qi.averaging_win         = dynamic_5qi.averaging_win.value();
    }
    if (dynamic_5qi.max_data_burst_volume.has_value()) {
      asn1_dynamic_5qi.max_data_burst_volume_present = true;
      asn1_dynamic_5qi.max_data_burst_volume         = dynamic_5qi.max_data_burst_volume.value();
    }
  } else /* Fill non dynamic 5QI. */ {
    auto& non_dynamic_5qi = qos_flow_level_qos_params.qos_desc.get_nondyn_5qi();
    asn1_qos_flow_level_qos_params.qos_characteristics.set_non_dyn_5qi();
    auto& asn1_non_dynamic_5qi = asn1_qos_flow_level_qos_params.qos_characteristics.non_dyn_5qi();

    asn1_non_dynamic_5qi.five_qi = five_qi_to_uint(non_dynamic_5qi.five_qi);

    if (non_dynamic_5qi.qos_prio_level.has_value()) {
      asn1_non_dynamic_5qi.qos_prio_level_present = true;
      asn1_non_dynamic_5qi.qos_prio_level         = non_dynamic_5qi.qos_prio_level.value().value();
    }
    if (non_dynamic_5qi.averaging_win.has_value()) {
      asn1_non_dynamic_5qi.averaging_win_present = true;
      asn1_non_dynamic_5qi.averaging_win         = non_dynamic_5qi.averaging_win.value();
    }
    if (non_dynamic_5qi.max_data_burst_volume.has_value()) {
      asn1_non_dynamic_5qi.max_data_burst_volume_present = true;
      asn1_non_dynamic_5qi.max_data_burst_volume         = non_dynamic_5qi.max_data_burst_volume.value();
    }
  }

  // Fill NG-RAN Allocation and Retention Priority (M).
  asn1_qos_flow_level_qos_params.ngra_nalloc_retention_prio.prio_level =
      qos_flow_level_qos_params.ng_ran_alloc_retention.prio_level_arp.value();
  asn1_qos_flow_level_qos_params.ngra_nalloc_retention_prio.pre_emption_cap.value =
      qos_flow_level_qos_params.ng_ran_alloc_retention.may_trigger_preemption
          ? asn1::e1ap::pre_emption_cap_opts::may_trigger_pre_emption
          : asn1::e1ap::pre_emption_cap_opts::shall_not_trigger_pre_emption;
  asn1_qos_flow_level_qos_params.ngra_nalloc_retention_prio.pre_emption_vulnerability.value =
      qos_flow_level_qos_params.ng_ran_alloc_retention.is_preemptable
          ? asn1::e1ap::pre_emption_vulnerability_opts::pre_emptable
          : asn1::e1ap::pre_emption_vulnerability_opts::not_pre_emptable;

  // Fill GBR QoS Flow Information (O).
  if (qos_flow_level_qos_params.gbr_qos_flow_info.has_value()) {
    asn1_qos_flow_level_qos_params.gbr_qos_flow_info_present = true;
    asn1_qos_flow_level_qos_params.gbr_qos_flow_info.max_flow_bit_rate_dl =
        qos_flow_level_qos_params.gbr_qos_flow_info.value().max_br_dl;
    asn1_qos_flow_level_qos_params.gbr_qos_flow_info.max_flow_bit_rate_ul =
        qos_flow_level_qos_params.gbr_qos_flow_info.value().max_br_ul;
    asn1_qos_flow_level_qos_params.gbr_qos_flow_info.guaranteed_flow_bit_rate_dl =
        qos_flow_level_qos_params.gbr_qos_flow_info.value().gbr_dl;
    asn1_qos_flow_level_qos_params.gbr_qos_flow_info.guaranteed_flow_bit_rate_ul =
        qos_flow_level_qos_params.gbr_qos_flow_info.value().gbr_ul;
    if (qos_flow_level_qos_params.gbr_qos_flow_info.value().max_packet_loss_rate_dl.has_value()) {
      asn1_qos_flow_level_qos_params.gbr_qos_flow_info.max_packet_loss_rate_dl_present = true;
      asn1_qos_flow_level_qos_params.gbr_qos_flow_info.max_packet_loss_rate_dl =
          qos_flow_level_qos_params.gbr_qos_flow_info.value().max_packet_loss_rate_dl.value();
    }
    if (qos_flow_level_qos_params.gbr_qos_flow_info.value().max_packet_loss_rate_ul.has_value()) {
      asn1_qos_flow_level_qos_params.gbr_qos_flow_info.max_packet_loss_rate_ul_present = true;
      asn1_qos_flow_level_qos_params.gbr_qos_flow_info.max_packet_loss_rate_ul =
          qos_flow_level_qos_params.gbr_qos_flow_info.value().max_packet_loss_rate_ul.value();
    }
  }

  // Fill Reflective QoS Attribute (O).
  if (qos_flow_level_qos_params.reflective_qos_attribute.has_value()) {
    asn1_qos_flow_level_qos_params.reflective_qos_attribute_present = true;
    asn1::bool_to_enum(asn1_qos_flow_level_qos_params.reflective_qos_attribute,
                       qos_flow_level_qos_params.reflective_qos_attribute.value());
  }

  // Fill Additional QoS Flow Information (O).
  if (qos_flow_level_qos_params.add_qos_info.has_value()) {
    asn1_qos_flow_level_qos_params.add_qos_info_present = true;
    asn1::bool_to_enum(asn1_qos_flow_level_qos_params.add_qos_info,
                       qos_flow_level_qos_params.add_qos_info.value());
  }

  // Fill Paging Policy Index (O).
  if (qos_flow_level_qos_params.paging_policy_ind.has_value()) {
    asn1_qos_flow_level_qos_params.paging_policy_idx_present = true;
    asn1_qos_flow_level_qos_params.paging_policy_idx =
        qos_flow_level_qos_params.paging_policy_ind.value();
  }

  // Fill Reflective QoS indication (O).
  if (qos_flow_level_qos_params.reflective_qos_ind.has_value()) {
    asn1_qos_flow_level_qos_params.reflective_qos_ind_present = true;
    asn1::bool_to_enum(asn1_qos_flow_level_qos_params.reflective_qos_ind,
                       qos_flow_level_qos_params.reflective_qos_ind.value());
  }
}

inline void fill_asn1_qos_flow_info_item(asn1::e1ap::qos_flow_qos_param_item_s& asn1_qos_flow_info_item,
                                         const e1ap_qos_flow_qos_param_item&    qos_flow_info_item)
{
  asn1_qos_flow_info_item.qos_flow_id = qos_flow_id_to_uint(qos_flow_info_item.qos_flow_id);

  // Fill QoS flow level Qos params.
  auto& qos_flow_level_params = qos_flow_info_item.qos_flow_level_qos_params;
  fill_asn1_qos_flow_level_qos_params(asn1_qos_flow_info_item.qos_flow_level_qos_params, qos_flow_level_params);

  // Fill QoS flow map indication.
  if (qos_flow_info_item.qos_flow_map_ind.has_value()) {
    asn1_qos_flow_info_item.qos_flow_map_ind_present = true;
    asn1_qos_flow_info_item.qos_flow_map_ind =
        static_cast<asn1::e1ap::qos_flow_map_ind_opts::options>((int)qos_flow_info_item.qos_flow_map_ind.value());
  }
}

inline void fill_asn1_drb_to_setup_item(asn1::e1ap::drb_to_setup_item_ng_ran_s& asn1_drb_to_setup_item,
                                        const e1ap_drb_to_setup_item_ng_ran&    drb_to_setup_item)
{
  asn1_drb_to_setup_item.drb_id = drb_id_to_uint(drb_to_setup_item.drb_id);

  // Fill SDAP config.
  asn1_drb_to_setup_item.sdap_cfg = sdap_config_to_e1ap_asn1(drb_to_setup_item.sdap_cfg);

  // Fill PDCP config.
  asn1_drb_to_setup_item.pdcp_cfg = pdcp_config_to_e1ap_asn1(drb_to_setup_item.pdcp_cfg);

  // Fill cell group info.
  for (const auto& cell_group_info_item : drb_to_setup_item.cell_group_info) {
    asn1::e1ap::cell_group_info_item_s asn1_cell_group_info_item;

    asn1_cell_group_info_item.cell_group_id = cell_group_info_item.cell_group_id;

    if (cell_group_info_item.ul_cfg.has_value()) {
      asn1_cell_group_info_item.ul_cfg_present = true;
      asn1_cell_group_info_item.ul_cfg =
          static_cast<asn1::e1ap::ul_cfg_opts::options>((int)cell_group_info_item.ul_cfg.value());
    }

    if (cell_group_info_item.dl_tx_stop.has_value()) {
      asn1_cell_group_info_item.dl_tx_stop_present = true;
      asn1_cell_group_info_item.dl_tx_stop =
          static_cast<asn1::e1ap::dl_tx_stop_opts::options>((int)cell_group_info_item.dl_tx_stop.value());
    }

    if (cell_group_info_item.rat_type.has_value()) {
      asn1_cell_group_info_item.rat_type_present = true;
      asn1_cell_group_info_item.rat_type =
          static_cast<asn1::e1ap::rat_type_opts::options>((int)cell_group_info_item.rat_type.value());
    }

    asn1_drb_to_setup_item.cell_group_info.push_back(asn1_cell_group_info_item);
  }

  // Fill QoS flow info to be setup.
  for (const auto& qos_flow_info_item : drb_to_setup_item.qos_flow_info_to_be_setup) {
    asn1::e1ap::qos_flow_qos_param_item_s asn1_qos_flow_info_item;
    fill_asn1_qos_flow_info_item(asn1_qos_flow_info_item, qos_flow_info_item);
    asn1_drb_to_setup_item.qos_flow_info_to_be_setup.push_back(asn1_qos_flow_info_item);
  }

  // Fill DRB data forwarding info request.
  if (drb_to_setup_item.drb_data_forwarding_info_request.has_value()) {
    asn1_drb_to_setup_item.drb_data_forwarding_info_request_present = true;
    asn1::string_to_enum(asn1_drb_to_setup_item.drb_data_forwarding_info_request.data_forwarding_request,
                         drb_to_setup_item.drb_data_forwarding_info_request.value().data_forwarding_request);
    for (const auto& qos_flow_map_item :
         drb_to_setup_item.drb_data_forwarding_info_request.value().qos_flows_forwarded_on_fwd_tunnels) {
      asn1::e1ap::qos_flow_map_item_s asn1_qos_flow_map_item;
      asn1_qos_flow_map_item.qos_flow_id = qos_flow_id_to_uint(qos_flow_map_item.qos_flow_id);
      if (qos_flow_map_item.qos_flow_map_ind.has_value()) {
        asn1_qos_flow_map_item.qos_flow_map_ind_present = true;
        asn1_qos_flow_map_item.qos_flow_map_ind =
            static_cast<asn1::e1ap::qos_flow_map_ind_opts::options>((int)qos_flow_map_item.qos_flow_map_ind.value());
      }

      asn1_drb_to_setup_item.drb_data_forwarding_info_request.qos_flows_forwarded_on_fwd_tunnels.push_back(
          asn1_qos_flow_map_item);
    }
  }

  // Fill DRB inactivity timer.
  if (drb_to_setup_item.drb_inactivity_timer.has_value()) {
    asn1_drb_to_setup_item.drb_inactivity_timer_present = true;
    asn1_drb_to_setup_item.drb_inactivity_timer         = drb_to_setup_item.drb_inactivity_timer.value().count();
  }

  // Fill PDCP SN status info.
  if (drb_to_setup_item.pdcp_sn_status_info.has_value()) {
    asn1_drb_to_setup_item.pdcp_sn_status_info_present = true;

    asn1_drb_to_setup_item.pdcp_sn_status_info.pdcp_status_transfer_ul.count_value.pdcp_sn =
        drb_to_setup_item.pdcp_sn_status_info.value().pdcp_status_transfer_ul.count_value.pdcp_sn;
    asn1_drb_to_setup_item.pdcp_sn_status_info.pdcp_status_transfer_ul.count_value.hfn =
        drb_to_setup_item.pdcp_sn_status_info.value().pdcp_status_transfer_ul.count_value.hfn;
    if (drb_to_setup_item.pdcp_sn_status_info.value().pdcp_status_transfer_ul.receive_status_of_pdcp_sdu.has_value()) {
      asn1_drb_to_setup_item.pdcp_sn_status_info.pdcp_status_transfer_ul.receive_statusof_pdcp_sdu_present = true;
      asn1_drb_to_setup_item.pdcp_sn_status_info.pdcp_status_transfer_ul.receive_statusof_pdcp_sdu.from_number(
          drb_to_setup_item.pdcp_sn_status_info.value().pdcp_status_transfer_ul.receive_status_of_pdcp_sdu.value());
    }

    asn1_drb_to_setup_item.pdcp_sn_status_info.pdcp_status_transfer_dl.pdcp_sn =
        drb_to_setup_item.pdcp_sn_status_info.value().pdcp_status_transfer_dl.pdcp_sn;
    asn1_drb_to_setup_item.pdcp_sn_status_info.pdcp_status_transfer_dl.hfn =
        drb_to_setup_item.pdcp_sn_status_info.value().pdcp_status_transfer_dl.hfn;
  }
}

inline asn1::e1ap::pdu_session_type_e pdu_session_type_to_asn1(const pdu_session_type_t& pdu_session_type)
{
  switch (pdu_session_type) {
    case pdu_session_type_t::ipv4:
      return asn1::e1ap::pdu_session_type_e::ipv4;
    case pdu_session_type_t::ipv6:
      return asn1::e1ap::pdu_session_type_e::ipv6;
    case pdu_session_type_t::ipv4v6:
      return asn1::e1ap::pdu_session_type_e::ipv4v6;
    default:
      return asn1::e1ap::pdu_session_type_e::ethernet;
  }
}

inline void fill_asn1_bearer_context_setup_request(asn1::e1ap::bearer_context_setup_request_s& asn1_request,
                                                   const e1ap_bearer_context_setup_request&    request)
{
  // Fill security info.
  asn1_request->security_info.security_algorithm.ciphering_algorithm =
      ciphering_algorithm_to_e1ap_asn1(request.security_info.security_algorithm.ciphering_algo);
  if (request.security_info.security_algorithm.integrity_protection_algorithm.has_value()) {
    asn1_request->security_info.security_algorithm.integrity_protection_algorithm_present = true;
    asn1_request->security_info.security_algorithm.integrity_protection_algorithm = integrity_algorithm_to_e1ap_asn1(
        request.security_info.security_algorithm.integrity_protection_algorithm.value());
  }
  asn1_request->security_info.up_securitykey.encryption_key =
      request.security_info.up_security_key.encryption_key.copy();
  asn1_request->security_info.up_securitykey.integrity_protection_key =
      request.security_info.up_security_key.integrity_protection_key.copy();

  // Fill UE DL aggregate maximum bit rate.
  asn1_request->ue_dl_aggr_max_bit_rate = request.ue_dl_aggregate_maximum_bit_rate;

  // Fill serving PLMN.
  asn1_request->serving_plmn = request.serving_plmn.to_bytes();

  // Fill activity notification level.
  asn1::string_to_enum(asn1_request->activity_notif_level, request.activity_notif_level);

  // Fill PDU session resource to setup list.
  asn1_request->sys_bearer_context_setup_request.set_ng_ran_bearer_context_setup_request();
  auto& ng_ran_bearer_context_setup_request =
      asn1_request->sys_bearer_context_setup_request.ng_ran_bearer_context_setup_request();

  for (const auto& pdu_session_res_item : request.pdu_session_res_to_setup_list) {
    asn1::protocol_ie_field_s<asn1::e1ap::ng_ran_bearer_context_setup_request_o> bearer_request_item;
    auto& asn1_pdu_session_res_items = bearer_request_item.value().pdu_session_res_to_setup_list();

    asn1::e1ap::pdu_session_res_to_setup_item_s asn1_pdu_session_res_item;

    // Fill PDU session ID.
    asn1_pdu_session_res_item.pdu_session_id = pdu_session_id_to_uint(pdu_session_res_item.pdu_session_id);

    // Fill PDU session type.
    asn1_pdu_session_res_item.pdu_session_type = pdu_session_type_to_asn1(pdu_session_res_item.pdu_session_type);

    // Fill S-NSSAI.
    asn1_pdu_session_res_item.snssai = snssai_to_e1ap_asn1(pdu_session_res_item.snssai);

    // Fill NG UL UP transport layer information.
    up_transport_layer_info_to_asn1(asn1_pdu_session_res_item.ng_ul_up_tnl_info,
                                    pdu_session_res_item.ng_ul_up_tnl_info);

    // Fill security indication.
    security_indication_to_asn1(asn1_pdu_session_res_item.security_ind, pdu_session_res_item.security_ind);

    // Fill DRB to setup list NG RAN.
    for (const auto& drb_to_setup_item : pdu_session_res_item.drb_to_setup_list_ng_ran) {
      asn1::e1ap::drb_to_setup_item_ng_ran_s asn1_drb_to_setup_item;
      fill_asn1_drb_to_setup_item(asn1_drb_to_setup_item, drb_to_setup_item);
      asn1_pdu_session_res_item.drb_to_setup_list_ng_ran.push_back(asn1_drb_to_setup_item);
    }

    // Fill PDU session resource DL aggregate maximum bit rate.
    if (pdu_session_res_item.pdu_session_res_dl_ambr.has_value()) {
      asn1_pdu_session_res_item.pdu_session_res_dl_ambr_present = true;
      asn1_pdu_session_res_item.pdu_session_res_dl_ambr         = pdu_session_res_item.pdu_session_res_dl_ambr.value();
    }

    // Fill PDU session data forwarding info request.
    if (pdu_session_res_item.pdu_session_data_forwarding_info_request.has_value()) {
      asn1_pdu_session_res_item.pdu_session_data_forwarding_info_request_present = true;

      asn1::string_to_enum(
          asn1_pdu_session_res_item.pdu_session_data_forwarding_info_request.data_forwarding_request,
          pdu_session_res_item.pdu_session_data_forwarding_info_request.value().data_forwarding_request);
      for (const auto& qos_flow_map_item :
           pdu_session_res_item.pdu_session_data_forwarding_info_request.value().qos_flows_forwarded_on_fwd_tunnels) {
        asn1::e1ap::qos_flow_map_item_s asn1_qos_flow_map_item;
        asn1_qos_flow_map_item.qos_flow_id = qos_flow_id_to_uint(qos_flow_map_item.qos_flow_id);
        if (qos_flow_map_item.qos_flow_map_ind.has_value()) {
          asn1_qos_flow_map_item.qos_flow_map_ind_present = true;
          asn1_qos_flow_map_item.qos_flow_map_ind =
              static_cast<asn1::e1ap::qos_flow_map_ind_opts::options>((int)qos_flow_map_item.qos_flow_map_ind.value());
        }

        asn1_pdu_session_res_item.pdu_session_data_forwarding_info_request.qos_flows_forwarded_on_fwd_tunnels.push_back(
            asn1_qos_flow_map_item);
      }
    }

    // Fill PDU session inactivity timer.
    if (pdu_session_res_item.pdu_session_inactivity_timer.has_value()) {
      asn1_pdu_session_res_item.pdu_session_inactivity_timer_present = true;
      asn1_pdu_session_res_item.pdu_session_inactivity_timer =
          pdu_session_res_item.pdu_session_inactivity_timer.value().count();
    }

    // Fill existing allocated NG DL UP TNL info.
    if (pdu_session_res_item.existing_allocated_ng_dl_up_tnl_info.has_value()) {
      asn1_pdu_session_res_item.existing_allocated_ng_dl_up_tnl_info_present = true;
      up_transport_layer_info_to_asn1(asn1_pdu_session_res_item.existing_allocated_ng_dl_up_tnl_info,
                                      pdu_session_res_item.existing_allocated_ng_dl_up_tnl_info.value());
    }

    // Fill network instance.
    if (pdu_session_res_item.network_instance.has_value()) {
      asn1_pdu_session_res_item.network_instance_present = true;
      asn1_pdu_session_res_item.network_instance         = pdu_session_res_item.network_instance.value();
    }

    asn1_pdu_session_res_items.push_back(asn1_pdu_session_res_item);

    ng_ran_bearer_context_setup_request.push_back(bearer_request_item);
  }

  // UE DL maximum integrity protection data rate.
  if (request.ue_dl_maximum_integrity_protected_data_rate.has_value()) {
    asn1_request->ue_dl_max_integrity_protected_data_rate_present = true;
    asn1_request->ue_dl_max_integrity_protected_data_rate = request.ue_dl_maximum_integrity_protected_data_rate.value();
  }

  // Fill UE inactivity timer.
  if (request.ue_inactivity_timer.has_value()) {
    asn1_request->ue_inactivity_timer_present = true;
    asn1_request->ue_inactivity_timer         = request.ue_inactivity_timer.value().count();
  }

  // Fill bearer context status change.
  if (request.bearer_context_status_change.has_value()) {
    asn1_request->bearer_context_status_change_present = true;
    asn1_request->bearer_context_status_change = static_cast<asn1::e1ap::bearer_context_status_change_opts::options>(
        (int)request.bearer_context_status_change.value());
  }

  // Fill RAN UE ID.
  if (request.ran_ue_id.has_value()) {
    asn1_request->ran_ue_id_present = true;
    asn1_request->ran_ue_id.from_number(ran_ue_id_to_uint(request.ran_ue_id.value()));
  }

  // Fill GNB DU ID.
  if (request.gnb_du_id.has_value()) {
    asn1_request->gnb_du_id_present = true;
    asn1_request->gnb_du_id         = request.gnb_du_id.value();
  }
}

inline void
fill_e1ap_bearer_context_setup_response(e1ap_bearer_context_setup_response&            res,
                                        const asn1::e1ap::bearer_context_setup_resp_s& asn1_bearer_context_setup_resp)
{
  // Fail if E-UTRAN bearer context setup is returned.
  if (asn1_bearer_context_setup_resp->sys_bearer_context_setup_resp.type() ==
      asn1::e1ap::sys_bearer_context_setup_resp_c::types::e_utran_bearer_context_setup_resp) {
    res.success = false;
    res.cause   = cause_protocol_t::unspecified;
    return;
  } else {
    res.success = true;

    auto& asn1_bearer_context_setup_response =
        asn1_bearer_context_setup_resp->sys_bearer_context_setup_resp.ng_ran_bearer_context_setup_resp();

    for (const auto& asn1_res_setup_item : asn1_bearer_context_setup_response.pdu_session_res_setup_list) {
      e1ap_pdu_session_resource_setup_modification_item res_setup_item;
      res_setup_item.pdu_session_id = uint_to_pdu_session_id(asn1_res_setup_item.pdu_session_id);

      // Fill NG DL UP TNL info.
      res_setup_item.ng_dl_up_tnl_info = asn1_to_up_transport_layer_info(asn1_res_setup_item.ng_dl_up_tnl_info);

      // Fill DRB setup list NG RAN.
      for (const auto& asn1_drb_setup_item : asn1_res_setup_item.drb_setup_list_ng_ran) {
        e1ap_drb_setup_item_ng_ran drb_setup_item;
        drb_setup_item.drb_id = uint_to_drb_id(asn1_drb_setup_item.drb_id);

        // Fill UL UP transport params.
        for (const auto& asn1_ul_up_transport_param : asn1_drb_setup_item.ul_up_transport_params) {
          e1ap_up_params_item ue_params_item;
          ue_params_item.up_tnl_info   = asn1_to_up_transport_layer_info(asn1_ul_up_transport_param.up_tnl_info);
          ue_params_item.cell_group_id = asn1_ul_up_transport_param.cell_group_id;

          drb_setup_item.ul_up_transport_params.push_back(ue_params_item);
        }

        // Fill flow setup list.
        for (auto& asn1_qos_flow_item : asn1_drb_setup_item.flow_setup_list) {
          e1ap_qos_flow_item qos_flow_item;
          qos_flow_item.qos_flow_id = uint_to_qos_flow_id(asn1_qos_flow_item.qos_flow_id);

          drb_setup_item.flow_setup_list.emplace(qos_flow_item.qos_flow_id, qos_flow_item);
        }

        // Fill flow failed list.
        for (const auto& asn1_failed_qos_flow_item : asn1_drb_setup_item.flow_failed_list) {
          e1ap_qos_flow_failed_item failed_qos_flow_item;

          failed_qos_flow_item.qos_flow_id = uint_to_qos_flow_id(asn1_failed_qos_flow_item.qos_flow_id);
          failed_qos_flow_item.cause       = asn1_to_cause(asn1_failed_qos_flow_item.cause);

          drb_setup_item.flow_failed_list.emplace(failed_qos_flow_item.qos_flow_id, failed_qos_flow_item);
        }

        // Fill DRB data forwarding info response.
        if (asn1_drb_setup_item.drb_data_forwarding_info_resp_present) {
          if (asn1_drb_setup_item.drb_data_forwarding_info_resp.ul_data_forwarding_present) {
            drb_setup_item.drb_data_forwarding_info_resp.value().ul_data_forwarding =
                asn1_to_up_transport_layer_info(asn1_drb_setup_item.drb_data_forwarding_info_resp.ul_data_forwarding);
          }
          if (asn1_drb_setup_item.drb_data_forwarding_info_resp.dl_data_forwarding_present) {
            drb_setup_item.drb_data_forwarding_info_resp.value().dl_data_forwarding =
                asn1_to_up_transport_layer_info(asn1_drb_setup_item.drb_data_forwarding_info_resp.dl_data_forwarding);
          }
        }

        res_setup_item.drb_setup_list_ng_ran.emplace(drb_setup_item.drb_id, drb_setup_item);
      }

      // Fill DRB failed list NG RAN.
      for (const auto& asn1_drb_failed_item : asn1_res_setup_item.drb_failed_list_ng_ran) {
        e1ap_drb_failed_item_ng_ran drb_failed_item;
        drb_failed_item.drb_id = uint_to_drb_id(asn1_drb_failed_item.drb_id);
        drb_failed_item.cause  = asn1_to_cause(asn1_drb_failed_item.cause);

        res_setup_item.drb_failed_list_ng_ran.emplace(drb_failed_item.drb_id, drb_failed_item);
      }

      // Fill security result.
      if (asn1_res_setup_item.security_result_present) {
        res_setup_item.security_result = security_result_t{};
        asn1_to_security_result(res_setup_item.security_result.value(), asn1_res_setup_item.security_result);
      }

      // Fill PDU session data forwarding info response.
      if (asn1_res_setup_item.pdu_session_data_forwarding_info_resp_present) {
        if (asn1_res_setup_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding_present) {
          res_setup_item.pdu_session_data_forwarding_info_resp.value().ul_data_forwarding =
              asn1_to_up_transport_layer_info(
                  asn1_res_setup_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding);
        }
        if (asn1_res_setup_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding_present) {
          res_setup_item.pdu_session_data_forwarding_info_resp.value().dl_data_forwarding =
              asn1_to_up_transport_layer_info(
                  asn1_res_setup_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding);
        }
      }

      // Fill NG DL UP unchanged.
      if (asn1_res_setup_item.ng_dl_up_unchanged_present) {
        res_setup_item.ng_dl_up_unchanged.value() =
            e1ap_asn1_ng_dl_up_unchanged_to_bool(asn1_res_setup_item.ng_dl_up_unchanged);
      }

      res.pdu_session_resource_setup_list.emplace(res_setup_item.pdu_session_id, res_setup_item);
    }

    // Fill PDU session res failed list.
    if (asn1_bearer_context_setup_response.pdu_session_res_failed_list_present) {
      for (const auto& asn1_failed_item : asn1_bearer_context_setup_response.pdu_session_res_failed_list) {
        e1ap_pdu_session_resource_failed_item failed_item;

        failed_item.pdu_session_id = uint_to_pdu_session_id(asn1_failed_item.pdu_session_id);
        failed_item.cause          = asn1_to_cause(asn1_failed_item.cause);

        res.pdu_session_resource_failed_list.emplace(failed_item.pdu_session_id, failed_item);
      }
    }
  }
}

inline void
fill_e1ap_bearer_context_setup_response(e1ap_bearer_context_setup_response&            res,
                                        const asn1::e1ap::bearer_context_setup_fail_s& e1ap_bearer_context_setup_fail)
{
  res.success = false;
  res.cause   = asn1_to_cause(e1ap_bearer_context_setup_fail->cause);
  if (e1ap_bearer_context_setup_fail->crit_diagnostics_present) {
    // TODO: Add crit diagnostics.
  }
}

inline void fill_asn1_bearer_context_modification_request(asn1::e1ap::bearer_context_mod_request_s&       asn1_request,
                                                          const e1ap_bearer_context_modification_request& request)
{
  if (request.new_ul_tnl_info_required.has_value()) {
    asn1_request->new_ul_tnl_info_required_present = true;
    asn1::bool_to_enum(asn1_request->new_ul_tnl_info_required, request.new_ul_tnl_info_required.value());
  }

  // Fill security info.
  if (request.security_info.has_value()) {
    asn1_request->security_info_present = true;
    asn1_request->security_info.security_algorithm.ciphering_algorithm =
        ciphering_algorithm_to_e1ap_asn1(request.security_info->security_algorithm.ciphering_algo);
    if (request.security_info->security_algorithm.integrity_protection_algorithm.has_value()) {
      asn1_request->security_info.security_algorithm.integrity_protection_algorithm_present = true;
      asn1_request->security_info.security_algorithm.integrity_protection_algorithm = integrity_algorithm_to_e1ap_asn1(
          request.security_info->security_algorithm.integrity_protection_algorithm.value());
    }
    asn1_request->security_info.up_securitykey.encryption_key =
        request.security_info->up_security_key.encryption_key.copy();
    asn1_request->security_info.up_securitykey.integrity_protection_key =
        request.security_info->up_security_key.integrity_protection_key.copy();
  }

  // Fill NG RAN bearer context mod.
  if (request.ng_ran_bearer_context_mod_request.has_value()) {
    asn1_request->sys_bearer_context_mod_request_present = true;
    asn1_request->sys_bearer_context_mod_request.set_ng_ran_bearer_context_mod_request();
    auto& asn1_bearer_context_mod = asn1_request->sys_bearer_context_mod_request.ng_ran_bearer_context_mod_request();

    if (!request.ng_ran_bearer_context_mod_request.value().pdu_session_res_to_modify_list.empty()) {
      asn1_bearer_context_mod.pdu_session_res_to_modify_list_present = true;
      for (const auto& res_to_mod_item :
           request.ng_ran_bearer_context_mod_request.value().pdu_session_res_to_modify_list) {
        asn1::e1ap::pdu_session_res_to_modify_item_s asn1_res_to_mod_item;
        asn1_res_to_mod_item.pdu_session_id = pdu_session_id_to_uint(res_to_mod_item.pdu_session_id);

        for (const auto& drb_to_setup_item : res_to_mod_item.drb_to_setup_list_ng_ran) {
          asn1::e1ap::drb_to_setup_item_ng_ran_s asn1_drb_to_setup_item;
          fill_asn1_drb_to_setup_item(asn1_drb_to_setup_item, drb_to_setup_item);
          asn1_res_to_mod_item.drb_to_setup_list_ng_ran.push_back(asn1_drb_to_setup_item);
        }

        for (const auto& drb_to_mod_item : res_to_mod_item.drb_to_modify_list_ng_ran) {
          asn1::e1ap::drb_to_modify_item_ng_ran_s asn1_drb_to_mod_item;

          asn1_drb_to_mod_item.drb_id = drb_id_to_uint(drb_to_mod_item.drb_id);

          for (const auto& dl_up_param : drb_to_mod_item.dl_up_params) {
            asn1::e1ap::up_params_item_s asn1_dl_up_param;
            up_transport_layer_info_to_asn1(asn1_dl_up_param.up_tnl_info, dl_up_param.up_tnl_info);
            asn1_dl_up_param.cell_group_id = dl_up_param.cell_group_id;
            asn1_drb_to_mod_item.dl_up_params.push_back(asn1_dl_up_param);
          }

          if (drb_to_mod_item.pdcp_cfg.has_value()) {
            asn1_drb_to_mod_item.pdcp_cfg_present = true;
            asn1_drb_to_mod_item.pdcp_cfg         = pdcp_config_to_e1ap_asn1(drb_to_mod_item.pdcp_cfg.value());
          }

          asn1_res_to_mod_item.drb_to_modify_list_ng_ran.push_back(asn1_drb_to_mod_item);
        }

        for (const auto& drb_to_rem_item : res_to_mod_item.drb_to_rem_list_ng_ran) {
          asn1::e1ap::drb_to_rem_item_ng_ran_s asn1_drb_to_rem_item;
          asn1_drb_to_rem_item.drb_id = drb_id_to_uint(drb_to_rem_item);
          asn1_res_to_mod_item.drb_to_rem_list_ng_ran.push_back(asn1_drb_to_rem_item);
        }

        asn1_bearer_context_mod.pdu_session_res_to_modify_list.push_back(asn1_res_to_mod_item);
      }
    }

    if (!request.ng_ran_bearer_context_mod_request.value().pdu_session_res_to_setup_mod_list.empty()) {
      asn1_bearer_context_mod.pdu_session_res_to_setup_mod_list_present = true;

      for (const auto& res_to_setup_mod_item :
           request.ng_ran_bearer_context_mod_request.value().pdu_session_res_to_setup_mod_list) {
        asn1::e1ap::pdu_session_res_to_setup_mod_item_s asn1_res_to_setup_mod_item;
        asn1_res_to_setup_mod_item.pdu_session_id = pdu_session_id_to_uint(res_to_setup_mod_item.pdu_session_id);

        // Fill PDU session type.
        asn1_res_to_setup_mod_item.pdu_session_type = pdu_session_type_to_asn1(res_to_setup_mod_item.pdu_session_type);

        // Fill S-NSSAI.
        asn1_res_to_setup_mod_item.snssai = snssai_to_e1ap_asn1(res_to_setup_mod_item.snssai);

        // Fill NG UL UP transport layer information.
        up_transport_layer_info_to_asn1(asn1_res_to_setup_mod_item.ng_ul_up_tnl_info,
                                        res_to_setup_mod_item.ng_ul_up_tnl_info);

        // Fill security indication.
        security_indication_to_asn1(asn1_res_to_setup_mod_item.security_ind, res_to_setup_mod_item.security_ind);

        for (const auto& drb_to_setup_mod_item : res_to_setup_mod_item.drb_to_setup_list_ng_ran) {
          asn1::e1ap::drb_to_setup_mod_item_ng_ran_s asn1_drb_to_setup_mod_item;
          asn1_drb_to_setup_mod_item.drb_id = drb_id_to_uint(drb_to_setup_mod_item.drb_id);

          // Fill SDAP config.
          asn1_drb_to_setup_mod_item.sdap_cfg = sdap_config_to_e1ap_asn1(drb_to_setup_mod_item.sdap_cfg);

          // Fill PDCP config.
          asn1_drb_to_setup_mod_item.pdcp_cfg = pdcp_config_to_e1ap_asn1(drb_to_setup_mod_item.pdcp_cfg);

          for (const auto& cell_group_info_item : drb_to_setup_mod_item.cell_group_info) {
            asn1::e1ap::cell_group_info_item_s asn1_cell_group_info_item;

            asn1_cell_group_info_item.cell_group_id = cell_group_info_item.cell_group_id;

            if (cell_group_info_item.ul_cfg.has_value()) {
              asn1_cell_group_info_item.ul_cfg_present = true;
              asn1_cell_group_info_item.ul_cfg =
                  static_cast<asn1::e1ap::ul_cfg_opts::options>((int)cell_group_info_item.ul_cfg.value());
            }

            if (cell_group_info_item.dl_tx_stop.has_value()) {
              asn1_cell_group_info_item.dl_tx_stop_present = true;
              asn1_cell_group_info_item.dl_tx_stop =
                  static_cast<asn1::e1ap::dl_tx_stop_opts::options>((int)cell_group_info_item.dl_tx_stop.value());
            }

            if (cell_group_info_item.rat_type.has_value()) {
              asn1_cell_group_info_item.rat_type_present = true;
              asn1_cell_group_info_item.rat_type =
                  static_cast<asn1::e1ap::rat_type_opts::options>((int)cell_group_info_item.rat_type.value());
            }

            asn1_drb_to_setup_mod_item.cell_group_info.push_back(asn1_cell_group_info_item);
          }

          // Fill QoS flow map info.
          for (const auto& qos_flow_info_item : drb_to_setup_mod_item.qos_flow_info_to_be_setup) {
            asn1::e1ap::qos_flow_qos_param_item_s asn1_qos_flow_info_item;
            fill_asn1_qos_flow_info_item(asn1_qos_flow_info_item, qos_flow_info_item);
            asn1_drb_to_setup_mod_item.flow_map_info.push_back(asn1_qos_flow_info_item);
          }

          // TODO: DL/UL tunnel info missing?
          asn1_res_to_setup_mod_item.drb_to_setup_mod_list_ng_ran.push_back(asn1_drb_to_setup_mod_item);
        }

        asn1_bearer_context_mod.pdu_session_res_to_setup_mod_list.push_back(asn1_res_to_setup_mod_item);
      }
    }
    if (!request.ng_ran_bearer_context_mod_request.value().pdu_session_res_to_rem_list.empty()) {
      asn1_bearer_context_mod.pdu_session_res_to_rem_list_present = true;
      for (const auto& psi_to_rem : request.ng_ran_bearer_context_mod_request.value().pdu_session_res_to_rem_list) {
        asn1::e1ap::pdu_session_res_to_rem_item_s asn1_res_to_rem_item;
        asn1_res_to_rem_item.pdu_session_id = pdu_session_id_to_uint(psi_to_rem);
        asn1_bearer_context_mod.pdu_session_res_to_rem_list.push_back(asn1_res_to_rem_item);
      }
    }
  }
}

inline void fill_e1ap_bearer_context_modification_response(
    e1ap_bearer_context_modification_response&   res,
    const asn1::e1ap::bearer_context_mod_resp_s& asn1_bearer_context_modification_resp)
{
  res.success = true;
  if (asn1_bearer_context_modification_resp->sys_bearer_context_mod_resp_present) {
    // Fail if E-UTRAN bearer context setup is returned.
    if (asn1_bearer_context_modification_resp->sys_bearer_context_mod_resp.type() ==
        asn1::e1ap::sys_bearer_context_mod_resp_c::types::e_utran_bearer_context_mod_resp) {
      res.success = false;
      res.cause   = cause_protocol_t::unspecified;
      return;
    } else {
      // Fill NG RAN bearer context modification response.
      auto& asn1_bearer_context_mod_response =
          asn1_bearer_context_modification_resp->sys_bearer_context_mod_resp.ng_ran_bearer_context_mod_resp();

      // Fill PDU session resource setup list.
      if (asn1_bearer_context_mod_response.pdu_session_res_setup_mod_list_present) {
        for (const auto& asn1_res_mod_item : asn1_bearer_context_mod_response.pdu_session_res_setup_mod_list) {
          e1ap_pdu_session_resource_setup_modification_item res_mod_item;
          res_mod_item.pdu_session_id = uint_to_pdu_session_id(asn1_res_mod_item.pdu_session_id);

          // Fill NG DL UP TNL info.
          res_mod_item.ng_dl_up_tnl_info = asn1_to_up_transport_layer_info(asn1_res_mod_item.ng_dl_up_tnl_info);

          // Fill DRB setup list NG RAN.
          for (const auto& asn1_drb_setup_item : asn1_res_mod_item.drb_setup_mod_list_ng_ran) {
            e1ap_drb_setup_item_ng_ran drb_setup_item;
            drb_setup_item.drb_id = uint_to_drb_id(asn1_drb_setup_item.drb_id);

            // Fill UL UP transport params.
            for (const auto& asn1_ul_up_transport_param : asn1_drb_setup_item.ul_up_transport_params) {
              e1ap_up_params_item ue_params_item;
              ue_params_item.up_tnl_info   = asn1_to_up_transport_layer_info(asn1_ul_up_transport_param.up_tnl_info);
              ue_params_item.cell_group_id = asn1_ul_up_transport_param.cell_group_id;

              drb_setup_item.ul_up_transport_params.push_back(ue_params_item);
            }

            // Fill flow setup list.
            for (const auto& asn1_qos_flow_item : asn1_drb_setup_item.flow_setup_list) {
              e1ap_qos_flow_item qos_flow_item;
              qos_flow_item.qos_flow_id = uint_to_qos_flow_id(asn1_qos_flow_item.qos_flow_id);

              drb_setup_item.flow_setup_list.emplace(qos_flow_item.qos_flow_id, qos_flow_item);
            }

            // Fill flow failed list.
            for (const auto& asn1_failed_qos_flow_item : asn1_drb_setup_item.flow_failed_list) {
              e1ap_qos_flow_failed_item failed_qos_flow_item;

              failed_qos_flow_item.qos_flow_id = uint_to_qos_flow_id(asn1_failed_qos_flow_item.qos_flow_id);
              failed_qos_flow_item.cause       = asn1_to_cause(asn1_failed_qos_flow_item.cause);

              drb_setup_item.flow_failed_list.emplace(failed_qos_flow_item.qos_flow_id, failed_qos_flow_item);
            }

            // Fill DRB data forwarding info response.
            if (asn1_drb_setup_item.drb_data_forwarding_info_resp_present) {
              if (asn1_drb_setup_item.drb_data_forwarding_info_resp.ul_data_forwarding_present) {
                drb_setup_item.drb_data_forwarding_info_resp.value().ul_data_forwarding =
                    asn1_to_up_transport_layer_info(
                        asn1_drb_setup_item.drb_data_forwarding_info_resp.ul_data_forwarding);
              }
              if (asn1_drb_setup_item.drb_data_forwarding_info_resp.dl_data_forwarding_present) {
                drb_setup_item.drb_data_forwarding_info_resp.value().dl_data_forwarding =
                    asn1_to_up_transport_layer_info(
                        asn1_drb_setup_item.drb_data_forwarding_info_resp.dl_data_forwarding);
              }
            }

            res_mod_item.drb_setup_list_ng_ran.emplace(drb_setup_item.drb_id, drb_setup_item);
          }

          // Fill DRB session resource failed list NG RAN.
          for (const auto& asn1_drb_failed_item : asn1_res_mod_item.drb_failed_mod_list_ng_ran) {
            e1ap_drb_failed_item_ng_ran drb_failed_item;
            drb_failed_item.drb_id = uint_to_drb_id(asn1_drb_failed_item.drb_id);
            drb_failed_item.cause  = asn1_to_cause(asn1_drb_failed_item.cause);

            res_mod_item.drb_failed_list_ng_ran.emplace(drb_failed_item.drb_id, drb_failed_item);
          }

          // Fill security result.
          if (asn1_res_mod_item.security_result_present) {
            asn1_to_security_result(res_mod_item.security_result.value(), asn1_res_mod_item.security_result);
          }

          // Fill PDU session data forwarding info response.
          if (asn1_res_mod_item.pdu_session_data_forwarding_info_resp_present) {
            if (asn1_res_mod_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding_present) {
              res_mod_item.pdu_session_data_forwarding_info_resp.value().ul_data_forwarding =
                  asn1_to_up_transport_layer_info(
                      asn1_res_mod_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding);
            }
            if (asn1_res_mod_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding_present) {
              res_mod_item.pdu_session_data_forwarding_info_resp.value().dl_data_forwarding =
                  asn1_to_up_transport_layer_info(
                      asn1_res_mod_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding);
            }
          }

          res.pdu_session_resource_setup_list.emplace(res_mod_item.pdu_session_id, res_mod_item);
        }
      }

      // Fill PDU session resource failed list.
      if (asn1_bearer_context_mod_response.pdu_session_res_failed_mod_list_present) {
        for (const auto& asn1_failed_item : asn1_bearer_context_mod_response.pdu_session_res_failed_mod_list) {
          e1ap_pdu_session_resource_failed_item failed_item;

          failed_item.pdu_session_id = uint_to_pdu_session_id(asn1_failed_item.pdu_session_id);
          failed_item.cause          = asn1_to_cause(asn1_failed_item.cause);

          res.pdu_session_resource_failed_list.emplace(failed_item.pdu_session_id, failed_item);
        }
      }

      // Fill PDU session resource modified list.
      if (asn1_bearer_context_mod_response.pdu_session_res_modified_list_present) {
        for (const auto& asn1_res_mod_item : asn1_bearer_context_mod_response.pdu_session_res_modified_list) {
          e1ap_pdu_session_resource_modified_item res_mod_item;

          res_mod_item.pdu_session_id = uint_to_pdu_session_id(asn1_res_mod_item.pdu_session_id);

          // Fill NG DL UP TNL info.
          if (asn1_res_mod_item.ng_dl_up_tnl_info_present) {
            res_mod_item.ng_dl_up_tnl_info = asn1_to_up_transport_layer_info(asn1_res_mod_item.ng_dl_up_tnl_info);
          }

          // Fill DRB setup list NG RAN.
          for (const auto& asn1_drb_setup_item : asn1_res_mod_item.drb_setup_list_ng_ran) {
            e1ap_drb_setup_item_ng_ran drb_setup_item;
            drb_setup_item.drb_id = uint_to_drb_id(asn1_drb_setup_item.drb_id);

            // Fill UL UP transport params.
            for (const auto& asn1_ul_up_transport_param : asn1_drb_setup_item.ul_up_transport_params) {
              e1ap_up_params_item ue_params_item;
              ue_params_item.up_tnl_info   = asn1_to_up_transport_layer_info(asn1_ul_up_transport_param.up_tnl_info);
              ue_params_item.cell_group_id = asn1_ul_up_transport_param.cell_group_id;

              drb_setup_item.ul_up_transport_params.push_back(ue_params_item);
            }

            // Fill flow setup list.
            for (const auto& asn1_qos_flow_item : asn1_drb_setup_item.flow_setup_list) {
              e1ap_qos_flow_item qos_flow_item;
              qos_flow_item.qos_flow_id = uint_to_qos_flow_id(asn1_qos_flow_item.qos_flow_id);

              drb_setup_item.flow_setup_list.emplace(qos_flow_item.qos_flow_id, qos_flow_item);
            }

            // Fill flow failed list.
            for (const auto& asn1_failed_qos_flow_item : asn1_drb_setup_item.flow_failed_list) {
              e1ap_qos_flow_failed_item failed_qos_flow_item;

              failed_qos_flow_item.qos_flow_id = uint_to_qos_flow_id(asn1_failed_qos_flow_item.qos_flow_id);
              failed_qos_flow_item.cause       = asn1_to_cause(asn1_failed_qos_flow_item.cause);

              drb_setup_item.flow_failed_list.emplace(failed_qos_flow_item.qos_flow_id, failed_qos_flow_item);
            }

            // Fill DRB data forwarding info response.
            if (asn1_drb_setup_item.drb_data_forwarding_info_resp_present) {
              if (asn1_drb_setup_item.drb_data_forwarding_info_resp.ul_data_forwarding_present) {
                drb_setup_item.drb_data_forwarding_info_resp.value().ul_data_forwarding =
                    asn1_to_up_transport_layer_info(
                        asn1_drb_setup_item.drb_data_forwarding_info_resp.ul_data_forwarding);
              }
              if (asn1_drb_setup_item.drb_data_forwarding_info_resp.dl_data_forwarding_present) {
                drb_setup_item.drb_data_forwarding_info_resp.value().dl_data_forwarding =
                    asn1_to_up_transport_layer_info(
                        asn1_drb_setup_item.drb_data_forwarding_info_resp.dl_data_forwarding);
              }
            }

            res_mod_item.drb_setup_list_ng_ran.emplace(drb_setup_item.drb_id, drb_setup_item);
          }

          // Fill DRB session resource failed list NG RAN.
          for (const auto& asn1_drb_failed_item : asn1_res_mod_item.drb_failed_list_ng_ran) {
            e1ap_drb_failed_item_ng_ran drb_failed_item;
            drb_failed_item.drb_id = uint_to_drb_id(asn1_drb_failed_item.drb_id);
            drb_failed_item.cause  = asn1_to_cause(asn1_drb_failed_item.cause);

            res_mod_item.drb_failed_list_ng_ran.emplace(drb_failed_item.drb_id, drb_failed_item);
          }

          // Fill DRB modified list NG RAN.
          for (const auto& asn1_drb_mod_item : asn1_res_mod_item.drb_modified_list_ng_ran) {
            e1ap_drb_modified_item_ng_ran drb_mod_item;
            drb_mod_item.drb_id = uint_to_drb_id(asn1_drb_mod_item.drb_id);

            // Fill UL UP transport params.
            for (const auto& asn1_ul_up_transport_param : asn1_drb_mod_item.ul_up_transport_params) {
              e1ap_up_params_item ue_params_item;
              ue_params_item.up_tnl_info   = asn1_to_up_transport_layer_info(asn1_ul_up_transport_param.up_tnl_info);
              ue_params_item.cell_group_id = asn1_ul_up_transport_param.cell_group_id;

              drb_mod_item.ul_up_transport_params.push_back(ue_params_item);
            }

            // Fill flow setup list.
            for (const auto& asn1_qos_flow_item : asn1_drb_mod_item.flow_setup_list) {
              e1ap_qos_flow_item qos_flow_item;
              qos_flow_item.qos_flow_id = uint_to_qos_flow_id(asn1_qos_flow_item.qos_flow_id);

              drb_mod_item.flow_setup_list.emplace(qos_flow_item.qos_flow_id, qos_flow_item);
            }

            // Fill flow failed list.
            for (const auto& asn1_failed_qos_flow_item : asn1_drb_mod_item.flow_failed_list) {
              e1ap_qos_flow_failed_item failed_qos_flow_item;

              failed_qos_flow_item.qos_flow_id = uint_to_qos_flow_id(asn1_failed_qos_flow_item.qos_flow_id);
              failed_qos_flow_item.cause       = asn1_to_cause(asn1_failed_qos_flow_item.cause);

              drb_mod_item.flow_failed_list.emplace(failed_qos_flow_item.qos_flow_id, failed_qos_flow_item);
            }

            // Fill PDCP SN status info.
            if (asn1_drb_mod_item.pdcp_sn_status_info_present) {
              auto& asn1_pdcp_sn_status_info = asn1_drb_mod_item.pdcp_sn_status_info;

              drb_mod_item.pdcp_sn_status_info.value().pdcp_status_transfer_ul.count_value =
                  e1ap_asn1_pdcp_count_to_pdcp_count(asn1_pdcp_sn_status_info.pdcp_status_transfer_ul.count_value);

              if (asn1_pdcp_sn_status_info.pdcp_status_transfer_ul.receive_statusof_pdcp_sdu_present) {
                drb_mod_item.pdcp_sn_status_info.value().pdcp_status_transfer_ul.receive_status_of_pdcp_sdu =
                    asn1_pdcp_sn_status_info.pdcp_status_transfer_ul.receive_statusof_pdcp_sdu.to_number();
              }

              drb_mod_item.pdcp_sn_status_info.value().pdcp_status_transfer_dl =
                  e1ap_asn1_pdcp_count_to_pdcp_count(asn1_pdcp_sn_status_info.pdcp_status_transfer_dl);
            }

            res_mod_item.drb_modified_list_ng_ran.emplace(drb_mod_item.drb_id, drb_mod_item);
          }

          // Fill DRB failed to modify list NG RAN.
          for (const auto& asn1_drb_failed_item : asn1_res_mod_item.drb_failed_to_modify_list_ng_ran) {
            e1ap_drb_failed_item_ng_ran drb_failed_item;
            drb_failed_item.drb_id = uint_to_drb_id(asn1_drb_failed_item.drb_id);
            drb_failed_item.cause  = asn1_to_cause(asn1_drb_failed_item.cause);

            res_mod_item.drb_failed_to_modify_list_ng_ran.emplace(drb_failed_item.drb_id, drb_failed_item);
          }

          // Fill security result.
          if (asn1_res_mod_item.security_result_present) {
            asn1_to_security_result(res_mod_item.security_result.value(), asn1_res_mod_item.security_result);
          }

          // Fill PDU session data forwarding info response.
          if (asn1_res_mod_item.pdu_session_data_forwarding_info_resp_present) {
            if (asn1_res_mod_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding_present) {
              res_mod_item.pdu_session_data_forwarding_info_resp.value().ul_data_forwarding =
                  asn1_to_up_transport_layer_info(
                      asn1_res_mod_item.pdu_session_data_forwarding_info_resp.ul_data_forwarding);
            }
            if (asn1_res_mod_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding_present) {
              res_mod_item.pdu_session_data_forwarding_info_resp.value().dl_data_forwarding =
                  asn1_to_up_transport_layer_info(
                      asn1_res_mod_item.pdu_session_data_forwarding_info_resp.dl_data_forwarding);
            }
          }

          res.pdu_session_resource_modified_list.emplace(res_mod_item.pdu_session_id, res_mod_item);
        }
      }

      // Fill PDU session resource failed to modify list.
      if (asn1_bearer_context_mod_response.pdu_session_res_failed_to_modify_list_present) {
        for (const auto& asn1_failed_item : asn1_bearer_context_mod_response.pdu_session_res_failed_to_modify_list) {
          e1ap_pdu_session_resource_failed_item failed_item;

          failed_item.pdu_session_id = uint_to_pdu_session_id(asn1_failed_item.pdu_session_id);
          failed_item.cause          = asn1_to_cause(asn1_failed_item.cause);

          res.pdu_session_resource_failed_to_modify_list.emplace(failed_item.pdu_session_id, failed_item);
        }
      }
    }
  }
}

inline void fill_e1ap_bearer_context_modification_response(
    e1ap_bearer_context_modification_response&   res,
    const asn1::e1ap::bearer_context_mod_fail_s& asn1_bearer_context_modification_fail)
{
  res.success = false;
  res.cause   = asn1_to_cause(asn1_bearer_context_modification_fail->cause);
  if (asn1_bearer_context_modification_fail->crit_diagnostics_present) {
    // TODO: Add crit diagnostics.
  }
}

inline void fill_asn1_bearer_context_release_command(asn1::e1ap::bearer_context_release_cmd_s&  asn1_command,
                                                     const e1ap_bearer_context_release_command& command)
{
  asn1_command->cause = cause_to_asn1(command.cause);
}

inline void fill_asn1_bc_bearer_context_setup_request(asn1::e1ap::bc_bearer_context_setup_request_s& asn1_request,
                                                      e1ap_bc_bearer_context_setup_request& request)
{
  // Fill gNB CU-CP MBS E1AP ID.
  asn1_request->gnb_cu_cp_mbs_e1ap_id = gnb_cu_cp_mbs_e1ap_id_to_uint(request.gnb_cu_cp_mbs_e1ap_id);

  // Fill Global MBS Session ID.
  asn1_request->global_mbs_session_id = mbs_session_id_to_e1ap_asn1(request.global_mbs_session_id);

  // Fill BC Bearer Context To Setup.
  // Fill S-NSSAI.
  asn1_request->bc_bearer_context_to_setup.snssai = snssai_to_e1ap_asn1(request.bc_bearer_context_to_setup.s_nssai);

  // Fill BC Bearer Context NG-U TNL Info at 5GC.
  if (request.bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_info_at_5gc.has_value()) {
    asn1_request->bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_infoat5_gc_present = true;
    // Fill location dependent
    if (request.bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_info_at_5gc.value().is_locationdependent()) {
      auto& asn1_locationdependent = asn1_request->bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_infoat5_gc.set_locationdependent();

      for (const auto& locationdependent_item : request.bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_info_at_5gc.value().get_locationdependent().location_dependent_mbs_ngu_info_at_5gc) {
        asn1::e1ap::location_dependent_mb_sn_gu_info_at5_gc_item_s asn1_locationdependent_item;

        asn1_locationdependent_item.mbs_area_session_id = area_session_id_to_uint(locationdependent_item.mbs_area_session_id);
        asn1_locationdependent_item.mbs_ngu_info_at5_gc = mbs_ngu_info_at_5gc_to_asn1(locationdependent_item.mbs_ngu_information_at_5gc);

        asn1_locationdependent.push_back(asn1_locationdependent_item);
      }
    // Fill location independent
    } else {
      auto& asn1_locationindependent = asn1_request->bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_infoat5_gc.set_locationindependent();

      const auto& locationdependent = request.bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_info_at_5gc.value().get_locationindependent().mbs_ngu_information_at_5gc;
      asn1_locationindependent = mbs_ngu_info_at_5gc_to_asn1(locationdependent);
    }
  }

  // Fill BC MRB To Setup List.
  for (const auto& bc_mrb_to_setup_item : request.bc_bearer_context_to_setup.bc_mrb_to_setup_list) {
    asn1::e1ap::bcmrb_setup_cfg_item_s asn1_bc_mrb_to_setup_item;

    // Fill MRB ID.
    asn1_bc_mrb_to_setup_item.mrb_id = mrb_id_to_uint(bc_mrb_to_setup_item.mrb_id);

    // Fill MBS PDCP Configuration.
    asn1_bc_mrb_to_setup_item.mbs_pdcp_cfg = pdcp_config_to_e1ap_asn1(bc_mrb_to_setup_item.mbs_pdcp_cfg);

    // Fill MBS QoS Flows Information To Be Setup.
    for (const auto& mbs_qos_flow_info_to_be_setup_item : bc_mrb_to_setup_item.mbs_qos_flow_info_to_be_setup) {
      asn1::e1ap::qos_flow_qos_param_item_s asn1_mbs_qos_flow_info_to_be_setup_item;

      fill_asn1_qos_flow_info_item(asn1_mbs_qos_flow_info_to_be_setup_item, mbs_qos_flow_info_to_be_setup_item);

      asn1_bc_mrb_to_setup_item.qos_flow_qos_param_list.push_back(asn1_mbs_qos_flow_info_to_be_setup_item);
    }

    // Fill MRB QoS.
    if (bc_mrb_to_setup_item.mrb_qos.has_value()) {
      asn1_bc_mrb_to_setup_item.qos_flow_level_qos_params_present = true;
      fill_asn1_qos_flow_level_qos_params( asn1_bc_mrb_to_setup_item.qos_flow_level_qos_params,
          bc_mrb_to_setup_item.mrb_qos.value());
    }

    // F1-U TNL Info to Add List.
    if (!bc_mrb_to_setup_item.f1u_tnl_info_to_add_list.empty()) {
      // NOTE (borieher): This is an extension IE
      asn1_bc_mrb_to_setup_item.ext = true;

      asn1::protocol_ext_field_s<asn1::e1ap::bcmrb_setup_cfg_item_ext_ies_o> bc_mrb_to_setup_item_ext_ie;
      auto& asn1_f1u_tnl_info_to_add_list = bc_mrb_to_setup_item_ext_ie.value().f1_u_tnl_info_to_add_list();

      for (const auto& f1u_tnl_info_to_add_item : bc_mrb_to_setup_item.f1u_tnl_info_to_add_list) {
        asn1::e1ap::f1_u_tnl_info_to_add_item_s asn1_f1u_tnl_info_to_add_item{};

        asn1_f1u_tnl_info_to_add_item.broadcast_f1_u_context_ref_e1.from_number(
            e1ap_bc_f1u_context_reference_e1_to_uint(f1u_tnl_info_to_add_item));

        asn1_f1u_tnl_info_to_add_list.push_back(asn1_f1u_tnl_info_to_add_item);
      }

      // Add extension IE to the list
      asn1_bc_mrb_to_setup_item.ie_exts.push_back(bc_mrb_to_setup_item_ext_ie);
    }

    asn1_request->bc_bearer_context_to_setup.bc_mrb_to_setup_list.push_back(asn1_bc_mrb_to_setup_item);
  }

  // Fill Requested Action for Available Shared NG-U Termination.
  if (request.bc_bearer_context_to_setup.requested_action_for_available_shared_ngu_termination.has_value()) {
    asn1_request->bc_bearer_context_to_setup.requested_action_present = true;
    asn1_request->bc_bearer_context_to_setup.requested_action =
      requested_action_for_available_shared_ngu_termination_to_asn1(request.bc_bearer_context_to_setup.requested_action_for_available_shared_ngu_termination.value());
  }
}

inline void
fill_e1ap_bc_bearer_context_setup_response(e1ap_bc_bearer_context_setup_response&            response,
                                          const asn1::e1ap::bc_bearer_context_setup_resp_s&  asn1_response)
{
  // Fill gNB-CU-CP MBS E1AP ID (M).
  response.gnb_cu_cp_mbs_e1ap_id = uint_to_gnb_cu_cp_mbs_e1ap_id(asn1_response->gnb_cu_cp_mbs_e1ap_id);

  // Fill gNB-CU-UP MBS E1AP ID (M).
  response.gnb_cu_up_mbs_e1ap_id = uint_to_gnb_cu_up_mbs_e1ap_id(asn1_response->gnb_cu_up_mbs_e1ap_id);

  // Fill BC Bearer Context To Setup Response (M).
  // TODO (borieher): Fill BC Bearer Context NG-U TNL Info at NG-RAN (O).

  // Fill BC MRB Setup Response List <1..maxnoofMRBs>.
  for (const auto& asn1_bc_mrb_setup_response_item : asn1_response->bc_bearer_context_to_setup_resp.bc_mrb_setup_resp_list) {
    e1ap_bc_mrb_setup_response_item bc_mrb_setup_response_item;

    // Fill MRB ID (M).
    bc_mrb_setup_response_item.mrb_id = uint_to_mrb_id(asn1_bc_mrb_setup_response_item.mrb_id);

    // Fill MBS QoS Flow Setup List (M).
    for (const auto& asn1_mbs_qos_flow_setup_item : asn1_bc_mrb_setup_response_item.qosflow_setup) {
      e1ap_qos_flow_item mbs_qos_flow_setup_item;

      // Fill QoS Flow Identifier (M).
      mbs_qos_flow_setup_item.qos_flow_id = uint_to_qos_flow_id(asn1_mbs_qos_flow_setup_item.qos_flow_id);

      // TODO (borieher): Fill QoS Flow Mapping Indication (O).
      // TODO (borieher): Fill Data Forwarding Source IP Address (O).

      bc_mrb_setup_response_item.mbs_qos_flow_setup_list.push_back(mbs_qos_flow_setup_item);
    }

    // Fill MBS QoS Flow Failed List (O).
    if (asn1_bc_mrb_setup_response_item.qosflow_failed.size() > 0) {
      for (const auto& asn1_mbs_qos_flow_failed_item : asn1_bc_mrb_setup_response_item.qosflow_failed) {
        e1ap_qos_flow_failed_item mbs_qos_flow_failed_item;

        // Fill QoS Flow Identifier (M).
        mbs_qos_flow_failed_item.qos_flow_id = uint_to_qos_flow_id(asn1_mbs_qos_flow_failed_item.qos_flow_id);

        // Fill Cause (M).
        mbs_qos_flow_failed_item.cause = asn1_to_cause(asn1_mbs_qos_flow_failed_item.cause);

        bc_mrb_setup_response_item.mbs_qos_flow_failed_list.value().push_back(mbs_qos_flow_failed_item);
      }
    }

    // Fill BC Bearer Context F1-U TNL Info at CU (M).
    bc_mrb_setup_response_item.bc_bearer_context_f1u_tnl_info_at_cu =
        asn1_to_bc_bearer_context_f1u_tnl_info_at_cu(asn1_bc_mrb_setup_response_item.bc_bearer_context_f1_u_tnl_infoat_cu);

    // Extension IEs (O).
    if (asn1_bc_mrb_setup_response_item.ie_exts.size() > 0) {
      for (const auto& asn1_bc_mrb_to_setup_item_ext_ie : asn1_bc_mrb_setup_response_item.ie_exts) {
        switch (asn1_bc_mrb_to_setup_item_ext_ie.value().type()) {
          // F1-U TNL Info Added List <0..1>.
          case asn1::e1ap::bcmrb_setup_resp_list_item_ext_ies_o::ext_c::types::f1_u_tnl_info_added_list: {
            const auto& asn1_f1u_tnl_info_added_list = asn1_bc_mrb_to_setup_item_ext_ie.value().f1_u_tnl_info_added_list();

            for (const auto& asn1_f1u_tnl_info_added_item : asn1_f1u_tnl_info_added_list) {
              e1ap_f1u_tnl_info_added_item f1u_tnl_info_added_item;

              f1u_tnl_info_added_item.bc_f1u_context_reference_e1 = uint_to_e1ap_bc_f1u_context_reference_e1(
                  asn1_f1u_tnl_info_added_item.broadcast_f1_u_context_ref_e1.to_number());

              f1u_tnl_info_added_item.bc_bearer_context_f1u_tnl_info_at_cu =
                  asn1_to_bc_bearer_context_f1u_tnl_info_at_cu(asn1_f1u_tnl_info_added_item.bc_bearer_context_f1_u_tnl_infoat_cu);

              bc_mrb_setup_response_item.f1u_tnl_info_added_list.push_back(f1u_tnl_info_added_item);
            }
            break;
          }
          case asn1::e1ap::bcmrb_setup_resp_list_item_ext_ies_o::ext_c::types::nulltype: {
            // TODO (borieher): Handle nulltype case
            break;
          }
        }
      }
    }
    response.bc_bearer_context_to_setup_response.bc_mrb_setup_response_list.push_back(bc_mrb_setup_response_item);
  }

  // Fill BC MRB Failed List <0..maxnoofMRBs>.
  for (const auto& asn1_bc_mrb_failed_item : asn1_response->bc_bearer_context_to_setup_resp.bc_mrb_failed_list) {
    e1ap_bc_mrb_failed_item bc_mrb_failed_item;

    // Fill MRB ID (M).
    bc_mrb_failed_item.mrb_id = uint_to_mrb_id(asn1_bc_mrb_failed_item.mrb_id);

    // Fill Cause (M).
    bc_mrb_failed_item.cause = asn1_to_cause(asn1_bc_mrb_failed_item.cause);

    response.bc_bearer_context_to_setup_response.bc_mrb_failed_list.push_back(bc_mrb_failed_item);
  }

  // TODO (borieher): Fill Available BC MRB Configuration (O).
  // TODO (borieher): Fill Criticality Diagnostics (O).
}

inline void fill_asn1_bc_bearer_context_modification_request(asn1::e1ap::bc_bearer_context_mod_request_s& asn1_request,
                                                             e1ap_bc_bearer_context_modification_request& request)
{
  // Fill gNB CU-CP MBS E1AP ID (M).
  asn1_request->gnb_cu_cp_mbs_e1ap_id = gnb_cu_cp_mbs_e1ap_id_to_uint(request.gnb_cu_cp_mbs_e1ap_id);

  // Fill gNB-CU-UP MBS E1AP ID (M).
  asn1_request->gnb_cu_up_mbs_e1ap_id = gnb_cu_up_mbs_e1ap_id_to_uint(request.gnb_cu_up_mbs_e1ap_id);

  // Fill BC Bearer Context To Modify (M).
  // Fill BC Bearer Context NG-U TNL Info at 5GC To Setup or Modify (O).
  if (request.bc_bearer_context_to_modify.bc_bearer_context_ngu_tnl_info_at_5gc_to_setup_or_modify.has_value()) {
    asn1_request->bc_bearer_context_to_modify.bc_bearer_context_ngu_tnl_infoat5_gc_present = true;
    // Fill location dependent
    if (request.bc_bearer_context_to_modify.bc_bearer_context_ngu_tnl_info_at_5gc_to_setup_or_modify.value().is_locationdependent()) {
      auto& asn1_locationdependent = asn1_request->bc_bearer_context_to_modify.bc_bearer_context_ngu_tnl_infoat5_gc.set_locationdependent();

      for (const auto& locationdependent_item : request.bc_bearer_context_to_modify.bc_bearer_context_ngu_tnl_info_at_5gc_to_setup_or_modify
            .value().get_locationdependent().location_dependent_mbs_ngu_info_at_5gc) {
        asn1::e1ap::location_dependent_mb_sn_gu_info_at5_gc_item_s asn1_locationdependent_item;

        asn1_locationdependent_item.mbs_area_session_id = area_session_id_to_uint(locationdependent_item.mbs_area_session_id);
        asn1_locationdependent_item.mbs_ngu_info_at5_gc = mbs_ngu_info_at_5gc_to_asn1(locationdependent_item.mbs_ngu_information_at_5gc);

        asn1_locationdependent.push_back(asn1_locationdependent_item);
      }
    // Fill location independent
    } else {
      auto& asn1_locationindependent = asn1_request->bc_bearer_context_to_modify.bc_bearer_context_ngu_tnl_infoat5_gc.set_locationindependent();

      const auto& locationdependent = request.bc_bearer_context_to_modify.bc_bearer_context_ngu_tnl_info_at_5gc_to_setup_or_modify
          .value().get_locationindependent().mbs_ngu_information_at_5gc;
      asn1_locationindependent = mbs_ngu_info_at_5gc_to_asn1(locationdependent);
    }
  }

  // Fill BC MRB To Setup List (O).
  if (request.bc_bearer_context_to_modify.bc_mrb_to_setup_list.has_value()) {
    for (const auto& bc_mrb_to_setup_item : request.bc_bearer_context_to_modify.bc_mrb_to_setup_list.value()) {
      asn1::e1ap::bcmrb_setup_cfg_item_s asn1_bc_mrb_to_setup_item;

      // Fill MRB ID.
      asn1_bc_mrb_to_setup_item.mrb_id = mrb_id_to_uint(bc_mrb_to_setup_item.mrb_id);

      // Fill MBS PDCP Configuration.
      asn1_bc_mrb_to_setup_item.mbs_pdcp_cfg = pdcp_config_to_e1ap_asn1(bc_mrb_to_setup_item.mbs_pdcp_cfg);

      // Fill MBS QoS Flows Information To Be Setup.
      for (const auto& mbs_qos_flow_info_to_be_setup_item : bc_mrb_to_setup_item.mbs_qos_flow_info_to_be_setup) {
        asn1::e1ap::qos_flow_qos_param_item_s asn1_mbs_qos_flow_info_to_be_setup_item;

        fill_asn1_qos_flow_info_item(asn1_mbs_qos_flow_info_to_be_setup_item, mbs_qos_flow_info_to_be_setup_item);

        asn1_bc_mrb_to_setup_item.qos_flow_qos_param_list.push_back(asn1_mbs_qos_flow_info_to_be_setup_item);
      }

      // Fill MRB QoS.
      if (bc_mrb_to_setup_item.mrb_qos.has_value()) {
        asn1_bc_mrb_to_setup_item.qos_flow_level_qos_params_present = true;
        fill_asn1_qos_flow_level_qos_params(asn1_bc_mrb_to_setup_item.qos_flow_level_qos_params,
            bc_mrb_to_setup_item.mrb_qos.value());
      }

      // F1-U TNL Info to Add List.
      if (!bc_mrb_to_setup_item.f1u_tnl_info_to_add_list.empty()) {
        // NOTE (borieher): This is an extension IE
        asn1_bc_mrb_to_setup_item.ext = true;

        asn1::protocol_ext_field_s<asn1::e1ap::bcmrb_setup_cfg_item_ext_ies_o> bc_mrb_to_setup_item_ext_ie;
        auto& asn1_f1u_tnl_info_to_add_list = bc_mrb_to_setup_item_ext_ie.value().f1_u_tnl_info_to_add_list();

        for (const auto& f1u_tnl_info_to_add_item : bc_mrb_to_setup_item.f1u_tnl_info_to_add_list) {
          asn1::e1ap::f1_u_tnl_info_to_add_item_s asn1_f1u_tnl_info_to_add_item{};

          asn1_f1u_tnl_info_to_add_item.broadcast_f1_u_context_ref_e1.from_number(
              e1ap_bc_f1u_context_reference_e1_to_uint(f1u_tnl_info_to_add_item));

          asn1_f1u_tnl_info_to_add_list.push_back(asn1_f1u_tnl_info_to_add_item);
        }

        // Add extension IE to the list
        asn1_bc_mrb_to_setup_item.ie_exts.push_back(bc_mrb_to_setup_item_ext_ie);
      }

      asn1_request->bc_bearer_context_to_modify.bc_mrb_to_setup_list.push_back(asn1_bc_mrb_to_setup_item);
    }
  }

  // Fill BC MRB To Modify List (0..maxnoofMRBs).
  for (const auto& bc_mrb_to_modify_item : request.bc_bearer_context_to_modify.bc_mrb_to_modify_list) {
    asn1::e1ap::bcmrb_modify_cfg_item_s asn1_bc_mrb_to_modify_item;

    // Fill MRB ID (M).
    asn1_bc_mrb_to_modify_item.mrb_id = mrb_id_to_uint(bc_mrb_to_modify_item.mrb_id);

    // Fill BC Bearer Context F1-U TNL Info at DU (O).
    if (bc_mrb_to_modify_item.bc_bearer_context_f1u_tnl_info_at_du.has_value()) {
      asn1_bc_mrb_to_modify_item.bc_bearer_context_f1_u_tnl_infoat_du_present = true;
      asn1_bc_mrb_to_modify_item.bc_bearer_context_f1_u_tnl_infoat_du =
        bc_bearer_context_f1u_tnl_info_at_du_to_asn1(bc_mrb_to_modify_item.bc_bearer_context_f1u_tnl_info_at_du.value());
    }

    // Fill MBS PDCP Configuration (O).
    if (bc_mrb_to_modify_item.mbs_pdcp_cfg.has_value()) {
      asn1_bc_mrb_to_modify_item.mbs_pdcp_cfg_present = true;
      asn1_bc_mrb_to_modify_item.mbs_pdcp_cfg = pdcp_config_to_e1ap_asn1(bc_mrb_to_modify_item.mbs_pdcp_cfg.value());
    }

    // Fill MBS QoS Flows Information To Be Setup (O).
    if (bc_mrb_to_modify_item.mbs_qos_flow_info_to_be_setup.has_value()) {
      for (const auto& mbs_qos_flow_info_to_be_setup_item : bc_mrb_to_modify_item.mbs_qos_flow_info_to_be_setup.value()) {
        asn1::e1ap::qos_flow_qos_param_item_s asn1_mbs_qos_flow_info_to_be_setup_item;

        fill_asn1_qos_flow_info_item(asn1_mbs_qos_flow_info_to_be_setup_item, mbs_qos_flow_info_to_be_setup_item);

        asn1_bc_mrb_to_modify_item.qos_flow_qos_param_list.push_back(asn1_mbs_qos_flow_info_to_be_setup_item);
      }
    }

    // Fill MRB QoS (O).
    if (bc_mrb_to_modify_item.mrb_qos.has_value()) {
      asn1_bc_mrb_to_modify_item.qos_flow_level_qos_params_present = true;
      fill_asn1_qos_flow_level_qos_params(asn1_bc_mrb_to_modify_item.qos_flow_level_qos_params,
            bc_mrb_to_modify_item.mrb_qos.value());
    }

    // Fill F1-U TNL Info to Add or Modify List (0..1).
    if (!bc_mrb_to_modify_item.f1u_tnl_info_to_add_or_modify_list.empty()) {
      // NOTE (borieher): This is an extension IE
      asn1_bc_mrb_to_modify_item.ext = true;

      asn1::e1ap::f1_u_tnl_info_to_add_or_modify_list_l asn1_f1u_tnl_info_to_add_or_modify_list;

      for (const auto& f1u_tnl_info_to_add_or_modify_item : bc_mrb_to_modify_item.f1u_tnl_info_to_add_or_modify_list) {
        asn1::e1ap::f1_u_tnl_info_to_add_or_modify_item_s asn1_f1u_tnl_info_to_add_or_modify_item{};

        asn1_f1u_tnl_info_to_add_or_modify_item.broadcast_f1_u_context_ref_e1.from_number(
            e1ap_bc_f1u_context_reference_e1_to_uint(f1u_tnl_info_to_add_or_modify_item.bc_f1u_context_reference_e1));

        if (f1u_tnl_info_to_add_or_modify_item.bc_bearer_context_f1u_tnl_info_at_du.has_value()) {
          asn1_f1u_tnl_info_to_add_or_modify_item.bc_bearer_context_f1_u_tnl_infoat_du_present = true;
          asn1_f1u_tnl_info_to_add_or_modify_item.bc_bearer_context_f1_u_tnl_infoat_du =
              bc_bearer_context_f1u_tnl_info_at_du_to_asn1(
                  f1u_tnl_info_to_add_or_modify_item.bc_bearer_context_f1u_tnl_info_at_du.value());
        }

        asn1_f1u_tnl_info_to_add_or_modify_list.push_back(asn1_f1u_tnl_info_to_add_or_modify_item);
      }

      // Add extension IE
      asn1_bc_mrb_to_modify_item.ie_exts.f1_u_tnl_info_to_add_or_modify_list_present = true;
      asn1_bc_mrb_to_modify_item.ie_exts.f1_u_tnl_info_to_add_or_modify_list =
          asn1_f1u_tnl_info_to_add_or_modify_list;
    }

    // Fill F1-U TNL Info to Release List (0..1).
    if (!bc_mrb_to_modify_item.f1u_tnl_info_to_release_list.empty()) {
      // NOTE (borieher): This is an extension IE
      asn1_bc_mrb_to_modify_item.ext = true;

      asn1::e1ap::f1_u_tnl_info_to_release_list_l asn1_f1u_tnl_info_to_release_list;

      for (const auto& f1u_tnl_info_to_release_item : bc_mrb_to_modify_item.f1u_tnl_info_to_release_list) {
        asn1::e1ap::f1_u_tnl_info_to_release_item_s asn1_f1u_tnl_info_to_release_item{};

        asn1_f1u_tnl_info_to_release_item.broadcast_f1_u_context_ref_e1.from_number(
            e1ap_bc_f1u_context_reference_e1_to_uint(f1u_tnl_info_to_release_item));

        asn1_f1u_tnl_info_to_release_list.push_back(asn1_f1u_tnl_info_to_release_item);
      }

      // Add extension IE
      asn1_bc_mrb_to_modify_item.ie_exts.f1_u_tnl_info_to_release_list_present = true;
      asn1_bc_mrb_to_modify_item.ie_exts.f1_u_tnl_info_to_release_list = asn1_f1u_tnl_info_to_release_list;
    }

    asn1_request->bc_bearer_context_to_modify.bc_mrb_to_modify_list.push_back(asn1_bc_mrb_to_modify_item);
  }

  // Fill BC MRB To Remove List (0..maxnoofMRBs).
  for (const auto& bc_mrb_to_remove_item : request.bc_bearer_context_to_modify.bc_mrb_to_remove_list) {
    uint16_t asn1_bc_mrb_to_remove_item;

    // Fill MRB ID (M).
    asn1_bc_mrb_to_remove_item = mrb_id_to_uint(bc_mrb_to_remove_item.mrb_id);

    asn1_request->bc_bearer_context_to_modify.bc_mrb_to_rem_list.push_back(asn1_bc_mrb_to_remove_item);
  }
}

inline void
fill_e1ap_bc_bearer_context_modification_response(e1ap_bc_bearer_context_modification_response&   response,
                                                  const asn1::e1ap::bc_bearer_context_mod_resp_s& asn1_response)
{
  // Fill gNB-CU-CP MBS E1AP ID (M).
  response.gnb_cu_cp_mbs_e1ap_id = uint_to_gnb_cu_cp_mbs_e1ap_id(asn1_response->gnb_cu_cp_mbs_e1ap_id);

  // Fill gNB-CU-UP MBS E1AP ID (M).
  response.gnb_cu_up_mbs_e1ap_id = uint_to_gnb_cu_up_mbs_e1ap_id(asn1_response->gnb_cu_up_mbs_e1ap_id);

  // Fill BC Bearer Context To Modify Response (M).
  // TODO (borieher): Fill BC Bearer Context NG-U TNL Info at NG-RAN (O).

  // Fill BC MRB Setup or Modify Response List (1..maxnoofMRBs).
  for (const auto& asn1_bc_mrb_setup_or_modify_response_item : asn1_response->bc_bearer_context_to_modify_resp.bc_mrb_setup_modify_resp_list) {
    e1ap_bc_mrb_setup_or_modify_response_item bc_mrb_setup_or_modify_response_item;

    // Fill MRB ID (M).
    bc_mrb_setup_or_modify_response_item.mrb_id = uint_to_mrb_id(asn1_bc_mrb_setup_or_modify_response_item.mrb_id);

    // Fill MBS QoS Flow Setup List (O).
    for (const auto& asn1_mbs_qos_flow_setup_item : asn1_bc_mrb_setup_or_modify_response_item.qosflow_setup) {
      // Initialize MBS QoS Flow Setup List (O) if not already done
      if (!bc_mrb_setup_or_modify_response_item.mbs_qos_flow_setup_list.has_value()) {
        bc_mrb_setup_or_modify_response_item.mbs_qos_flow_setup_list.emplace();
      }

      e1ap_qos_flow_item mbs_qos_flow_setup_item;

      // Fill QoS Flow Identifier (M).
      mbs_qos_flow_setup_item.qos_flow_id = uint_to_qos_flow_id(asn1_mbs_qos_flow_setup_item.qos_flow_id);

      bc_mrb_setup_or_modify_response_item.mbs_qos_flow_setup_list->push_back(mbs_qos_flow_setup_item);
    }

    // Fill MBS QoS Flow Failed List (O).
    for (const auto& asn1_mbs_qos_flow_failed_item : asn1_bc_mrb_setup_or_modify_response_item.qosflow_failed) {
      // Initialize MBS QoS Flow Failed List (O) if not already done
      if (!bc_mrb_setup_or_modify_response_item.mbs_qos_flow_failed_list.has_value()) {
        bc_mrb_setup_or_modify_response_item.mbs_qos_flow_failed_list.emplace();
      }

      e1ap_qos_flow_failed_item mbs_qos_flow_failed_item;

      // Fill QoS Flow Identifier (M).
      mbs_qos_flow_failed_item.qos_flow_id = uint_to_qos_flow_id(asn1_mbs_qos_flow_failed_item.qos_flow_id);

      // Fill Cause (M).
      mbs_qos_flow_failed_item.cause = asn1_to_cause(asn1_mbs_qos_flow_failed_item.cause);

      bc_mrb_setup_or_modify_response_item.mbs_qos_flow_failed_list->push_back(mbs_qos_flow_failed_item);
    }

    // Fill BC Bearer Context F1-U TNL Info at CU (O).
    if (asn1_bc_mrb_setup_or_modify_response_item.bc_bearer_context_f1_u_tnl_infoat_cu_present) {
      bc_mrb_setup_or_modify_response_item.bc_bearer_context_f1u_tnl_info_at_cu =
        asn1_to_bc_bearer_context_f1u_tnl_info_at_cu(asn1_bc_mrb_setup_or_modify_response_item.bc_bearer_context_f1_u_tnl_infoat_cu);
    }

    // Extension IEs (O).
    if (asn1_bc_mrb_setup_or_modify_response_item.ie_exts.size() > 0) {
      for (const auto& asn1_bc_mrb_setup_or_modify_item_ext_ie : asn1_bc_mrb_setup_or_modify_response_item.ie_exts) {
        switch (asn1_bc_mrb_setup_or_modify_item_ext_ie.value().type()) {
          // Fill F1-U TNL Info Added or Modified List (0..1).
          case asn1::e1ap::bcmrb_setup_modify_resp_list_item_ext_ies_o::ext_c::types::f1_u_tnl_info_added_or_modified_list: {
            const auto& asn1_f1u_tnl_info_added_or_modified_list = asn1_bc_mrb_setup_or_modify_item_ext_ie.value().f1_u_tnl_info_added_or_modified_list();

            for (const auto& asn1_f1u_tnl_info_added_or_modified_item : asn1_f1u_tnl_info_added_or_modified_list) {
              e1ap_f1u_tnl_info_added_or_modified_item f1u_tnl_info_added_or_modified_item;

              f1u_tnl_info_added_or_modified_item.bc_f1u_context_reference_e1 =
                  uint_to_e1ap_bc_f1u_context_reference_e1(
                    asn1_f1u_tnl_info_added_or_modified_item.broadcast_f1_u_context_ref_e1.to_number());

              if (asn1_f1u_tnl_info_added_or_modified_item.bc_bearer_context_f1_u_tnl_infoat_cu_present) {
                f1u_tnl_info_added_or_modified_item.bc_bearer_context_f1u_tnl_info_at_cu =
                    asn1_to_bc_bearer_context_f1u_tnl_info_at_cu(asn1_f1u_tnl_info_added_or_modified_item.bc_bearer_context_f1_u_tnl_infoat_cu);
              }

              bc_mrb_setup_or_modify_response_item.f1u_tnl_info_added_or_modified_list.push_back(f1u_tnl_info_added_or_modified_item);
            }
            break;
          }
          case asn1::e1ap::bcmrb_setup_modify_resp_list_item_ext_ies_o::ext_c::types::nulltype: {
            // TODO (borieher): Handle nulltype case
            break;
          }
        }
      }
    }
    response.bc_bearer_context_to_modify_response.bc_mrb_setup_or_modify_response_list.push_back(bc_mrb_setup_or_modify_response_item);
  }

  // Fill BC MRB Failed List (0..maxnoofMRBs).
  for (const auto& asn1_bc_mrb_failed_item : asn1_response->bc_bearer_context_to_modify_resp.bc_mrb_failed_list) {
    e1ap_bc_mrb_failed_item bc_mrb_failed_item;

    // Fill MRB ID (M).
    bc_mrb_failed_item.mrb_id = uint_to_mrb_id(asn1_bc_mrb_failed_item.mrb_id);

    // Fill Cause (M).
    bc_mrb_failed_item.cause = asn1_to_cause(asn1_bc_mrb_failed_item.cause);

    response.bc_bearer_context_to_modify_response.bc_mrb_failed_list.push_back(bc_mrb_failed_item);
  }

  // TODO (borieher): Fill Available BC MRB Configuration (O).
  // TODO (borieher): Fill Criticality Diagnostics (O).
}

} // namespace srs_cu_cp
} // namespace srsran
