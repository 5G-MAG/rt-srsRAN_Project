/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/phy/support/prach_buffer.h"
#include "srsran/phy/support/prach_buffer_context.h"
#include "srsran/phy/support/resource_grid.h"
#include "srsran/ran/slot_point.h"

namespace srsran {

/// Describes the context of the new received symbol.
struct lower_phy_rx_symbol_context {
  /// Slot context.
  slot_point slot;
  /// Radio sector identifier.
  unsigned sector;
  /// The last processed symbol index within the slot.
  unsigned nof_symbols;
};

/// \brief Lower physical layer notifier for events related to received symbols.
///
/// The events generated by this interface are triggered by the requests handled by the \ref lower_phy_request_handler
/// interface.
class lower_phy_rx_symbol_notifier
{
public:
  /// Default destructor.
  virtual ~lower_phy_rx_symbol_notifier() = default;

  /// \brief Notifies the completion of an OFDM symbol for a given context.
  ///
  /// \param[in] context Notification context.
  /// \param[in] grid    Resource grid that belongs to the context.
  virtual void on_rx_symbol(const lower_phy_rx_symbol_context& context, const resource_grid_reader& grid) = 0;

  /// \brief Notifies the completion of PRACH window.
  ///
  /// The lower PHY uses this method to notify that a PRACH window identified by \c context has been written in \c
  /// buffer.
  ///
  /// \param[in] context PRACH context.
  /// \param[in] buffer  Read-only PRACH buffer.
  virtual void on_rx_prach_window(const prach_buffer_context& context, const prach_buffer& buffer) = 0;

  /// \brief Notifies the completion of SRS symbols.
  ///
  /// \param[in] context Notification context.
  virtual void on_rx_srs_symbol(const lower_phy_rx_symbol_context& context) = 0;
};

} // namespace srsran