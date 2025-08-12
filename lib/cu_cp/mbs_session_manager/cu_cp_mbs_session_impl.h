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
#include "cu_cp_mbs_session_impl_interface.h"
#include "srsran/ran/mbs.h"

namespace srsran {

namespace srs_cu_cp {

/// \brief Context of a CU-CP MBS Session.
struct cu_cp_mbs_session_context {
  mbs_index_t mbs_index = mbs_index_t::invalid;
};

class cu_cp_mbs_session : public cu_cp_mbs_session_impl_interface
{
public:
  cu_cp_mbs_session(mbs_index_t mbs_index_,
                    mbs_session_id_t mbs_session_id_);

  /// \brief Get the MBS index of the MBS Session.
  mbs_index_t get_mbs_index() const override { return mbs_index; }

  /// \brief Get the MBS Session ID of the MBS Session.
  [[nodiscard]] mbs_session_id_t get_mbs_session_id() const { return mbs_session_id; }

  cu_cp_mbs_session_context& get_mbs_context() { return mbs_ctxt; }
  [[nodiscard]] const cu_cp_mbs_session_context& get_mbs_context() const { return mbs_ctxt; }

private:
  // Common context.
  mbs_index_t      mbs_index = mbs_index_t::invalid;
  mbs_session_id_t mbs_session_id;

  cu_cp_mbs_session_context mbs_ctxt;

  // NGAP MBS Session context.
};


} // namespace srs_cu_cp
} // namespace srsran
