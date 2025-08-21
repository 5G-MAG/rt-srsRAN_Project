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

#include "e1ap_cu_cp_mbs_session_transaction_manager.h"
#include "srsran/cu_cp/cu_cp_types.h"
#include "srsran/e1ap/common/e1ap_types.h"
#include "srsran/ran/mbs.h"

namespace srsran {
namespace srs_cu_cp {

struct e1ap_mbs_ids {
  /// MBS index assigned by the CU-CP MBS manager
  mbs_index_t mbs_index = mbs_index_t::invalid;
  const gnb_cu_cp_mbs_e1ap_id_t cu_cp_mbs_e1ap_id = gnb_cu_cp_mbs_e1ap_id_t::invalid;
  gnb_cu_up_mbs_e1ap_id_t cu_up_mbs_e1ap_id = gnb_cu_up_mbs_e1ap_id_t::invalid;
};

/// \brief E1AP CU-CP MBS Session context.
class e1ap_mbs_session_context
{
public:
  e1ap_mbs_ids mbs_ids;

  e1ap_cu_cp_mbs_session_transaction_manager mbs_session_ev_mng;

  e1ap_mbs_session_context(mbs_index_t mbs_index_, gnb_cu_cp_mbs_e1ap_id_t cu_cp_mbs_e1ap_id_, timer_factory timers_) :
  mbs_ids({mbs_index_, cu_cp_mbs_e1ap_id_}), mbs_session_ev_mng(timers_)
  {
  }

  const e1ap_mbs_ids& get_mbs_ids() const { return mbs_ids; }

  void add_cu_up_mbs_e1ap_id(gnb_cu_up_mbs_e1ap_id_t cu_up_mbs_e1ap_id)
  {
    srsran_assert(cu_up_mbs_e1ap_id != gnb_cu_up_mbs_e1ap_id_t::invalid, "Invalid cu_up_mbs_e1ap_id");
    mbs_ids.cu_up_mbs_e1ap_id = cu_up_mbs_e1ap_id;
  }
};

/// \brief List of E1AP MBS Session contexts in the CU-CP for a given CU-UP.
class e1ap_mbs_session_context_list
{
public:
  e1ap_mbs_session_context_list(timer_factory timers_, srslog::basic_logger& logger_) : timers(timers_), logger(logger_) {}

  /// \brief Checks whether an MBS Session with the given gNB-CU-CP MBS E1AP ID exists.
  bool contains(gnb_cu_cp_mbs_e1ap_id_t cu_cp_mbs_e1ap_id) const { return mbs_sessions.find(cu_cp_mbs_e1ap_id) != mbs_sessions.end(); }

  /// \brief Checks whether a MBS Session with the given MBS index exists.
  /// \param[in] mbs_index The MBS index used to find the MBS Session.
  /// \return True when a MBS Session for the given MBS index exists, false otherwise.
  bool contains(mbs_index_t mbs_index) const
  {
    if (mbs_index_to_mbs_e1ap_id.find(mbs_index) == mbs_index_to_mbs_e1ap_id.end()) {
      return false;
    }
    if (mbs_sessions.find(mbs_index_to_mbs_e1ap_id.at(mbs_index)) == mbs_sessions.end()) {
      logger.warning("No MBS Session context found for gNB-CU-CP MBS E1AP ID");
      return false;
    }
    return true;
  }

  e1ap_mbs_session_context& operator[](gnb_cu_cp_mbs_e1ap_id_t cu_cp_mbs_e1ap_id)
  {
    srsran_assert(mbs_sessions.find(cu_cp_mbs_e1ap_id) != mbs_sessions.end(),
                  "E1AP MBS Session context not found");
    return mbs_sessions.at(cu_cp_mbs_e1ap_id);
  }

  e1ap_mbs_session_context& operator[](mbs_index_t mbs_index)
  {
    srsran_assert(mbs_index_to_mbs_e1ap_id.find(mbs_index) != mbs_index_to_mbs_e1ap_id.end(),
                  "gNB-CU-CP MBS E1AP ID not found");
    srsran_assert(mbs_sessions.find(mbs_index_to_mbs_e1ap_id.at(mbs_index)) != mbs_sessions.end(),
                  "E1AP MBS Session context not found");
    return mbs_sessions.at(mbs_index_to_mbs_e1ap_id.at(mbs_index));
  }

  /// \brief Search for an MBS Session based on its gNB-CU-CP MBS E1AP ID.
  const e1ap_mbs_session_context* find(gnb_cu_cp_mbs_e1ap_id_t cu_cp_mbs_e1ap_id) const
  {
    auto it = mbs_sessions.find(cu_cp_mbs_e1ap_id);
    if (it == mbs_sessions.end()) {
      return nullptr;
    }
    return &it->second;
  }

  e1ap_mbs_session_context* find(gnb_cu_cp_mbs_e1ap_id_t cu_cp_mbs_e1ap_id)
  {
    auto it = mbs_sessions.find(cu_cp_mbs_e1ap_id);
    if (it == mbs_sessions.end()) {
      return nullptr;
    }
    return &it->second;
  }

  /// \brief Create new E1AP MBS Session context.
  e1ap_mbs_session_context* add_mbs_session_context(mbs_index_t mbs_index, gnb_cu_cp_mbs_e1ap_id_t cu_cp_mbs_e1ap_id)
  {
    srsran_assert(mbs_index != mbs_index_t::invalid, "Invalid MBS index");
    srsran_assert(cu_cp_mbs_e1ap_id != gnb_cu_cp_mbs_e1ap_id_t::invalid, "Invalid gNB-CU-CP MBS E1AP ID");

    if (mbs_index_to_mbs_e1ap_id.find(mbs_index) != mbs_index_to_mbs_e1ap_id.end()) {
      logger.error("E1AP MBS Session context already exists");
      return nullptr;
    }

    auto ret = mbs_sessions.emplace(std::piecewise_construct,
                         std::forward_as_tuple(cu_cp_mbs_e1ap_id),
                         std::forward_as_tuple(mbs_index, cu_cp_mbs_e1ap_id, timers));

    if (not ret.second) {
      logger.error("{}: Failed to create E1AP MBS Session context");
      return nullptr;
    }
    mbs_index_to_mbs_e1ap_id.emplace(mbs_index, cu_cp_mbs_e1ap_id);

    logger.debug("E1AP MBS Session context created");
    return &ret.first->second;
  }

  void remove_mbs_session_context(mbs_index_t mbs_index) {
    srsran_assert(mbs_index != mbs_index_t::invalid, "Invalid MBS index");

    if (mbs_index_to_mbs_e1ap_id.find(mbs_index) == mbs_index_to_mbs_e1ap_id.end()) {
      logger.warning("gNB-CU-CP MBS E1AP ID not found");
      return;
    }

    // Remove gNB-CU-CP MBS E1AP ID from lookup.
    gnb_cu_cp_mbs_e1ap_id_t cu_cp_mbs_e1ap_id = mbs_index_to_mbs_e1ap_id.at(mbs_index);
    mbs_index_to_mbs_e1ap_id.erase(mbs_index);

    if (mbs_sessions.find(cu_cp_mbs_e1ap_id) == mbs_sessions.end()) {
      logger.warning("E1AP MBS Session context not found");
      return;
    }

    logger.debug("Removing E1AP MBS Session context");
    mbs_sessions.erase(cu_cp_mbs_e1ap_id);
  }

  size_t size() const { return mbs_sessions.size(); }

  /// \brief Get the next available gNB-CU-CP MBS E1AP ID.
  gnb_cu_cp_mbs_e1ap_id_t allocate_gnb_cu_cp_mbs_e1ap_id()
  {
    // Return invalid when no gNB-CU-CP MBS E1AP ID is available.
    if (mbs_index_to_mbs_e1ap_id.size() >= MAX_NOF_CU_MBS) {
      return gnb_cu_cp_mbs_e1ap_id_t::invalid;
    }

    // Check if the next_cu_cp_mbs_e1ap_id is available.
    if (mbs_sessions.find(next_cu_cp_mbs_e1ap_id) == mbs_sessions.end()) {
      gnb_cu_cp_mbs_e1ap_id_t ret = next_cu_cp_mbs_e1ap_id;
      // Increase the next gNB-CU-CP MBS E1AP ID.
      increase_next_cu_cp_mbs_e1ap_id();
      return ret;
    }

    // Find holes in the allocated IDs by iterating over all ids starting with the next_cu_cp_mbs_e1ap_id to find the
    // available id
    while (true) {
      // Only iterate over mbs_index_to_mbs_e1ap_id (size=MAX_NOF_CU_MBS)
      // to avoid iterating over all possible values of gnb_cu_cp_mbs_e1ap_id_t (size=2^32-1)
      auto it = std::find_if(mbs_index_to_mbs_e1ap_id.begin(), mbs_index_to_mbs_e1ap_id.end(), [this](auto& u) {
        return u.second == next_cu_cp_mbs_e1ap_id;
      });

      // Return the ID if it is not already used.
      if (it == mbs_index_to_mbs_e1ap_id.end()) {
        gnb_cu_cp_mbs_e1ap_id_t ret = next_cu_cp_mbs_e1ap_id;
        // Increase the next gNB-CU-CP MBS E1AP ID.
        increase_next_cu_cp_mbs_e1ap_id();
        return ret;
      }

      // Increase the next gNB-CU-CP MBS E1AP ID and try again.
      increase_next_cu_cp_mbs_e1ap_id();
    }

    return gnb_cu_cp_mbs_e1ap_id_t::invalid;
  }

protected:
  gnb_cu_cp_mbs_e1ap_id_t next_cu_cp_mbs_e1ap_id = gnb_cu_cp_mbs_e1ap_id_t::min;

private:
  timer_factory         timers;
  srslog::basic_logger& logger;

  inline void increase_next_cu_cp_mbs_e1ap_id()
  {
    if (next_cu_cp_mbs_e1ap_id == gnb_cu_cp_mbs_e1ap_id_t::max) {
      // Reset gNB-CU-CP MBS E1AP ID counter.
      next_cu_cp_mbs_e1ap_id = gnb_cu_cp_mbs_e1ap_id_t::min;
    } else {
      // Increase gNB-CU-CP MBS E1AP ID counter.
      next_cu_cp_mbs_e1ap_id = uint_to_gnb_cu_cp_mbs_e1ap_id(gnb_cu_cp_mbs_e1ap_id_to_uint(next_cu_cp_mbs_e1ap_id) + 1);
    }
  }

  // Note: Given that MBS Sessions will self-remove from the map, we don't want to destructor to clear the lookups beforehand.
  std::unordered_map<mbs_index_t, gnb_cu_cp_mbs_e1ap_id_t>              mbs_index_to_mbs_e1ap_id; // indexed by mbs_index
  std::unordered_map<gnb_cu_cp_mbs_e1ap_id_t, e1ap_mbs_session_context> mbs_sessions;             // indexed by gnb_cu_cp_mbs_e1ap_id
};

} // namespace srs_cu_cp
} // namespace srsran
