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
    mbs_session_manager&                        mbs_session_mng_,
    e1ap_mbs_session_context_manager&           e1ap_mbs_session_ctxt_mng_,
    f1ap_mbs_session_context_manager&           f1ap_mbs_session_ctxt_mng_,
    srslog::basic_logger&                       logger_) :
    request(request_),
    mbs_session_mng(mbs_session_mng_),
    e1ap_mbs_session_ctxt_mng(e1ap_mbs_session_ctxt_mng_),
    f1ap_mbs_session_ctxt_mng(f1ap_mbs_session_ctxt_mng_),
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
    // TODO (borieher): Function to check MBS Session Setup Request Transfer -> MBS QoS Flows To Be Setup or Modified List and map to MRBs

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
      // NOTE (borieher): This shouldn't return the NGAP response yet
      resp_msg = handle_bc_bearer_context_setup_response(bc_bearer_context_setup_procedure_outcome.value());
    }
  }

  // Prepare F1AP Broadcast Context Setup Request and call F1AP notifier.
  {
    if (!fill_f1ap_broadcast_context_setup_request(broadcast_context_setup_request)) {
      logger.error("{}\" failed to fill F1AP Broadcast Context Setup Request", name());
      CORO_EARLY_RETURN(make_unexpected(fail_msg));
    }

    // TODO (borieher): Send F1AP Broadcast Context Setup Request to each DU involved
    CORO_AWAIT_VALUE(broadcast_context_setup_procedure_outcome,
        f1ap_mbs_session_ctxt_mng.handle_broadcast_context_setup_request(broadcast_context_setup_request));

    // Handle F1AP Broadcast Context Setup Response/Failure
    if (not broadcast_context_setup_procedure_outcome.has_value()) {
      logger.error("{}\" failed to setup broadcast context at DU", name());
      fail_msg = handle_broadcast_context_setup_failure(broadcast_context_setup_procedure_outcome.error());
      CORO_EARLY_RETURN(make_unexpected(fail_msg));
    } else {
      // NOTE (borieher): This shouldn't return the NGAP response yet
      resp_msg = handle_broadcast_context_setup_response(broadcast_context_setup_procedure_outcome.value());

      // NOTE (borieher): Working with the resp_msg created from the E1AP BC Bearer Context Setup Response for now
    }
  }

  // Prepare E1AP BC Bearer Context Modification Request and call E1AP notifier.
  {
    if (!fill_e1ap_bc_bearer_context_modification_request(bc_bearer_context_modification_request)) {
      logger.error("{}\" failed to fill E1AP BC Bearer Context Modification Request", name());
      CORO_EARLY_RETURN(make_unexpected(fail_msg));
    }

    // TODO (borieher): Send E1AP BC Bearer Context Modification Request to each CU-UP involved
    CORO_AWAIT_VALUE(bc_bearer_context_modification_procedure_outcome,
        e1ap_mbs_session_ctxt_mng.handle_bc_bearer_context_modification_request(bc_bearer_context_modification_request));

    // Handle E1AP BC Bearer Context Modification Response/Failure
    if (not bc_bearer_context_modification_procedure_outcome.has_value()) {
      logger.error("\"{}\" failed to modify BC Bearer at CU-CP", name());
      fail_msg = handle_bc_bearer_context_modification_failure(bc_bearer_context_modification_procedure_outcome.error());
      CORO_EARLY_RETURN(make_unexpected(fail_msg));
    } else {
      // NOTE (borieher): This should return the NGAP response
      resp_msg = handle_bc_bearer_context_modification_response(bc_bearer_context_modification_procedure_outcome.value());

      // NOTE (borieher): Working with the resp_msg created from the E1AP BC Bearer Context Setup Response for now
    }
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
    e1ap_bc_mrb_setup_config bc_mrb_to_setup_item;

    // Fill MRB ID.
    bc_mrb_to_setup_item.mrb_id = mbs_session_mng.get_next_mrb_id();

    // Fill MBS PDCP Configuration.
    // PDCP configuration extracted from 3GPP TS 38.331 clause 9.1.1.7 - MTCH configuration for MBS broadcast
    {
      e1ap_pdcp_config mbs_pdcp_cfg;
      mbs_pdcp_cfg.t_reordering_timer = pdcp_t_reordering::ms0; // No reordering timer for MBS
      mbs_pdcp_cfg.pdcp_sn_size_dl = pdcp_sn_size::size18bits;  // DL PDCP SN size for MBS
      mbs_pdcp_cfg.rlc_mod = pdcp_rlc_mode::um;                 // UM RLC mode
      mbs_pdcp_cfg.pdcp_sn_size_ul = pdcp_sn_size::size18bits;  // No UL PDCP SN size for MBS, workaround

      bc_mrb_to_setup_item.mbs_pdcp_cfg = mbs_pdcp_cfg;
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

    bc_mrb_to_setup_item.mbs_qos_flow_info_to_be_setup.push_back(qos_param_item);

    // Fill MRB QoS (O).
    // TODO (borieher): If more than 1 QoS Flow is mapped to this MRB
    //if (bc_mrb_setup_config.mbs_qos_flow_info_to_be_setup.size() > 1) {
    bc_mrb_to_setup_item.mrb_qos.emplace(qos_param_item.qos_flow_level_qos_params);
    //}

    // Fill F1-U TNL Info to Add List <0..1>.
    // TODO (borieher): Remove hardcoded value
    e1ap_bc_f1u_context_reference_e1 bc_f1u_context_reference_e1 = e1ap_bc_f1u_context_reference_e1::min;
    bc_mrb_to_setup_item.f1u_tnl_info_to_add_list.push_back(bc_f1u_context_reference_e1);

    e1ap_request.bc_bearer_context_to_setup.bc_mrb_to_setup_list.push_back(bc_mrb_to_setup_item);
  }

  // Fill Requested Action for Available Shared NG-U Termination (O).
  // TODO (borieher): Make it a configuration option
  e1ap_request.bc_bearer_context_to_setup.requested_action_for_available_shared_ngu_termination =
      e1ap_requested_action_for_available_shared_ngu_termination::apply_requested_config;

  return true;
}


ngap_broadcast_session_setup_response
broadcast_session_setup_routine::handle_bc_bearer_context_setup_response(const e1ap_bc_bearer_context_setup_response& msg)
{
  // Fill MBS Session ID (M).
  resp_msg.mbs_session_id = request.mbs_session_id;

  // TODO (borieher): Fill MBS Session Setup Response Transfer (O).
  // NOTE (borieher): MBS Session TNL Information NG-RAN (O) not being used.
  // TODO (borieher): Fill Criticality Diagnostics (O).

  return resp_msg;
}

ngap_broadcast_session_setup_failure
broadcast_session_setup_routine::handle_bc_bearer_context_setup_failure(const e1ap_bc_bearer_context_setup_failure& msg)
{
  // Fill MBS Session ID (M).
  fail_msg.mbs_session_id = request.mbs_session_id;

  // TODO (borieher): Fill MBS Session Setup or Modification Failure Transfer (O).
  // TODO (borieher): Fill Cause (M).
  // TODO (borieher): Fill Criticality Diagnostics (O).

  return fail_msg;
}

bool broadcast_session_setup_routine::fill_f1ap_broadcast_context_setup_request(
    f1ap_broadcast_context_setup_request& f1ap_request)
{
  f1ap_request.mbs_index = request.mbs_index;

  // Fill MBS Session ID.
  f1ap_request.mbs_session_id = request.mbs_session_id;

  // Fill MBS Service Area.
  // Parse location dependent from request
  if (request.mbs_service_area.is_locationdependent()) {
    f1ap_mbs_service_area_location_dependent locationdependent = {};

    for (const auto& locationdependent_item : request.mbs_service_area.get_locationdependent().mbs_service_area_information_list) {
      f1ap_mbs_service_area_information_item mbs_service_area_information_item;

      mbs_service_area_information_item.mbs_area_session_id = locationdependent_item.mbs_area_session_id;

      mbs_service_area_information_item.mbs_service_area_information.mbs_service_area_cell_list = locationdependent_item.mbs_service_area_information.mbs_service_area_cell_list;
      mbs_service_area_information_item.mbs_service_area_information.mbs_service_area_tai_list = locationdependent_item.mbs_service_area_information.mbs_service_area_tai_list;

      locationdependent.mbs_service_area_information_list.push_back(mbs_service_area_information_item);
    }

    f1ap_request.mbs_service_area.emplace(locationdependent);

  // Parse location independent from request
  } else {
    f1ap_mbs_service_area_location_independent locationindependent = {};
    const auto& mbs_service_area_information = request.mbs_service_area.get_locationindependent().mbs_service_area_information;

    locationindependent.mbs_service_area_information.mbs_service_area_cell_list = mbs_service_area_information.mbs_service_area_cell_list;
    locationindependent.mbs_service_area_information.mbs_service_area_tai_list = mbs_service_area_information.mbs_service_area_tai_list;

    f1ap_request.mbs_service_area.emplace(locationindependent);
  }

  // Fill MBS CU to DU RRC Information.

  // Grab NR CGI items from the MBS Service Area IE in the NGAP Broadcast Session Setup Request
  // NOTE (borieher): What happens if the NGAP Request has only the TAI list?
  // Parse location dependent
  if (request.mbs_service_area.is_locationdependent()) {
    for (const auto& locationdependent_item : request.mbs_service_area.get_locationdependent().mbs_service_area_information_list) {
      // NOTE (borieher): Each MBS Service Area information has a cell list
      if (!locationdependent_item.mbs_service_area_information.mbs_service_area_cell_list.empty()) {
        for (const auto& mbs_service_area_cell_item: locationdependent_item.mbs_service_area_information.mbs_service_area_cell_list) {
          // Check if this gNB has the specific NR CGI configured
          // TODO (borieher): if (nr_cgi is configured)
          f1ap_mbs_broadcast_cell_item mbs_broadcast_cell_item;
          mbs_broadcast_cell_item.nr_cgi = mbs_service_area_cell_item;

          // Fill RRC mtch_neighbour_cell
          mbs_broadcast_cell_item.mtch_neighbour_cell = get_packed_mtch_neighbour_cell_r17_ie();

          f1ap_request.mbs_cu_to_du_rrc_information.mbs_broadcast_cell_list.push_back(mbs_broadcast_cell_item);
        }
      }
    }
  // Parse location independent
  } else {
    const auto& mbs_service_area_information = request.mbs_service_area.get_locationindependent().mbs_service_area_information;
    // NOTE (borieher): Each MBS Service Area information item has a cell list
    if (!mbs_service_area_information.mbs_service_area_cell_list.empty()) {
      for (const auto& mbs_service_area_cell_item : mbs_service_area_information.mbs_service_area_cell_list) {
        // Check if this gNB has the specific NR CGI configured
        // TODO (borieher): if (nr_cgi is configured)
        f1ap_mbs_broadcast_cell_item mbs_broadcast_cell_item;
        mbs_broadcast_cell_item.nr_cgi = mbs_service_area_cell_item;

        // Fill RRC mtch_neighbour_cell
        mbs_broadcast_cell_item.mtch_neighbour_cell = get_packed_mtch_neighbour_cell_r17_ie();

        f1ap_request.mbs_cu_to_du_rrc_information.mbs_broadcast_cell_list.push_back(mbs_broadcast_cell_item);
      }
    }
  }

  // Grab MRB ID items from the BC MRB Setup Response List IE in the E1AP BC Bearer Context Setup Response
  if (bc_bearer_context_setup_procedure_outcome.has_value()) {
    e1ap_bc_bearer_context_setup_response bc_bearer_context_setup_response;
    bc_bearer_context_setup_response = bc_bearer_context_setup_procedure_outcome.value();

    for (const auto& bc_mrb_setup_response_item : bc_bearer_context_setup_response.bc_bearer_context_to_setup_response.bc_mrb_setup_response_list) {
      f1ap_mbs_broadcast_mrb_item mbs_broadcast_mrb_item;
      mbs_broadcast_mrb_item.mrb_id = bc_mrb_setup_response_item.mrb_id;

      // Fill the RRC mrb_pdcp_config_broadcast
      mbs_broadcast_mrb_item.mrb_pdcp_config_broadcast = get_packed_mrb_pdcp_config_broadcast_r17_ie();

      f1ap_request.mbs_cu_to_du_rrc_information.mbs_broadcast_mrb_list.push_back(mbs_broadcast_mrb_item);
    }
  }

  // Fill S-NSSAI.
  f1ap_request.s_nssai = request.s_nssai;

  // Fill Broadcast MRB To Be Setup List.
  // Grab MRBs from the BC MRB Setup Response List IE in the E1AP BC Bearer Context Setup Response
  if (bc_bearer_context_setup_procedure_outcome.has_value()) {
    e1ap_bc_bearer_context_setup_response bc_bearer_context_setup_response;
    bc_bearer_context_setup_response = bc_bearer_context_setup_procedure_outcome.value();

    for (auto& bc_mrb_setup_response_item : bc_bearer_context_setup_response.bc_bearer_context_to_setup_response.bc_mrb_setup_response_list) {
      f1ap_broadcast_mrb_to_be_setup_item mbs_mrb_to_be_setup_item;

      mbs_mrb_to_be_setup_item.mrb_id = bc_mrb_setup_response_item.mrb_id;

      // NOTE (borieher): This needs to be checked, both rely on the optional MRB QoS IE,
      // if they are not present should put the MRB QoS in there
      // Fill MRB QoS Information IE with the E1AP BC Bearer Context Setup Response Available BC MRB Configuration IE
      if (bc_bearer_context_setup_response.bc_bearer_context_to_setup_response.available_bc_mrb_configuration.has_value()) {
        for (const auto& available_bc_mrb_config_item : bc_bearer_context_setup_response.bc_bearer_context_to_setup_response.available_bc_mrb_configuration.value()) {
          if (available_bc_mrb_config_item.mrb_id == bc_mrb_setup_response_item.mrb_id) {
            if (available_bc_mrb_config_item.mrb_qos.has_value()) {
              mbs_mrb_to_be_setup_item.mrb_qos_information.qos_desc = available_bc_mrb_config_item.mrb_qos.value().qos_desc;
              mbs_mrb_to_be_setup_item.mrb_qos_information.alloc_retention_prio = available_bc_mrb_config_item.mrb_qos.value().ng_ran_alloc_retention;

              if (available_bc_mrb_config_item.mrb_qos.value().gbr_qos_flow_info.has_value()) {
                mbs_mrb_to_be_setup_item.mrb_qos_information.gbr_qos_info = available_bc_mrb_config_item.mrb_qos.value().gbr_qos_flow_info.value();
              }
            }
          }
        }
      } else {
        // Fill MRB QoS Information IE with the E1AP BC Bearer Context Setup Request BC MRB Setup Configuration IE
        // Search in the MRB To Setup List of the E1AP BC Bearer Context Setup Request for the MRB ID
        for (const auto& bc_mrb_to_setup_item : bc_bearer_context_setup_request.bc_bearer_context_to_setup.bc_mrb_to_setup_list) {
          if (bc_mrb_to_setup_item.mrb_id == bc_mrb_setup_response_item.mrb_id) {
            if (bc_mrb_to_setup_item.mrb_qos.has_value()) {
              mbs_mrb_to_be_setup_item.mrb_qos_information.qos_desc = bc_mrb_to_setup_item.mrb_qos.value().qos_desc;
              mbs_mrb_to_be_setup_item.mrb_qos_information.alloc_retention_prio = bc_mrb_to_setup_item.mrb_qos.value().ng_ran_alloc_retention;

              if (bc_mrb_to_setup_item.mrb_qos.value().gbr_qos_flow_info.has_value()) {
                mbs_mrb_to_be_setup_item.mrb_qos_information.gbr_qos_info = bc_mrb_to_setup_item.mrb_qos.value().gbr_qos_flow_info.value();
              }
            } else {
              // Only 1 QoS Flow mapped to this MRB
              if (bc_mrb_to_setup_item.mbs_qos_flow_info_to_be_setup.size() == 1) {
                mbs_mrb_to_be_setup_item.mrb_qos_information.qos_desc =
                  bc_mrb_to_setup_item.mbs_qos_flow_info_to_be_setup[0].qos_flow_level_qos_params.qos_desc;
                mbs_mrb_to_be_setup_item.mrb_qos_information.alloc_retention_prio =
                  bc_mrb_to_setup_item.mbs_qos_flow_info_to_be_setup[0].qos_flow_level_qos_params.ng_ran_alloc_retention;

                if (bc_mrb_to_setup_item.mbs_qos_flow_info_to_be_setup[0].qos_flow_level_qos_params.gbr_qos_flow_info.has_value()) {
                  mbs_mrb_to_be_setup_item.mrb_qos_information.gbr_qos_info =
                    bc_mrb_to_setup_item.mbs_qos_flow_info_to_be_setup[0].qos_flow_level_qos_params.gbr_qos_flow_info.value();
                }
              } else {
                // TODO: Send error, more than 1 QoS Flow mapped to the same MRB and no bc_mrb_to_setup_item.mrb_qos
              }
            }
          }
        }
      }

      // Fill MBS QoS Flows Mapped to MRB Item.
      for (const auto& qos_flow_to_be_mapped_to_mrb : bc_mrb_setup_response_item.mbs_qos_flow_setup_list) {
        // Grab the QoS Flow ID from the E1AP BC Bearer Context Setup Response and the MRB QoS Information IE from the F1AP BC Bearer Context Setup Request itself
        f1ap_mbs_qos_flows_mapped_to_mrb_item mbs_qos_flow_mapped_to_mrb;

        mbs_qos_flow_mapped_to_mrb.mbs_qos_flow_id = qos_flow_to_be_mapped_to_mrb.qos_flow_id;
        mbs_qos_flow_mapped_to_mrb.mbs_qos_flow_level_qos_parameters = mbs_mrb_to_be_setup_item.mrb_qos_information;

        mbs_mrb_to_be_setup_item.mbs_qos_flows_mapped_to_mrb.push_back(mbs_qos_flow_mapped_to_mrb);
      }

      // Fill BC Bearer Context F1-U TNL Info at CU.
      // If F1-U TNL Info Added List has value, the BC Bearer Context F1-U TNL Info at CU gets ignored
      if (!bc_mrb_setup_response_item.f1u_tnl_info_added_list.empty()) {
        for (auto& e1ap_f1u_tnl_info_added_item : bc_mrb_setup_response_item.f1u_tnl_info_added_list) {
          // Parse location dependent
          if (e1ap_f1u_tnl_info_added_item.bc_bearer_context_f1u_tnl_info_at_cu.is_locationdependent()) {
            f1ap_bc_bearer_ctxt_f1u_tnl_info_location_dependent f1ap_locationdependent;
            for (const auto& e1ap_locationdependent_item : e1ap_f1u_tnl_info_added_item.bc_bearer_context_f1u_tnl_info_at_cu
                .get_locationdependent().location_dependent_mbs_f1u_information_at_cu) {
              f1ap_bc_bearer_ctxt_f1u_tnl_info_location_dependent_item f1ap_locationdependent_item;

              f1ap_locationdependent_item.mbs_area_session_id = e1ap_locationdependent_item.mbs_area_session_id;

              f1ap_locationdependent_item.mbs_f1u_information = up_transport_layer_info(
                e1ap_locationdependent_item.mbs_f1u_information_at_cu.tp_address, e1ap_locationdependent_item.mbs_f1u_information_at_cu.gtp_teid);

              f1ap_locationdependent.location_dependent_mbs_f1u_information.push_back(f1ap_locationdependent_item);
            }
            mbs_mrb_to_be_setup_item.bc_bearer_context_f1u_tnl_info_at_cu = f1ap_locationdependent;
          // Parse location independent
          } else {
            const auto& e1ap_locationindependent = e1ap_f1u_tnl_info_added_item.bc_bearer_context_f1u_tnl_info_at_cu.get_locationindependent();
            f1ap_bc_bearer_ctxt_f1u_tnl_info_location_independent f1ap_locationindependent;

            f1ap_locationindependent.mbs_f1u_information = up_transport_layer_info(
                e1ap_locationindependent.mbs_f1u_information_at_cu.tp_address, e1ap_locationindependent.mbs_f1u_information_at_cu.gtp_teid);

            mbs_mrb_to_be_setup_item.bc_bearer_context_f1u_tnl_info_at_cu.get_locationindependent() = f1ap_locationindependent;
          }
        }
      // Grab it from BC Bearer Context F1-U TNL Info at CU
      } else {
        // Parse location dependent
        if (bc_mrb_setup_response_item.bc_bearer_context_f1u_tnl_info_at_cu.is_locationdependent()) {
          f1ap_bc_bearer_ctxt_f1u_tnl_info_location_dependent f1ap_locationdependent;
          for (const auto& e1ap_locationdependent_item : bc_mrb_setup_response_item.bc_bearer_context_f1u_tnl_info_at_cu
                .get_locationdependent().location_dependent_mbs_f1u_information_at_cu) {
            f1ap_bc_bearer_ctxt_f1u_tnl_info_location_dependent_item f1ap_locationdependent_item;

            f1ap_locationdependent_item.mbs_area_session_id = e1ap_locationdependent_item.mbs_area_session_id;

            f1ap_locationdependent_item.mbs_f1u_information = up_transport_layer_info(
                e1ap_locationdependent_item.mbs_f1u_information_at_cu.tp_address, e1ap_locationdependent_item.mbs_f1u_information_at_cu.gtp_teid);

            f1ap_locationdependent.location_dependent_mbs_f1u_information.push_back(f1ap_locationdependent_item);
          }
          mbs_mrb_to_be_setup_item.bc_bearer_context_f1u_tnl_info_at_cu = f1ap_locationdependent;
        // Parse location independent
        } else {
          const auto& e1ap_locationindependent = bc_mrb_setup_response_item.bc_bearer_context_f1u_tnl_info_at_cu
            .get_locationindependent().mbs_f1u_information_at_cu;
          f1ap_bc_bearer_ctxt_f1u_tnl_info_location_independent f1ap_locationindependent;

          f1ap_locationindependent.mbs_f1u_information = up_transport_layer_info(
              e1ap_locationindependent.tp_address, e1ap_locationindependent.gtp_teid);

          mbs_mrb_to_be_setup_item.bc_bearer_context_f1u_tnl_info_at_cu.get_locationindependent() = f1ap_locationindependent;
        }
      }

      f1ap_request.broadcast_mrb_to_be_setup_list.push_back(mbs_mrb_to_be_setup_item);
    }
  }

  return true;
}

ngap_broadcast_session_setup_response
broadcast_session_setup_routine::handle_broadcast_context_setup_response(const f1ap_broadcast_context_setup_response& msg)
{
  return resp_msg;
}

ngap_broadcast_session_setup_failure
broadcast_session_setup_routine::handle_broadcast_context_setup_failure(const f1ap_broadcast_context_setup_failure& msg)
{
  return fail_msg;
}

bool broadcast_session_setup_routine::fill_e1ap_bc_bearer_context_modification_request(
    e1ap_bc_bearer_context_modification_request& e1ap_request){

  // Fill gNB-CU-CP MBS E1AP ID (M).
  e1ap_request.gnb_cu_cp_mbs_e1ap_id = bc_bearer_context_setup_request.gnb_cu_cp_mbs_e1ap_id;

  // Fill gNB-CU-UP MBS E1AP ID (M).
  // Grab gNB-CU-UP MBS E1AP ID from the E1AP BC Bearer Context Setup Response
  if (bc_bearer_context_setup_procedure_outcome.has_value()) {
    e1ap_request.gnb_cu_up_mbs_e1ap_id = bc_bearer_context_setup_procedure_outcome.value().gnb_cu_up_mbs_e1ap_id;
  }

  // Fill BC Bearer Context To Modify (M).
  // TODO (borieher): Fill BC Bearer Context NGU TNL Info at 5GC To Setup or Modify (O).

  // TODO (borieher): Fill BC MRB To Setup List (O).
  // NOTE (borieher): For now, no more MRBs to setup.

  // Fill BC MRB To Modify List (0..maxnoofMRBs).
  // NOTE (borieher): Modify the MRBs set up in the F1AP Broadcast Context Setup Response
  if (broadcast_context_setup_procedure_outcome.has_value()) {
    for (auto& broadcast_context_setup_item : broadcast_context_setup_procedure_outcome.value().broadcast_mrb_setup_list) {
      e1ap_bc_mrb_to_modify_item mrb_to_modify_item;

      // Fill MRB ID (M).
      mrb_to_modify_item.mrb_id = broadcast_context_setup_item.mrb_id;

      // Fill BC Bearer Context F1-U TNL Info at DU (O).
      // NOTE (borieher): Grab this from the F1AP Broadcast Context Setup Response
      // Parse location dependent
      if (broadcast_context_setup_item.bc_bearer_context_f1u_tnl_info_at_du.is_locationdependent()) {
        e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_dependent locationdependent;

        for (const auto& locationdependent_item : broadcast_context_setup_item.bc_bearer_context_f1u_tnl_info_at_du
            .get_locationdependent().location_dependent_mbs_f1u_information) {
          e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_dependent_item e1ap_locationdependent_item;

          e1ap_locationdependent_item.mbs_area_session_id = locationdependent_item.mbs_area_session_id;

          e1ap_locationdependent_item.mbs_f1u_information_at_du = locationdependent_item.mbs_f1u_information;

          e1ap_locationdependent_item.mbs_f1u_information_at_du = up_transport_layer_info(
              locationdependent_item.mbs_f1u_information.tp_address, locationdependent_item.mbs_f1u_information.gtp_teid);

          locationdependent.location_dependent_mbs_f1u_information_at_du.push_back(e1ap_locationdependent_item);
        }

        mrb_to_modify_item.bc_bearer_context_f1u_tnl_info_at_du.emplace(locationdependent);

      // Parse location independent
      } else {
        e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_independent e1ap_locationindependent;

        const auto& locationdependent = broadcast_context_setup_item.bc_bearer_context_f1u_tnl_info_at_du
          .get_locationindependent().mbs_f1u_information;

        e1ap_locationindependent.mbs_f1u_information_at_du = up_transport_layer_info(
              locationdependent.tp_address, locationdependent.gtp_teid);

        mrb_to_modify_item.bc_bearer_context_f1u_tnl_info_at_du.emplace(e1ap_locationindependent);
      }

      // Fill MBS PDCP Configuration (O).
      // NOTE (borieher): For now, no MBS PDCP Configuration to modify.

      // Fill MBS QoS Flows Information To Be Setup (O).
      // NOTE (borieher): For now, no more QoS Flows to setup.

      // Fill MRB QoS (O).
      // NOTE (borieher): For now, no MRB QoS to modify.

      // Fill F1-U TNL Info to Add or Modify List (0..1).
      // NOTE (borieher): Not sure for now how to update this based on F1AP information

      // TODO (borieher): Fill F1-U TNL Info to Release List (0..1).
      // NOTE (borieher): For now, no F1-U TNL Info to release.

      e1ap_request.bc_bearer_context_to_modify.bc_mrb_to_modify_list.push_back(mrb_to_modify_item);
    }
  }

  // TODO (borieher): Fill BC MRB To Remove List (0..maxnoofMRBs).
  // NOTE (borieher): For now, no MRBs to remove.

  return true;
}

ngap_broadcast_session_setup_response
broadcast_session_setup_routine::handle_bc_bearer_context_modification_response(const e1ap_bc_bearer_context_modification_response& msg)
{
  return resp_msg;
}

ngap_broadcast_session_setup_failure
broadcast_session_setup_routine::handle_bc_bearer_context_modification_failure(const e1ap_bc_bearer_context_modification_failure& msg)
{
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
