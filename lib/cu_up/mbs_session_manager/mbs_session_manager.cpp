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

#include "mbs_session_manager.h"

using namespace srsran;
using namespace srs_cu_up;

mbs_session_manager::mbs_session_manager(const mbs_session_manager_config& config,
                       const mbs_session_manager_dependencies& dependencies) :
  n3_config(config.n3_config),
  test_mode_config(config.test_mode_config),
  max_nof_mbs_sessions(config.mbs_config.max_nof_mbs_sessions),
  ngu_session_mngr(dependencies.ngu_session_mngr),
  f1u_gw(dependencies.f1u_gw),
  gtpu_rx_demux(dependencies.gtpu_rx_demux),
  f1u_teid_allocator(dependencies.f1u_teid_allocator),
  timers(dependencies.timers),
  exec_pool(dependencies.exec_pool),
  gtpu_pcap(dependencies.gtpu_pcap)
{
}

async_task<void> mbs_session_manager::stop()
{
  // Stop GTP-U demux.
  gtpu_rx_demux.stop();

  // Remove all the MBS Sessions
  auto mbs_session_it = mbs_session_db.begin();

  return launch_async([this, mbs_session_it](coro_context<async_task<void>>& ctx) mutable {
    CORO_BEGIN(ctx);
      while (mbs_session_it != mbs_session_db.end()) {
        mbs_index_t mbs_index = (mbs_session_it++)->first;
        mbs_session_context* mbs_session_ctxt = find_mbs_session(mbs_index);
        if (mbs_session_ctxt == nullptr) {
            logger.error("Cannot remove MBS Session context, could not find MBS Session");
        } else {
          remove_mbs_session(mbs_index);
        }
      }
    CORO_RETURN();
  });
}

mbs_session_context*
mbs_session_manager::add_mbs_session(const mbs_session_context_cfg& mbs_session_cfg,
    const mbs_session_id_t mbs_session_id, std::optional<area_session_id_t> area_session_id)
{
  if (mbs_session_db.size() >= max_nof_mbs_sessions) {
    logger.error(
        "CU-UP MBS Session creation Failed. Cause: Maximum number of MBS Sessions supported by the CU-UP ({}) has been reached",
        max_nof_mbs_sessions);
    fmt::print("CU-UP MBS Session creation failed. Cause: Maximum number of MBS Sessions supported by the CU-UP ({}) has been reached. "
               "To increase the number of supported "
               "MBS Sessions change the \"--max_nof_mbs_sessions\" in the MBS configuration\n",
               max_nof_mbs_sessions);
    return nullptr;
  }

  mbs_index_t new_mbs_index = get_next_mbs_index();
  if (new_mbs_index == mbs_index_t::invalid) {
    logger.error("No free mbs_index available");
    return nullptr;
  }

  // Create MBS Session executors (these are an extension of the UE executors)
  // TODO (borieher): These should be created within the same function, so that UL, DL and CTRL executors
  //                  can point to the same executor.

  // NOTE (borieher): Ignore the UE part of the name, these are MBS Session executors
  //                  In the end, you can see an MBS Session as a special UE PDU Session that gets shared between UEs
  std::unique_ptr<ue_executor_mapper> mbs_session_exec_mapper = exec_pool.create_ue_executor_mapper();

  // Create executor-specific timer factories
  timer_factory mbs_session_dl_timer_factory =  {timers, mbs_session_exec_mapper->dl_pdu_executor()};
  timer_factory mbs_session_ul_timer_factory   = {timers, mbs_session_exec_mapper->ul_pdu_executor()};
  timer_factory mbs_session_ctrl_timer_factory = {timers, mbs_session_exec_mapper->ctrl_executor()};

  // Create CU-UP MBS Session object
  std::unique_ptr<mbs_session_context> new_mbs_session_ctxt = std::make_unique<mbs_session_context>(new_mbs_index,
                                                                                                    mbs_session_cfg,
                                                                                                    mbs_session_id,
                                                                                                    area_session_id,
                                                                                                    std::move(mbs_session_exec_mapper),
                                                                                                    ngu_session_mngr,
                                                                                                    f1u_gw,
                                                                                                    gtpu_rx_demux,
                                                                                                    f1u_teid_allocator,
                                                                                                    mbs_session_dl_timer_factory,
                                                                                                    mbs_session_ul_timer_factory,
                                                                                                    mbs_session_ctrl_timer_factory,
                                                                                                    gtpu_pcap);

  // Add to DB
  mbs_session_db.emplace(new_mbs_index, std::move(new_mbs_session_ctxt));

  logger.info("Created new CU-UP MBS Session");
  return mbs_session_db[new_mbs_index].get();
}

void mbs_session_manager::remove_mbs_session(mbs_index_t mbs_index)
{
  logger.debug("Scheduling MBS Session deletion");
  srsran_assert(mbs_session_db.find(mbs_index) != mbs_session_db.end(), "Remove MBS Session called for nonexistent MBS index");

  // TODO (borieher): Re-implement this after all the MBS Session user plane stuff

  // Move MBS context out from mbs_session_db and erase the slot
  std::unique_ptr<mbs_session_context> mbs_ctxt = std::move(mbs_session_db[mbs_index]);
  mbs_session_db.erase(mbs_index);
}

mbs_session_context* mbs_session_manager::find_mbs_session(mbs_index_t mbs_index)
{
  if (mbs_index == mbs_index_t::invalid) {
    return nullptr;
  } else {
    return mbs_session_db.find(mbs_index) != mbs_session_db.end() ? mbs_session_db[mbs_index].get() : nullptr;
  }
}

mbs_index_t mbs_session_manager::get_next_mbs_index()
{
 // Search unallocated MBS index
 for (uint64_t i = 0; i < MAX_NOF_CU_MBS; i++) {
    if (mbs_session_db.find(static_cast<mbs_index_t>(i)) == mbs_session_db.end()) {
      return uint_to_mbs_index(i);
      break;
    }
  }
  return mbs_index_t::invalid;
}
