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

#include "../f1ap_cu_impl.h"
#include "srsran/f1ap/cu_cp/f1ap_configuration.h"
#include "srsran/f1ap/cu_cp/f1ap_cu.h"
#include "srsran/asn1/f1ap/f1ap.h"
#include "srsran/asn1/f1ap/common.h"

namespace srsran {
namespace srs_cu_cp {

  /// \brief This class handles the F1AP Broadcast Context Setup Procedure, as per TS 38.473 8.14.1.
class broadcast_context_setup_procedure
{
public:
  broadcast_context_setup_procedure(const f1ap_configuration&                   f1ap_cfg,
                                    const f1ap_broadcast_context_setup_request& request_,
                                    f1ap_mbs_session_context_list&              mbs_session_ctxt_list_,
                                    f1ap_message_notifier&                      f1ap_notif_,
                                    srslog::basic_logger&                       logger_);

  void operator()(coro_context<async_task<expected<f1ap_broadcast_context_setup_response, f1ap_broadcast_context_setup_failure>>>& ctx);

  static const char* name() { return "Broadcast Context Setup Procedure"; }

private:
  bool create_f1ap_mbs_session_context();

  /// Send F1AP Broadcast Context Setup Request to DU.
  void send_broadcast_context_setup_request();

  /// Creates procedure result to send back to procedure caller.
  expected<f1ap_broadcast_context_setup_response, f1ap_broadcast_context_setup_failure>
  handle_broadcast_context_setup_outcome();

  const f1ap_configuration&                  f1ap_cfg;
  f1ap_broadcast_context_setup_request       request;
  f1ap_mbs_session_context_list&             mbs_session_ctxt_list;
  f1ap_message_notifier&                     f1ap_notifier;
  srslog::basic_logger&                      logger;

  // Context of the created MBS Session.
  f1ap_mbs_session_context* mbs_session_ctxt = nullptr;

  protocol_transaction_outcome_observer<asn1::f1ap::broadcast_context_setup_resp_s, asn1::f1ap::broadcast_context_setup_fail_s>
      transaction_sink;
};

} // namespace srs_cu_cp
} // namespace srsran
