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

#include "srsran/ngap/ngap.h"
#include "srsran/ngap/ngap_broadcast_session_setup.h"
#include "srsran/support/async/async_task.h"

namespace srsran {
namespace srs_cu_cp {

class ngap_broadcast_session_setup_procedure
{
public:
  ngap_broadcast_session_setup_procedure(const ngap_broadcast_session_setup_request&  request_,
                                        ngap_cu_cp_notifier&                          cu_cp_notifier_,
                                        ngap_message_notifier&                        amf_notifier_,
                                        srslog::basic_logger&                         logger_);


  void operator()(coro_context<async_task<void>>& ctx);

  static const char* name() { return "Broadcast Session Setup Procedure"; }

private:
  // results senders
  void send_broadcast_session_setup_response(const ngap_broadcast_session_setup_response& msg);
  void send_broadcast_session_setup_failure(const ngap_broadcast_session_setup_failure& msg);

  ngap_broadcast_session_setup_request  request;
  ngap_cu_cp_notifier&                  cu_cp_notifier;
  ngap_message_notifier&                amf_notifier;
  srslog::basic_logger&                 logger;

  // routine result
  expected<ngap_broadcast_session_setup_response, ngap_broadcast_session_setup_failure> broadcast_session_setup_routine_outcome;
};

} // namespace srs_cu_cp
} // namespace srsran
