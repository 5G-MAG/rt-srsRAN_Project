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

#include "sdap_session_logger.h"
#include "srsran/sdap/sdap.h"

namespace srsran {

namespace srs_cu_up {

class sdap_entity_rx_impl : public sdap_rx_pdu_handler
{
public:
  sdap_entity_rx_impl(uint32_t              ue_index,
                      pdu_session_id_t      psi,
                      qos_flow_id_t         qfi_,
                      rb_id_t               rb_id_,
                      sdap_rx_sdu_notifier& sdu_notifier_) :
    logger("SDAP", {ue_index, psi, qfi_, rb_id_, "UL"}), qfi(qfi_), rb_id(rb_id_), sdu_notifier(sdu_notifier_)
  {
  }

  void handle_pdu(byte_buffer pdu) final
  {
    // pass through with qfi
    logger.log_debug("RX SDU. {} sdu_len={}", qfi, pdu.length());
    sdu_notifier.on_new_sdu(std::move(pdu), qfi);
  }

  drb_id_t get_drb_id() {
    if (rb_id.is_drb()) {
      return rb_id.get_drb_id();
    }
    return drb_id_t::invalid;
  }

  mrb_id_t get_mrb_id() {
    if (rb_id.is_mrb()) {
      return rb_id.get_mrb_id();
    }
    return mrb_id_t::invalid;
  }

private:
  sdap_session_trx_logger logger;
  qos_flow_id_t           qfi;
  rb_id_t                 rb_id;
  sdap_rx_sdu_notifier&   sdu_notifier;
};

} // namespace srs_cu_up

} // namespace srsran
