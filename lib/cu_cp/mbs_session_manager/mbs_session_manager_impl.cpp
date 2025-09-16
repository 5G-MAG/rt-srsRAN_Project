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

#include "mbs_session_manager_impl.h"

using namespace srsran;
using namespace srs_cu_cp;

mbs_session_manager::mbs_session_manager(const cu_cp_configuration& cu_cp_cfg) :
  max_nof_mbs_sessions(cu_cp_cfg.mbs.max_nof_mbs_sessions)
{
}

// common

void mbs_session_manager::stop()
{
  mbs_common_task_sched.stop();
}

void mbs_session_manager::remove_mbs_session(mbs_index_t mbs_index)
{
  if (mbs_index == mbs_index_t::invalid) {
    logger.warning("Can't remove MBS Session with invalid MBS index");
    return;
  }

  if (mbs_sessions.find(mbs_index) == mbs_sessions.end()) {
    logger.warning("Remove MBS Session called for inexistent MBS Session");
    return;
  }

  // Remove MBS Session from lookups
  mbs_session_id_t mbs_session_id = mbs_sessions.at(mbs_index).get_mbs_session_id();
  if (mbs_session_id.tmgi != tmgi_t::invalid) {
    if (!mbs_session_id.nid.has_value() ||
       (mbs_session_id.nid.has_value() && mbs_session_id.nid.value() != nid_t::invalid)) {
      mbs_session_id_to_mbs_index.erase(mbs_session_id);
    } else {
        logger.warning("MBS Session ID [NID] not found");
    }
  } else {
    logger.warning("MBS Session ID [TMGI] not found");
  }

  // Remove CU-CP MBS Session from database
  mbs_sessions.erase(mbs_index);

  logger.debug("MBS Session removed");
}

mbs_index_t mbs_session_manager::get_mbs_index(mbs_session_id_t mbs_session_id)
{
  if (mbs_session_id_to_mbs_index.find(mbs_session_id) != mbs_session_id_to_mbs_index.end()) {
    return mbs_session_id_to_mbs_index.at(mbs_session_id);
  }
  logger.debug("MBS index for MBS Session ID not found");
  return mbs_index_t::invalid;
}

cu_cp_mbs_session* mbs_session_manager::find_mbs_session(mbs_index_t mbs_index)
{
  if (mbs_sessions.find(mbs_index) != mbs_sessions.end()) {
    return &mbs_sessions.at(mbs_index);
  }
  return nullptr;
}

// ngap

mbs_index_t mbs_session_manager::add_mbs_session(mbs_session_id_t mbs_session_id)
{
  if (mbs_sessions.size() == max_nof_mbs_sessions) {
    logger.warning(
        "CU-CP MBS Session creation Failed. Cause: Maximum number of MBS Sessions supported by the CU-CP ({}) has been reached",
        max_nof_mbs_sessions);
    fmt::print("CU-CP MBS Session creation failed. Cause: Maximum number of MBS Sessions supported by the CU-CP ({}) has been reached. "
               "To increase the number of supported "
               "MBS Sessions change the \"--max_nof_mbs_sessions\" in the MBS configuration\n",
               max_nof_mbs_sessions);
    return mbs_index_t::invalid;
  }

  if (mbs_session_id.tmgi == tmgi_t::invalid) {
    logger.warning("CU-CP MBS Session creation Failed. Cause: Invalid TMGI");
    return mbs_index_t::invalid;
  }

  if (mbs_session_id.nid.has_value() && mbs_session_id.nid.value() == nid_t::invalid) {
    logger.warning("CU-CP MBS Session creation Failed. Cause: Invalid NID");
    return mbs_index_t::invalid;
  }

  // Check if the MBS Session ID already exists
  if (get_mbs_index(mbs_session_id) != mbs_index_t::invalid) {
    logger.warning("CU-CP MBS Session creation Failed. Cause: MBS Session ID already exists");
    return mbs_index_t::invalid;
  }

  mbs_index_t new_mbs_index = allocate_mbs_index();
  if (new_mbs_index == mbs_index_t::invalid) {
    logger.warning("CU-CP MBS Session creation Failed. Cause: No free MBS index available");
    return mbs_index_t::invalid;
  }

  // Create CU-CP MBS Session object
  mbs_sessions.emplace(std::piecewise_construct,
                       std::forward_as_tuple(new_mbs_index),
                       std::forward_as_tuple(new_mbs_index, mbs_session_id));

  // MBS Session ID to lookup
  mbs_session_id_to_mbs_index.emplace(mbs_session_id, new_mbs_index);

  logger.info("Created new CU-CP MBS Session");

  return new_mbs_index;
}

mrb_id_t mbs_session_manager::get_next_mrb_id()
{
  mrb_id_t ret = next_mrb_id;
  increase_next_mrb_id();
  return ret;
}

// private functions

mbs_index_t mbs_session_manager::allocate_mbs_index()
{
  // return invalid when no MBS index is available
  if (mbs_sessions.size() == max_nof_mbs_sessions) {
    return mbs_index_t::invalid;
  }

  // Check if the next_mbs_index is available
  if (mbs_sessions.find(next_mbs_index) == mbs_sessions.end()) {
    mbs_index_t ret = next_mbs_index;
    // increase the next_mbs_index
    increase_next_mbs_index();
    return ret;
  }

  // Find holes in the allocated IDs by iterating over all ids starting with the next_mbs_index to find the
  // available id
  while (true) {
    // increase the next_mbs_index and try again
    increase_next_mbs_index();

    // return the id if it is not already used
    if (mbs_sessions.find(next_mbs_index) == mbs_sessions.end()) {
      mbs_index_t ret = next_mbs_index;
      // increase the next_mbs_index
      increase_next_mbs_index();
      return ret;
    }
  }

  return mbs_index_t::invalid;
}