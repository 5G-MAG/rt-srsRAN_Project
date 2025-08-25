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

#include "broadcast_session_setup_routine.h"
#include "srsran/pdcp/pdcp_sn_size.h"
#include "srsran/asn1/rrc_nr/rrc_nr.h"
#include "srsran/asn1/asn1_utils.h"

using namespace srsran;
using namespace srsran::srs_cu_cp;

broadcast_session_setup_routine::broadcast_session_setup_routine(
    const ngap_broadcast_session_setup_request& request_,
    e1ap_mbs_session_context_manager&           e1ap_mbs_session_ctxt_mng_,
    srslog::basic_logger&                       logger_) :
    request(request_),
    e1ap_mbs_session_ctxt_mng(e1ap_mbs_session_ctxt_mng_),
    logger(logger_)
{
}

void broadcast_session_setup_routine::operator()(
    coro_context<async_task<expected<ngap_broadcast_session_setup_response, ngap_broadcast_session_setup_failure>>>& ctx)
{
  CORO_BEGIN(ctx);

  logger.debug("\"{}\" initialized", name());

  // Prepare E1AP BC Bearer Context Setup Request and call E1AP notifier.
  {
    if (!fill_e1ap_bc_bearer_context_setup_request(bc_bearer_context_setup_request)) {
      logger.error("{}\" failed to fill E1AP BC Bearer Context Setup Request", name());
      CORO_EARLY_RETURN(make_unexpected(fail_msg));
    }

    // TODO (borieher): Send E1AP BC Bearer Context Setup Request to each CU-UP involved
    CORO_AWAIT_VALUE(bc_bearer_context_setup_procedure_outcome,
        e1ap_mbs_session_ctxt_mng.handle_bc_bearer_context_setup_request(bc_bearer_context_setup_request));

    // Handle E1AP BC Bearer Context Setup Response/Failure
    if (not bc_bearer_context_setup_procedure_outcome.has_value()) {
      logger.error("\"{}\" failed to setup BC Bearer at CU-CP", name());
      fail_msg = handle_bc_bearer_context_setup_failure(bc_bearer_context_setup_procedure_outcome.error());
      CORO_EARLY_RETURN(make_unexpected(fail_msg));
    } else {
      // TODO (borieher): Perform Multicast join here at the CU-UP with the E1AP information

      resp_msg = handle_bc_bearer_context_setup_response(bc_bearer_context_setup_procedure_outcome.value());

      // How do I get the mbs_session_id from the gnb_cu_cp_mbs_e1ap_id_t?
    }
  }

  // Prepare F1AP Broadcast Context Setup Request and call F1AP notifier.
  {
    // TODO (borieher): Send F1AP Broadcast Context Setup Request to each DU involved

    // Handle F1AP Broadcast Context Setup Response/Failure
  }

  // Prepare E1AP BC Bearer Context Modification Request and call E1AP notifier.
  {
    // TODO (borieher): Send E1AP BC Bearer Context Modification Request to each CU-UP involved

    // Handle E1AP BC Bearer Context Modification Response/Failure
  }

  logger.debug("\"{}\" setup broadcast bearer at CU-CP finished", name());
  CORO_RETURN(resp_msg);
}

bool broadcast_session_setup_routine::fill_e1ap_bc_bearer_context_setup_request(
    e1ap_bc_bearer_context_setup_request& e1ap_request)
{
  e1ap_request.mbs_index = request.mbs_index;

  // Fill Global MBS Session ID.
  e1ap_request.global_mbs_session_id = request.mbs_session_id;

  // Fill BC Bearer Context To Setup.
  // Fill S-NSSAI.
  e1ap_request.bc_bearer_context_to_setup.s_nssai = request.s_nssai;

  // Fill BC Bearer Context NG-U TNL Info at 5GC.
  if (request.mbs_session_setup_request_transfer.mbs_session_tnl_information_5gc.has_value()) {
    // Parse location dependent from request
    if (request.mbs_session_setup_request_transfer.mbs_session_tnl_information_5gc.value().is_locationdependent()) {
      e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent locationdependent = {};

      for (const auto& locationdependent_item : request.mbs_session_setup_request_transfer.mbs_session_tnl_information_5gc.value()
        .get_locationdependent().mbs_session_tnl_information_5gc_list) {
        e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent_item bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent_item;

        bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent_item.mbs_area_session_id = locationdependent_item.mbs_area_session_id;

        bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent_item.mbs_ngu_information_at_5gc.ip_multicast_address =
          locationdependent_item.shared_ngu_multicast_tnl_information.ip_multicast_address;
        bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent_item.mbs_ngu_information_at_5gc.ip_source_address =
          locationdependent_item.shared_ngu_multicast_tnl_information.ip_source_address;
        bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent_item.mbs_ngu_information_at_5gc.gtp_dl_teid =
          locationdependent_item.shared_ngu_multicast_tnl_information.gtp_teid_at_5gc;

        locationdependent.location_dependent_mbs_ngu_info_at_5gc.push_back(bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent_item);
      }

      e1ap_request.bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_info_at_5gc.emplace(locationdependent);

    // Parse location independent from request
    } else {
      e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_independent locationindependent = {};

      locationindependent.mbs_ngu_information_at_5gc.ip_multicast_address =
        request.mbs_session_setup_request_transfer.mbs_session_tnl_information_5gc.value()
          .get_locationindependent().shared_ngu_multicast_tnl_information.ip_multicast_address;
      locationindependent.mbs_ngu_information_at_5gc.ip_source_address =
        request.mbs_session_setup_request_transfer.mbs_session_tnl_information_5gc.value()
          .get_locationindependent().shared_ngu_multicast_tnl_information.ip_source_address;
      locationindependent.mbs_ngu_information_at_5gc.gtp_dl_teid =
        request.mbs_session_setup_request_transfer.mbs_session_tnl_information_5gc.value()
          .get_locationindependent().shared_ngu_multicast_tnl_information.gtp_teid_at_5gc;

        e1ap_request.bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_info_at_5gc.emplace(locationindependent);
    }
  }

  // Fill BC MRB To Setup List.
  // TODO: Allow mapping more than 1 QoS Flow to the same MRB, currently 1 QoS Flow <-> 1 MRB
  // A pre-processing of the QoS Flows will be needed for that
  for (const auto& mbs_qos_flow_to_be_setup_item : request.mbs_session_setup_request_transfer.mbs_qos_flows_to_be_setup_or_modified_list) {
    e1ap_bc_mrb_setup_config bc_mrb_setup_config;

    // Fill MRB ID.
    // TODO: Remove this hardcoded MRB ID value
    // Check the QoS Flows and map them to available MRBs
    bc_mrb_setup_config.mrb_id = mrb_id_t::mrb1;

    // Fill MBS PDCP Configuration.
    // PDCP configuration extracted from 3GPP TS 38.331 clause 9.1.1.7 - MTCH configuration for MBS broadcast
    {
      e1ap_pdcp_config mbs_pdcp_cfg;
      mbs_pdcp_cfg.t_reordering_timer = pdcp_t_reordering::ms0; // No reordering timer for MBS
      mbs_pdcp_cfg.pdcp_sn_size_dl = pdcp_sn_size::size18bits;  // DL PDCP SN size for MBS
      mbs_pdcp_cfg.rlc_mod = pdcp_rlc_mode::um;                 // UM RLC mode
      mbs_pdcp_cfg.pdcp_sn_size_ul = pdcp_sn_size::size18bits;  // No UL PDCP SN size for MBS, workaround

      bc_mrb_setup_config.mbs_pdcp_cfg = mbs_pdcp_cfg;
    }

    // NOTE (borieher): More than one QoS flow could be mapped to the same MRB. For now each flow has its own MRB.
    // Fill MBS QoS Flows Information To Be Setup.
    e1ap_qos_flow_qos_param_item qos_param_item;
    // Fill QoS Flow Identifier.
    qos_param_item.qos_flow_id = mbs_qos_flow_to_be_setup_item.mbs_qos_flow_identifier;
    // Fill QoS Flow Level QoS Parameters.
    qos_param_item.qos_flow_level_qos_params.qos_desc = mbs_qos_flow_to_be_setup_item.mbs_qos_flow_level_qos_parameters.qos_desc;
    qos_param_item.qos_flow_level_qos_params.ng_ran_alloc_retention = mbs_qos_flow_to_be_setup_item.mbs_qos_flow_level_qos_parameters.allocation_and_retention_priority;
    if (mbs_qos_flow_to_be_setup_item.mbs_qos_flow_level_qos_parameters.gbr_qos_flow_info.has_value()) {
      qos_param_item.qos_flow_level_qos_params.gbr_qos_flow_info.emplace(mbs_qos_flow_to_be_setup_item.mbs_qos_flow_level_qos_parameters.gbr_qos_flow_info.value());
    }

    bc_mrb_setup_config.mbs_qos_flow_info_to_be_setup.push_back(qos_param_item);

    // Fill MRB QoS (O).
    // If more than 1 QoS Flow is mapped to this MRB
    if (bc_mrb_setup_config.mbs_qos_flow_info_to_be_setup.size() > 1) {
      bc_mrb_setup_config.mrb_qos.emplace(qos_param_item.qos_flow_level_qos_params);
    }

    // Fill F1-U TNL Info to Add List <0..1>.
    // TODO (borieher): Remove hardcoded value
    e1ap_bc_f1u_context_reference_e1 bc_f1u_context_reference_e1 = e1ap_bc_f1u_context_reference_e1::min;
    bc_mrb_setup_config.f1u_tnl_info_to_add_list.push_back(bc_f1u_context_reference_e1);

    e1ap_request.bc_bearer_context_to_setup.bc_mrb_to_setup_list.push_back(bc_mrb_setup_config);
  }

  // Fill Requested Action for Available Shared NG-U Termination (O).
  // TODO (borieher): Make it a configuration option
  e1ap_request.bc_bearer_context_to_setup.requested_action_for_available_shared_ngu_termination =
      e1ap_requested_action_for_available_shared_ngu_termination::apply_requested_config;

  return true;
}


ngap_broadcast_session_setup_response
broadcast_session_setup_routine::handle_bc_bearer_context_setup_response(const e1ap_bc_bearer_context_setup_response& msg) {
  // Fill MBS Session ID (M).
  resp_msg.mbs_session_id = request.mbs_session_id;

  // TODO (borieher): Fill MBS Session Setup Response Transfer (O).
  // TODO (borieher): Fill MBS Session TNL Information NG-RAN (O).
  if (msg.bc_bearer_context_to_setup_response.bc_bearer_context_ngu_tnl_info_at_ng_ran.has_value()) {}
  // TODO (borieher): Fill Criticality Diagnostics (O).

  return resp_msg;
}

ngap_broadcast_session_setup_failure
broadcast_session_setup_routine::handle_bc_bearer_context_setup_failure(const e1ap_bc_bearer_context_setup_failure& msg) {
  // Fill MBS Session ID (M).
  fail_msg.mbs_session_id = request.mbs_session_id;

  // TODO (borieher): Fill MBS Session Setup or Modification Failure Transfer (O).
  // TODO (borieher): Fill Cause (M).
  // TODO (borieher): Fill Criticality Diagnostics (O).

  return fail_msg;
}

// RRC helpers
byte_buffer broadcast_session_setup_routine::get_packed_mtch_neighbour_cell_r17_ie()
{
  asn1::fixed_bitstring<8> mtch_neighbour_cell_r17;
  byte_buffer   pdu{};
  asn1::bit_ref bref{pdu};

  // TODO (borieher): Grab the MBS-NeighbourCellList-r17

  // NOTE (borieher): No MTCH neighbour cells
  mtch_neighbour_cell_r17.from_number(0);

  if (mtch_neighbour_cell_r17.pack(bref) == asn1::SRSASN_ERROR_ENCODE_FAIL) {
    logger.error("Failed to pack mtch-NeighbourCell-r17 IE. Discarding it.");
  }
  return pdu;
}

byte_buffer broadcast_session_setup_routine::get_packed_mrb_pdcp_config_broadcast_r17_ie()
{
  struct asn1::rrc_nr::mrb_pdcp_cfg_broadcast_r17_s mrb_pdcp_cfg_broadcast_r17;
  byte_buffer   pdu{};
  asn1::bit_ref bref{pdu};

  // PDCP configuration extracted from 3GPP TS 38.331 clause 9.1.1.7 - MTCH configuration for MBS broadcast
  // If not set, the default value for pdcp_sn_size_dl_r17 is len18bits
  mrb_pdcp_cfg_broadcast_r17.pdcp_sn_size_dl_r17_present = false;

  // If not set, the default value for t_reordering_r17 is ms0
  mrb_pdcp_cfg_broadcast_r17.t_reordering_r17_present = false;

  // NOTE (borieher): Not using PDCP ROHC
  mrb_pdcp_cfg_broadcast_r17.hdr_compress_r17.set_not_used();

  if (mrb_pdcp_cfg_broadcast_r17.pack(bref) == asn1::SRSASN_ERROR_ENCODE_FAIL) {
    logger.error("Failed to pack MRB-PDCP-ConfigBroadcast-r17 IE. Discarding it.");
  }
  return pdu;
}
