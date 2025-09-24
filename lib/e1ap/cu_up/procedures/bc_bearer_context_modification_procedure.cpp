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

#include "bc_bearer_context_modification_procedure.h"
#include "cu_up/e1ap_cu_up_asn1_helpers.h"

using namespace srsran;
using namespace srsran::srs_cu_up;
using namespace asn1::e1ap;

bc_bearer_context_modification_procedure::bc_bearer_context_modification_procedure(const e1ap_bc_bearer_context_modification_request& request_,
                                                                                   e1ap_message_notifier&                      pdu_notifier_,
                                                                                   mbs_broadcast_session_modification_result&  broadcast_session_modification_result_,
                                                                                   srslog::basic_logger&                       logger_) :
request(request_), pdu_notifier(pdu_notifier_), broadcast_session_modification_result(broadcast_session_modification_result_), logger(logger_)
{
}

void bc_bearer_context_modification_procedure::operator()(coro_context<async_task<void>>& ctx)
{
  CORO_BEGIN(ctx);

  logger.debug("\"{}\" initialized", name());

  // Process the E1AP BC Bearer Context Modification Request
  fill_e1ap_bc_bearer_context_modification_response();

  // Send command to CU-CP.
  send_bc_bearer_context_modification_response();

  CORO_RETURN();
}

bool bc_bearer_context_modification_procedure::fill_e1ap_bc_bearer_context_modification_response()
{
  // Fill gNB-CU-CP MBS E1AP ID (M).
  response.gnb_cu_cp_mbs_e1ap_id = request.gnb_cu_cp_mbs_e1ap_id;

  // Fill gNB-CU-UP MBS E1AP ID (M).
  response.gnb_cu_up_mbs_e1ap_id = request.gnb_cu_up_mbs_e1ap_id;

  // Fill BC Bearer Context To Modify Response (M).
  // TODO (borieher): Fill BC Bearer Context NG-U TNL Info at NG-RAN (O).

  // Fill BC MRB Setup or Modify Response List <1..maxnoofMRBs> and BC MRB Failed List <0..maxnoofMRBs>.
  for (const auto& mrb_to_modify_result_item : broadcast_session_modification_result.mrb_modification_results) {
    // Fill BC MRB Setup or Modify Response List <1..maxnoofMRBs>.
    if (mrb_to_modify_result_item.success) {
      e1ap_bc_mrb_setup_or_modify_response_item bc_mrb_setup_or_modify_response_item;

      // Fill MRB ID (M).
      bc_mrb_setup_or_modify_response_item.mrb_id = mrb_to_modify_result_item.mrb_id;

      // Fill MBS QoS Flow Setup List (O) and MBS QoS Flow Failed List (O).
      for (const auto& mrb_qos_flow_item : mrb_to_modify_result_item.mbs_qos_flow_results) {
        // Fill MBS QoS Flow Setup List (O).
        if (mrb_qos_flow_item.success) {
          // Initialize MBS QoS Flow Setup List (O) if not already done
          if (!bc_mrb_setup_or_modify_response_item.mbs_qos_flow_setup_list.has_value()) {
            bc_mrb_setup_or_modify_response_item.mbs_qos_flow_setup_list.emplace();
          }
          e1ap_qos_flow_item mbs_qos_flow_setup_item;

          // Fill QoS Flow Identifier (M).
          mbs_qos_flow_setup_item.qos_flow_id = mrb_qos_flow_item.qos_flow_id;

          bc_mrb_setup_or_modify_response_item.mbs_qos_flow_setup_list->push_back(mbs_qos_flow_setup_item);

        // Fill MBS QoS Flow Failed List (O).
        } else if (!mrb_qos_flow_item.success) {
          // Initialize MBS QoS Flow Failed List (O) if not already done
          if (!bc_mrb_setup_or_modify_response_item.mbs_qos_flow_failed_list.has_value()) {
            bc_mrb_setup_or_modify_response_item.mbs_qos_flow_failed_list.emplace();
          }
          e1ap_qos_flow_failed_item mbs_qos_flow_failed_item;

          // Fill QoS Flow Identifier (M).
          mbs_qos_flow_failed_item.qos_flow_id = mrb_qos_flow_item.qos_flow_id;

          // Fill Cause (M).
          mbs_qos_flow_failed_item.cause = mrb_qos_flow_item.cause;

          bc_mrb_setup_or_modify_response_item.mbs_qos_flow_failed_list->push_back(mbs_qos_flow_failed_item);
        }
      }

      // Fill BC Bearer Context F1-U TNL Info at CU (O).
      // Iterate over the MRB To Modify List from the E1AP BC Bearer Context Modification Request and check only this MRB
      for (auto& f1u_tnl_info_to_modify_item : request.bc_bearer_context_to_modify.bc_mrb_to_modify_list) {
        // Check if the MRB IDs match
        if (f1u_tnl_info_to_modify_item.mrb_id == mrb_to_modify_result_item.mrb_id) {
          if (f1u_tnl_info_to_modify_item.bc_bearer_context_f1u_tnl_info_at_du.has_value()) {
            // Parse location dependent from request
            if (f1u_tnl_info_to_modify_item.bc_bearer_context_f1u_tnl_info_at_du.value().is_locationdependent()) {
              e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent locationdependent = {};

              for (const auto& request_locationdependent_item : f1u_tnl_info_to_modify_item.bc_bearer_context_f1u_tnl_info_at_du.value()
                      .get_locationdependent().location_dependent_mbs_f1u_information_at_du) {
                e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent_item locationdependent_item;

                // Fill MBS Area Session ID (M).
                locationdependent_item.mbs_area_session_id = request_locationdependent_item.mbs_area_session_id;

                // Fill MBS F1-U Information at CU (M).
                up_transport_layer_info mbs_f1u_information_at_cu;

                mbs_f1u_information_at_cu.tp_address =
                    transport_layer_address::create_from_string(mrb_to_modify_result_item.gtp_tunnel.tp_address.to_string());
                mbs_f1u_information_at_cu.gtp_teid =
                    int_to_gtpu_teid(mrb_to_modify_result_item.gtp_tunnel.gtp_teid.value());

                locationdependent_item.mbs_f1u_information_at_cu = mbs_f1u_information_at_cu;

                locationdependent.location_dependent_mbs_f1u_information_at_cu.push_back(locationdependent_item);
              }

              bc_mrb_setup_or_modify_response_item.bc_bearer_context_f1u_tnl_info_at_cu.emplace(locationdependent);

            // Parse location independent from request
            } else {
              e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_independent locationindependent = {};

              up_transport_layer_info mbs_f1u_information_at_cu;
              mbs_f1u_information_at_cu.tp_address =
                  transport_layer_address::create_from_string(mrb_to_modify_result_item.gtp_tunnel.tp_address.to_string());
              mbs_f1u_information_at_cu.gtp_teid =
                  int_to_gtpu_teid(mrb_to_modify_result_item.gtp_tunnel.gtp_teid.value());

              locationindependent.mbs_f1u_information_at_cu = mbs_f1u_information_at_cu;

              bc_mrb_setup_or_modify_response_item.bc_bearer_context_f1u_tnl_info_at_cu.emplace(locationindependent);

            }
          }
        // MRB IDs do not match
        } else if (f1u_tnl_info_to_modify_item.mrb_id != mrb_to_modify_result_item.mrb_id) {
          continue;
        }
      }

      // TODO (borieher): Fill F1-U TNL Info Added or Modified List <0..1>.

      response.bc_bearer_context_to_modify_response.bc_mrb_setup_or_modify_response_list.push_back(bc_mrb_setup_or_modify_response_item);

    // Fill BC MRB Failed List <0..maxnoofMRBs>.
    } else if (!mrb_to_modify_result_item.success) {
      e1ap_bc_mrb_failed_item bc_mrb_failed_item;

      // Fill MRB ID (M).
      bc_mrb_failed_item.mrb_id = mrb_to_modify_result_item.mrb_id;

      // Fill Cause (M).
      bc_mrb_failed_item.cause = mrb_to_modify_result_item.cause;

      response.bc_bearer_context_to_modify_response.bc_mrb_failed_list.push_back(bc_mrb_failed_item);
    }
  }

  // TODO (borieher): Fill Available BC MRB Configuration (O).
  // TODO (borieher): Fill Criticality Diagnostics (O).
  return true;
}

void bc_bearer_context_modification_procedure::send_bc_bearer_context_modification_response()
{
  e1ap_message e1ap_msg;
  e1ap_msg.pdu.set_successful_outcome();
  e1ap_msg.pdu.successful_outcome().load_info_obj(ASN1_E1AP_ID_BC_BEARER_CONTEXT_MOD);
  auto& bc_bearer_context_modification_response = e1ap_msg.pdu.successful_outcome().value.bc_bearer_context_mod_resp();

  fill_asn1_bc_bearer_context_modification_response(bc_bearer_context_modification_response, response);

  // Send response.
  pdu_notifier.on_new_message(e1ap_msg);
}

void bc_bearer_context_modification_procedure::send_bc_bearer_context_modification_failure()
{
  e1ap_message e1ap_msg;
  e1ap_msg.pdu.set_unsuccessful_outcome();
  e1ap_msg.pdu.unsuccessful_outcome().load_info_obj(ASN1_E1AP_ID_BC_BEARER_CONTEXT_MOD);
  auto& bc_bearer_context_modification_failure = e1ap_msg.pdu.unsuccessful_outcome().value.bc_bearer_context_mod_fail();

  //fill_asn1_bc_bearer_context_modification_failure(bc_bearer_context_modification_failure, failure);
  bc_bearer_context_modification_failure->gnb_cu_cp_mbs_e1ap_id = gnb_cu_cp_mbs_e1ap_id_to_uint(failure.gnb_cu_cp_mbs_e1ap_id);

  // Send failure.
  pdu_notifier.on_new_message(e1ap_msg);
}