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

#include "srsran/e1ap/cu_up/e1ap_cu_up.h"
#include "srsran/asn1/e1ap/e1ap.h"
#include "srsran/e1ap/common/e1ap_message.h"
#include "srsran/e1ap/common/e1ap_common.h"

namespace srsran {
namespace srs_cu_up {

/// \brief This class handles the E1AP BC Bearer Context Modification Procedure, as per TS 37.483 8.6.1.2.
class bc_bearer_context_modification_procedure
{
  public:
  bc_bearer_context_modification_procedure(const e1ap_bc_bearer_context_modification_request& request_,
                                           e1ap_message_notifier&                             pdu_notifier_,
                                           mbs_broadcast_session_modification_result&         broadcast_session_modification_result_,
                                           srslog::basic_logger&                              logger_);

  void operator()(coro_context<async_task<void>>& ctx);

  static const char* name() { return "E1AP CU-UP BC Bearer Context Modification Procedure"; }

private:
  bool fill_e1ap_bc_bearer_context_modification_response();

  /// Send BC Bearer Context Modification Response to CU-CP.
  void send_bc_bearer_context_modification_response();

  /// Send BC Bearer Context Modification Failure to CU-CP.
  void send_bc_bearer_context_modification_failure();

  e1ap_bc_bearer_context_modification_request          request;
  e1ap_bc_bearer_context_modification_response         response;
  e1ap_bc_bearer_context_modification_failure          failure;
  e1ap_message_notifier&                               pdu_notifier;
  mbs_broadcast_session_modification_result&           broadcast_session_modification_result;
  srslog::basic_logger&                                logger;
};

} // namespace srs_cu_up
} // namespace srsran
