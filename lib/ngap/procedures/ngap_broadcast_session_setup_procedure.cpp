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

#include "ngap_broadcast_session_setup_procedure.h"
#include "../ngap_asn1_helpers.h"
#include "srsran/asn1/ngap/common.h"
#include "srsran/ngap/ngap_message.h"
#include "srsran/support/async/coroutine.h"

using namespace srsran;
using namespace srsran::srs_cu_cp;
using namespace asn1::ngap;

ngap_broadcast_session_setup_procedure::ngap_broadcast_session_setup_procedure(
    const ngap_broadcast_session_setup_request& request_,
    ngap_cu_cp_notifier&                   cu_cp_notifier_,
    ngap_message_notifier&                 amf_notifier_,
    srslog::basic_logger&                  logger_) :
  request(request_), cu_cp_notifier(cu_cp_notifier_), amf_notifier(amf_notifier_), logger(logger_)
{
}

void ngap_broadcast_session_setup_procedure::operator()(coro_context<async_task<void>>& ctx)
{
  CORO_BEGIN(ctx);

  logger.debug("\"{}\" started...", name());

  // Start the routine in the CU-CP
  CORO_AWAIT_VALUE(broadcast_session_setup_routine_outcome, cu_cp_notifier.on_broadcast_session_setup_request(request));

  if (not broadcast_session_setup_routine_outcome.has_value()) {
    send_broadcast_session_setup_failure(broadcast_session_setup_routine_outcome.error());
    logger.error("\"{}\" failed", name());
  } else {
    // NOTE (borieher): Fill the TMGI in the response, should be done on the broadcast_session_setup_routine_outcome
    broadcast_session_setup_routine_outcome.value().mbs_session_id.tmgi = request.mbs_session_id.tmgi;

    send_broadcast_session_setup_response(broadcast_session_setup_routine_outcome.value());
    logger.debug("\"{}\" finished successfully", name());
  }

  CORO_RETURN();
}

void ngap_broadcast_session_setup_procedure::send_broadcast_session_setup_response(
    const ngap_broadcast_session_setup_response& msg)
{
  ngap_message ngap_msg = {};

  ngap_msg.pdu.set_successful_outcome();
  ngap_msg.pdu.successful_outcome().load_info_obj(ASN1_NGAP_ID_BROADCAST_SESSION_SETUP);
  auto& broadcast_session_setup_response = ngap_msg.pdu.successful_outcome().value.broadcast_session_setup_resp();

  fill_asn1_broadcast_session_setup_response(broadcast_session_setup_response, msg);

  // Forward message to the AMF
  if (!amf_notifier.on_new_message(ngap_msg)) {
    logger.warning("AMF notifier is not set. Cannot send Broadcast Session Setup Response");
    return;
  }
}

void ngap_broadcast_session_setup_procedure::send_broadcast_session_setup_failure(
    const ngap_broadcast_session_setup_failure& msg)
{
  ngap_message ngap_msg = {};

  ngap_msg.pdu.set_unsuccessful_outcome();
  ngap_msg.pdu.unsuccessful_outcome().load_info_obj(ASN1_NGAP_ID_BROADCAST_SESSION_SETUP);
  auto& broadcast_session_setup_failure = ngap_msg.pdu.unsuccessful_outcome().value.broadcast_session_setup_fail();

  fill_asn1_broadcast_session_setup_failure(broadcast_session_setup_failure, msg);

  // Forward message to the AMF
  if (!amf_notifier.on_new_message(ngap_msg)) {
    logger.warning("AMF notifier is not set. Cannot send Broadcast Session Setup Failure");
    return;
  }
}