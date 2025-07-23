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

namespace srsran {
namespace srs_cu_cp {

struct ngap_mbs_ids {
  /// MBS index assigned by the CU-CP MBS manager
  mbs_index_t mbs_index = mbs_index_t::invalid;
  mbs_session_id_t mbs_session_id;
};

struct ngap_mbs_session_context {
  ngap_mbs_ids mbs_ids;
  ngap_mbs_session_context(mbs_index_t mbs_index_,
                           mbs_session_id_t mbs_session_id_) :
    mbs_ids({mbs_index_, mbs_session_id_})
  {
  }
};

class ngap_mbs_session_context_list {
public:
  ngap_mbs_session_context_list(srslog::basic_logger& logger_) : logger(logger_) {}

  /// \brief Checks whether a MBS Session with the given MBS Session ID exists.
  /// \param[in] mbs_session_id The MBS Session ID used to find the MBS Session.
  /// \return True when a MBS Session for the given MBS Session ID exists, false otherwise.
  bool contains(mbs_session_id_t mbs_session_id) const { return mbs_sessions.find(mbs_session_id) != mbs_sessions.end(); }

  /// \brief Checks whether a MBS Session with the given MBS index exists.
  /// \param[in] mbs_index The MBS index used to find the MBS Session.
  /// \return True when a MBS Session for the given MBS index exists, false otherwise.
  bool contains(mbs_index_t mbs_index) const
  {
    if (mbs_index_to_mbs_session_id.find(mbs_index) == mbs_index_to_mbs_session_id.end()) {
      return false;
    }
    if (mbs_sessions.find(mbs_index_to_mbs_session_id.at(mbs_index)) == mbs_sessions.end()) {
      return false;
    }
    return true;
  }

  ngap_mbs_session_context& operator[](mbs_session_id_t mbs_session_id)
  {
    srsran_assert(mbs_sessions.find(mbs_session_id) != mbs_sessions.end(), "NGAP MBS Session context not found");
    return mbs_sessions.at(mbs_session_id);
  }

  ngap_mbs_session_context& operator[](mbs_index_t mbs_index)
  {
    srsran_assert(
        mbs_index_to_mbs_session_id.find(mbs_index) != mbs_index_to_mbs_session_id.end(), "MBS Session ID not found");
    srsran_assert(mbs_sessions.find(mbs_index_to_mbs_session_id.at(mbs_index)) != mbs_sessions.end(),
                  "NGAP MBS Session context not found");
    return mbs_sessions.at(mbs_index_to_mbs_session_id.at(mbs_index));
  }

  ngap_mbs_session_context* find(mbs_session_id_t mbs_session_id)
  {
    auto it = mbs_sessions.find(mbs_session_id);
    if (it == mbs_sessions.end()) {
      return nullptr;
    }
    return &it->second;
  }

  const ngap_mbs_session_context* find(mbs_session_id_t mbs_session_id) const
  {
    auto it = mbs_sessions.find(mbs_session_id);
    if (it == mbs_sessions.end()) {
      return nullptr;
    }
    return &it->second;
  }

  ngap_mbs_session_context* add_mbs_session_context(mbs_index_t mbs_index, mbs_session_id_t mbs_session_id)
  {
    srsran_assert(mbs_session_id.tmgi != tmgi_t::invalid, "Invalid TMGI in MBS Session ID");
    if (mbs_session_id.nid.has_value()) {
      srsran_assert(mbs_session_id.nid.value() != nid_t::invalid, "Invalid NID in MBS Session ID");
    }

    if (mbs_index_to_mbs_session_id.find(mbs_index) != mbs_index_to_mbs_session_id.end()) {
      logger.error("NGAP MBS Session context already exists");
      return nullptr;
    }

    auto ret = mbs_sessions.emplace(std::piecewise_construct,
                std::forward_as_tuple(mbs_session_id),
                std::forward_as_tuple(mbs_index, mbs_session_id));

    if (not ret.second) {
      logger.error("{}: Failed to create NGAP MBS Session context");
      return nullptr;
    }

    mbs_index_to_mbs_session_id.emplace(mbs_index, mbs_session_id);

    logger.debug("NGAP MBS Session context created");
    return &ret.first->second;
  }

  void remove_mbs_session_context(mbs_index_t mbs_index)
  {
    srsran_assert(mbs_index != mbs_index_t::invalid, "Invalid MBS index");

    if (mbs_index_to_mbs_session_id.find(mbs_index) == mbs_index_to_mbs_session_id.end()) {
      logger.warning("MBS Session ID not found");
      return;
    }

    // Remove MBS Session from lookup.
    mbs_session_id_t mbs_session_id = mbs_index_to_mbs_session_id.at(mbs_index);
    mbs_index_to_mbs_session_id.erase(mbs_index);

    if (mbs_sessions.find(mbs_session_id) == mbs_sessions.end()) {
      logger.warning("NGAP MBS Session context not found");
      return;
    }

    logger.debug("Removing NGAP MBS Session context");
    mbs_sessions.erase(mbs_session_id);
  }

  size_t size() const { return mbs_sessions.size(); }

private:
  srslog::basic_logger& logger;

  // Note: Given that MBS Sessions will self-remove from the map, we don't want to destructor to clear the lookups beforehand.
  std::unordered_map<mbs_index_t, mbs_session_id_t>               mbs_index_to_mbs_session_id; // indexed by mbs_index
  std::unordered_map<mbs_session_id_t, ngap_mbs_session_context>  mbs_sessions;                // indexed by mbs_session_id_t
};

} // namespace srs_cu_cp
} // namespace srsran
