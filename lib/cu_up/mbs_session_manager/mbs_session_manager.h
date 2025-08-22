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

#include "mbs_session_manager_interfaces.h"
#include "srsran/ran/mbs.h"
#include "srsran/cu_up/cu_up_config.h"
#include "srsran/srslog/srslog.h"
#include "srsran/support/async/fifo_async_task_scheduler.h"

namespace srsran {

namespace srs_cu_up {

class mbs_common_task_scheduler
{
public:
  mbs_common_task_scheduler() : main_ctrl_loop(128) {}

  bool schedule_async_task(async_task<void> task) { return main_ctrl_loop.schedule(std::move(task)); }

  void stop() { main_ctrl_loop.request_stop(); }

private:
  // mbs task event loop
  fifo_async_task_scheduler main_ctrl_loop;
};

/// MBS Session manager configuration.
struct mbs_session_manager_config {
  const n3_interface_config&    n3_config;
  const srsran::mbs_config&     mbs_config;
};

/// MBS Session manager dependencies.
struct mbs_session_manager_dependencies {};

class mbs_session_manager : public mbs_session_manager_ctrl
{
public:
  explicit mbs_session_manager(const mbs_session_manager_config& config, const mbs_session_manager_dependencies& dependencies);

  using mbs_session_db_t       = std::unordered_map<mbs_index_t, std::unique_ptr<mbs_session_context>>;
  const mbs_session_db_t& get_mbs_sessions() const { return mbs_session_db; }

  async_task<void> stop()                                                               override;
  mbs_session_context* add_mbs_session(const mbs_session_context_cfg& mbs_session_cfg)  override;
  void remove_mbs_session(mbs_index_t mbs_index)                                        override;
  mbs_session_context* find_mbs_session(mbs_index_t mbs_index)                          override;
  size_t               get_nof_mbs_sessions() const override { return mbs_session_db.size(); }

  /// \brief Schedule an MBS Session associated task.
  bool schedule_mbs_task(async_task<void> task) {
    return mbs_common_task_sched.schedule_async_task(std::move(task));
  }

private:
  /// \brief Get the next available MBS index.
  /// \return The MBS index.
  mbs_index_t get_next_mbs_index();

  const n3_interface_config&    n3_config;
  const unsigned                max_nof_mbs_sessions;
  // Common task scheduler for MBS Sessions.
  mbs_common_task_scheduler     mbs_common_task_sched;
  mbs_session_db_t              mbs_session_db;
  srslog::basic_logger&         logger = srslog::fetch_basic_logger("CU-UP-MBSMNG");
};

} // namespace srs_cu_up
} // namespace srsran
