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

#include "rlc_config_helpers.h"

using namespace srsran;
using namespace srs_du;

template <typename Bearer>
static void fill_rlc_entity_creation_message_common(rlc_entity_creation_message&             msg,
                                                    gnb_du_id_t                              du_id,
                                                    du_ue_index_t                            ue_index,
                                                    du_cell_index_t                          pcell_index,
                                                    Bearer&                                  bearer,
                                                    const rlc_config&                        rlc_cfg,
                                                    const du_manager_params::service_params& du_services,
                                                    rlc_tx_upper_layer_control_notifier&     rlc_rlf_notifier,
                                                    rlc_pcap&                                pcap_writer)
{
  msg.gnb_du_id      = du_id;
  msg.ue_index       = ue_index;
  msg.config         = rlc_cfg;
  msg.rx_upper_dn    = &bearer.connector.rlc_rx_sdu_notif;
  msg.tx_upper_dn    = &bearer.connector.rlc_tx_data_notif;
  msg.tx_upper_cn    = &rlc_rlf_notifier;
  msg.tx_lower_dn    = &bearer.connector.rlc_tx_buffer_state_notif;
  msg.timers         = &du_services.timers;
  msg.pcell_executor = &du_services.cell_execs.executor(pcell_index);
  msg.ue_executor    = &du_services.ue_execs.ctrl_executor(ue_index);
  msg.pcap_writer    = &pcap_writer;
}

// for SRBs
rlc_entity_creation_message
srsran::srs_du::make_rlc_entity_creation_message(gnb_du_id_t                              gnb_du_id,
                                                 du_ue_index_t                            ue_index,
                                                 du_cell_index_t                          pcell_index,
                                                 du_ue_srb&                               bearer,
                                                 const rlc_config&                        rlc_cfg,
                                                 const du_manager_params::service_params& du_services,
                                                 rlc_tx_upper_layer_control_notifier&     rlc_rlf_notifier,
                                                 rlc_pcap&                                pcap_writer)
{
  rlc_entity_creation_message msg;
  fill_rlc_entity_creation_message_common(
      msg, gnb_du_id, ue_index, pcell_index, bearer, rlc_cfg, du_services, rlc_rlf_notifier, pcap_writer);
  msg.rb_id = bearer.srb_id;
  return msg;
}

// for DRBs
rlc_entity_creation_message
srsran::srs_du::make_rlc_entity_creation_message(gnb_du_id_t                              gnb_du_id,
                                                 du_ue_index_t                            ue_index,
                                                 du_cell_index_t                          pcell_index,
                                                 du_ue_drb&                               bearer,
                                                 const rlc_config&                        rlc_cfg,
                                                 const du_manager_params::service_params& du_services,
                                                 rlc_tx_upper_layer_control_notifier&     rlc_rlf_notifier,
                                                 rlc_metrics_notifier*                    rlc_metrics_notifier_,
                                                 rlc_pcap&                                pcap_writer)
{
  rlc_entity_creation_message msg;
  fill_rlc_entity_creation_message_common(
      msg, gnb_du_id, ue_index, pcell_index, bearer, rlc_cfg, du_services, rlc_rlf_notifier, pcap_writer);
  msg.rb_id             = bearer.drb_id;
  msg.rlc_metrics_notif = rlc_metrics_notifier_;
  if (msg.rlc_metrics_notif == nullptr) {
    msg.config.metrics_period = timer_duration{0};
  }
  return msg;
}
