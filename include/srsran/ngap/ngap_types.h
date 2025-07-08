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

#include "srsran/ran/plmn_identity.h"
#include "srsran/ran/nr_cgi.h"
#include "srsran/ran/mbs.h"
#include <limits>

namespace srsran {
namespace srs_cu_cp {

/// \brief AMF_UE_ID (non ASN1 type of AMF_UE_NGAP_ID) used to identify the UE in the AMF.
/// \remark See TS 38.413 Section 9.3.3.1: AMF_UE_NGAP_ID valid values: (0..2^40-1)
static constexpr uint64_t MAX_NOF_AMF_UES = ((uint64_t)1 << 40);
enum class amf_ue_id_t : uint64_t { min = 0, max = MAX_NOF_AMF_UES - 1, invalid = 0x1ffffffffff };

/// Convert AMF_UE_ID type to integer.
inline uint64_t amf_ue_id_to_uint(amf_ue_id_t id)
{
  return static_cast<uint64_t>(id);
}

/// Convert integer to AMF_UE_ID type.
inline amf_ue_id_t uint_to_amf_ue_id(std::underlying_type_t<amf_ue_id_t> id)
{
  return static_cast<amf_ue_id_t>(id);
}

// Globally unique AMF identifier.
struct guami_t {
  plmn_identity plmn = plmn_identity::test_value();
  uint16_t      amf_set_id;
  uint8_t       amf_pointer;
  uint8_t       amf_region_id;
};

struct ngap_ue_aggr_max_bit_rate {
  uint64_t ue_aggr_max_bit_rate_dl;
  uint64_t ue_aggr_max_bit_rate_ul;
};

/// \brief MBS Service Area Information.
/// \remark See 3GPP TS 38.413, 9.3.1.209 - MBS Service Area Information.
struct ngap_mbs_service_area_information {
  //mbs_service_area_cell_list (0..maxnoofCellsforMBS)
  std::vector<nr_cell_global_id_t> mbs_service_area_cell_list;
  //mbs_service_area_tai_list (0..maxnoofTAIforMBS)
  std::vector<tai_t> mbs_service_area_tai_list;
};

struct ngap_mbs_service_area_information_item {
  //mbs_area_session_id (M)
  area_session_id_t mbs_area_session_id;
  //mbs_service_area_information (M)
  ngap_mbs_service_area_information mbs_service_area_information;
};

struct ngap_location_independent {
  //mbs_service_area_information (M)
  ngap_mbs_service_area_information mbs_service_area_information;
};

struct ngap_location_dependent {
  //mbs_service_area_information_list (1)
  std::vector<ngap_mbs_service_area_information_item> mbs_service_area_information_list;
};

/// \brief MBS Service Area.
/// \remark See 3GPP TS 38.413, 9.3.1.208 - MBS Service Area.
struct ngap_mbs_service_area {
  ngap_mbs_service_area() : choice(ngap_location_independent{}) {}
  ngap_mbs_service_area(const ngap_location_independent& val) : choice(val) {}
  ngap_mbs_service_area(const ngap_location_dependent& val) : choice(val) {}

  bool is_locationdependent() const { return std::holds_alternative<ngap_location_dependent>(choice); }

  ngap_location_independent get_locationindependent() { return std::get<ngap_location_independent>(choice); }
  ngap_location_dependent get_locationdependent() { return std::get<ngap_location_dependent>(choice); }

private:
  std::variant<ngap_location_independent, ngap_location_dependent> choice;
};

} // namespace srs_cu_cp
} // namespace srsran
