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
#include "srsran/cu_up/cu_up_config.h"
#include "srsran/f1u/cu_up/f1u_gateway.h"
#include "srsran/gtpu/gtpu_teid_pool.h"
#include "srsran/gtpu/gtpu_demux.h"
#include "srsran/gtpu/gtpu_tunnel_ngu.h"
#include "srsran/ran/up_transport_layer_info.h"
#include "../ngu_session_manager.h"
#include "../adapters/sdap_adapters.h"
#include "../adapters/gtpu_adapters.h"
#include "srsran/support/timers.h"
#include "srsran/gtpu/gtpu_tunnel_ngu_factory.h"
#include "mrb_context.h"
#include <map>

namespace srsran::srs_cu_up {

/// 3GPP TS 37.483 clause 9.3.1.114 - BC MRB Setup Configuration: maxnoofMRBs
#define MAX_NUM_MRBS_PER_MBS_SESSION (32)

/// \brief MBS Session context setup configuration
struct mbs_session_context_cfg {
  std::map<five_qi_t, srs_cu_up::cu_up_qos_config> qos;
  n3_interface_config                              n3_config;
  cu_up_test_mode_config                           test_mode_cfg;
  uint64_t                                         mbs_session_dl_ambr;
};

/// \brief Context for an MBS Session within the CU-UP.
class mbs_session_context
{
public:
  mbs_session_context(mbs_index_t                      mbs_index_,
                      mbs_session_context_cfg          cfg_,
                      mbs_session_id_t                 mbs_session_id_,
                      std::optional<area_session_id_t> area_session_id_,
                      std::unique_ptr<ue_executor_mapper> mbs_session_exec_mapper_,
                      ngu_session_manager&             ngu_session_mngr_,
                      f1u_cu_up_gateway&               f1u_gw_,
                      gtpu_demux_ctrl&                 gtpu_rx_demux_,
                      gtpu_teid_pool&                  f1u_teid_allocator_,
                      timer_factory                    mbs_session_dl_timer_factory_,
                      timer_factory                    mbs_session_ul_timer_factory_,
                      timer_factory                    mbs_session_ctrl_timer_factory_,
                      dlt_pcap&                        gtpu_pcap_);

  ~mbs_session_context() { stop(); }

  [[nodiscard]] mbs_index_t get_index() const { return mbs_index; }

  mbs_broadcast_session_setup_result
  setup_mbs_broadcast_session(e1ap_bc_bearer_context_to_setup& bc_bearer_context_to_setup);

  mrb_setup_result
  handle_mrb_to_setup_item(const e1ap_bc_mrb_setup_config& mrb_to_setup_item);

  mbs_broadcast_session_modification_result
  modify_mbs_broadcast_session(e1ap_bc_bearer_context_to_modify& bc_bearer_context_to_modify);

  mrb_modification_result
  handle_mrb_to_modify_item(e1ap_bc_mrb_to_modify_item& mrb_to_modify_item);

private:
  void stop()
  {
    if (not stopped) {
      gtpu_rx_demux.remove_tunnel(c_teid);
      //(void)n3_teid_allocator.release_teid(local_teid);

      // if (dispatch_queue != nullptr) {
      //   dispatch_queue->stop();
      // }
      gtpu->stop();

      // Stop MRBs
      for (const auto& mrb : mrbs) {
        mrb.second->stop();

        // if (!f1u_teid_allocator.release_teid(mrb.second->f1u_ul_teid)) {
        //   logger.log_error(
        //       "could not remove ul_teid at session termination. ul_teid={}", mrb.second->f1u_ul_teid);
        // }
      }

    }
    stopped = true;
  }

  srslog::basic_logger& logger = srslog::fetch_basic_logger("CU-UP-MBSCTXT");

  bool stopped = false;

  std::unique_ptr<sdap_entity>     sdap;
  std::unique_ptr<gtpu_tunnel_ngu> gtpu;

  // Adapters between SDAP and GTPU
  sdap_gtpu_adapter sdap_to_gtpu_adapter;
  gtpu_sdap_adapter gtpu_to_sdap_adapter;

  // Adapters between GTP-U and NG-U/F1-U
  gtpu_network_gateway_adapter gtpu_to_udp_adapter;

  mbs_index_t             mbs_index;
  mbs_session_context_cfg cfg;

  // MBS Session parameters
  gtpu_teid_t                      c_teid;         // the common teid used to receive this MBS Session
  mbs_session_id_t                 mbs_session_id;
  std::optional<area_session_id_t> area_session_id;

  uint64_t                         mbs_session_dl_ambr;
  unique_timer                     mbs_session_inactivity_timer; // NOTE (borieher): Is this needed for MBS?
  std::unique_ptr<token_bucket>    mbs_session_ambr_limiter;

  std::unique_ptr<ue_executor_mapper> mbs_session_exec_mapper;

  ngu_session_manager&    ngu_session_mngr;
  f1u_cu_up_gateway&      f1u_gw;

  // GTP-U demux parameters
  gtpu_demux_ctrl&        gtpu_rx_demux;     // The demux entity to register/remove the tunnel.
  std::unique_ptr<gtpu_demux_dispatch_queue> dispatch_queue;

  gtpu_teid_pool&         f1u_teid_allocator;

  task_executor&                   mbs_session_dl_exec;
  task_executor&                   mbs_session_ul_exec;
  task_executor&                   mbs_session_ctrl_exec;
  task_executor&                   crypto_exec;

  timer_factory                    mbs_session_dl_timer_factory;
  timer_factory                    mbs_session_ul_timer_factory;
  timer_factory                    mbs_session_ctrl_timer_factory;

  dlt_pcap&                        gtpu_pcap;

  // MRB contexts
  std::map<mrb_id_t, std::unique_ptr<mrb_context>> mrbs; // key is mrb_id
};

} // namespace srsran::srs_cu_up
