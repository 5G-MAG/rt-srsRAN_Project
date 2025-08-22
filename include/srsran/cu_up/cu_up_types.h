/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
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

#include "srsran/f1u/cu_up/f1u_config.h"
#include "srsran/pdcp/pdcp_config.h"
#include "srsran/ran/cu_types.h"
#include "srsran/ran/rb_id.h"
#include <cstdint>
#include <type_traits>
#include <vector>

namespace srsran {
namespace srs_cu_up {

/// Maximum number of MBS Sessions supported by CU-UP (implementation-defined).
const uint64_t MAX_NOF_CU_MBS = 16777215; // 2^24 - 1

/// Maximum number of UEs supported by CU-UP (implementation-defined).
enum ue_index_t : uint16_t {
  MIN_UE_INDEX     = 0,
  MAX_UE_INDEX     = 1023,
  MAX_NOF_UES      = 1024,
  INVALID_UE_INDEX = MAX_NOF_UES
};

/// Convert integer to CU UE index type.
constexpr ue_index_t int_to_ue_index(std::underlying_type_t<ue_index_t> idx)
{
  return static_cast<ue_index_t>(idx);
}

constexpr bool is_ue_index_valid(ue_index_t ue_idx)
{
  return ue_idx < MAX_NOF_UES;
}

/// \brief mbs_index internally used to identify the MBS Session CU-UP-wide.
/// \remark The mbs_index is derived from the maximum number E1AP IDs in the CU-UP.
enum class mbs_index_t : uint64_t { min = 0, max = MAX_NOF_CU_MBS - 1, invalid = MAX_NOF_CU_MBS };

/// Convert mbs_index type to integer.
inline uint64_t mbs_index_to_uint(mbs_index_t index)
{
  return static_cast<uint64_t>(index);
}

/// Convert integer to mbs_index type.
inline mbs_index_t uint_to_mbs_index(std::underlying_type_t<mbs_index_t> index)
{
  return static_cast<mbs_index_t>(index);
}

struct e1ap_bearer_context_inactivity_notification {
  ue_index_t                    ue_index    = ue_index_t::INVALID_UE_INDEX;
  bool                          ue_inactive = false;
  std::vector<drb_id_t>         inactive_drbs;
  std::vector<pdu_session_id_t> inactive_pdu_sessions;
};

/// QoS Configuration, i.e. 5QI and the associated SDAP, PDCP and F1-U configuration for DRBs.
struct cu_up_qos_config {
  /// Implementation-specific parameters for PDCP.
  pdcp_custom_config pdcp_custom_cfg;
  /// Implementation-specific parameters for F1-U.
  f1u_config f1u_cfg;
};

} // namespace srs_cu_up
} // namespace srsran
