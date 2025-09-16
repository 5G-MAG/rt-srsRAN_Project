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

#include "srsran/ngap/ngap_broadcast_session_setup.h"
#include "srsran/e1ap/cu_cp/e1ap_cu_cp.h"
#include "srsran/f1ap/cu_cp/f1ap_cu.h"
#include "srsran/support/async/async_task.h"
#include "srsran/adt/expected.h"
#include "../mbs_session_manager/mbs_session_manager_impl.h"

namespace srsran {
namespace srs_cu_cp {

/// \brief Handles the setup of an MBS Broadcast Session as in 3GPP TS 38.401 section 8.15.1.1.
class broadcast_session_setup_routine
{
public:
  broadcast_session_setup_routine(const ngap_broadcast_session_setup_request& request_,
                                  mbs_session_manager&                        mbs_session_mng_,
                                  e1ap_mbs_session_context_manager&           e1ap_mbs_session_ctxt_mng_,
                                  f1ap_mbs_session_context_manager&           f1ap_mbs_session_ctxt_mng_,
                                  srslog::basic_logger&                       logger_);

  void operator()(
    coro_context<async_task<expected<ngap_broadcast_session_setup_response, ngap_broadcast_session_setup_failure>>>& ctx);

  static const char* name() { return "Broadcast Session Setup Routine"; }

private:
  ngap_broadcast_session_setup_request request;

  mbs_session_manager&                 mbs_session_mng;
  e1ap_mbs_session_context_manager&    e1ap_mbs_session_ctxt_mng;
  f1ap_mbs_session_context_manager&    f1ap_mbs_session_ctxt_mng;
  srslog::basic_logger&                logger;

  bool fill_e1ap_bc_bearer_context_setup_request(e1ap_bc_bearer_context_setup_request& e1ap_request);

  ngap_broadcast_session_setup_response
  handle_bc_bearer_context_setup_response(const e1ap_bc_bearer_context_setup_response& msg);

  ngap_broadcast_session_setup_failure
  handle_bc_bearer_context_setup_failure(const e1ap_bc_bearer_context_setup_failure& msg);

  bool fill_f1ap_broadcast_context_setup_request(f1ap_broadcast_context_setup_request& f1ap_request);

  ngap_broadcast_session_setup_response
  handle_broadcast_context_setup_response(const f1ap_broadcast_context_setup_response& msg);

  ngap_broadcast_session_setup_failure
  handle_broadcast_context_setup_failure(const f1ap_broadcast_context_setup_failure& msg);

  bool fill_e1ap_bc_bearer_context_modification_request(e1ap_bc_bearer_context_modification_request& e1ap_request);

  ngap_broadcast_session_setup_response
  handle_bc_bearer_context_modification_response(const e1ap_bc_bearer_context_modification_response& msg);

  ngap_broadcast_session_setup_failure
  handle_bc_bearer_context_modification_failure(const e1ap_bc_bearer_context_modification_failure& msg);

  // RRC helpers
  byte_buffer get_packed_mtch_neighbour_cell_r17_ie();
  byte_buffer get_packed_mrb_pdcp_config_broadcast_r17_ie();

  // (sub-)routine requests
  e1ap_bc_bearer_context_setup_request        bc_bearer_context_setup_request;
  f1ap_broadcast_context_setup_request        broadcast_context_setup_request;
  e1ap_bc_bearer_context_modification_request bc_bearer_context_modification_request;

  // (sub-)routine results
  expected<e1ap_bc_bearer_context_setup_response, e1ap_bc_bearer_context_setup_failure>               bc_bearer_context_setup_procedure_outcome;
  expected<f1ap_broadcast_context_setup_response, f1ap_broadcast_context_setup_failure>               broadcast_context_setup_procedure_outcome;
  expected<e1ap_bc_bearer_context_modification_response, e1ap_bc_bearer_context_modification_failure> bc_bearer_context_modification_procedure_outcome;

  // final routine result
  ngap_broadcast_session_setup_response resp_msg;
  ngap_broadcast_session_setup_failure fail_msg;
};

} // namespace srs_cu_cp
} // namespace srsran
