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

#include "srsran/phy/metrics/phy_metrics_notifiers.h"
#include "srsran/phy/metrics/phy_metrics_reports.h"
#include <atomic>

namespace srsran {

/// Modulation mapper metric producer.
class channel_modulation_metric_producer_impl : private detail::phy_metric_notifier<channel_modulation_metrics>
{
public:
  /// Gets the modulation mapper metric interface.
  detail::phy_metric_notifier<channel_modulation_metrics>& get_notifier() { return *this; }

  /// Gets the processing rate in megabits per second.
  double get_avg_rate_Mbps(modulation_scheme modulation) const
  {
    const metrics_per_modulation& metrics = select_metrics(modulation);
    return static_cast<double>(get_bits_per_symbol(modulation) * metrics.sum_nof_symbols) /
           static_cast<double>(metrics.sum_elapsed_ns) * 1000;
  }

  /// Gets the processing rate in millions of bauds per second.
  double get_avg_rate_MBaudps(modulation_scheme modulation) const
  {
    const metrics_per_modulation& metrics = select_metrics(modulation);
    return static_cast<double>(metrics.sum_nof_symbols) / static_cast<double>(metrics.sum_elapsed_ns) * 1000;
  }

  /// Gets the total amount of time the CRC calculator spent calculating.
  std::chrono::nanoseconds get_total_time() const
  {
    return std::chrono::nanoseconds(qpsk_metrics_collection.sum_elapsed_ns + qam16_metrics_collection.sum_elapsed_ns +
                                    qam64_metrics_collection.sum_elapsed_ns + qam256_metrics_collection.sum_elapsed_ns +
                                    other_metrics_collection.sum_elapsed_ns);
  }

private:
  /// Groups all metrics of a single modulation.
  struct metrics_per_modulation {
    std::atomic<uint64_t> sum_nof_symbols = {};
    std::atomic<uint64_t> sum_elapsed_ns  = {};
  };

  /// Selects the CRC calculator metrics depending on the polynomial for read-write operations.
  metrics_per_modulation& select_metrics(modulation_scheme modulation)
  {
    switch (modulation) {
      case modulation_scheme::QPSK:
        return qpsk_metrics_collection;
      case modulation_scheme::QAM16:
        return qam16_metrics_collection;
      case modulation_scheme::QAM64:
        return qam64_metrics_collection;
      case modulation_scheme::QAM256:
        return qam256_metrics_collection;
      case modulation_scheme::PI_2_BPSK:
      case modulation_scheme::BPSK:
      default:
        return other_metrics_collection;
    }
  }

  /// Selects the CRC calculator metrics depending on the polynomial for read-only operations.
  const metrics_per_modulation& select_metrics(modulation_scheme modulation) const
  {
    switch (modulation) {
      case modulation_scheme::QPSK:
        return qpsk_metrics_collection;
      case modulation_scheme::QAM16:
        return qam16_metrics_collection;
      case modulation_scheme::QAM64:
        return qam64_metrics_collection;
      case modulation_scheme::QAM256:
        return qam256_metrics_collection;
      case modulation_scheme::PI_2_BPSK:
      case modulation_scheme::BPSK:
      default:
        return other_metrics_collection;
    }
  }

  // See interface for documentation.
  void new_metric(const channel_modulation_metrics& metrics) override
  {
    metrics_per_modulation& metrics_modulation = select_metrics(metrics.modulation);
    metrics_modulation.sum_nof_symbols += metrics.nof_symbols;
    metrics_modulation.sum_elapsed_ns += metrics.elapsed.count();
  }

  metrics_per_modulation qpsk_metrics_collection;
  metrics_per_modulation qam16_metrics_collection;
  metrics_per_modulation qam64_metrics_collection;
  metrics_per_modulation qam256_metrics_collection;
  metrics_per_modulation other_metrics_collection;
};

} // namespace srsran
