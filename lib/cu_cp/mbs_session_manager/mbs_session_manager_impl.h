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

#include "cu_cp_mbs_session_impl.h"
#include "srsran/cu_cp/common_task_scheduler.h"
#include "srsran/support/async/fifo_async_task_scheduler.h"
#include "srsran/ran/mbs.h"
#include "srsran/cu_cp/cu_cp_types.h"
#include "srsran/cu_cp/cu_cp_configuration.h"
#include <unordered_map>

namespace srsran {

namespace srs_cu_cp {

class mbs_common_task_scheduler : public common_task_scheduler
{
public:
  mbs_common_task_scheduler() : main_ctrl_loop(128) {}

  bool schedule_async_task(async_task<void> task) override { return main_ctrl_loop.schedule(std::move(task)); }

  void stop() { main_ctrl_loop.request_stop(); }

private:
  // mbs task event loop
  fifo_async_task_scheduler main_ctrl_loop;
};

class mbs_session_manager
{
public:
  explicit mbs_session_manager(const cu_cp_configuration& cu_cp_cfg);

  // common

  /// Stop MBS activity.
  void stop();

  /// \brief Remove the MBS Session context with the given MBS index.
  /// \param[in] mbs_index Index of the MBS Session to be removed.
  void remove_mbs_session(mbs_index_t mbs_index);

  /// \brief Get the MBS index of the MBS Session.
  /// \param[in] mbs_session_id The MBS Session ID.
  mbs_index_t get_mbs_index(mbs_session_id_t mbs_session_id);

  /// \brief Get the number of MBS Sessions.
  /// \return Number of MBS Sessions.
  size_t get_nof_mbs_sessions() const { return mbs_sessions.size(); }

  /// \brief Find the MBS Session with the given MBS index.
  /// \param[in] mbs_index Index of the MBS Session to be found.
  /// \return Pointer to the MBS Session if found, nullptr otherwise.
  cu_cp_mbs_session* find_mbs_session(mbs_index_t mbs_index);

  // ngap

  /// \brief Allocate resources for the MBS Session in the CU-CP.
  mbs_index_t add_mbs_session(mbs_session_id_t mbs_session_id);

  /// \brief Schedule an MBS Session associated task.
  bool schedule_mbs_task(async_task<void> task) {
    return mbs_common_task_sched.schedule_async_task(std::move(task));
  }

protected:
  mbs_index_t next_mbs_index = mbs_index_t::min;

private:
  /// \brief Get the next available MBS index.
  /// \return The MBS index.
  mbs_index_t allocate_mbs_index();

  inline void increase_next_mbs_index()
  {
    if (next_mbs_index == mbs_index_t::max) {
      // reset cu mbs id counter
      next_mbs_index = mbs_index_t::min;
    } else {
      // increase CU-CP MBS id counter
      next_mbs_index = uint_to_mbs_index(mbs_index_to_uint(next_mbs_index) + 1);
    }
  }
  srslog::basic_logger& logger = srslog::fetch_basic_logger("CU-CP-MBSMNG");
  const unsigned        max_nof_mbs_sessions;

  // Common task scheduler for MBS Sessions.
  mbs_common_task_scheduler mbs_common_task_sched;

  // Container of MBS Session contexts handled by the CU-CP.
  std::unordered_map<mbs_index_t, cu_cp_mbs_session> mbs_sessions;

  // mbs_index lookups
  std::map<mbs_session_id_t, mbs_index_t> mbs_session_id_to_mbs_index; // mbs_indexes indexed by MBS Sesion ID
};

} // namespace srs_cu_cp
} // namespace srsran
