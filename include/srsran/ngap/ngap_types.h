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

#include "srsran/ran/plmn_identity.h"
#include "srsran/ran/nr_cgi.h"
#include "srsran/ran/mbs.h"
#include <limits>

namespace srsran {
namespace srs_cu_cp {

/// \brief AMF_UE_ID (non ASN1 type of AMF_UE_NGAP_ID) used to identify the UE in the AMF.
/// \remark See TS 38.413 Section 9.3.3.1: AMF_UE_NGAP_ID valid values: (0..2^40-1)
static constexpr uint64_t MAX_NOF_AMF_UES = ((uint64_t)1 << 40);
enum class amf_ue_id_t : uint64_t { min = 0, max = MAX_NOF_AMF_UES - 1, invalid = 0x1ffffffffff };

/// Convert AMF_UE_ID type to integer.
inline uint64_t amf_ue_id_to_uint(amf_ue_id_t id)
{
  return static_cast<uint64_t>(id);
}

/// Convert integer to AMF_UE_ID type.
inline amf_ue_id_t uint_to_amf_ue_id(std::underlying_type_t<amf_ue_id_t> id)
{
  return static_cast<amf_ue_id_t>(id);
}

// Globally unique AMF identifier.
struct guami_t {
  plmn_identity plmn = plmn_identity::test_value();
  uint16_t      amf_set_id;
  uint8_t       amf_pointer;
  uint8_t       amf_region_id;
};

struct ngap_ue_aggr_max_bit_rate {
  uint64_t ue_aggr_max_bit_rate_dl;
  uint64_t ue_aggr_max_bit_rate_ul;
};

/// \brief MBS Service Area Information.
/// \remark See 3GPP TS 38.413, 9.3.1.209 - MBS Service Area Information.
struct ngap_mbs_service_area_information {
  //mbs_service_area_cell_list (0..maxnoofCellsforMBS)
  std::vector<nr_cell_global_id_t> mbs_service_area_cell_list;
  //mbs_service_area_tai_list (0..maxnoofTAIforMBS)
  std::vector<tai_t> mbs_service_area_tai_list;
};

struct ngap_mbs_service_area_information_item {
  //mbs_area_session_id (M)
  area_session_id_t mbs_area_session_id;
  //mbs_service_area_information (M)
  ngap_mbs_service_area_information mbs_service_area_information;
};

struct ngap_mbs_service_area_location_independent {
  //mbs_service_area_information (M)
  ngap_mbs_service_area_information mbs_service_area_information;
};

struct ngap_mbs_service_area_location_dependent {
  //mbs_service_area_information_list (1)
  std::vector<ngap_mbs_service_area_information_item> mbs_service_area_information_list;
};

/// \brief MBS Service Area.
/// \remark See 3GPP TS 38.413, 9.3.1.208 - MBS Service Area.
struct ngap_mbs_service_area {
  ngap_mbs_service_area() : choice(ngap_mbs_service_area_location_dependent{}) {}
  ngap_mbs_service_area(const ngap_mbs_service_area_location_independent& val) : choice(val) {}
  ngap_mbs_service_area(const ngap_mbs_service_area_location_dependent& val) : choice(val) {}

  bool is_locationdependent() const { return std::holds_alternative<ngap_mbs_service_area_location_dependent>(choice); }

  ngap_mbs_service_area_location_independent get_locationindependent() { return std::get<ngap_mbs_service_area_location_independent>(choice); }
  ngap_mbs_service_area_location_dependent get_locationdependent() { return std::get<ngap_mbs_service_area_location_dependent>(choice); }

private:
  std::variant<ngap_mbs_service_area_location_independent, ngap_mbs_service_area_location_dependent> choice;
};

struct ngap_shared_ngu_multicast_tnl_information {
  //ip_multicast_address (M)
  transport_layer_address ip_multicast_address;
  //ip_source_address (M)
  transport_layer_address ip_source_address;
  //gtp_teid_at_5gc (M)
  gtpu_teid_t gtp_teid_at_5gc;
};

struct ngap_mbs_session_tnl_information_5gc_item {
  //mbs_area_session_id (M)
  area_session_id_t mbs_area_session_id;
  //shared_ngu_multicast_tnl_information (M)
  ngap_shared_ngu_multicast_tnl_information shared_ngu_multicast_tnl_information;
};

struct ngap_mbs_session_tnl_info_5gc_location_independent {
  //shared_ngu_multicast_tnl_information (M)
  ngap_shared_ngu_multicast_tnl_information shared_ngu_multicast_tnl_information;
};

struct ngap_mbs_session_tnl_info_5gc_location_dependent {
  //mbs_session_tnl_information_5gc_list (1)
  std::vector<ngap_mbs_session_tnl_information_5gc_item> mbs_session_tnl_information_5gc_list;
};

/// \brief MBS Session TNL Information 5GC.
/// \remark See 3GPP TS 38.413, 9.3.2.15 - MBS Session TNL Information 5GC.
struct ngap_mbs_session_tnl_information_5gc {
  ngap_mbs_session_tnl_information_5gc() : choice(ngap_mbs_session_tnl_info_5gc_location_dependent{}) {}
  ngap_mbs_session_tnl_information_5gc(const ngap_mbs_session_tnl_info_5gc_location_independent& val) : choice(val) {}
  ngap_mbs_session_tnl_information_5gc(const ngap_mbs_session_tnl_info_5gc_location_dependent& val) : choice(val) {}

  bool is_locationdependent() const { return std::holds_alternative<ngap_mbs_session_tnl_info_5gc_location_dependent>(choice); }

  ngap_mbs_session_tnl_info_5gc_location_independent get_locationindependent() { return std::get<ngap_mbs_session_tnl_info_5gc_location_independent>(choice); }
  ngap_mbs_session_tnl_info_5gc_location_dependent get_locationdependent() { return std::get<ngap_mbs_session_tnl_info_5gc_location_dependent>(choice); }

private:
  std::variant<ngap_mbs_session_tnl_info_5gc_location_independent, ngap_mbs_session_tnl_info_5gc_location_dependent> choice;
};

/// \brief QoS Flow Level QoS Parameters.
/// \remark See 3GPP TS 38.413, 9.3.1.12 - QoS Flow Level QoS Parameters.
struct ngap_qos_flow_level_qos_params {
  //qos_characteristics (M)
  qos_characteristics qos_desc;
  //allocation_and_retention_priority (M)
  alloc_and_retention_priority allocation_and_retention_priority;
  //gbr_qos_flow_information (O)
  std::optional<gbr_qos_flow_information> gbr_qos_flow_info;
  //reflective_qos_attribute (O)

  //additional_qos_flow_information (O)

  //qos_monitoring_request (O)

  //qos_monitoring_reporting_frequency (O)
};

/// \brief MBS QoS Flows To Be Setup List.
/// \remark See 3GPP TS 38.413, 9.3.1.236 - MBS QoS Flows To Be Setup List.
struct ngap_mbs_qos_flows_setup_request_item {
  //mbs_qos_flow_identifier (M)
  qos_flow_id_t mbs_qos_flow_identifier = qos_flow_id_t::invalid;
  //mbs_qos_flow_level_qos_parameters (M)
  ngap_qos_flow_level_qos_params mbs_qos_flow_level_qos_parameters;
};

/// \brief MBS Session Setup or Modification Request Transfer.
/// \remark See 3GPP TS 38.413, 9.3.5.3 - MBS Session Setup or Modification Request Transfer.
struct ngap_mbs_session_setup_or_modification_request_transfer {
  //mbs_session_tnl_information_5gc (O)
  std::optional<ngap_mbs_session_tnl_information_5gc> mbs_session_tnl_information_5gc;
  //mbs_qos_flows_to_be_setup_or_modified_list (M)
  std::vector<ngap_mbs_qos_flows_setup_request_item> mbs_qos_flows_to_be_setup_or_modified_list;
  //mbs_session_fsa_id_list (0..maxnoofMBSFSAs)
  std::vector<fsa_id_t> mbs_session_fsa_id_list;
};

} // namespace srs_cu_cp
} // namespace srsran
