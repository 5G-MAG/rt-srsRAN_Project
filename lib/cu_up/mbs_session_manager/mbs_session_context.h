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

#include "srsran/cu_up/cu_up_types.h"
#include <map>

namespace srsran::srs_cu_up {

/// \brief MBS Session context setup configuration
struct mbs_session_context_cfg {
  std::map<five_qi_t, srs_cu_up::cu_up_qos_config> qos;
};

/// \brief Context for an MBS Session within the CU-UP.
class mbs_session_context
{
public:
  mbs_session_context(mbs_index_t             mbs_index_,
                      mbs_session_context_cfg cfg_) :
    mbs_index(mbs_index_),
    cfg(cfg_)
{
}
  ~mbs_session_context() = default;

  [[nodiscard]] mbs_index_t get_index() const { return mbs_index; }

private:
  mbs_index_t             mbs_index;
  mbs_session_context_cfg cfg;
};

} // namespace srsran::srs_cu_up
