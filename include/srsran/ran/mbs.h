/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
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
#include <string>
#include <variant>

namespace srsran {

struct mbs_fsai_inter_freq_t {
  /// Provides the ARFCN of the neighbour cell providing MBS services.
  uint32_t dl_carrier_freq; // Max value 3279165.
  /// List of MBS FSAIs in the neighbour cell.
  std::vector<uint32_t> mbs_fsai_lst; // List of máximum 8 elements of OCTECT STRING(SIZE(3))
};


struct mbs_config {
  /// SIB 21 values 

  /// Vector to store the intra frequency selection area identities passed from the conf file. Maximum length 64 elements, and max value each 16777215 (3 bytes).
  std::vector<uint32_t> fsai_intra;  // TODO (JAISANRO) put a default AERFCN for MBS intra service.

  /// Vector to store the inter frequency selection area identities passed from the conf file.
  std::vector<mbs_fsai_inter_freq_t> mbs_inter_lst; // We can have a set of inter neighboors with multiple fsai, i.e. a vector of neighboors with a vector of fsai
};

} // namespace srsran
