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

#include "mbs_session_context.h"

using namespace srsran;
using namespace srs_cu_up;

mbs_session_context::mbs_session_context(mbs_index_t                      mbs_index_,
                                         mbs_session_context_cfg          cfg_,
                                         mbs_session_id_t                 mbs_session_id_,
                                         std::optional<area_session_id_t> area_session_id_) :
  mbs_index(mbs_index_),
  cfg(cfg_),
  mbs_session_id(mbs_session_id_),
  area_session_id(area_session_id_)
{
}
}
