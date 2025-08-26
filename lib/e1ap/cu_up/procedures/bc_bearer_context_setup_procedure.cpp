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

#include "bc_bearer_context_setup_procedure.h"
#include "cu_up/e1ap_cu_up_asn1_helpers.h"

using namespace srsran;
using namespace srsran::srs_cu_up;
using namespace asn1::e1ap;

bc_bearer_context_setup_procedure::bc_bearer_context_setup_procedure(const e1ap_bc_bearer_context_setup_request& request_,
                                                                     e1ap_mbs_session_context&                   mbs_session_ctxt_,
                                                                     e1ap_message_notifier&                      pdu_notifier_,
                                                                     srslog::basic_logger&                       logger_) :
request(request_), mbs_session_ctxt(mbs_session_ctxt_), pdu_notifier(pdu_notifier_), logger(logger_)
{
}

void bc_bearer_context_setup_procedure::operator()(coro_context<async_task<void>>& ctx)
{
  CORO_BEGIN(ctx);

  logger.debug("\"{}\" initialized", name());

  // TODO (borieher): Perform Multicast join here with the E1AP information

  // Process the E1AP BC Bearer Context Setup Request and create the E1AP BC Bearer Context Setup Response
  fill_e1ap_bc_bearer_context_setup_response();

  // Send command to CU-CP.
  send_bc_bearer_context_setup_response();

  CORO_RETURN();
}

bool bc_bearer_context_setup_procedure::fill_e1ap_bc_bearer_context_setup_response()
{
  // Fill gNB-CU-CP MBS E1AP ID (M).
  response.gnb_cu_cp_mbs_e1ap_id = mbs_session_ctxt.mbs_ids.cu_cp_mbs_e1ap_id;

  // Fill gNB-CU-UP MBS E1AP ID (M).
  response.gnb_cu_up_mbs_e1ap_id = mbs_session_ctxt.mbs_ids.cu_up_mbs_e1ap_id;

  // Fill BC Bearer Context To Setup Response (M).
  // NOTE (borieher): BC Bearer Context NG-U TNL Info at NG-RAN (O) not being used.

  // Fill BC MRB Setup Response List <1..maxnoofMRBs>.
  for (const auto& bc_mrb_to_setup_item : request.bc_bearer_context_to_setup.bc_mrb_to_setup_list) {
    e1ap_bc_mrb_setup_response_item bc_mrb_setup_response_item;

    // Fill MRB ID (M).
    bc_mrb_setup_response_item.mrb_id = bc_mrb_to_setup_item.mrb_id;

    // Fill MBS QoS Flow Setup List (M).
    for (const auto& bc_mrb_to_setup_qos_flows_info_item : bc_mrb_to_setup_item.mbs_qos_flow_info_to_be_setup) {
      e1ap_qos_flow_item mbs_qos_flow_setup_item;

      // Fill QoS Flow Identifier (M).
      mbs_qos_flow_setup_item.qos_flow_id = bc_mrb_to_setup_qos_flows_info_item.qos_flow_id;

      bc_mrb_setup_response_item.mbs_qos_flow_setup_list.push_back(mbs_qos_flow_setup_item);
    }

    // TODO (borieher): Fill MBS QoS Flow Failed List (O).

    // NOTE (borieher): Right now filling BC Bearer Context F1-U TNL Info at CU (M) with the last value.
    // Fill BC Bearer Context F1-U TNL Info at CU (M).
    // Fill F1-U TNL Info Added List <0..1>.
    if (!bc_mrb_to_setup_item.f1u_tnl_info_to_add_list.empty()) {
      for (const auto& f1u_tnl_info_to_add_item : bc_mrb_to_setup_item.f1u_tnl_info_to_add_list) {
        e1ap_f1u_tnl_info_added_item f1u_tnl_info_added_item;

        f1u_tnl_info_added_item.bc_f1u_context_reference_e1 = f1u_tnl_info_to_add_item;

        // NOTE (borieher): Extracting the information from the BC Bearer Context NG-U TNL Info at 5GC (O)
        if (request.bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_info_at_5gc.has_value()) {
          // Parse location dependent from request
          if (request.bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_info_at_5gc.value().is_locationdependent()) {
            e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent locationdependent = {};

            for (const auto& locationdependent_item : request.bc_bearer_context_to_setup.bc_bearer_context_ngu_tnl_info_at_5gc.value()
                .get_locationdependent().location_dependent_mbs_ngu_info_at_5gc) {
              e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent_item bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent_item;

              bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent_item.mbs_area_session_id = locationdependent_item.mbs_area_session_id;

              // TODO (borieher): Remove hardcoded values
              up_transport_layer_info mbs_f1u_information_at_cu;
              mbs_f1u_information_at_cu.tp_address = transport_layer_address::create_from_string("1.1.1.1");
              mbs_f1u_information_at_cu.gtp_teid = int_to_gtpu_teid(1111);

              bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent_item.mbs_f1u_information_at_cu = mbs_f1u_information_at_cu;

              locationdependent.location_dependent_mbs_f1u_information_at_cu.push_back(bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent_item);
            }

            f1u_tnl_info_added_item.bc_bearer_context_f1u_tnl_info_at_cu = locationdependent;

          // Parse location independent from request
          } else {
            e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_independent locationindependent = {};

            // TODO (borieher): Remove hardcoded values
            up_transport_layer_info mbs_f1u_information_at_cu;
            mbs_f1u_information_at_cu.tp_address = transport_layer_address::create_from_string("1.1.1.1");
            mbs_f1u_information_at_cu.gtp_teid = int_to_gtpu_teid(1111);

            locationindependent.mbs_f1u_information_at_cu = mbs_f1u_information_at_cu;

            f1u_tnl_info_added_item.bc_bearer_context_f1u_tnl_info_at_cu = locationindependent;
          }
        }

        bc_mrb_setup_response_item.f1u_tnl_info_added_list.push_back(f1u_tnl_info_added_item);

        // NOTE (borieher): Hack to fill the BC Bearer Context F1-U TNL Info at CU (M).
        bc_mrb_setup_response_item.bc_bearer_context_f1u_tnl_info_at_cu = f1u_tnl_info_added_item.bc_bearer_context_f1u_tnl_info_at_cu;
      }
    }

    response.bc_bearer_context_to_setup_response.bc_mrb_setup_response_list.push_back(bc_mrb_setup_response_item);
  }

  // Fill BC MRB Failed List <0..maxnoofMRBs>.
  // TODO (borieher)

  // Fill Available BC MRB Configuration (O).
  // NOTE (borieher): Not being used for now, to simplify.

  // Fill Criticality Diagnostics (O).
  // TODO (borieher)
  return true;
}

void bc_bearer_context_setup_procedure::send_bc_bearer_context_setup_response()
{
  e1ap_message e1ap_msg;
  e1ap_msg.pdu.set_successful_outcome();
  e1ap_msg.pdu.successful_outcome().load_info_obj(ASN1_E1AP_ID_BC_BEARER_CONTEXT_SETUP);
  auto& bc_bearer_context_setup_response = e1ap_msg.pdu.successful_outcome().value.bc_bearer_context_setup_resp();

  fill_asn1_bc_bearer_context_setup_response(bc_bearer_context_setup_response, response);

  // Send response.
  pdu_notifier.on_new_message(e1ap_msg);
}

void bc_bearer_context_setup_procedure::send_bc_bearer_context_setup_failure()
{
  e1ap_message e1ap_msg;
  e1ap_msg.pdu.set_unsuccessful_outcome();
  e1ap_msg.pdu.unsuccessful_outcome().load_info_obj(ASN1_E1AP_ID_BC_BEARER_CONTEXT_SETUP);
  auto& bc_bearer_context_setup_failure = e1ap_msg.pdu.unsuccessful_outcome().value.bc_bearer_context_setup_fail();

  //fill_asn1_bc_bearer_context_setup_failure(bc_bearer_context_setup_failure, failure);
  bc_bearer_context_setup_failure->gnb_cu_cp_mbs_e1ap_id = gnb_cu_cp_mbs_e1ap_id_to_uint(failure.gnb_cu_cp_mbs_e1ap_id);

  // Send failure.
  pdu_notifier.on_new_message(e1ap_msg);
}
