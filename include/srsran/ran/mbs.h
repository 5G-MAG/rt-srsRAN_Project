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
#include "srsran/ran/plmn_identity.h"
#include "srsran/ran/tac.h"
#include "srsran/ran/pci.h"

namespace srsran {

// NOTE (borieher): Move TAI definition to srsran/ran/tai.h in upstream srsRAN or use cu_cp_tai?
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
public:
  tmgi_t                tmgi = tmgi_t::invalid;
  std::optional<nid_t>  nid = std::nullopt;

  bool operator==(const mbs_session_id_t& other) const {
    return tmgi == other.tmgi && nid == other.nid;
  }

  bool operator<(const mbs_session_id_t& other) const {
    if (tmgi != other.tmgi) {
      return tmgi < other.tmgi;
    }
    return nid < other.nid;
  }
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

/// \remark See 3GPP TS 23.003, 30.4 - Frequency Selection Area ID.
/// Note from 3GPP TS 38.413, 9.3.5.3 - MBS Frequency Selection Area ID ::= OCTET STRING (SIZE (3)).
static constexpr uint32_t INVALID_FSA_ID = ((uint32_t)1 << 24);

/// \brief Frequency Selection Area ID.
/// \remark See 3GPP TS 23.003, 30.4 - Frequency Selection Area ID.
enum class fsa_id_t : uint32_t { min = 0, max = INVALID_FSA_ID - 1, invalid = INVALID_FSA_ID };

/// Convert Frequency Selection Area ID type to integer.
constexpr inline uint32_t fsa_id_to_uint(fsa_id_t fsa_id)
{
  return static_cast<uint32_t>(fsa_id);
}

/// Convert integer to Frequency Selection Area ID type.
constexpr inline fsa_id_t uint_to_fsa_id(uint32_t idx)
{
  return static_cast<fsa_id_t>(idx);
}

/// \brief MBS-NeighbourCell-r17.
/// \remark See 3GPP TS 38.331, 6.3.6 - MBS information elements, MBS-NeighbourCell-r17.
struct mbs_neighbour_cell_item {
  pci_t phys_cell_id;
  unsigned carrier_freq;
};

/// Stores all the MBS-related cell configuration
struct mbs_cell_config {
  // MBS-NeighbourCellList-r17
  std::vector<mbs_neighbour_cell_item> mbs_neighbour_cell_list;
};

struct mbs_config {
  unsigned max_nof_mbs_sessions = 25;
  mbs_cell_config cell_config;
};

} // namespace srsran

namespace std {

template <>
struct hash<srsran::mbs_session_id_t> {
  size_t operator()(const srsran::mbs_session_id_t& s) const noexcept {
    size_t h1 = std::hash<uint64_t>{}(static_cast<uint64_t>(s.tmgi));
    size_t h2 = s.nid.has_value() ? std::hash<uint64_t>{}(static_cast<uint64_t>(*s.nid)) : 0;
    return h1 ^ (h2 << 1);
  }
};

} // namespace std

