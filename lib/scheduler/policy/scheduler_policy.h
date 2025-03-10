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

#include "../slicing/ran_slice_candidate.h"
#include "ue_allocator.h"

namespace srsran {

/// This struct provides a view of the current resource grid state to the PDSCH and PUSCH allocators.
class ue_resource_grid_view
{
public:
  void add_cell(const cell_resource_allocator& cell_grid)
  {
    cell_res_grids.emplace(cell_grid.cell_index(), &cell_grid);
  }

  slot_point get_pdcch_slot(du_cell_index_t cell_index) const { return cell_res_grids[cell_index]->slot_tx(); }

  slot_point get_pusch_slot(du_cell_index_t cell_index, unsigned k2) const
  {
    return (*cell_res_grids[cell_index])[k2].slot;
  }

  const cell_configuration& get_cell_cfg_common(du_cell_index_t cell_index) const
  {
    return cell_res_grids[cell_index]->cfg;
  }

  span<const pdcch_dl_information> get_dl_pdcch_sched_results(du_cell_index_t cell_index) const
  {
    return (*cell_res_grids[cell_index])[0].result.dl.dl_pdcchs;
  }
  span<const dl_msg_alloc> get_ue_pdsch_sched_results(du_cell_index_t cell_index, slot_point pdsch_slot) const
  {
    return (*cell_res_grids[cell_index])[pdsch_slot].result.dl.ue_grants;
  }
  span<const ul_sched_info> get_ue_pusch_sched_results(du_cell_index_t cell_index, slot_point pusch_slot) const
  {
    return (*cell_res_grids[cell_index])[pusch_slot].result.ul.puschs;
  }

  const cell_resource_allocator& get_grid(du_cell_index_t cell_index) const { return *cell_res_grids[cell_index]; }

  const cell_slot_resource_grid& get_pdcch_grid(du_cell_index_t cell_index) const
  {
    return (*cell_res_grids[cell_index])[0].dl_res_grid;
  }

  const cell_slot_resource_grid& get_pdsch_grid(du_cell_index_t cell_index, unsigned k0) const
  {
    return (*cell_res_grids[cell_index])[k0].dl_res_grid;
  }

  const cell_slot_resource_grid& get_pusch_grid(du_cell_index_t cell_index, unsigned k2) const
  {
    return (*cell_res_grids[cell_index])[k2].ul_res_grid;
  }

  span<const dl_msg_alloc> get_ue_pdsch_grants(du_cell_index_t cell_index, unsigned k0) const
  {
    return (*cell_res_grids[cell_index])[k0].result.dl.ue_grants;
  }

  bool has_ue_dl_pdcch(du_cell_index_t cell_index, rnti_t rnti) const
  {
    const auto& pdcchs = (*cell_res_grids[cell_index])[0].result.dl.dl_pdcchs;
    return std::any_of(
        pdcchs.begin(), pdcchs.end(), [rnti](const pdcch_dl_information& pdcch) { return pdcch.ctx.rnti == rnti; });
  }

  bool has_ue_ul_pdcch(du_cell_index_t cell_index, rnti_t rnti) const
  {
    const auto& pdcchs = (*cell_res_grids[cell_index])[0].result.dl.ul_pdcchs;
    return std::any_of(
        pdcchs.begin(), pdcchs.end(), [rnti](const pdcch_ul_information& pdcch) { return pdcch.ctx.rnti == rnti; });
  }

  bool has_ue_dl_grant(du_cell_index_t cell_index, rnti_t rnti, unsigned k0) const
  {
    const auto& grants = (*cell_res_grids[cell_index])[k0].result.dl.ue_grants;
    return std::any_of(
        grants.begin(), grants.end(), [rnti](const auto& grant) { return grant.pdsch_cfg.rnti == rnti; });
  }

  bool has_ue_ul_grant(du_cell_index_t cell_index, rnti_t rnti, unsigned k2) const
  {
    const auto& puschs = (*cell_res_grids[cell_index])[k2].result.ul.puschs;
    return std::any_of(
        puschs.begin(), puschs.end(), [rnti](const auto& pusch) { return pusch.pusch_cfg.rnti == rnti; });
  }

  unsigned nof_cells() const { return cell_res_grids.size(); }

private:
  slotted_array<const cell_resource_allocator*, MAX_NOF_DU_CELLS> cell_res_grids;
};

/// Contextual information used by the scheduler policy to make decisions for a slot and slice in DL.
struct dl_sched_context {
  /// PDSCH grant allocator. This object provides a handle to allocate PDSCH grants in the gNB resource grid.
  ue_pdsch_allocator& pdsch_alloc;
  /// View of the current resource grid occupancy state for all gnb cells.
  const ue_resource_grid_view& res_grid;
  /// Slice candidate to be scheduled in the given slot.
  dl_ran_slice_candidate& slice_candidate;
  /// List of DL HARQs pending retransmissions.
  dl_harq_pending_retx_list harq_pending_retx_list;
};

/// Contextual information used by the scheduler policy to make decisions for a slot and slice in UL.
struct ul_sched_context {
  /// PUSCH grant allocator. This object provides a handle to allocate PUSCH grants in the gNB resource grid.
  ue_pusch_allocator& pusch_alloc;
  /// View of the current resource grid occupancy state for all gnb cells.
  const ue_resource_grid_view& res_grid;
  /// Slice candidate to be scheduled in the given slot.
  ul_ran_slice_candidate& slice_candidate;
  /// List of UL HARQs pending retransmissions.
  ul_harq_pending_retx_list harq_pending_retx_list;
};

/// Interface of data scheduler that is used to allocate UE DL and UL grants in a given slot
/// The data_scheduler object will be common to all cells and slots.
class scheduler_policy
{
public:
  virtual ~scheduler_policy() = default;

  /// Schedule UE DL grants for a given slot and one or more cells.
  virtual void dl_sched(dl_sched_context dl_ctxt) = 0;

  /// Schedule UE UL grants for a given {slot, cell}.
  virtual void ul_sched(ul_sched_context ul_ctxt) = 0;
};

} // namespace srsran
