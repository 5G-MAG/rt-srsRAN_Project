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

#include "srsran/ran/nr_cgi.h"
#include "srsran/adt/byte_buffer.h"
#include "srsran/ran/rb_id.h"
#include "srsran/ran/cause/f1ap_cause.h"
#include "srsran/ran/mbs.h"
#include <cstdint>
#include <vector>

namespace srsran {

/// \brief gNB-CU MBS F1AP ID used to identify the MBS context in the CU F1AP.
/// \remark See TS 38.473 Section 9.3.1.219: gNB-CU MBS F1AP ID valid values: (0..2^32-1)
static constexpr uint64_t MAX_NOF_CU_F1AP_MBS_IDS = ((uint64_t)1 << 32);
enum class gnb_cu_mbs_f1ap_id_t : uint64_t { min = 0, max = MAX_NOF_CU_F1AP_MBS_IDS - 1, invalid = MAX_NOF_CU_F1AP_MBS_IDS };

constexpr uint64_t gnb_cu_mbs_f1ap_id_to_uint(gnb_cu_mbs_f1ap_id_t id)
{
  return static_cast<uint64_t>(id);
}

/// Convert integer to gNB-CU MBS F1AP ID type.
constexpr gnb_cu_mbs_f1ap_id_t uint_to_gnb_cu_mbs_f1ap_id(uint64_t idx)
{
  return static_cast<gnb_cu_mbs_f1ap_id_t>(idx);
}

/// \brief gNB-DU MBS F1AP ID used to identify the MBS context in the DU F1AP.
/// \remark See TS 38.473 Section 9.3.1.220: gNB-DU MBS F1AP ID valid values: (0..2^32-1)
static constexpr uint64_t MAX_NOF_DU_F1AP_MBS_IDS = ((uint64_t)1 << 32);
enum class gnb_du_mbs_f1ap_id_t : uint64_t { min = 0, max = MAX_NOF_DU_F1AP_MBS_IDS - 1, invalid = MAX_NOF_DU_F1AP_MBS_IDS };

constexpr uint64_t gnb_du_mbs_f1ap_id_to_uint(gnb_du_mbs_f1ap_id_t id)
{
  return static_cast<uint64_t>(id);
}

/// Convert integer to gNB-DU MBS F1AP ID type.
constexpr gnb_du_mbs_f1ap_id_t uint_to_gnb_du_mbs_f1ap_id(uint64_t idx)
{
  return static_cast<gnb_du_mbs_f1ap_id_t>(idx);
}

/// \brief MBS Service Area Information.
/// \remark See 3GPP TS 38.473, 9.3.1.223 - MBS Service Area Information.
struct f1ap_mbs_service_area_information {
  //mbs_service_area_cell_list (0..maxnoofCellsforMBS)
  std::vector<nr_cell_global_id_t> mbs_service_area_cell_list;
  //mbs_service_area_tai_list (0..maxnoofTAIforMBS)
  std::vector<tai_t> mbs_service_area_tai_list;
};

struct f1ap_mbs_service_area_information_item {
  //mbs_area_session_id (M)
  area_session_id_t mbs_area_session_id;
  //mbs_service_area_information (M)
  f1ap_mbs_service_area_information mbs_service_area_information;
};

struct f1ap_mbs_service_area_location_independent {
  //mbs_service_area_information (M)
  f1ap_mbs_service_area_information mbs_service_area_information;
};

struct f1ap_mbs_service_area_location_dependent {
  //mbs_service_area_information_list (1.maxnoofMBSServiceAreaInformation)
  std::vector<f1ap_mbs_service_area_information_item> mbs_service_area_information_list;
};

/// \brief MBS Service Area.
/// \remark See 3GPP TS 38.473, 9.3.1.222 - MBS Service Area.
struct f1ap_mbs_service_area {
  f1ap_mbs_service_area() : choice(f1ap_mbs_service_area_location_dependent{}) {}
  f1ap_mbs_service_area(const f1ap_mbs_service_area_location_independent& val) : choice(val) {}
  f1ap_mbs_service_area(const f1ap_mbs_service_area_location_dependent& val) : choice(val) {}

  bool is_locationdependent() const { return std::holds_alternative<f1ap_mbs_service_area_location_dependent>(choice); }

  f1ap_mbs_service_area_location_independent get_locationindependent() { return std::get<f1ap_mbs_service_area_location_independent>(choice); }
  f1ap_mbs_service_area_location_dependent get_locationdependent() { return std::get<f1ap_mbs_service_area_location_dependent>(choice); }

private:
  std::variant<f1ap_mbs_service_area_location_independent, f1ap_mbs_service_area_location_dependent> choice;
};

struct f1ap_mbs_broadcast_cell_item {
  //nr_cgi (M)
  nr_cell_global_id_t nr_cgi;
  //mtch_neighbour_cell (O)
  std::optional<byte_buffer> mtch_neighbour_cell;
};

struct f1ap_mbs_broadcast_mrb_item {
  //mrb_id (M)
  mrb_id_t mrb_id = mrb_id_t::invalid;
  //mrb_pdcp_config_broadcast (M)
  byte_buffer mrb_pdcp_config_broadcast;
};

/// \brief MBS CU to DU RRC Information.
/// \remark See 3GPP TS 38.473, 9.3.1.225 - MBS CU to DU RRC Information.
struct f1ap_mbs_cu_to_du_rrc_information {
  //mbs_broadcast_cell_list (1..maxCellingNBDU)
  std::vector<f1ap_mbs_broadcast_cell_item> mbs_broadcast_cell_list;
  //mbs_broadcast_mrb_list (1..maxnoofMRBs)
  std::vector<f1ap_mbs_broadcast_mrb_item>  mbs_broadcast_mrb_list;
};

struct f1ap_bc_bearer_ctxt_f1u_tnl_info_location_dependent_item {
  //mbs_area_session_id (M)
  area_session_id_t mbs_area_session_id;
  //mbs_f1u_information (M)
  up_transport_layer_info mbs_f1u_information;
};

struct f1ap_bc_bearer_ctxt_f1u_tnl_info_location_independent {
  //mbs_f1u_information (M)
  up_transport_layer_info mbs_f1u_information;
};

struct f1ap_bc_bearer_ctxt_f1u_tnl_info_location_dependent {
    //location_dependent_mbs_f1u_information_at_cu (1..maxnoofMBSAreaSessionIDs)
  std::vector<f1ap_bc_bearer_ctxt_f1u_tnl_info_location_dependent_item> location_dependent_mbs_f1u_information;
};

/// \brief BC Bearer Context F1-U TNL Info.
/// \remark See TS 38.473, 9.3.2.7 - BC Bearer Context F1-U TNL Info.
struct f1ap_bc_bearer_context_f1u_tnl_info {
  f1ap_bc_bearer_context_f1u_tnl_info() : choice(f1ap_bc_bearer_ctxt_f1u_tnl_info_location_dependent{}) {}
  f1ap_bc_bearer_context_f1u_tnl_info(const f1ap_bc_bearer_ctxt_f1u_tnl_info_location_independent& val) : choice(val) {}
  f1ap_bc_bearer_context_f1u_tnl_info(const f1ap_bc_bearer_ctxt_f1u_tnl_info_location_dependent& val) : choice(val) {}

  bool is_locationdependent() const { return std::holds_alternative<f1ap_bc_bearer_ctxt_f1u_tnl_info_location_dependent>(choice); }

  f1ap_bc_bearer_ctxt_f1u_tnl_info_location_independent get_locationindependent() { return std::get<f1ap_bc_bearer_ctxt_f1u_tnl_info_location_independent>(choice); }
  f1ap_bc_bearer_ctxt_f1u_tnl_info_location_dependent get_locationdependent() { return std::get<f1ap_bc_bearer_ctxt_f1u_tnl_info_location_dependent>(choice); }

private:
  std::variant<f1ap_bc_bearer_ctxt_f1u_tnl_info_location_independent, f1ap_bc_bearer_ctxt_f1u_tnl_info_location_dependent> choice;
};

struct f1ap_mbs_qos_flows_mapped_to_mrb_item {
  //mbs_qos_flow_identifier (M)
  qos_flow_id_t mbs_qos_flow_id = qos_flow_id_t::invalid;
  //mbs_qos_flow_level_qos_parameters (M)
  qos_flow_level_qos_parameters mbs_qos_flow_level_qos_parameters;
};

struct f1ap_broadcast_mrb_to_be_setup_item {
  //mrb_id (M)
  mrb_id_t mrb_id = mrb_id_t::invalid;
  //mrb_qos_information (M)
  qos_flow_level_qos_parameters mrb_qos_information;
  //mbs_qos_flows_mapped_to_mrb (1..maxnoofMBSQoSFlows)
  std::vector<f1ap_mbs_qos_flows_mapped_to_mrb_item> mbs_qos_flows_mapped_to_mrb;
  //bc_bearer_context_f1u_tnl_info_at_cu (M)
  f1ap_bc_bearer_context_f1u_tnl_info bc_bearer_context_f1u_tnl_info_at_cu;
};

struct f1ap_broadcast_mrb_setup_item {
  //mrb_id (M)
  mrb_id_t mrb_id = mrb_id_t::invalid;
  //bc_bearer_context_f1u_tnl_info_at_du (M)
  f1ap_bc_bearer_context_f1u_tnl_info bc_bearer_context_f1u_tnl_info_at_du;
};

struct f1ap_broadcast_mrb_failed_to_be_setup_item {
  //mrb_id (M)
  mrb_id_t mrb_id = mrb_id_t::invalid;
  //cause (O)
  std::optional<f1ap_cause_t> cause;
};

} // namespace srsran