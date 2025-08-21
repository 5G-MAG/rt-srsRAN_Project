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
#include "srsran/ngap/ngap_types.h"
#include "srsran/ran/mbs.h"
#include "srsran/ran/s_nssai.h"
#include "srsran/ran/crit_diagnostics.h"
#include "srsran/ran/cause/ngap_cause.h"
#include "srsran/adt/byte_buffer.h"

namespace srsran {
namespace srs_cu_cp {

struct ngap_broadcast_session_setup_request {
  mbs_index_t mbs_index = mbs_index_t::invalid;
  //mbs_session_id (M)
  mbs_session_id_t mbs_session_id;
  //s_nssai (M)
  s_nssai_t s_nssai;
  //mbs_service_area (M)
  ngap_mbs_service_area mbs_service_area;
  //mbs_session_setup_request_transfer (M)
  ngap_mbs_session_setup_or_modification_request_transfer mbs_session_setup_request_transfer;
};

struct ngap_broadcast_session_setup_response {
  //mbs_session_id (M)
  mbs_session_id_t mbs_session_id;
  //mbs_session_setup_response_transfer (O)
  std::optional<byte_buffer> mbs_session_setup_response_transfer;
  //criticality_diagnostics (O)
  std::optional<crit_diagnostics_t> crit_diagnostics;
};

struct ngap_broadcast_session_setup_failure {
  //mbs_session_id (M)
  mbs_session_id_t mbs_session_id;
  //mbs_session_setup_failure_transfer (O)
  std::optional<byte_buffer> mbs_session_setup_failure_transfer;
  //cause (M)
  ngap_cause_t cause;
  //criticality_diagnostics (O)
  std::optional<crit_diagnostics_t> crit_diagnostics;
};

} // namespace srs_cu_cp
} // namespace srsran
