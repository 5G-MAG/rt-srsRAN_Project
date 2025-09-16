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

#pragma once

#include "../e1ap_cu_cp_impl.h"
#include "srsran/asn1/e1ap/e1ap.h"
#include "srsran/e1ap/common/e1ap_message.h"
#include "srsran/support/async/async_task.h"

namespace srsran {
namespace srs_cu_cp {

/// \brief This class handles the E1AP BC Bearer Context Modification Procedure, as per TS 37.483 8.6.1.2.
class bc_bearer_context_modification_procedure
{
public:
  bc_bearer_context_modification_procedure(const e1ap_configuration&                   e1ap_cfg_,
                                           const e1ap_message&                         request_,
                                           e1ap_cu_cp_mbs_session_transaction_manager& ev_mng_,
                                           e1ap_message_notifier&                      e1ap_notif_,
                                           srslog::basic_logger&                       logger_);

  void operator()(coro_context<async_task<expected<e1ap_bc_bearer_context_modification_response, e1ap_bc_bearer_context_modification_failure>>>& ctx);

  static const char* name() { return "E1AP CU-CP BC Bearer Context Modification Procedure"; }

private:
  /// Send BC Bearer Context Modification Request to CU-UP.
  void send_bc_bearer_context_modification_request();

  /// Creates procedure result to send back to procedure caller.
  expected<e1ap_bc_bearer_context_modification_response, e1ap_bc_bearer_context_modification_failure>
  handle_bc_bearer_context_modification_outcome();

  e1ap_configuration                            e1ap_cfg;
  e1ap_message                                  request;
  e1ap_cu_cp_mbs_session_transaction_manager&   ev_mng;
  e1ap_message_notifier&                        e1ap_notifier;
  srslog::basic_logger&                         logger;

  protocol_transaction_outcome_observer<asn1::e1ap::bc_bearer_context_mod_resp_s, asn1::e1ap::bc_bearer_context_mod_fail_s>
      transaction_sink;
};

} // namespace srs_cu_cp
} // namespace srsran
