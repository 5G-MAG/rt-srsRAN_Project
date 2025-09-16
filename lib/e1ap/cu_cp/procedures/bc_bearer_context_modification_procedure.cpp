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
#include "../e1ap_cu_cp_asn1_helpers.h"

using namespace srsran;
using namespace srsran::srs_cu_cp;
using namespace asn1::e1ap;

bc_bearer_context_modification_procedure::bc_bearer_context_modification_procedure(const e1ap_configuration&                      e1ap_cfg_,
                                                                                   const e1ap_message&                            request_,
                                                                                   e1ap_cu_cp_mbs_session_transaction_manager&    ev_mng_,
                                                                                   e1ap_message_notifier&                         e1ap_notif_,
                                                                                   srslog::basic_logger&                          logger_) :
e1ap_cfg(e1ap_cfg_), request(request_), ev_mng(ev_mng_), e1ap_notifier(e1ap_notif_), logger(logger_)
{
}

void bc_bearer_context_modification_procedure::operator()(coro_context<async_task<expected<e1ap_bc_bearer_context_modification_response, e1ap_bc_bearer_context_modification_failure>>>& ctx)
{
  CORO_BEGIN(ctx);

  logger.debug("\"{}\" initialized", name());

  // Subscribe to respective publisher to receive BC Bearer Context Modification Response/Failure message.
  transaction_sink.subscribe_to(ev_mng.bc_bearer_context_modification_outcome, e1ap_cfg.proc_timeout);

  // Send command to CU-UP.
  send_bc_bearer_context_modification_request();

  // Await response.
  CORO_AWAIT(transaction_sink);

  // Handle response from CU-UP and return bearer index.
  CORO_RETURN(handle_bc_bearer_context_modification_outcome());
}

void bc_bearer_context_modification_procedure::send_bc_bearer_context_modification_request()
{
  logger.info("Sending BC Bearer Context Modification Request on the CU-CP");

  // Send BC Bearer Context Modification Request message.
  e1ap_notifier.on_new_message(request);
}

expected<e1ap_bc_bearer_context_modification_response, e1ap_bc_bearer_context_modification_failure>
bc_bearer_context_modification_procedure::handle_bc_bearer_context_modification_outcome()
{
  if (transaction_sink.successful()) {
    logger.info("Received BC Bearer Context Modification Response on the CU-CP");
    e1ap_bc_bearer_context_modification_response response;
    const asn1::e1ap::bc_bearer_context_mod_resp_s& asn1_resp = transaction_sink.response();

    fill_e1ap_bc_bearer_context_modification_response(response, asn1_resp);

    logger.debug("\"{}\" finalized", name());

    return response;
  } else if (transaction_sink.failed()) {
    logger.info("Received BC Bearer Context Modification Failure on the CU-CP");
    e1ap_bc_bearer_context_modification_failure failure;
    const asn1::e1ap::bc_bearer_context_mod_fail_s asn1_fail = transaction_sink.failure();

    //fill_e1ap_bc_bearer_context_setup_failure(failure, asn1_fail);
    failure.gnb_cu_cp_mbs_e1ap_id = uint_to_gnb_cu_cp_mbs_e1ap_id(asn1_fail->gnb_cu_cp_mbs_e1ap_id);
    failure.gnb_cu_up_mbs_e1ap_id = uint_to_gnb_cu_up_mbs_e1ap_id(asn1_fail->gnb_cu_up_mbs_e1ap_id);

    logger.debug("\"{}\" finalized", name());

    return make_unexpected(failure);
  } else {
    if (transaction_sink.timeout_expired()) {
      logger.warning("E1AP BC Bearer Context Modification Response timeout");
    } else {
      logger.warning("E1AP BC Bearer Context Modification Response cancelled");
    }

    e1ap_bc_bearer_context_modification_failure failure;
    request.pdu.init_msg().load_info_obj(ASN1_E1AP_ID_BC_BEARER_CONTEXT_MOD);
    auto& bc_bearer_context_modification_request = request.pdu.init_msg().value.bc_bearer_context_mod_request();
    failure.gnb_cu_cp_mbs_e1ap_id = uint_to_gnb_cu_cp_mbs_e1ap_id(bc_bearer_context_modification_request->gnb_cu_cp_mbs_e1ap_id);

    logger.error("\"{}\" failed", name());
    return make_unexpected(failure);
  }
}
