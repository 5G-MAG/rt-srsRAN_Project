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

#include "srsran/cu_up/cu_up_types.h"
#include "srsran/e1ap/common/e1ap_types.h"
#include "srsran/ran/mbs.h"

namespace srsran {
namespace srs_cu_up {

struct e1ap_bc_bearer_context_modification_request {
  mbs_index_t mbs_index = mbs_index_t::invalid;
  // gnb_cu_cp_mbs_e1ap_id (M)
  gnb_cu_cp_mbs_e1ap_id_t gnb_cu_cp_mbs_e1ap_id;
  // gnb_cu_up_mbs_e1ap_id (M)
  gnb_cu_up_mbs_e1ap_id_t gnb_cu_up_mbs_e1ap_id;
  // bc_bearer_context_to_modify (M)
  e1ap_bc_bearer_context_to_modify bc_bearer_context_to_modify;
};

struct e1ap_bc_bearer_context_modification_response {
  // gnb_cu_cp_mbs_e1ap_id (M)
  gnb_cu_cp_mbs_e1ap_id_t gnb_cu_cp_mbs_e1ap_id;
  // gnb_cu_up_mbs_e1ap_id (M)
  gnb_cu_up_mbs_e1ap_id_t gnb_cu_up_mbs_e1ap_id;
  // bc_bearer_context_to_modify_response (M)
  e1ap_bc_bearer_context_to_modify_response bc_bearer_context_to_modify_response;
  // criticality_diagnostics (O)
  std::optional<e1ap_crit_diagnostics> crit_diagnostics;
};

struct e1ap_bc_bearer_context_modification_failure {
  // gnb_cu_cp_mbs_e1ap_id (M)
  gnb_cu_cp_mbs_e1ap_id_t gnb_cu_cp_mbs_e1ap_id;
  // gnb_cu_up_mbs_e1ap_id (M)
  gnb_cu_up_mbs_e1ap_id_t gnb_cu_up_mbs_e1ap_id;
  // cause (M)
  e1ap_cause_t cause;
  // criticality_diagnostics (O)
  std::optional<e1ap_crit_diagnostics> crit_diagnostics;
};

// QoS Flow modification result
struct mbs_qos_flow_modification_result {
  bool          success     = false;
  qos_flow_id_t qos_flow_id = qos_flow_id_t::invalid;
  e1ap_cause_t  cause; // Cause if setup was unsuccessful.
};

// MRB modification result, including associated QoS flows
struct mrb_modification_result {
  bool                                          success = false;
  mrb_id_t                                      mrb_id  = mrb_id_t::invalid;
  e1ap_cause_t                                  cause; // Cause if setup was unsuccessful.
  up_transport_layer_info                       gtp_tunnel;
  std::vector<mbs_qos_flow_modification_result> mbs_qos_flow_results;
};

// Broadcast MBS Session modification result, including mbs_index, associated MRBs and their QoS flows
struct mbs_broadcast_session_modification_result {
  mbs_index_t                          mbs_index = mbs_index_t::invalid; // Index of the modified MBS Session context.
  bool                                 success   = false;                // True if Broadcast MBS session could be modified.
  e1ap_cause_t                         cause;
  up_transport_layer_info              gtp_tunnel;
  std::vector<mrb_modification_result> mrb_modification_results;
};

} // namespace srs_cu_up
} // namespace srsran
