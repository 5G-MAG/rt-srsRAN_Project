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

#include "e2.h"
#include "e2_connection_client.h"
#include "e2ap_configuration.h"
#include "e2sm/e2sm_manager.h"
#include "subscription/e2_subscription.h"
#include "srsran/du/du_high/du_manager/du_configurator.h"
#include "srsran/f1ap/du/f1ap_du.h"
#include "srsran/gateways/sctp_network_gateway.h"
#include "srsran/pcap/dlt_pcap.h"
#include "srsran/support/timers.h"
#include <memory>

namespace srsran {

/// Creates an instance of an E2 interface, notifying outgoing packets on the specified listener object.
std::unique_ptr<e2_interface> create_e2(e2ap_configuration&      e2ap_cfg_,
                                        timer_factory            timers_,
                                        e2_message_notifier&     e2_pdu_notifier_,
                                        e2_subscription_manager& e2_subscriber_,
                                        e2sm_manager&            e2sm_manager_);

/// Creates a decorated instance of an E2 interface (with a task executor)
std::unique_ptr<e2_interface> create_e2_with_task_exec(e2ap_configuration&      e2ap_cfg_,
                                                       timer_factory            timers_,
                                                       e2_message_notifier&     e2_pdu_notifier_,
                                                       e2_subscription_manager& e2_subscription_mngr_,
                                                       e2sm_manager&            e2sm_manager_,
                                                       task_executor&           e2_exec_);

/// Creates a instance of an E2 interface (with subscription manager)
std::unique_ptr<e2_interface> create_e2_entity(e2ap_configuration&            e2ap_cfg_,
                                               e2_connection_client*          e2_client_,
                                               e2_du_metrics_interface&       e2_du_metrics_,
                                               srs_du::f1ap_ue_id_translator& f1ap_ue_id_translator_,
                                               srs_du::du_configurator&       du_configurator_,
                                               timer_factory                  timers_,
                                               task_executor&                 e2_exec_);

/// Creates an instance of an E2AP ASN1 packer.
std::unique_ptr<e2ap_packer>
create_e2ap_asn1_packer(sctp_network_gateway_data_handler& gw_, e2_message_handler& e2_, dlt_pcap& pcap_);
} // namespace srsran
