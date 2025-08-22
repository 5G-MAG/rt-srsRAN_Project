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

#include "mbs_session_context.h"
#include "srsran/support/async/async_task.h"

namespace srsran {

namespace srs_cu_up {

class mbs_session_manager_ctrl
{
public:
  virtual ~mbs_session_manager_ctrl() = default;

  virtual async_task<void> stop()                                                               = 0;
  virtual mbs_session_context* add_mbs_session(const mbs_session_context_cfg& mbs_session_cfg)  = 0;
  virtual void remove_mbs_session(mbs_index_t mbs_index)                                        = 0;
  virtual mbs_session_context* find_mbs_session(mbs_index_t mbs_index)                          = 0;
  virtual size_t               get_nof_mbs_sessions() const                                     = 0;
};

} // namespace srs_cu_up

} // namespace srsran
