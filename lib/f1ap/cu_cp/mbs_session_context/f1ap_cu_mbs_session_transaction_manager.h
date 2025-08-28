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

#include "srsran/adt/expected.h"
#include "srsran/asn1/f1ap/f1ap.h"
#include "srsran/asn1/f1ap/f1ap_pdu_contents.h"
#include "srsran/asn1/f1ap/f1ap_pdu_contents_ue.h"
#include "srsran/support/async/event_signal.h"
#include "srsran/support/async/protocol_transaction_manager.h"

namespace srsran {
namespace srs_cu_cp {

class f1ap_mbs_session_transaction_manager
{
public:
  f1ap_mbs_session_transaction_manager(timer_factory timers) :
    broadcast_context_setup_outcome(timers)
  {
  }

  void cancel_all()
  {
    broadcast_context_setup_outcome.stop();
  }

  /// F1AP Broadcast Context Setup Response/Failure Event Source.
  protocol_transaction_event_source<asn1::f1ap::broadcast_context_setup_resp_s, asn1::f1ap::broadcast_context_setup_fail_s>
      broadcast_context_setup_outcome;
};

} // namespace srs_cu_cp
} // namespace srsran
