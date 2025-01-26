/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
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

#include "srsran/ofh/compression/compression_params.h"
#include "srsran/ofh/serdes/ofh_message_properties.h"
#include "srsran/ran/slot_point.h"

namespace srsran {
namespace ofh {

/// Open Fronthaul User-Plane message parameters.
struct uplane_message_params {
  /// Data direction.
  data_direction direction;
  /// Frame, subframe and slot information.
  slot_point slot;
  /// Filter index.
  filter_index_type filter_index;
  /// Initial PRB index used in the given /c symbol_id.
  unsigned start_prb;
  /// Number of PRBs in this symbol.
  unsigned nof_prb;
  /// Symbol identifier.
  unsigned symbol_id;
  /// Section type of the message to be built.
  section_type sect_type;
  /// IQ data compression parameters.
  ru_compression_params compression_params;
};

} // namespace ofh
} // namespace srsran
