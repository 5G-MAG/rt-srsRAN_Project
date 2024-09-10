/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "srsran/du/du_low/du_low_factory.h"
#include "du_low_impl.h"
#include "srsran/du/du_low/du_low_config.h"
#include "srsran/support/error_handling.h"

using namespace srsran;
using namespace srs_du;

static std::unique_ptr<upper_phy> create_upper_phy(const upper_phy_config&                     upper_config,
                                                   const downlink_processor_factory_sw_config& dl_fact_config)
{
  // Create downlink processor factory.
  std::shared_ptr<downlink_processor_factory> dl_proc_factory;
  // Check if a hardware-accelerated PDSCH processor is requested.
  if (!upper_config.hal_config.hwacc_pdsch_processor) {
    dl_proc_factory = create_downlink_processor_factory_sw(dl_fact_config);
  } else {
    downlink_processor_factory_hw_config hw_cfg = {};
    hw_cfg.crc_calculator_type                  = upper_config.crc_calculator_type;
    hw_cfg.nof_concurrent_threads               = dl_fact_config.nof_concurrent_threads;
#ifdef HWACC_PDSCH_ENABLED
    hw_cfg.hwacc_pdsch_enc_cfg = upper_config.hal_config.hwacc_pdsch_enc_cfg;
#endif // HWACC_PDSCH_ENABLED
    dl_proc_factory = create_downlink_processor_factory_hw(hw_cfg);
  }
  report_fatal_error_if_not(dl_proc_factory, "Invalid DL processor factory.");

  // Create channel precoder factory.
  std::shared_ptr<channel_precoder_factory> precoding_factory = create_channel_precoder_factory("auto");
  report_fatal_error_if_not(precoding_factory, "Invalid channel precoder factory.");

  // Create resource grid factory.
  std::shared_ptr<resource_grid_factory> rg_factory = create_resource_grid_factory(precoding_factory);
  report_fatal_error_if_not(rg_factory, "Invalid resource grid factory.");

  // Create upper PHY factory.
  std::unique_ptr<upper_phy_factory> upper_phy_factory = create_upper_phy_factory(dl_proc_factory, rg_factory);
  report_fatal_error_if_not(upper_phy_factory, "Invalid upper PHY factory.");

  // Instantiate upper PHY.
  return upper_phy_factory->create(upper_config);
}

std::unique_ptr<du_low> srsran::srs_du::make_du_low(const du_low_config& config)
{
  std::vector<std::unique_ptr<upper_phy>> upper;
  for (const auto& cell_cfg : config.cells) {
    upper.push_back(create_upper_phy(cell_cfg.upper_phy_cfg, cell_cfg.dl_proc_cfg));
  }

  return std::make_unique<du_low_impl>(std::move(upper));
}
