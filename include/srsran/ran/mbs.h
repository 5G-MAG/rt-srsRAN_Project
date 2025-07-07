/*
 * 5G-MAG Reference Tools
 * Copyright (C) 2025 iTEAM UPV <jaisanro@iteam.upv.es> <borieher@iteam.upv.es>
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

#include <cstdint>
#include <optional>
#include <variant>
#include "srsran/ran/nr_cgi.h"
#include "srsran/ran/plmn_identity.h"
#include "srsran/ran/tac.h"
#include "srsran/adt/slotted_vector.h"

namespace srsran {

// NOTE (borieher): Move TAI definition to srsran/ran/tai.h in upstream srsRAN.
/// \brief TAI.
/// \remark See 3GPP TS 38.413, 9.3.3.11 - TAI.
struct tai_t {
  plmn_identity plmn_id = plmn_identity::test_value();
  tac_t         tac;
};

/// \remark See 3GPP TS 23.003, 30.2 - TMGI.
/// Note from 3GPP TS 38.413, 9.3.1.206 - MBS Session ID - TMGI ::= OCTET STRING (SIZE (6)).
static constexpr uint64_t INVALID_TMGI = ((uint64_t)1 << 48);

/// \brief TMGI.
/// \remark See 3GPP TS 23.003, 30.2 - TMGI.
enum class tmgi_t : uint64_t { min = 0, max = INVALID_TMGI - 1, invalid = INVALID_TMGI };

/// Convert TMGI type to integer.
constexpr inline uint64_t tmgi_to_uint(tmgi_t tmgi)
{
  return static_cast<uint64_t>(tmgi);
}

/// Convert integer to TMGI type.
constexpr inline tmgi_t uint_to_tmgi(uint64_t idx)
{
  return static_cast<tmgi_t>(idx);
}

/// \remark See 3GPP TS 23.003, 12.7.1 - NID.
/// Note from 3GPP TS 38.413, 9.3.3.42 - NID ::= BIT STRING (SIZE (44)).
static constexpr uint64_t INVALID_NID = ((uint64_t)1 << 44);

/// \brief NID.
/// \remark See 3GPP TS 23.003, 12.7.1 - NID.
enum class nid_t : uint64_t { min = 0, max = INVALID_NID - 1, invalid = INVALID_NID };

/// Convert NID type to integer.
constexpr inline uint64_t nid_to_uint(nid_t nid)
{
  return static_cast<uint64_t>(nid);
}

/// Convert integer to NID type.
constexpr inline nid_t uint_to_nid(uint64_t idx)
{
  return static_cast<nid_t>(idx);
}

/// \brief MBS Session ID.
/// \remark See 3GPP TS 38.413, 9.3.1.206 - MBS Session ID.
struct mbs_session_id_t {
  tmgi_t                tmgi;
  std::optional<nid_t>  nid;
};

/// \remark See 3GPP TS 23.003, 30.3 - Area Session ID.
/// Note from 3GPP TS 38.413, 9.3.1.207 - MBS Area Session ID ::= INTEGER (0..65535).
static constexpr uint32_t INVALID_AREA_SESSION_ID = ((uint32_t)1 << 16);

/// \brief Area Session ID.
/// \remark See 3GPP TS 23.003, 30.3 - Area Session ID.
enum class area_session_id_t : uint32_t { min = 0, max = INVALID_AREA_SESSION_ID - 1, invalid = INVALID_AREA_SESSION_ID };

/// Convert Area Session ID type to integer.
constexpr inline uint32_t area_session_id_to_uint(area_session_id_t area_session_id)
{
  return static_cast<uint32_t>(area_session_id);
}

/// Convert integer to Area Session ID type.
constexpr inline area_session_id_t uint_to_area_session_id(uint32_t idx)
{
  return static_cast<area_session_id_t>(idx);
}

/// \brief MBS Service Area Information.
/// \remark See 3GPP TS 38.413, 9.3.1.209 - MBS Service Area Information.
struct mbs_service_area_information_t {
  std::vector<nr_cell_global_id_t> mbs_service_area_cell_list;
  std::vector<tai_t> mbs_service_area_tai_list;
};

struct mbs_service_area_information_item {
  area_session_id_t mbs_area_session_id;
  mbs_service_area_information_t mbs_service_area_information;
};

struct location_independent {
  mbs_service_area_information_t mbs_service_area_information;
};

struct location_dependent {
  std::vector<mbs_service_area_information_item> mbs_service_area_information_list;
};

/// \brief MBS Service Area.
/// \remark See 3GPP TS 38.413, 9.3.1.208 - MBS Service Area.
struct mbs_service_area_t {
  mbs_service_area_t() : choice(location_independent{}) {}
  mbs_service_area_t(const location_independent& val) : choice(val) {}
  mbs_service_area_t(const location_dependent& val) : choice(val) {}

  bool is_locationdependent() const { return std::holds_alternative<location_dependent>(choice); }

  location_independent get_locationindependent() { return std::get<location_independent>(choice); }
  location_dependent get_locationdependent() { return std::get<location_dependent>(choice); }

private:
  std::variant<location_independent, location_dependent> choice;
};

struct mbs_config {};

} // namespace srsran
