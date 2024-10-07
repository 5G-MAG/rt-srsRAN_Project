/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/adt/mpmc_queue.h"
#include "srsran/adt/unique_function.h"
#include "srsran/du/du_high/du_test_mode_config.h"
#include "srsran/mac/mac.h"
#include "srsran/mac/mac_cell_result.h"
#include "srsran/srslog/srslog.h"
#include <mutex>
#include <unordered_map>

namespace srsran {
namespace srs_du {

/// \brief Handles information related to the test UE(s).
class test_ue_info_manager
{
public:
  test_ue_info_manager(rnti_t rnti_start_, uint16_t nof_ues_);

  du_ue_index_t rnti_to_du_ue_idx(rnti_t rnti) const
  {
    if (rnti_to_ue_info_lookup.count(rnti) == 0) {
      return INVALID_DU_UE_INDEX;
    }
    return rnti_to_ue_info_lookup.at(rnti).ue_idx;
  }

  bool is_test_ue(du_ue_index_t ue_idx) const { return ue_idx < nof_ues; }

  bool is_test_ue(rnti_t rnti) const
  {
    return (rnti >= rnti_start) and (rnti < to_rnti(to_value(rnti_start) + nof_ues));
  }

  void add_ue(rnti_t rnti, du_ue_index_t ue_idx_, const sched_ue_config_request& sched_ue_cfg_req_);

  void remove_ue(rnti_t rnti);

  const sched_ue_config_request& get_sched_ue_cfg_request(rnti_t rnti) const
  {
    return rnti_to_ue_info_lookup.at(rnti).sched_ue_cfg_req;
  }

  bool is_msg4_rxed(rnti_t rnti) const
  {
    if (rnti_to_ue_info_lookup.count(rnti) > 0) {
      return rnti_to_ue_info_lookup.at(rnti).msg4_rx_flag;
    }
    return false;
  }

  void msg4_rxed(rnti_t rnti, bool msg4_rx_flag_)
  {
    if (rnti_to_ue_info_lookup.count(rnti) > 0) {
      rnti_to_ue_info_lookup.at(rnti).msg4_rx_flag = msg4_rx_flag_;
    }
  }

  void process_pending_tasks();

private:
  struct test_ue_info {
    du_ue_index_t           ue_idx;
    sched_ue_config_request sched_ue_cfg_req;
    bool                    msg4_rx_flag;
  };

  // Parameters received from configuration.
  rnti_t   rnti_start;
  uint16_t nof_ues;

  // Mapping between UE RNTI and test UE information.
  std::unordered_map<rnti_t, test_ue_info> rnti_to_ue_info_lookup;

  concurrent_queue<unique_task, concurrent_queue_policy::lockfree_mpmc, concurrent_queue_wait_policy::non_blocking>
      pending_tasks;
};

class phy_test_mode_adapter : public mac_result_notifier
{
public:
  phy_test_mode_adapter(mac_result_notifier& phy_notifier_) : adapted_phy(phy_notifier_), cells(MAX_NOF_DU_CELLS) {}

  mac_cell_result_notifier& get_cell(du_cell_index_t cell_index) override;

  void connect(du_cell_index_t cell_index, mac_cell_result_notifier& test_mode_cell_notifier);

  void disconnect(du_cell_index_t cell_index);

  mac_result_notifier& adapted_phy;

private:
  class phy_cell final : public mac_cell_result_notifier
  {
  public:
    mac_cell_result_notifier* ptr = nullptr;

    void on_new_downlink_scheduler_results(const mac_dl_sched_result& dl_res) override;
    void on_new_downlink_data(const mac_dl_data_result& dl_data) override;
    void on_new_uplink_scheduler_results(const mac_ul_sched_result& ul_res) override;
    void on_cell_results_completion(slot_point slot) override;
  };

  std::vector<phy_cell> cells;
};

/// \brief Adapter of MAC cell for testing purposes. It automatically forces ACK/CRC=OK for the test UE.
class mac_test_mode_cell_adapter : public mac_cell_control_information_handler,
                                   public mac_cell_result_notifier,
                                   public mac_cell_slot_handler
{
public:
  mac_test_mode_cell_adapter(const srs_du::du_test_mode_config::test_mode_ue_config& test_ue_cfg_,
                             const mac_cell_creation_request&                        cell_cfg,
                             mac_cell_control_information_handler&                   adapted_,
                             mac_pdu_handler&                                        pdu_handler_,
                             mac_cell_slot_handler&                                  slot_handler_,
                             mac_cell_result_notifier&                               result_notifier_,
                             std::function<void(rnti_t)>                             dl_bs_notifier_,
                             test_ue_info_manager&                                   ue_info_mgr_);

  void on_new_downlink_scheduler_results(const mac_dl_sched_result& dl_res) override;

  void on_new_downlink_data(const mac_dl_data_result& dl_data) override
  {
    result_notifier.on_new_downlink_data(dl_data);
  }

  // Intercepts the UL results coming from the MAC.
  void on_new_uplink_scheduler_results(const mac_ul_sched_result& ul_res) override;

  void on_cell_results_completion(slot_point slot) override { result_notifier.on_cell_results_completion(slot); }

  void handle_slot_indication(slot_point sl_tx) override;
  void handle_error_indication(slot_point sl_tx, error_event event) override;

  void handle_crc(const mac_crc_indication_message& msg) override;

  void handle_uci(const mac_uci_indication_message& msg) override;

  void handle_srs(const mac_srs_indication_message& msg) override;

private:
  struct slot_decision_history {
    // Locks a given slot.
    // Note: In normal scenarios, this mutex will have no contention, as the times of write and read are separate.
    // However, if the ring buffer is too small, this may stop being true.
    mutable std::mutex         mutex;
    slot_point                 slot;
    std::vector<pucch_info>    pucchs;
    std::vector<ul_sched_info> puschs;
  };

  void fill_csi_bits(rnti_t rnti, bounded_bitset<uci_constants::MAX_NOF_CSI_PART1_OR_PART2_BITS>& payload) const;
  void fill_uci_pdu(mac_uci_pdu::pucch_f0_or_f1_type& pucch_ind, const pucch_info& pucch) const;
  void fill_uci_pdu(mac_uci_pdu::pucch_f2_or_f3_or_f4_type& pucch_ind, const pucch_info& pucch) const;
  void fill_uci_pdu(mac_uci_pdu::pusch_type& pusch_ind, const ul_sched_info& ul_grant) const;
  void forward_uci_ind_to_mac(const mac_uci_indication_message& uci_msg);
  void forward_crc_ind_to_mac(const mac_crc_indication_message& crc_msg);

  size_t get_ring_idx(slot_point sl) const { return sl.to_uint() % sched_decision_history.size(); }

  const srs_du::du_test_mode_config::test_mode_ue_config& test_ue_cfg;
  mac_cell_control_information_handler&                   adapted;
  mac_pdu_handler&                                        pdu_handler;
  mac_cell_slot_handler&                                  slot_handler;
  mac_cell_result_notifier&                               result_notifier;
  std::function<void(rnti_t)>                             dl_bs_notifier;
  srslog::basic_logger&                                   logger;

  std::vector<slot_decision_history> sched_decision_history;

  test_ue_info_manager& ue_info_mgr;
};

class mac_test_mode_adapter final : public mac_interface,
                                    public mac_ue_control_information_handler,
                                    public mac_ue_configurator,
                                    public mac_cell_manager
{
public:
  explicit mac_test_mode_adapter(const srs_du::du_test_mode_config::test_mode_ue_config& test_ue_cfg,
                                 mac_result_notifier&                                    phy_notifier_);
  ~mac_test_mode_adapter() override;

  void connect(std::unique_ptr<mac_interface> mac_ptr);

  // mac_cell_manager
  void                 add_cell(const mac_cell_creation_request& cell_cfg) override;
  void                 remove_cell(du_cell_index_t cell_index) override;
  mac_cell_controller& get_cell_controller(du_cell_index_t cell_index) override;

  mac_cell_rach_handler& get_rach_handler(du_cell_index_t cell_index) override
  {
    return mac_adapted->get_rach_handler(cell_index);
  }

  mac_cell_slot_handler& get_slot_handler(du_cell_index_t cell_index) override
  {
    return *cell_info_handler[cell_index];
  }

  mac_cell_manager& get_cell_manager() override { return *this; }

  mac_ue_control_information_handler& get_ue_control_info_handler() override { return *this; }

  mac_pdu_handler& get_pdu_handler() override { return mac_adapted->get_pdu_handler(); }

  mac_paging_information_handler& get_cell_paging_info_handler() override
  {
    return mac_adapted->get_cell_paging_info_handler();
  }

  mac_ue_configurator& get_ue_configurator() override { return *this; }

  mac_cell_control_information_handler& get_control_info_handler(du_cell_index_t cell_index) override;

  mac_result_notifier& get_phy_notifier() { return *phy_notifier; }

private:
  void handle_dl_buffer_state_update(const mac_dl_buffer_state_indication_message& dl_bs) override;

  // mac_ue_configurator interface.
  async_task<mac_ue_create_response> handle_ue_create_request(const mac_ue_create_request& cfg) override;
  async_task<mac_ue_reconfiguration_response>
  handle_ue_reconfiguration_request(const mac_ue_reconfiguration_request& cfg) override;
  async_task<mac_ue_delete_response> handle_ue_delete_request(const mac_ue_delete_request& cfg) override;
  bool                               handle_ul_ccch_msg(du_ue_index_t ue_index, byte_buffer pdu) override;
  void                               handle_ue_config_applied(du_ue_index_t ue_idx) override;

  std::vector<mac_logical_channel_config>
  adapt_bearers(const std::vector<mac_logical_channel_config>& orig_bearers) const;

  srs_du::du_test_mode_config::test_mode_ue_config test_ue;
  std::unique_ptr<mac_interface>                   mac_adapted;

  test_ue_info_manager ue_info_mgr;

  std::unique_ptr<phy_test_mode_adapter> phy_notifier;

  std::vector<std::unique_ptr<mac_test_mode_cell_adapter>> cell_info_handler;
};

} // namespace srs_du
} // namespace srsran
