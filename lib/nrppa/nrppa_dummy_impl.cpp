/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "nrppa_dummy_impl.h"
#include "srsran/cu_cp/cu_cp_types.h"
#include "srsran/srslog/srslog.h"

using namespace srsran;
using namespace srs_cu_cp;

nrppa_dummy_impl::nrppa_dummy_impl() : logger(srslog::fetch_basic_logger("NRPPA")) {}

// Note: For fwd declaration of member types, dtor cannot be trivial.
nrppa_dummy_impl::~nrppa_dummy_impl() {}

void nrppa_dummy_impl::remove_ue_context(ue_index_t ue_index) {}

void nrppa_dummy_impl::handle_new_nrppa_pdu(const byte_buffer&                    nrppa_pdu,
                                            std::variant<ue_index_t, amf_index_t> ue_or_amf_index)
{
  logger.info("NRPPa messages are not supported");
}
