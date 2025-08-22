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

#include "srsran/cu_cp/cu_cp_types.h"
#include "srsran/ran/mbs.h"
#include "srsran/ran/s_nssai.h"
#include "srsran/ran/crit_diagnostics.h"
#include "srsran/ran/cause/f1ap_cause.h"
#include "srsran/f1ap/f1ap_mbs_types.h"

namespace srsran {
namespace srs_cu_cp {

struct f1ap_broadcast_context_setup_request {
  mbs_index_t mbs_index = mbs_index_t::invalid;
  //gnb_cu_mbs_f1ap_id (M)
  gnb_cu_mbs_f1ap_id_t gnb_cu_mbs_f1ap_id;
  //mbs_session_id (M)
  mbs_session_id_t mbs_session_id;
  //mbs_service_area (O)
  std::optional<f1ap_mbs_service_area> mbs_service_area;
  //mbs_cu_to_du_rrc_information (M)
  f1ap_mbs_cu_to_du_rrc_information mbs_cu_to_du_rrc_information;
  //s_nssai (M)
  s_nssai_t s_nssai;
  //broadcast_mrb_to_be_setup_list (1)
  std::vector<f1ap_broadcast_mrb_to_be_setup_item> broadcast_mrb_to_be_setup_list;
};

struct f1ap_broadcast_context_setup_response {
  //gnb_cu_mbs_f1ap_id (M)
  gnb_cu_mbs_f1ap_id_t gnb_cu_mbs_f1ap_id;
  //gnb_du_mbs_f1ap_id (M)
  gnb_du_mbs_f1ap_id_t gnb_du_mbs_f1ap_id;
  //broadcast_mrb_setup_list (1)
  std::vector<f1ap_broadcast_mrb_setup_item> broadcast_mrb_setup_list;
  //broadcast_mrb_failed_to_be_setup_list (0..1)
  std::vector<f1ap_broadcast_mrb_failed_to_be_setup_item> broadcast_mrb_failed_to_be_setup_list;
  //broadcast_area_scope (O)
  //crit_diagnostics (O)
  std::optional<crit_diagnostics_t> crit_diagnostics;
};

struct f1ap_broadcast_context_setup_failure {
  //gnb_cu_mbs_f1ap_id (M)
  gnb_cu_mbs_f1ap_id_t gnb_cu_mbs_f1ap_id;
  //gnb_du_mbs_f1ap_id (O)
  std::optional<gnb_du_mbs_f1ap_id_t> gnb_du_mbs_f1ap_id;
  //cause (M)
  f1ap_cause_t cause;
  //crit_diagnostics (O)
  std::optional<crit_diagnostics_t> crit_diagnostics;
};

} // namespace srs_cu_cp
} // namespace srsran
