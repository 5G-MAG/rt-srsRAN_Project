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

#include "srsran/du/du_low/du_low.h"
#include "srsran/du/du_low/du_low_wrapper.h"
#include "srsran/fapi_adaptor/phy/phy_fapi_adaptor.h"
#include <memory>
#include <vector>

namespace srsran {
namespace srs_du {

class du_low_wrapper_impl final : public du_low_wrapper
{
public:
  du_low_wrapper_impl(std::unique_ptr<du_low>                                      du_lo_,
                      std::vector<std::unique_ptr<fapi_adaptor::phy_fapi_adaptor>> fapi_adaptors_);

  // See interface for documentation.
  du_low& get_du_low() override;

  // See interface for documentation.
  fapi::slot_message_gateway& get_slot_message_gateway(unsigned cell_id) override;

  // See interface for documentation.
  fapi::slot_last_message_notifier& get_slot_last_message_notifier(unsigned cell_id) override;

  // See interface for documentation.
  void set_slot_time_message_notifier(unsigned cell_id, fapi::slot_time_message_notifier& fapi_time_notifier) override;

  // See interface for documentation.
  void set_slot_error_message_notifier(unsigned                           cell_id,
                                       fapi::slot_error_message_notifier& fapi_error_notifier) override;

  // See interface for documentation.
  void set_slot_data_message_notifier(unsigned cell_id, fapi::slot_data_message_notifier& fapi_data_notifier) override;

private:
  std::unique_ptr<du_low>                                      du_lo;
  std::vector<std::unique_ptr<fapi_adaptor::phy_fapi_adaptor>> fapi_adaptors;
};

} // namespace srs_du
} // namespace srsran
