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
#include "../e1ap_cu_cp_asn1_helpers.h"

using namespace srsran;
using namespace srsran::srs_cu_cp;
using namespace asn1::e1ap;

bc_bearer_context_setup_procedure::bc_bearer_context_setup_procedure(const e1ap_configuration&                      e1ap_cfg_,
                                                                     const e1ap_message&                            request_,
                                                                     e1ap_cu_cp_mbs_session_transaction_manager&    ev_mng_,
                                                                     e1ap_mbs_session_context_list&                 mbs_session_ctxt_list_,
                                                                     e1ap_message_notifier&                         e1ap_notif_,
                                                                     srslog::basic_logger&                          logger_) :
e1ap_cfg(e1ap_cfg_), request(request_), ev_mng(ev_mng_), mbs_session_ctxt_list(mbs_session_ctxt_list_), e1ap_notifier(e1ap_notif_), logger(logger_)
{
}

void bc_bearer_context_setup_procedure::operator()(coro_context<async_task<expected<e1ap_bc_bearer_context_setup_response, e1ap_bc_bearer_context_setup_failure>>>& ctx)
{
  CORO_BEGIN(ctx);

  logger.debug("\"{}\" initialized", name());

  // Send command to CU-UP.
  send_bc_bearer_context_setup_request();

  // Handle response from CU-UP and return bearer index.
  CORO_RETURN(handle_bc_bearer_context_setup_outcome());
}

void bc_bearer_context_setup_procedure::send_bc_bearer_context_setup_request()
{
  logger.info("Sending BC Bearer Context Setup Request on the CU-CP");

  // Send BC Bearer Context Setup Request message.
  e1ap_notifier.on_new_message(request);
}

expected<e1ap_bc_bearer_context_setup_response, e1ap_bc_bearer_context_setup_failure>
bc_bearer_context_setup_procedure::handle_bc_bearer_context_setup_outcome()
{
  e1ap_bc_bearer_context_setup_response response;
  return response;
}
