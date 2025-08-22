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

struct e1ap_bc_bearer_context_setup_request {
  mbs_index_t mbs_index = mbs_index_t::invalid;
  //gnb_cu_cp_mbs_e1ap_id (M)
  gnb_cu_cp_mbs_e1ap_id_t   gnb_cu_cp_mbs_e1ap_id;
  //global_mbs_session_id (M)
  mbs_session_id_t          global_mbs_session_id;
  //bc_bearer_context_to_setup (M)
  e1ap_bc_bearer_context_to_setup bc_bearer_context_to_setup;
};

struct e1ap_bc_bearer_context_setup_response {
  //gnb_cu_cp_mbs_e1ap_id (M)
  gnb_cu_cp_mbs_e1ap_id_t gnb_cu_cp_mbs_e1ap_id;
  //gnb_cu_up_mbs_e1ap_id (M)
  gnb_cu_up_mbs_e1ap_id_t gnb_cu_up_mbs_e1ap_id;
  //bc_bearer_context_to_setup_response (M)
  e1ap_bc_bearer_context_to_setup_response bc_bearer_context_to_setup_response;
  //crit_diagnostics (O)
  std::optional<e1ap_crit_diagnostics> crit_diagnostics;
};

struct e1ap_bc_bearer_context_setup_failure {
  //gnb_cu_cp_mbs_e1ap_id (M)
  gnb_cu_cp_mbs_e1ap_id_t gnb_cu_cp_mbs_e1ap_id;
  //gnb_cu_up_mbs_e1ap_id (O)
  std::optional<gnb_cu_up_mbs_e1ap_id_t> gnb_cu_up_mbs_e1ap_id;
  //cause (M)
  e1ap_cause_t cause;
  //crit_diagnostics (O)
  std::optional<e1ap_crit_diagnostics> crit_diagnostics;
};

} // namespace srs_cu_up
} // namespace srsran
