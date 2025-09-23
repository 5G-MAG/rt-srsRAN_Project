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

#include "broadcast_context_setup_procedure.h"
#include "srsran/f1ap/f1ap_message.h"
#include "../../../f1ap/asn1_helpers.h"

using namespace srsran;
using namespace srsran::srs_cu_cp;
using namespace asn1::f1ap;

/// \brief Convert the Broadcast Context Setup Request from common type to ASN.1.
/// \param[out] asn1_request The ASN.1 Broadcast Context Setup Request.
/// \param[in] request The common type Broadcast Context Setup Request.
static void fill_asn1_broadcast_context_setup_request(broadcast_context_setup_request_s&      asn1_request,
                                                      f1ap_broadcast_context_setup_request&   request);

/// \brief Convert the Broadcast Context Setup Response from ASN.1 to common type.
/// \param[out] response  The common type Broadcast Context Setup Response.
/// \param[in] asn1_response The ASN.1 Broadcast Context Setup Response.
static void fill_f1ap_broadcast_context_setup_response(f1ap_broadcast_context_setup_response& response,
                                                       const broadcast_context_setup_resp_s&  asn1_response);

/// \brief Convert the Broadcast Context Setup Failure from ASN.1 to common type.
/// \param[in] response The common type Broadcast Context Setup Failure.
/// \param[in] mbs_index MBS index.
/// \param[in] asn1_failure The ASN.1 Broadcast Context Setup Failure.
static void fill_f1ap_broadcast_context_setup_failure(f1ap_broadcast_context_setup_failure& response,
                                                      const broadcast_context_setup_fail_s& asn1_failure);

// ---- Broadcast Context Setup Procedure ----

broadcast_context_setup_procedure::broadcast_context_setup_procedure(const f1ap_configuration&                   f1ap_cfg_,
                                                                     const f1ap_broadcast_context_setup_request& request_,
                                                                     f1ap_mbs_session_context_list&              mbs_session_ctxt_list_,
                                                                     f1ap_message_notifier&                      f1ap_notif_,
                                                                     srslog::basic_logger&                       logger_) :
  f1ap_cfg(f1ap_cfg_),
  request(request_),
  mbs_session_ctxt_list(mbs_session_ctxt_list_),
  f1ap_notifier(f1ap_notif_),
  logger(logger_)
{
  srsran_assert(request.mbs_index != mbs_index_t::invalid, "MBS index of F1AP Broadcast Context Setup Request must not be invalid");
}

void broadcast_context_setup_procedure::operator()(coro_context<async_task<expected<f1ap_broadcast_context_setup_response, f1ap_broadcast_context_setup_failure>>>& ctx)
{
  CORO_BEGIN(ctx);

  logger.debug("\"{}\" started...", name());

  // Create F1AP MBS Session context if it doesn't exist.
  if (not create_f1ap_mbs_session_context()) {
    CORO_EARLY_RETURN(handle_broadcast_context_setup_outcome());
  }

  // Subscribe to respective publisher to receive BROADCAST CONTEXT SETUP RESPONSE/FAILURE message.
  transaction_sink.subscribe_to(mbs_session_ctxt->ev_mng.broadcast_context_setup_outcome, f1ap_cfg.proc_timeout);

  // Send command to DU.
  send_broadcast_context_setup_request();

  // Await DU response.
  CORO_AWAIT(transaction_sink);

  // Handle result of the transaction.
  CORO_RETURN(handle_broadcast_context_setup_outcome());
}

bool broadcast_context_setup_procedure::create_f1ap_mbs_session_context()
{
  // Allocate gNB-CU MBS F1AP ID.
  gnb_cu_mbs_f1ap_id_t gnb_cu_mbs_f1ap_id = mbs_session_ctxt_list.allocate_gnb_cu_mbs_f1ap_id();
  if (gnb_cu_mbs_f1ap_id == gnb_cu_mbs_f1ap_id_t::invalid) {
    logger.warning("No gNB-CU MBS F1AP ID available");
    return false;
  }

  // Add new F1AP MBS Session context.
  if (mbs_session_ctxt_list.add_mbs_session_context(request.mbs_index, gnb_cu_mbs_f1ap_id) == nullptr) {
    logger.warning("F1AP Broadcast Context Setup failed. Cause: F1AP MBS Session context already exists");
    return false;
  }

  mbs_session_ctxt = &mbs_session_ctxt_list[gnb_cu_mbs_f1ap_id];

  return true;
}

void broadcast_context_setup_procedure::send_broadcast_context_setup_request()
{
  request.gnb_cu_mbs_f1ap_id = mbs_session_ctxt->mbs_ids.cu_mbs_f1ap_id;

  f1ap_message f1ap_msg;
  f1ap_msg.pdu.set_init_msg();
  f1ap_msg.pdu.init_msg().load_info_obj(ASN1_F1AP_ID_BROADCAST_CONTEXT_SETUP);
  auto& broadcast_context_setup_request = f1ap_msg.pdu.init_msg().value.broadcast_context_setup_request();

  // Convert common type to asn1
  fill_asn1_broadcast_context_setup_request(broadcast_context_setup_request, request);

  logger.info("Sending Broadcast Context Setup Request from the CU");

  // Send Broadcast Context Setup Request message
  f1ap_notifier.on_new_message(f1ap_msg);
}

expected<f1ap_broadcast_context_setup_response, f1ap_broadcast_context_setup_failure>
    broadcast_context_setup_procedure::handle_broadcast_context_setup_outcome()
{
  if (transaction_sink.successful()) {
    logger.info("Received Broadcast Context Setup Response on the CU");
    f1ap_broadcast_context_setup_response response;
    const asn1::f1ap::broadcast_context_setup_resp_s& asn1_resp = transaction_sink.response();

    fill_f1ap_broadcast_context_setup_response(response, asn1_resp);

    logger.debug("\"{}\" finalized", name());

    return response;
  } else {
    logger.info("Received Broadcast Context Setup Failure on the CU");
    f1ap_broadcast_context_setup_failure failure;
    const asn1::f1ap::broadcast_context_setup_fail_s& asn1_fail = transaction_sink.failure();

    fill_f1ap_broadcast_context_setup_failure(failure, asn1_fail);
    failure.gnb_cu_mbs_f1ap_id = mbs_session_ctxt->mbs_ids.cu_mbs_f1ap_id;

    logger.error("\"{}\" failed", name());

    return make_unexpected(failure);
  }
}

static void fill_asn1_broadcast_context_setup_request(broadcast_context_setup_request_s&          asn1_request,
                                                      f1ap_broadcast_context_setup_request&       request)
{
  // Fill gNB-CU MBS F1AP ID.
  asn1_request->gnb_cu_mbs_f1ap_id = gnb_cu_mbs_f1ap_id_to_uint(request.gnb_cu_mbs_f1ap_id);

  // Fill MBS Session ID.
  // Fill TMGI.
  asn1_request->mbs_session_id.tmgi.from_number(tmgi_to_uint(request.mbs_session_id.tmgi));

  // Fill NID.
  if (request.mbs_session_id.nid.has_value()) {
    asn1_request->mbs_session_id.nid_present = true;
    asn1_request->mbs_session_id.nid.from_number(nid_to_uint(request.mbs_session_id.nid.value()));
  }

  // Fill MBS Service Area.
  if (request.mbs_service_area.has_value()) {
    asn1_request->mbs_service_area_present = true;

    // Fill location dependent
    if (request.mbs_service_area.value().is_locationdependent()) {
      asn1_request->mbs_service_area.set_locationdependent();

      // Fill MBS Service Area Information Location Dependent List.
      for(const auto& locationdependent_item : request.mbs_service_area.value().get_locationdependent().mbs_service_area_information_list) {
        // Fill MBS Area Session ID.
        mbs_service_area_info_item_s asn1_mbs_service_area_info_item;
        asn1_mbs_service_area_info_item.mbs_area_session_id = area_session_id_to_uint(locationdependent_item.mbs_area_session_id);

        // Fill MBS Service Area Cell List.
        if (!locationdependent_item.mbs_service_area_information.mbs_service_area_cell_list.empty()) {
          for (const auto& nr_cgi_item: locationdependent_item.mbs_service_area_information.mbs_service_area_cell_list) {
            nr_cgi_s asn1_nr_cgi_item;

            asn1_nr_cgi_item.plmn_id.from_number(nr_cgi_item.plmn_id.to_bcd());
            asn1_nr_cgi_item.nr_cell_id.from_number(nr_cgi_item.nci.value());

            asn1_mbs_service_area_info_item.mbs_service_area_info.mbs_service_area_cell_list.push_back(asn1_nr_cgi_item);
          }
        }

        // Fill MBS Service Area TAI List.
        if (!locationdependent_item.mbs_service_area_information.mbs_service_area_tai_list.empty()) {
          for (const auto& tai_item: locationdependent_item.mbs_service_area_information.mbs_service_area_tai_list) {
            mbs_service_area_tai_list_item_s asn1_tai_item;

            asn1_tai_item.plmn_id.from_number(tai_item.plmn_id.to_bcd());
            asn1_tai_item.five5_tac.from_number(tai_item.tac);

            asn1_mbs_service_area_info_item.mbs_service_area_info.mbs_service_area_tai_list.push_back(asn1_tai_item);
          }
        }
        asn1_request->mbs_service_area.locationdependent().push_back(asn1_mbs_service_area_info_item);
      }
    // Fill location independent
    } else {
      asn1_request->mbs_service_area.set_locationindependent();

      // Fill MBS Service Area Cell List.
      if (!request.mbs_service_area.value().get_locationindependent().mbs_service_area_information.mbs_service_area_cell_list.empty()) {
        for (const auto& nr_cgi_item : request.mbs_service_area.value().get_locationindependent().mbs_service_area_information.mbs_service_area_cell_list) {
          nr_cgi_s asn1_nr_cgi_item;

          asn1_nr_cgi_item.plmn_id.from_number(nr_cgi_item.plmn_id.to_bcd());
          asn1_nr_cgi_item.nr_cell_id.from_number(nr_cgi_item.nci.value());

          asn1_request->mbs_service_area.locationindependent().mbs_service_area_cell_list.push_back(asn1_nr_cgi_item);
        }
      }

      // Fill MBS Service Area TAI List.
      if (!request.mbs_service_area.value().get_locationindependent().mbs_service_area_information.mbs_service_area_tai_list.empty()) {
        for (const auto& tai_item : request.mbs_service_area.value().get_locationindependent().mbs_service_area_information.mbs_service_area_tai_list) {
          mbs_service_area_tai_list_item_s asn1_tai_item;

          asn1_tai_item.plmn_id.from_number(tai_item.plmn_id.to_bcd());
          asn1_tai_item.five5_tac.from_number(tai_item.tac);

          asn1_request->mbs_service_area.locationindependent().mbs_service_area_tai_list.push_back(asn1_tai_item);
        }
      }
    }
  }

  // Fill MBS CU to DU RRC Information.
  // Fill MBS Broadcast Cell List.
  for (const auto& mbs_broadcast_cell_item : request.mbs_cu_to_du_rrc_information.mbs_broadcast_cell_list) {
    mbs_broadcast_cell_item_s asn1_mbs_broadcast_cell_item;

    asn1_mbs_broadcast_cell_item.nr_cgi.plmn_id.from_number(mbs_broadcast_cell_item.nr_cgi.plmn_id.to_bcd());
    asn1_mbs_broadcast_cell_item.nr_cgi.nr_cell_id.from_number(mbs_broadcast_cell_item.nr_cgi.nci.value());

    if (mbs_broadcast_cell_item.mtch_neighbour_cell.has_value()) {
      if (!mbs_broadcast_cell_item.mtch_neighbour_cell.value().empty()) {
        asn1_mbs_broadcast_cell_item.mtch_neighbour_cell = mbs_broadcast_cell_item.mtch_neighbour_cell.value().copy();
      }
    }

    asn1_request->mbs_cu_to_du_rrc_info.mbs_broadcast_cell_list.push_back(asn1_mbs_broadcast_cell_item);
  }

  // Fill MBS Broadcast MRB List.
  for (const auto& mbs_broadcast_mrb_item : request.mbs_cu_to_du_rrc_information.mbs_broadcast_mrb_list) {
    mbs_broadcast_mrb_item_s asn1_mbs_broadcast_mrb_item;

    asn1_mbs_broadcast_mrb_item.mrb_id = mrb_id_to_uint(mbs_broadcast_mrb_item.mrb_id);

    if (!mbs_broadcast_mrb_item.mrb_pdcp_config_broadcast.empty()) {
      asn1_mbs_broadcast_mrb_item.mrb_pdcp_cfg_broadcast = mbs_broadcast_mrb_item.mrb_pdcp_config_broadcast.copy();
    }

    asn1_request->mbs_cu_to_du_rrc_info.mbs_broadcast_mrb_list.push_back(asn1_mbs_broadcast_mrb_item);
  }

  // Fill S-NSSAI.
  // Fill SST.
  asn1_request->snssai.sst.from_number(request.s_nssai.sst.value());

  // Fill SSD.
  if (request.s_nssai.sd.is_set()) {
    asn1_request->snssai.sd_present = true;
    asn1_request->snssai.sd.from_number(request.s_nssai.sd.value());
  }

  // Fill Broadcast MRB To Be Setup List.
  asn1_request->broadcast_m_rbs_to_be_setup_list = make_broadcast_mrb_to_be_setup_list(request.broadcast_mrb_to_be_setup_list);
}

static void fill_f1ap_broadcast_context_setup_response(f1ap_broadcast_context_setup_response& response,
                                                       const broadcast_context_setup_resp_s&  asn1_response)
{
  // Fill gNB-CU MBS F1AP ID (M).
  response.gnb_cu_mbs_f1ap_id = uint_to_gnb_cu_mbs_f1ap_id(asn1_response->gnb_cu_mbs_f1ap_id);

  // Fill gNB-DU MBS F1AP ID (M).
  response.gnb_du_mbs_f1ap_id = uint_to_gnb_du_mbs_f1ap_id(asn1_response->gnb_du_mbs_f1ap_id);

  // Fill Broadcast MRB Setup List (1).
  for (const auto& asn1_mrb_setup_item_ies : asn1_response->broadcast_m_rbs_setup_list) {
    auto& asn1_mrb_setup_item = asn1_mrb_setup_item_ies.value().broadcast_m_rbs_setup_item();

    response.broadcast_mrb_setup_list.push_back(make_mrb_setup(asn1_mrb_setup_item));
  }

  // Fill Broadcast MRB Failed To Be Setup List (0..1).
  if (asn1_response->broadcast_m_rbs_failed_to_be_setup_list_present) {
    for (const auto& asn1_mrb_failed_to_be_setup_item_ies : asn1_response->broadcast_m_rbs_failed_to_be_setup_list) {
      auto& asn1_mrb_failed_to_be_setup_item = asn1_mrb_failed_to_be_setup_item_ies.value().broadcast_m_rbs_failed_to_be_setup_item();
      f1ap_broadcast_mrb_failed_to_be_setup_item mrb_failed_to_be_setup_item;

      // Fill MRB ID (M).
      mrb_failed_to_be_setup_item.mrb_id = uint_to_mrb_id(asn1_mrb_failed_to_be_setup_item.mrb_id);

      // Fill Cause (O).
      mrb_failed_to_be_setup_item.cause = asn1_to_cause(asn1_mrb_failed_to_be_setup_item.cause);
    }
  }

  // TODO (borieher): Fill Broadcast Area Scope (O).
  // TODO (borieher): Fill Criticality Diagnostics (O).
}

static void fill_f1ap_broadcast_context_setup_failure(f1ap_broadcast_context_setup_failure& response,
                                                      const broadcast_context_setup_fail_s& asn1_failure)
{
  return;
}
