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

using namespace srsran;
using namespace srsran::srs_cu_cp;

broadcast_session_setup_routine::broadcast_session_setup_routine(
    const ngap_broadcast_session_setup_request& request_,
    e1ap_mbs_session_context_manager&           e1ap_mbs_session_ctxt_mng_,
    srslog::basic_logger&                       logger_) :
    request(request_),
    e1ap_mbs_session_ctxt_mng(e1ap_mbs_session_ctxt_mng_),
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
    // TODO (borieher): Send E1AP BC Bearer Context Setup Request to each CU-UP involved

    // Handle E1AP BC Bearer Context Setup Response/Failure
  }

  // Prepare F1AP Broadcast Context Setup Request and call F1AP notifier.
  {
    // TODO (borieher): Send F1AP Broadcast Context Setup Request to each DU involved

    // Handle F1AP Broadcast Context Setup Response/Failure
  }

  // Prepare E1AP BC Bearer Context Modification Request and call E1AP notifier.
  {
    // TODO (borieher): Send E1AP BC Bearer Context Modification Request to each CU-UP involved

    // Handle E1AP BC Bearer Context Modification Response/Failure
  }

  logger.debug("\"{}\" setup broadcast bearer at CU-CP finished", name());
  CORO_RETURN(resp_msg);
}
