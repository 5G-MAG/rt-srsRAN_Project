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

#include "scheduler_time_qos.h"
#include "../support/csi_report_helpers.h"

using namespace srsran;

// [Implementation-defined] Limit for the coefficient of the proportional fair metric to avoid issues with double
// imprecision.
constexpr unsigned MAX_PF_COEFF = 10;

// [Implementation-defined] Maximum number of slots skipped between scheduling opportunities.
constexpr unsigned MAX_SLOT_SKIPPED = 20;

scheduler_time_qos::scheduler_time_qos(const scheduler_ue_expert_config& expert_cfg_) :
  params(std::get<time_qos_scheduler_expert_config>(expert_cfg_.strategy_cfg))
{
}

dl_alloc_result scheduler_time_qos::schedule_dl_retxs(dl_sched_context ctxt)
{
  const ran_slice_id_t slice_id = ctxt.slice_candidate.id();
  auto&                ue_db    = ctxt.slice_candidate.get_slice_ues();

  for (auto it = ctxt.harq_pending_retx_list.begin(); it != ctxt.harq_pending_retx_list.end();) {
    // Note: During retx alloc, the pending HARQ list will mutate. So, we prefetch the next node.
    auto prev_it = it++;
    auto h       = *prev_it;
    if (h.get_grant_params().slice_id != slice_id or not ue_db.contains(h.ue_index())) {
      continue;
    }
    const slice_ue& u = ue_db[h.ue_index()];

    // Prioritize PCell over SCells.
    for (unsigned i = 0; i != u.nof_cells(); ++i) {
      const ue_cell& ue_cc = u.get_cell(to_ue_cell_index(i));
      srsran_assert(ue_cc.is_active() and not ue_cc.is_in_fallback_mode(),
                    "Policy scheduler called for UE={} in fallback",
                    fmt::underlying(ue_cc.ue_index));

      // [Implementation-defined] Skip UE if PDCCH is already allocated for this UE in this slot.
      if (ctxt.res_grid.has_ue_dl_pdcch(ue_cc.cell_index, u.crnti()) or
          not ue_cc.is_pdcch_enabled(ctxt.res_grid.get_pdcch_slot(ue_cc.cell_index))) {
        continue;
      }

      ue_pdsch_grant        grant{&u, ue_cc.cell_index, h.id()};
      const dl_alloc_result result = ctxt.pdsch_alloc.allocate_dl_grant(grant);
      // Continue iteration until skip slot indication is received.
      // NOTE: Allocation status other than skip_slot can be ignored because allocation of reTxs is done from oldest
      // HARQ pending to newest. Hence, other allocation status are redundant.
      if (result.status == alloc_status::skip_slot) {
        return result;
      }
    }
  }

  // Return successful outcome in all other cases.
  // Other cases:
  //  - No pending HARQs to allocate.
  //  - At the end of pending HARQs iteration.
  return {alloc_status::success};
}

void scheduler_time_qos::dl_sched(dl_sched_context ctxt)
{
  slot_point pdsch_slot      = ctxt.slice_candidate.get_slot_tx();
  unsigned nof_slots_elapsed = std::min(last_pdsch_slot.valid() ? pdsch_slot - last_pdsch_slot : 1U, MAX_SLOT_SKIPPED);
  last_pdsch_slot            = pdsch_slot;

  const slice_ue_repository& ues = ctxt.slice_candidate.get_slice_ues();

  // Remove deleted users from history.
  for (auto it = ue_history_db.begin(); it != ue_history_db.end();) {
    if (not ues.contains(it->ue_index)) {
      ue_history_db.erase(it->ue_index);
    }
    ++it;
  }
  // Add new users to history db, and update DL priority queue.
  for (const auto& u : ues) {
    if (not ue_history_db.contains(u.ue_index())) {
      // TODO: Consider other cells when carrier aggregation is supported.
      ue_history_db.emplace(u.ue_index(), ue_ctxt{u.ue_index(), u.get_pcell().cell_index, this});
    }
  }

  // Schedule HARQ retxs.
  auto retx_result = schedule_dl_retxs(ctxt);
  if (retx_result.status == alloc_status::skip_slot) {
    return;
  }

  // Update DL priority queue.
  dl_queue.clear();
  for (const auto& u : ues) {
    ue_ctxt& uectxt = ue_history_db[u.ue_index()];
    uectxt.compute_dl_prio(u,
                           ctxt.slice_candidate.id(),
                           ctxt.res_grid.get_pdcch_slot(u.get_pcell().cell_index),
                           pdsch_slot,
                           nof_slots_elapsed);
    dl_queue.push(&uectxt);
  }

  dl_alloc_result alloc_result = {alloc_status::invalid_params};
  unsigned        rem_rbs      = ctxt.slice_candidate.remaining_rbs();
  while (not dl_queue.empty() and rem_rbs > 0 and alloc_result.status != alloc_status::skip_slot) {
    ue_ctxt& ue  = *dl_queue.top();
    alloc_result = try_dl_alloc(ue, ues, ctxt.pdsch_alloc, rem_rbs);
    dl_queue.pop();
    rem_rbs = ctxt.slice_candidate.remaining_rbs();
  }
}

ul_alloc_result scheduler_time_qos::schedule_ul_retxs(ul_sched_context ctxt)
{
  auto& ue_db = ctxt.slice_candidate.get_slice_ues();

  for (auto it = ctxt.harq_pending_retx_list.begin(); it != ctxt.harq_pending_retx_list.end();) {
    // Note: During retx alloc, the pending HARQ list will mutate. So, we prefetch the next node.
    auto prev_it = it++;
    auto h       = *prev_it;
    if (h.get_grant_params().slice_id != ctxt.slice_candidate.id() or not ue_db.contains(h.ue_index())) {
      continue;
    }
    const slice_ue& u = ue_db[h.ue_index()];
    // Prioritize PCell over SCells.
    for (unsigned i = 0; i != u.nof_cells(); ++i) {
      const ue_cell& ue_cc = u.get_cell(to_ue_cell_index(i));
      srsran_assert(ue_cc.is_active() and not ue_cc.is_in_fallback_mode(),
                    "Policy scheduler called for UE={} in fallback",
                    fmt::underlying(ue_cc.ue_index));

      if (not ue_cc.is_pdcch_enabled(ctxt.res_grid.get_pdcch_slot(ue_cc.cell_index)) or
          not ue_cc.is_ul_enabled(ctxt.slice_candidate.get_slot_tx())) {
        // Either the PDCCH slot or PUSCH slots are not available.
        continue;
      }

      ue_pusch_grant        grant{&u, ue_cc.cell_index, h.id()};
      const ul_alloc_result result = ctxt.pusch_alloc.allocate_ul_grant(grant);
      // Continue iteration until skip slot indication is received.
      // NOTE: Allocation status other than skip_slot can be ignored because allocation of reTxs is done from oldest
      // HARQ pending to newest. Hence, other allocation status are redundant.
      if (result.status == alloc_status::skip_slot) {
        return result;
      }
    }
  }
  // Return successful outcome in all other cases.
  // Other cases:
  //  - No pending HARQs to allocate.
  //  - At the end of pending HARQs iteration.
  return {alloc_status::success};
}

void scheduler_time_qos::ul_sched(ul_sched_context ul_ctxt)
{
  slot_point pusch_slot      = ul_ctxt.slice_candidate.get_slot_tx();
  unsigned nof_slots_elapsed = std::min(last_pusch_slot.valid() ? pusch_slot - last_pusch_slot : 1U, MAX_SLOT_SKIPPED);
  last_pusch_slot            = pusch_slot;

  const slice_ue_repository& ues = ul_ctxt.slice_candidate.get_slice_ues();
  // Remove deleted users from history.
  for (auto it = ue_history_db.begin(); it != ue_history_db.end();) {
    if (not ues.contains(it->ue_index)) {
      ue_history_db.erase(it->ue_index);
    }
    ++it;
  }
  // Add new users to history db, and update UL priority queue.
  for (const auto& u : ues) {
    if (not ue_history_db.contains(u.ue_index())) {
      // TODO: Consider other cells when carrier aggregation is supported.
      ue_history_db.emplace(u.ue_index(), ue_ctxt{u.ue_index(), u.get_pcell().cell_index, this});
    }
  }

  // Schedule HARQ retxs.
  auto retx_result = schedule_ul_retxs(ul_ctxt);
  if (retx_result.status == alloc_status::skip_slot) {
    return;
  }

  // Clear the existing contents of the queue.
  ul_queue.clear();
  for (const auto& u : ues) {
    ue_ctxt& ctxt = ue_history_db[u.ue_index()];
    ctxt.compute_ul_prio(u,
                         ul_ctxt.slice_candidate.id(),
                         ul_ctxt.res_grid.get_pdcch_slot(u.get_pcell().cell_index),
                         ul_ctxt.slice_candidate.get_slot_tx(),
                         nof_slots_elapsed);
    ul_queue.push(&ctxt);
  }

  ul_alloc_result alloc_result = {alloc_status::invalid_params};
  unsigned        rem_rbs      = ul_ctxt.slice_candidate.remaining_rbs();
  while (not ul_queue.empty() and rem_rbs > 0 and alloc_result.status != alloc_status::skip_slot) {
    ue_ctxt& ue  = *ul_queue.top();
    alloc_result = try_ul_alloc(ue, ues, ul_ctxt.pusch_alloc, rem_rbs);
    ul_queue.pop();
    rem_rbs = ul_ctxt.slice_candidate.remaining_rbs();
  }
}

dl_alloc_result scheduler_time_qos::try_dl_alloc(ue_ctxt&                   ctxt,
                                                 const slice_ue_repository& ues,
                                                 ue_pdsch_allocator&        pdsch_alloc,
                                                 unsigned                   max_rbs)
{
  dl_alloc_result alloc_result = {alloc_status::invalid_params};
  ue_pdsch_grant  grant{&ues[ctxt.ue_index], ctxt.cell_index};

  grant.h_id                  = INVALID_HARQ_ID;
  grant.recommended_nof_bytes = ues[ctxt.ue_index].pending_dl_newtx_bytes();
  grant.max_nof_rbs           = max_rbs;
  alloc_result                = pdsch_alloc.allocate_dl_grant(grant);
  if (alloc_result.status == alloc_status::success) {
    ctxt.dl_prio = forbid_prio;
  }
  ctxt.save_dl_alloc(alloc_result.alloc_bytes, alloc_result.tb_info);
  return alloc_result;
}

ul_alloc_result scheduler_time_qos::try_ul_alloc(ue_ctxt&                   ctxt,
                                                 const slice_ue_repository& ues,
                                                 ue_pusch_allocator&        pusch_alloc,
                                                 unsigned                   max_rbs)
{
  ul_alloc_result alloc_result = {alloc_status::invalid_params};
  ue_pusch_grant  grant{&ues[ctxt.ue_index], ctxt.cell_index};

  grant.h_id                  = INVALID_HARQ_ID;
  grant.recommended_nof_bytes = ues[ctxt.ue_index].pending_ul_newtx_bytes();
  grant.max_nof_rbs           = max_rbs;
  alloc_result                = pusch_alloc.allocate_ul_grant(grant);
  if (alloc_result.status == alloc_status::success) {
    ctxt.ul_prio = forbid_prio;
  }
  ctxt.save_ul_alloc(alloc_result.alloc_bytes);
  return alloc_result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace {

// [Implementation-defined] Helper value to set a maximum metric weight that is low enough to avoid overflows during
// the final QoS weight computation.
constexpr double max_metric_weight = 1.0e12;

double compute_pf_metric(double estim_rate, double avg_rate, double fairness_coeff)
{
  double pf_weight = 0.0;
  if (estim_rate > 0) {
    if (avg_rate != 0) {
      if (fairness_coeff >= MAX_PF_COEFF) {
        // For very high coefficients, the pow(.) will be very high, leading to pf_weight of 0 due to lack of precision.
        // In such scenarios, we change the way to compute the PF weight. Instead, we completely disregard the estimated
        // rate, as its impact is minimal.
        pf_weight = 1 / avg_rate;
      } else {
        pf_weight = estim_rate / pow(avg_rate, fairness_coeff);
      }
    } else {
      // In case the avg rate is zero, the division would be inf. Instead, we give the highest priority to the UE.
      pf_weight = max_metric_weight;
    }
  }
  return pf_weight;
}

double combine_qos_metrics(double                                  pf_weight,
                           double                                  gbr_weight,
                           double                                  prio_weight,
                           double                                  delay_weight,
                           const time_qos_scheduler_expert_config& policy_params)
{
  if (policy_params.qos_weight_func == time_qos_scheduler_expert_config::weight_function::gbr_prioritized and
      gbr_weight > 1.0) {
    // GBR target has not been met and we prioritize GBR over PF.
    pf_weight = std::max(1.0, pf_weight);
  }

  // The return is a combination of QoS priority, GBR and PF weight functions.
  return gbr_weight * pf_weight * prio_weight * delay_weight;
}

/// \brief Computes DL rate weight used in computation of DL priority value for a UE in a slot.
double compute_dl_qos_weights(const slice_ue&                         u,
                              double                                  estim_dl_rate,
                              double                                  avg_dl_rate,
                              slot_point                              slot_tx,
                              const time_qos_scheduler_expert_config& policy_params)
{
  if (avg_dl_rate == 0) {
    // Highest priority to UEs that have not yet received any allocation.
    return std::numeric_limits<double>::max();
  }

  uint8_t min_prio_level = qos_prio_level_t::max();
  double  gbr_weight     = 0;
  double  delay_weight   = 0;
  if (policy_params.gbr_enabled or policy_params.priority_enabled or policy_params.pdb_enabled) {
    for (const logical_channel_config& lc : u.logical_channels()) {
      if (not u.contains(lc.lcid) or not lc.qos.has_value() or u.pending_dl_newtx_bytes(lc.lcid) == 0) {
        // LC is not part of the slice, No QoS config was provided for this LC or there is no pending data for this LC
        continue;
      }

      // Track the LC with the lowest priority.
      min_prio_level = std::min(lc.qos->qos.priority.value(), min_prio_level);

      slot_point hol_toa = u.dl_hol_toa(lc.lcid);
      if (hol_toa.valid() and slot_tx >= hol_toa) {
        const unsigned hol_delay_ms = (slot_tx - hol_toa) / slot_tx.nof_slots_per_subframe();
        const unsigned pdb          = lc.qos->qos.packet_delay_budget_ms;
        delay_weight += hol_delay_ms / static_cast<double>(pdb);
      }

      if (not lc.qos->gbr_qos_info.has_value()) {
        // LC is a non-GBR flow.
        continue;
      }

      // GBR flow.
      double dl_avg_rate = u.dl_avg_bit_rate(lc.lcid);
      if (dl_avg_rate != 0) {
        gbr_weight += std::min(lc.qos->gbr_qos_info->gbr_dl / dl_avg_rate, max_metric_weight);
      } else {
        gbr_weight += max_metric_weight;
      }
    }
  }

  // If no QoS flows are configured, the weight is set to 1.0.
  gbr_weight   = policy_params.gbr_enabled and gbr_weight != 0 ? gbr_weight : 1.0;
  delay_weight = policy_params.pdb_enabled and delay_weight != 0 ? delay_weight : 1.0;

  double pf_weight   = compute_pf_metric(estim_dl_rate, avg_dl_rate, policy_params.pf_fairness_coeff);
  double prio_weight = policy_params.priority_enabled ? (qos_prio_level_t::max() + 1 - min_prio_level) /
                                                            static_cast<double>(qos_prio_level_t::max() + 1)
                                                      : 1.0;

  // The return is a combination of QoS priority, GBR and PF weight functions.
  return combine_qos_metrics(pf_weight, gbr_weight, prio_weight, delay_weight, policy_params);
}

/// \brief Computes UL weights used in computation of UL priority value for a UE in a slot.
double compute_ul_qos_weights(const slice_ue&                         u,
                              double                                  estim_ul_rate,
                              double                                  avg_ul_rate,
                              const time_qos_scheduler_expert_config& policy_params)
{
  if (u.has_pending_sr() or avg_ul_rate == 0) {
    // Highest priority to SRs and UEs that have not yet received any allocation.
    return std::numeric_limits<double>::max();
  }

  uint8_t min_prio_level = qos_prio_level_t::max();
  double  gbr_weight     = 0;
  if (policy_params.gbr_enabled or policy_params.priority_enabled) {
    for (const logical_channel_config& lc : u.logical_channels()) {
      if (not u.contains(lc.lcid) or not lc.qos.has_value() or u.pending_ul_unacked_bytes(lc.lc_group) == 0) {
        // LC is not part of the slice or no QoS config was provided for this LC or there are no pending bytes for this
        // group.
        continue;
      }

      // Track the LC with the lowest priority.
      min_prio_level = std::min(lc.qos->qos.priority.value(), min_prio_level);

      if (not lc.qos->gbr_qos_info.has_value()) {
        // LC is a non-GBR flow.
        continue;
      }

      // GBR flow.
      lcg_id_t lcg_id  = u.get_lcg_id(lc.lcid);
      double   ul_rate = u.ul_avg_bit_rate(lcg_id);
      if (ul_rate != 0) {
        gbr_weight += std::min(lc.qos->gbr_qos_info->gbr_ul / ul_rate, max_metric_weight);
      } else {
        gbr_weight = max_metric_weight;
      }
    }
  }

  // If no GBR flows are configured, the gbr rate is set to 1.0.
  gbr_weight = policy_params.gbr_enabled and gbr_weight != 0 ? gbr_weight : 1.0;

  double pf_weight   = compute_pf_metric(estim_ul_rate, avg_ul_rate, policy_params.pf_fairness_coeff);
  double prio_weight = policy_params.priority_enabled ? (qos_prio_level_t::max() + 1 - min_prio_level) /
                                                            static_cast<double>(qos_prio_level_t::max() + 1)
                                                      : 1.0;

  return combine_qos_metrics(pf_weight, gbr_weight, prio_weight, 1.0, policy_params);
}

} // namespace

scheduler_time_qos::ue_ctxt::ue_ctxt(du_ue_index_t             ue_index_,
                                     du_cell_index_t           cell_index_,
                                     const scheduler_time_qos* parent_) :
  ue_index(ue_index_),
  cell_index(cell_index_),
  parent(parent_),
  total_dl_avg_rate_(parent->exp_avg_alpha),
  total_ul_avg_rate_(parent->exp_avg_alpha)
{
}

void scheduler_time_qos::ue_ctxt::compute_dl_prio(const slice_ue& u,
                                                  ran_slice_id_t  slice_id,
                                                  slot_point      pdcch_slot,
                                                  slot_point      pdsch_slot,
                                                  unsigned        nof_slots_elapsed)
{
  dl_prio = forbid_prio;

  // Process previous slot allocated bytes and compute average.
  compute_dl_avg_rate(u, nof_slots_elapsed);

  const ue_cell* ue_cc = u.find_cell(cell_index);
  if (ue_cc == nullptr) {
    return;
  }
  srsran_assert(ue_cc->is_active() and not ue_cc->is_in_fallback_mode(),
                "Policy scheduler called for UE={} in fallback",
                fmt::underlying(ue_cc->ue_index));
  if (not ue_cc->is_pdcch_enabled(pdcch_slot) or not ue_cc->is_pdsch_enabled(pdsch_slot)) {
    // Cannot allocate PDCCH/PDSCH for this UE in this slot.
    return;
  }
  if (not ue_cc->harqs.has_empty_dl_harqs() or not u.has_pending_dl_newtx_bytes()) {
    // No available HARQs or no pending data.
    return;
  }

  // [Implementation-defined] We consider only the SearchSpace defined in UE dedicated configuration.
  const auto* ss_info =
      ue_cc->cfg().find_search_space(ue_cc->cfg().cfg_dedicated().init_dl_bwp.pdcch_cfg->search_spaces.back().get_id());
  if (ss_info == nullptr) {
    return;
  }
  span<const pdsch_time_domain_resource_allocation> pdsch_td_res_list = ss_info->pdsch_time_domain_list;
  // [Implementation-defined] We pick the first element since PDSCH time domain resource list is sorted in descending
  // order of nof. PDSCH symbols. And, we want to calculate estimate of instantaneous achievable rate with maximum
  // nof. PDSCH symbols.
  const pdsch_time_domain_resource_allocation& pdsch_td_cfg = pdsch_td_res_list.front();

  pdsch_config_params pdsch_cfg;
  switch (ss_info->get_dl_dci_format()) {
    case dci_dl_format::f1_0:
      pdsch_cfg = get_pdsch_config_f1_0_c_rnti(ue_cc->cfg().cell_cfg_common, &ue_cc->cfg(), pdsch_td_cfg);
      break;
    case dci_dl_format::f1_1:
      pdsch_cfg =
          get_pdsch_config_f1_1_c_rnti(ue_cc->cfg(), pdsch_td_cfg, ue_cc->channel_state_manager().get_nof_dl_layers());
      break;
    default:
      report_fatal_error("Unsupported PDCCH DCI DL format");
  }

  std::optional<sch_mcs_index> mcs = ue_cc->link_adaptation_controller().calculate_dl_mcs(pdsch_cfg.mcs_table);
  if (not mcs.has_value()) {
    // CQI is either 0 or above 15, which means no DL.
    return;
  }

  // Calculate DL PF priority.
  // NOTE: Estimated instantaneous DL rate is calculated assuming entire BWP CRBs are allocated to UE.
  const double estimated_rate = ue_cc->get_estimated_dl_rate(pdsch_cfg, mcs.value(), ss_info->dl_crb_lims.length());
  const double current_total_avg_rate = total_dl_avg_rate();
  dl_prio = compute_dl_qos_weights(u, estimated_rate, current_total_avg_rate, pdcch_slot, parent->params);
}

void scheduler_time_qos::ue_ctxt::compute_ul_prio(const slice_ue& u,
                                                  ran_slice_id_t  slice_id,
                                                  slot_point      pdcch_slot,
                                                  slot_point      pusch_slot,
                                                  unsigned        nof_slots_elapsed)
{
  ul_prio = forbid_prio;

  // Process bytes allocated in previous slot and compute average.
  compute_ul_avg_rate(u, nof_slots_elapsed);

  const ue_cell* ue_cc = u.find_cell(cell_index);
  if (ue_cc == nullptr) {
    return;
  }
  srsran_assert(ue_cc->is_active() and not ue_cc->is_in_fallback_mode(),
                "Policy scheduler called for UE={} in fallback",
                fmt::underlying(ue_cc->ue_index));
  if (not ue_cc->is_pdcch_enabled(pdcch_slot) or not ue_cc->is_ul_enabled(pusch_slot)) {
    // Cannot allocate PDCCH/PUSCH for this UE in the provided slots.
    return;
  }
  if (not ue_cc->harqs.has_empty_ul_harqs()) {
    // No HARQs for newTxs.
    return;
  }

  if (u.pending_ul_newtx_bytes() == 0) {
    // No new bytes to allocate.
    return;
  }

  // [Implementation-defined] We consider only the SearchSpace defined in UE dedicated configuration.
  const auto* ss_info =
      ue_cc->cfg().find_search_space(ue_cc->cfg().cfg_dedicated().init_dl_bwp.pdcch_cfg->search_spaces.back().get_id());
  if (ss_info == nullptr) {
    return;
  }

  span<const pusch_time_domain_resource_allocation> pusch_td_res_list = ss_info->pusch_time_domain_list;
  // [Implementation-defined] We pick the first element since PUSCH time domain resource list is sorted in descending
  // order of nof. PUSCH symbols. And, we want to calculate estimate of instantaneous achievable rate with maximum
  // nof. PUSCH symbols.
  const pusch_time_domain_resource_allocation& pusch_td_cfg = pusch_td_res_list.front();
  // [Implementation-defined] We assume nof. HARQ ACK bits is zero at PUSCH slot as a simplification in calculating
  // estimated instantaneous achievable rate.
  constexpr unsigned nof_harq_ack_bits = 0;
  const bool is_csi_report_slot = csi_helper::is_csi_reporting_slot(u.get_pcell().cfg().cfg_dedicated(), pusch_slot);

  pusch_config_params pusch_cfg;
  switch (ss_info->get_ul_dci_format()) {
    case dci_ul_format::f0_0:
      pusch_cfg = get_pusch_config_f0_0_c_rnti(ue_cc->cfg().cell_cfg_common,
                                               &ue_cc->cfg(),
                                               ue_cc->cfg().cell_cfg_common.ul_cfg_common.init_ul_bwp,
                                               pusch_td_cfg,
                                               nof_harq_ack_bits,
                                               is_csi_report_slot);
      break;
    case dci_ul_format::f0_1:
      pusch_cfg = get_pusch_config_f0_1_c_rnti(ue_cc->cfg(),
                                               pusch_td_cfg,
                                               ue_cc->channel_state_manager().get_nof_ul_layers(),
                                               nof_harq_ack_bits,
                                               is_csi_report_slot);
      break;
    default:
      report_fatal_error("Unsupported PDCCH DCI UL format");
  }

  sch_mcs_index mcs = ue_cc->link_adaptation_controller().calculate_ul_mcs(pusch_cfg.mcs_table);

  // Calculate UL PF priority.
  // NOTE: Estimated instantaneous UL rate is calculated assuming entire BWP CRBs are allocated to UE.
  const double estimated_rate   = ue_cc->get_estimated_ul_rate(pusch_cfg, mcs.value(), ss_info->ul_crb_lims.length());
  const double current_avg_rate = total_ul_avg_rate();

  // Compute LC weight function.
  ul_prio = compute_ul_qos_weights(u, estimated_rate, current_avg_rate, parent->params);
}

void scheduler_time_qos::ue_ctxt::compute_dl_avg_rate(const slice_ue& u, unsigned nof_slots_elapsed)
{
  // In case more than one slot elapsed.
  if (nof_slots_elapsed > 1) {
    total_dl_avg_rate_.push_zeros(nof_slots_elapsed - 1);
  }

  // Compute DL average rate of the UE.
  total_dl_avg_rate_.push(dl_sum_alloc_bytes);

  // Flush allocated bytes for the current slot.
  dl_sum_alloc_bytes = 0;
}

void scheduler_time_qos::ue_ctxt::compute_ul_avg_rate(const slice_ue& u, unsigned nof_slots_elapsed)
{
  // In case more than one slot elapsed.
  if (nof_slots_elapsed > 1) {
    total_ul_avg_rate_.push_zeros(nof_slots_elapsed - 1);
  }

  // Compute UL average rate of the UE.
  total_ul_avg_rate_.push(ul_sum_alloc_bytes);

  // Flush allocated bytes for the current slot.
  ul_sum_alloc_bytes = 0;
}

void scheduler_time_qos::ue_ctxt::save_dl_alloc(uint32_t total_alloc_bytes, const dl_msg_tb_info& tb_info)
{
  dl_sum_alloc_bytes += total_alloc_bytes;
}

void scheduler_time_qos::ue_ctxt::save_ul_alloc(unsigned alloc_bytes)
{
  if (alloc_bytes == 0) {
    return;
  }
  ul_sum_alloc_bytes += alloc_bytes;
}

bool scheduler_time_qos::ue_dl_prio_compare::operator()(const scheduler_time_qos::ue_ctxt* lhs,
                                                        const scheduler_time_qos::ue_ctxt* rhs) const
{
  return lhs->dl_prio < rhs->dl_prio;
}

bool scheduler_time_qos::ue_ul_prio_compare::operator()(const scheduler_time_qos::ue_ctxt* lhs,
                                                        const scheduler_time_qos::ue_ctxt* rhs) const
{
  // All other cases compare priorities.
  return lhs->ul_prio < rhs->ul_prio;
}
