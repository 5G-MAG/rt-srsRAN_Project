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

#include "srsran/adt/byte_buffer.h"
#include "srsran/adt/slotted_vector.h"
#include "srsran/pdcp/pdcp_config.h"
#include "srsran/ran/cause/e1ap_cause.h"
#include "srsran/ran/cu_types.h"
#include "srsran/ran/rb_id.h"
#include "srsran/ran/up_transport_layer_info.h"
#include "srsran/security/security.h"
#include "srsran/ran/mbs.h"
#include <cstdint>

namespace srsran {

/// \brief GNB-CU-CP-UE-E1AP-ID used to identify the UE in the CU-CP E1AP.
/// \remark See TS 38.463 Section 9.3.1.4: GNB-CU-UE-E1AP-ID valid values: (0..2^32-1).
static constexpr uint64_t MAX_NOF_CU_CP_E1AP_UES = ((uint64_t)1 << 32);
enum class gnb_cu_cp_ue_e1ap_id_t : uint64_t { min = 0, max = MAX_NOF_CU_CP_E1AP_UES - 1, invalid = 0x1ffffffff };

constexpr uint64_t gnb_cu_cp_ue_e1ap_id_to_uint(gnb_cu_cp_ue_e1ap_id_t id)
{
  return static_cast<uint64_t>(id);
}

/// Convert integer to GNB-CU-CP-UE-E1AP-ID type.
constexpr gnb_cu_cp_ue_e1ap_id_t int_to_gnb_cu_cp_ue_e1ap_id(uint64_t idx)
{
  return static_cast<gnb_cu_cp_ue_e1ap_id_t>(idx);
}

/// \brief GNB-CU-UP-UE-E1AP-ID used to identify the UE in the CU-UP E1AP.
/// \remark See TS 38.473 Section 9.3.1.5: GNB-CU-UP-UE-E1AP-ID valid values: (0..2^32-1).
static constexpr uint64_t MAX_NOF_CU_UP_E1AP_UES = ((uint64_t)1 << 32);
enum class gnb_cu_up_ue_e1ap_id_t : uint64_t { min = 0, max = MAX_NOF_CU_CP_E1AP_UES - 1, invalid = 0x1ffffffff };

constexpr uint64_t gnb_cu_up_ue_e1ap_id_to_uint(gnb_cu_up_ue_e1ap_id_t id)
{
  return static_cast<uint64_t>(id);
}

/// Convert integer to GNB-DU-UE-E1AP-ID type.
constexpr gnb_cu_up_ue_e1ap_id_t int_to_gnb_cu_up_ue_e1ap_id(uint64_t idx)
{
  return static_cast<gnb_cu_up_ue_e1ap_id_t>(idx);
}

/// \brief gNB-CU-CP MBS E1AP ID used to identify the MBS context in the CU-CP E1AP.
/// \remark See TS 37.483 Section 9.3.1.106: gNB-CU-CP MBS E1AP ID valid values: (0..2^24-1)
static constexpr uint32_t MAX_NOF_CU_CP_E1AP_MBS_IDS = ((uint32_t)1 << 24);
enum class gnb_cu_cp_mbs_e1ap_id_t : uint32_t { min = 0, max = MAX_NOF_CU_CP_E1AP_MBS_IDS - 1, invalid = MAX_NOF_CU_CP_E1AP_MBS_IDS };

constexpr uint32_t gnb_cu_cp_mbs_e1ap_id_to_uint(gnb_cu_cp_mbs_e1ap_id_t id)
{
  return static_cast<uint32_t>(id);
}

/// Convert integer to gNB-CU-CP MBS E1AP ID type.
constexpr gnb_cu_cp_mbs_e1ap_id_t uint_to_gnb_cu_cp_mbs_e1ap_id(uint32_t idx)
{
  return static_cast<gnb_cu_cp_mbs_e1ap_id_t>(idx);
}

/// \brief gNB-CU-UP MBS E1AP ID used to identify the MBS context in the CU-UP E1AP.
/// \remark See TS 37.483 Section 9.3.1.107: gNB-CU-UP MBS E1AP ID valid values: (0..2^16-1)
static constexpr uint32_t MAX_NOF_CU_UP_E1AP_MBS_IDS = ((uint32_t)1 << 16);
enum class gnb_cu_up_mbs_e1ap_id_t : uint32_t { min = 0, max = MAX_NOF_CU_UP_E1AP_MBS_IDS - 1, invalid = MAX_NOF_CU_UP_E1AP_MBS_IDS };

constexpr uint32_t gnb_cu_up_mbs_e1ap_id_to_uint(gnb_cu_up_mbs_e1ap_id_t id)
{
  return static_cast<uint32_t>(id);
}

/// Convert integer to gNB-CU-UP MBS E1AP ID type.
constexpr gnb_cu_up_mbs_e1ap_id_t uint_to_gnb_cu_up_mbs_e1ap_id(uint32_t idx)
{
  return static_cast<gnb_cu_up_mbs_e1ap_id_t>(idx);
}

/// \brief Broadcast F1-U Context ReferenceE1 used to identify the Broadcast F1-U context within an MBS-associated logical E1-connection.
/// \remark See TS 37.483 Section 9.3.1.139a: Broadcast F1-U Context ReferenceE1 valid values: (0..2^16-1)
static constexpr uint32_t MAX_NOF_F1U_CONTEXT_REFERENCE_E1 = ((uint32_t)1 << 16);
enum class e1ap_bc_f1u_context_reference_e1 : uint32_t { min = 0, max = MAX_NOF_F1U_CONTEXT_REFERENCE_E1 - 1, invalid = MAX_NOF_F1U_CONTEXT_REFERENCE_E1 };

constexpr uint32_t e1ap_bc_f1u_context_reference_e1_to_uint(e1ap_bc_f1u_context_reference_e1 id)
{
  return static_cast<uint32_t>(id);
}

/// Convert integer to Broadcast F1-U Context ReferenceE1 type.
constexpr e1ap_bc_f1u_context_reference_e1 uint_to_e1ap_bc_f1u_context_reference_e1(uint32_t idx)
{
  return static_cast<e1ap_bc_f1u_context_reference_e1>(idx);
}

enum class e1ap_ul_cfg { no_data = 0, shared, only };

enum class e1ap_dl_tx_stop { stop = 0, resume };

enum class e1ap_rat_type { e_utra = 0, nr };

struct e1ap_cell_group_info_item {
  uint8_t                        cell_group_id = 0;
  std::optional<e1ap_ul_cfg>     ul_cfg;
  std::optional<e1ap_dl_tx_stop> dl_tx_stop;
  std::optional<e1ap_rat_type>   rat_type;
};

struct e1ap_qos_flow_level_qos_params {
  qos_characteristics                     qos_desc;
  alloc_and_retention_priority            ng_ran_alloc_retention;
  std::optional<gbr_qos_flow_information> gbr_qos_flow_info;
  std::optional<bool>                     reflective_qos_attribute;
  std::optional<bool>                     add_qos_info;
  std::optional<uint8_t>                  paging_policy_ind;
  std::optional<bool>                     reflective_qos_ind;
};

enum class e1ap_qos_flow_map_ind { ul = 0, dl };

struct e1ap_qos_flow_qos_param_item {
  qos_flow_id_t                        qos_flow_id = qos_flow_id_t::invalid;
  e1ap_qos_flow_level_qos_params       qos_flow_level_qos_params;
  std::optional<e1ap_qos_flow_map_ind> qos_flow_map_ind;
};

struct e1ap_qos_flow_map_item {
  qos_flow_id_t                        qos_flow_id = qos_flow_id_t::invalid;
  std::optional<e1ap_qos_flow_map_ind> qos_flow_map_ind;
};

struct e1ap_data_forwarding_info_request {
  std::string                                              data_forwarding_request;
  slotted_id_vector<qos_flow_id_t, e1ap_qos_flow_map_item> qos_flows_forwarded_on_fwd_tunnels;
};

struct e1ap_pdcp_count {
  uint32_t pdcp_sn;
  uint64_t hfn;
};

struct e1ap_drb_status_transfer {
  e1ap_pdcp_count         count_value;
  std::optional<uint64_t> receive_status_of_pdcp_sdu;
};

struct e1ap_pdcp_sn_status_info {
  e1ap_drb_status_transfer pdcp_status_transfer_ul;
  e1ap_pdcp_count          pdcp_status_transfer_dl;
};

struct e1ap_rohc {
  uint16_t            max_cid;
  uint16_t            rohc_profiles;
  std::optional<bool> continue_rohc;
};

struct e1ap_rohc_params {
  std::optional<e1ap_rohc> rohc;
  std::optional<e1ap_rohc> ul_only_rohc;
};

enum class e1ap_dupl_activation { active = 0, inactive };

struct e1ap_pdcp_config {
  pdcp_sn_size                        pdcp_sn_size_ul;
  pdcp_sn_size                        pdcp_sn_size_dl;
  srsran::pdcp_rlc_mode               rlc_mod;
  std::optional<e1ap_rohc_params>     rohc_params;
  std::optional<pdcp_t_reordering>    t_reordering_timer;
  std::optional<pdcp_discard_timer>   discard_timer;
  std::optional<int32_t>              ul_data_split_thres;
  std::optional<bool>                 pdcp_dupl;
  std::optional<bool>                 pdcp_reest;
  std::optional<bool>                 pdcp_data_recovery;
  std::optional<e1ap_dupl_activation> dupl_activation;
  std::optional<bool>                 out_of_order_delivery;
};

struct e1ap_drb_to_setup_item_ng_ran {
  drb_id_t                                                       drb_id = drb_id_t::invalid;
  sdap_config_t                                                  sdap_cfg;
  e1ap_pdcp_config                                               pdcp_cfg;
  std::vector<e1ap_cell_group_info_item>                         cell_group_info;
  slotted_id_vector<qos_flow_id_t, e1ap_qos_flow_qos_param_item> qos_flow_info_to_be_setup;
  std::optional<e1ap_data_forwarding_info_request>               drb_data_forwarding_info_request;
  std::optional<std::chrono::seconds>                            drb_inactivity_timer;
  std::optional<e1ap_pdcp_sn_status_info>                        pdcp_sn_status_info;
};

struct e1ap_pdu_session_res_to_setup_item {
  pdu_session_id_t                                           pdu_session_id = pdu_session_id_t::invalid;
  pdu_session_type_t                                         pdu_session_type;
  s_nssai_t                                                  snssai;
  up_transport_layer_info                                    ng_ul_up_tnl_info;
  security_indication_t                                      security_ind;
  slotted_id_vector<drb_id_t, e1ap_drb_to_setup_item_ng_ran> drb_to_setup_list_ng_ran;

  std::optional<uint64_t>                          pdu_session_res_dl_ambr;
  std::optional<e1ap_data_forwarding_info_request> pdu_session_data_forwarding_info_request;
  std::optional<std::chrono::seconds>              pdu_session_inactivity_timer;
  std::optional<up_transport_layer_info>           existing_allocated_ng_dl_up_tnl_info;
  std::optional<uint16_t>                          network_instance;
};

struct e1ap_security_algorithm {
  srsran::security::ciphering_algorithm ciphering_algo;
  // Optional, TS 38.463 Sec. 9.4.5.
  std::optional<srsran::security::integrity_algorithm> integrity_protection_algorithm;
};

struct e1ap_up_security_key {
  e1ap_up_security_key& operator=(const e1ap_up_security_key& other)
  {
    encryption_key           = other.encryption_key.copy();
    integrity_protection_key = other.integrity_protection_key.copy();
    return *this;
  }
  byte_buffer encryption_key;
  // Optional, TS 38.463 Sec. 9.4.5.
  byte_buffer integrity_protection_key;
};

struct e1ap_security_info {
  e1ap_security_algorithm security_algorithm;
  e1ap_up_security_key    up_security_key;
};

enum class e1ap_bearer_context_status_change { suspend = 0, resume };

enum class e1ap_activity_notif_level { drb = 0, pdu_session, ue };

struct e1ap_up_params_item {
  up_transport_layer_info up_tnl_info;
  uint8_t                 cell_group_id;
};

/// See TS 37.483 Section 9.3.1.12: QoS Flow List
struct e1ap_qos_flow_item {
  //qos_flow_id (M)
  qos_flow_id_t qos_flow_id = qos_flow_id_t::invalid;
  //qos_flow_mapping_indication (O)
  //std::optional<qos_flow_mapping_indication>
  //data_forwarding_source_ip_address (O)
  //std::optional<data_forwarding_source_ip_address>
};

/// See TS 37.483 Section 9.3.1.45: Flow Failed List
struct e1ap_qos_flow_failed_item {
  qos_flow_id_t qos_flow_id = qos_flow_id_t::invalid;
  e1ap_cause_t  cause;
};

struct e1ap_data_forwarding_info {
  std::optional<up_transport_layer_info> ul_data_forwarding;
  std::optional<up_transport_layer_info> dl_data_forwarding;
};

struct e1ap_drb_setup_item_ng_ran {
  drb_id_t                                                    drb_id = drb_id_t::invalid;
  std::vector<e1ap_up_params_item>                            ul_up_transport_params;
  slotted_id_vector<qos_flow_id_t, e1ap_qos_flow_item>        flow_setup_list;
  slotted_id_vector<qos_flow_id_t, e1ap_qos_flow_failed_item> flow_failed_list;
  std::optional<e1ap_data_forwarding_info>                    drb_data_forwarding_info_resp;
};

struct e1ap_drb_failed_item_ng_ran {
  drb_id_t     drb_id = drb_id_t::invalid;
  e1ap_cause_t cause;
};

struct e1ap_pdu_session_resource_setup_modification_item {
  pdu_session_id_t                                         pdu_session_id = pdu_session_id_t::invalid;
  up_transport_layer_info                                  ng_dl_up_tnl_info;
  slotted_id_vector<drb_id_t, e1ap_drb_setup_item_ng_ran>  drb_setup_list_ng_ran;
  slotted_id_vector<drb_id_t, e1ap_drb_failed_item_ng_ran> drb_failed_list_ng_ran;
  std::optional<security_result_t>                         security_result;
  std::optional<e1ap_data_forwarding_info>                 pdu_session_data_forwarding_info_resp;
  std::optional<bool>                                      ng_dl_up_unchanged;
};

struct e1ap_pdu_session_resource_failed_item {
  pdu_session_id_t pdu_session_id = pdu_session_id_t::invalid;
  e1ap_cause_t     cause;
};

struct e1ap_crit_diagnostics_item {
  std::string iecrit;
  uint32_t    ie_id;
  std::string type_of_error;
};

enum class e1ap_trigger_msg { init_msg = 0, successful_outcome, unsuccessful_outcome };

enum class e1ap_proc_crit { reject = 0, ignore, notify };

struct e1ap_crit_diagnostics {
  std::vector<e1ap_crit_diagnostics_item> ies_crit_diagnostics;
  std::optional<uint16_t>                 proc_code;
  std::optional<e1ap_trigger_msg>         trigger_msg;
  std::optional<e1ap_proc_crit>           proc_crit;
  std::optional<uint16_t>                 transaction_id;
};

struct e1ap_drb_to_setup_mod_item_ng_ran {
  drb_id_t                                                       drb_id = drb_id_t::invalid;
  sdap_config_t                                                  sdap_cfg;
  e1ap_pdcp_config                                               pdcp_cfg;
  std::vector<e1ap_cell_group_info_item>                         cell_group_info;
  slotted_id_vector<qos_flow_id_t, e1ap_qos_flow_qos_param_item> flow_map_info;
  std::optional<e1ap_data_forwarding_info_request>               drb_data_forwarding_info_request;
  std::optional<uint16_t>                                        drb_inactivity_timer;
  std::optional<e1ap_pdcp_sn_status_info>                        pdcp_sn_status_info;
};

struct e1ap_drb_to_modify_item_ng_ran {
  drb_id_t                                                       drb_id = drb_id_t::invalid;
  std::optional<sdap_config_t>                                   sdap_cfg;
  std::optional<e1ap_pdcp_config>                                pdcp_cfg;
  std::optional<e1ap_data_forwarding_info>                       drb_data_forwarding_info;
  std::optional<bool>                                            pdcp_sn_status_request;
  std::vector<e1ap_up_params_item>                               dl_up_params;
  std::vector<e1ap_cell_group_info_item>                         cell_group_to_add;
  std::vector<e1ap_cell_group_info_item>                         cell_group_to_modify;
  std::vector<e1ap_cell_group_info_item>                         cell_group_to_rem;
  slotted_id_vector<qos_flow_id_t, e1ap_qos_flow_qos_param_item> flow_map_info;
  std::optional<uint16_t>                                        drb_inactivity_timer;
};

struct e1ap_pdu_session_res_to_modify_item {
  pdu_session_id_t                                            pdu_session_id = pdu_session_id_t::invalid;
  std::optional<security_indication_t>                        security_ind;
  std::optional<uint64_t>                                     pdu_session_res_dl_ambr;
  std::optional<up_transport_layer_info>                      ng_ul_up_tnl_info;
  std::optional<e1ap_data_forwarding_info_request>            pdu_session_data_forwarding_info_request;
  std::optional<e1ap_data_forwarding_info>                    pdu_session_data_forwarding_info;
  std::optional<uint16_t>                                     pdu_session_inactivity_timer;
  std::optional<uint16_t>                                     network_instance;
  slotted_id_vector<drb_id_t, e1ap_drb_to_setup_item_ng_ran>  drb_to_setup_list_ng_ran;
  slotted_id_vector<drb_id_t, e1ap_drb_to_modify_item_ng_ran> drb_to_modify_list_ng_ran;
  std::vector<drb_id_t>                                       drb_to_rem_list_ng_ran;

  slotted_id_vector<drb_id_t, e1ap_drb_to_setup_mod_item_ng_ran> drb_to_setup_mod_list_ng_ran;
};

struct e1ap_ng_ran_bearer_context_mod_request {
  slotted_id_vector<pdu_session_id_t, e1ap_pdu_session_res_to_setup_item>  pdu_session_res_to_setup_mod_list;
  slotted_id_vector<pdu_session_id_t, e1ap_pdu_session_res_to_modify_item> pdu_session_res_to_modify_list;
  std::vector<pdu_session_id_t>                                            pdu_session_res_to_rem_list;
};

struct e1ap_drb_modified_item_ng_ran {
  drb_id_t                                                    drb_id = drb_id_t::invalid;
  std::vector<e1ap_up_params_item>                            ul_up_transport_params;
  slotted_id_vector<qos_flow_id_t, e1ap_qos_flow_item>        flow_setup_list;
  slotted_id_vector<qos_flow_id_t, e1ap_qos_flow_failed_item> flow_failed_list;
  std::optional<e1ap_pdcp_sn_status_info>                     pdcp_sn_status_info;
};

struct e1ap_pdu_session_resource_modified_item {
  pdu_session_id_t                                           pdu_session_id = pdu_session_id_t::invalid;
  std::optional<up_transport_layer_info>                     ng_dl_up_tnl_info;
  slotted_id_vector<drb_id_t, e1ap_drb_setup_item_ng_ran>    drb_setup_list_ng_ran;
  slotted_id_vector<drb_id_t, e1ap_drb_failed_item_ng_ran>   drb_failed_list_ng_ran;
  slotted_id_vector<drb_id_t, e1ap_drb_modified_item_ng_ran> drb_modified_list_ng_ran;
  slotted_id_vector<drb_id_t, e1ap_drb_failed_item_ng_ran>   drb_failed_to_modify_list_ng_ran;
  std::optional<security_result_t>                           security_result;
  std::optional<e1ap_data_forwarding_info>                   pdu_session_data_forwarding_info_resp;
};

/// See TS 37.483 Section 9.3.1.114: BC MRB Setup Configuration
struct e1ap_bc_mrb_setup_config {
  //mrb_id (M)
  mrb_id_t mrb_id = mrb_id_t::invalid;
  //mbs_pdcp_configuration (M)
  e1ap_pdcp_config mbs_pdcp_cfg;
  //mbs_qos_flows_information_to_be_setup (M)
  std::vector<e1ap_qos_flow_qos_param_item> mbs_qos_flow_info_to_be_setup;
  //mrb_qos (O)
  std::optional<e1ap_qos_flow_level_qos_params> mrb_qos;
  //f1u_tnl_info_to_add_list (0..1)
  std::vector<e1ap_bc_f1u_context_reference_e1> f1u_tnl_info_to_add_list;
};

// NOTE (borieher): In the spec is a choice type with only one option
/// \brief MBS NG-U Information at 5GC.
/// \remark See TS 37.483, 9.3.1.113 - MBS NG-U Information at 5GC.
struct e1ap_mbs_ngu_information_at_5gc {
  //ip_multicast_address (M)
  transport_layer_address ip_multicast_address;
  //ip_source_address (M)
  transport_layer_address ip_source_address;
  //gtp_dl_teid (M)
  gtpu_teid_t gtp_dl_teid;
};

struct e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent_item {
  //mbs_area_session_id (M)
  area_session_id_t mbs_area_session_id;
  //mbs_ngu_information_at_5gc (M)
  e1ap_mbs_ngu_information_at_5gc mbs_ngu_information_at_5gc;
};

struct e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_independent {
  //mbs_ngu_information_at_5gc (M)
  e1ap_mbs_ngu_information_at_5gc mbs_ngu_information_at_5gc;
};

struct e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent {
  //location_dependent_mbs_ngu_info_at_5gc (1..maxnoofMBSAreaSessionIDs)
  std::vector<e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent_item> location_dependent_mbs_ngu_info_at_5gc;
};


/// \brief BC Bearer Context NG-U TNL Info at 5GC.
/// \remark See TS 37.483, 9.3.1.112 - BC Bearer Context NG-U TNL Info at 5GC.
struct e1ap_bc_bearer_context_ngu_tnl_info_at_5gc {
  e1ap_bc_bearer_context_ngu_tnl_info_at_5gc() : choice(e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent{}) {}
  e1ap_bc_bearer_context_ngu_tnl_info_at_5gc(const e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_independent& val) : choice(val) {}
  e1ap_bc_bearer_context_ngu_tnl_info_at_5gc(const e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent& val) : choice(val) {}

  bool is_locationdependent() const { return std::holds_alternative<e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent>(choice); }

  e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_independent get_locationindependent() { return std::get<e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_independent>(choice); }
  e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent get_locationdependent() { return std::get<e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent>(choice); }

private:
  std::variant<e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_independent, e1ap_bc_bearer_ctxt_ngu_tnl_info_at_5gc_location_dependent> choice;
};

/// See TS 37.483 Section 9.3.1.115: Requested Action for Available Shared NG-U Termination
enum class e1ap_requested_action_for_available_shared_ngu_termination : uint8_t {
  apply_available_config = 0,
  apply_requested_config,
  apply_available_config_if_same_as_requested,
  unspecified
};

/// See TS 37.483 Section 9.3.3.26: BC Bearer Context To Setup
struct e1ap_bc_bearer_context_to_setup {
  //s_nssai (M)
  s_nssai_t s_nssai;
  //bc_bearer_context_ngu_tnl_info_at_5gc (O)
  std::optional<e1ap_bc_bearer_context_ngu_tnl_info_at_5gc> bc_bearer_context_ngu_tnl_info_at_5gc;
  //bc_mrb_to_setup_list (M)
  std::vector<e1ap_bc_mrb_setup_config> bc_mrb_to_setup_list;
  //requested_action_for_available_shared_ngu_termination (O)
  std::optional<e1ap_requested_action_for_available_shared_ngu_termination> requested_action_for_available_shared_ngu_termination;
};

struct e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent_item {
  //mbs_area_session_id (M)
  area_session_id_t mbs_area_session_id;
  //mbs_f1u_information_at_cu (M)
  up_transport_layer_info mbs_f1u_information_at_cu;
};

struct e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_independent {
  //mbs_f1u_information_at_cu (M)
  up_transport_layer_info mbs_f1u_information_at_cu;
};

struct e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent {
  //location_dependent_mbs_f1u_information_at_cu (1..maxnoofMBSAreaSessionIDs)
  std::vector<e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent_item> location_dependent_mbs_f1u_information_at_cu;
};

/// \brief BC Bearer Context F1-U TNL Info at CU.
/// \remark See TS 37.483, 9.3.1.118 - BC Bearer Context F1-U TNL Info at CU.
struct e1ap_bc_bearer_context_f1u_tnl_info_at_cu {
  e1ap_bc_bearer_context_f1u_tnl_info_at_cu() : choice(e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent{}) {}
  e1ap_bc_bearer_context_f1u_tnl_info_at_cu(const e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_independent& val) : choice(val) {}
  e1ap_bc_bearer_context_f1u_tnl_info_at_cu(const e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent& val) : choice(val) {}

  bool is_locationdependent() const { return std::holds_alternative<e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent>(choice); }

  e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_independent get_locationindependent() { return std::get<e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_independent>(choice); }
  e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent get_locationdependent() { return std::get<e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent>(choice); }

private:
  std::variant<e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_independent, e1ap_bc_bearer_ctxt_f1u_tnl_at_cu_location_dependent> choice;
};

struct e1ap_f1u_tnl_info_added_item {
  //bc_f1u_context_reference_e1 (M)
  e1ap_bc_f1u_context_reference_e1 bc_f1u_context_reference_e1;
  //bc_bearer_context_f1u_tnl_info_at_cu (M)
  e1ap_bc_bearer_context_f1u_tnl_info_at_cu bc_bearer_context_f1u_tnl_info_at_cu;
};

struct e1ap_f1u_tnl_info_added_or_modified_item {
  //bc_f1u_context_reference_e1 (M)
  e1ap_bc_f1u_context_reference_e1 bc_f1u_context_reference_e1;
  //bc_bearer_context_f1u_tnl_info_at_cu (O)
  std::optional<e1ap_bc_bearer_context_f1u_tnl_info_at_cu> bc_bearer_context_f1u_tnl_info_at_cu;
};

struct e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_dependent_item {
  //mbs_area_session_id (M)
  area_session_id_t mbs_area_session_id;
  //mbs_f1u_information_at_du (M)
  up_transport_layer_info mbs_f1u_information_at_du;
};

struct e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_independent {
  //mbs_f1u_information_at_du (M)
  up_transport_layer_info mbs_f1u_information_at_du;
};

struct e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_dependent {
  //location_dependent_mbs_f1u_information_at_du (1..maxnoofMBSAreaSessionIDs)
  std::vector<e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_dependent_item> location_dependent_mbs_f1u_information_at_du;
};

/// \brief BC Bearer Context F1-U TNL Info at DU.
/// \remark See TS 37.483, 9.3.1.119 - BC Bearer Context F1-U TNL Info at DU.
struct e1ap_bc_bearer_context_f1u_tnl_info_at_du {
  e1ap_bc_bearer_context_f1u_tnl_info_at_du() : choice(e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_dependent{}) {}
  e1ap_bc_bearer_context_f1u_tnl_info_at_du(const e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_independent& val) : choice(val) {}
  e1ap_bc_bearer_context_f1u_tnl_info_at_du(const e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_dependent& val) : choice(val) {}

  bool is_locationdependent() const { return std::holds_alternative<e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_dependent>(choice); }

  e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_independent get_locationindependent() { return std::get<e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_independent>(choice); }
  e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_dependent get_locationdependent() { return std::get<e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_dependent>(choice); }

private:
  std::variant<e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_independent, e1ap_bc_bearer_ctxt_f1u_tnl_at_du_location_dependent> choice;
};

struct e1ap_f1u_tnl_info_to_add_or_modify_item {
  //bc_f1u_context_reference_e1 (M)
  e1ap_bc_f1u_context_reference_e1 bc_f1u_context_reference_e1;
  //bc_bearer_context_f1u_tnl_info_at_du (O)
  std::optional<e1ap_bc_bearer_context_f1u_tnl_info_at_du> bc_bearer_context_f1u_tnl_info_at_du;
};

struct e1ap_bc_mrb_setup_response_item {
  //mrb_id (M)
  mrb_id_t mrb_id = mrb_id_t::invalid;
  //mbs_qos_flow_setup_list (M)
  std::vector<e1ap_qos_flow_item> mbs_qos_flow_setup_list;
  //mbs_qos_flow_failed_list (O)
  std::optional<std::vector<e1ap_qos_flow_failed_item>> mbs_qos_flow_failed_list;
  //bc_bearer_context_f1u_tnl_info_at_cu (M)
  e1ap_bc_bearer_context_f1u_tnl_info_at_cu bc_bearer_context_f1u_tnl_info_at_cu;
  //f1u_tnl_info_added_list (0..1)
  std::vector<e1ap_f1u_tnl_info_added_item> f1u_tnl_info_added_list;
};

struct e1ap_bc_mrb_failed_item {
  //mrb_id (M)
  mrb_id_t mrb_id = mrb_id_t::invalid;
  //cause (M)
  e1ap_cause_t cause;
};

// NOTE (borieher): In the spec is a choice type with only one option
/// \brief MBS NG-U Information at NG-RAN.
/// \remark See TS 37.483, 9.3.1.117 - MBS NG-U Information at NG-RAN.
struct e1ap_mbs_ngu_information_at_ngran {
  //shared_ngu_dl_transport_layer_info (M)
  up_transport_layer_info shared_ngu_dl_transport_layer_info;
};

struct e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran_location_dependent_item {
  //mbs_area_session_id (M)
  area_session_id_t mbs_area_session_id;
  //mbs_ngu_information_at_ngran (M)
  e1ap_mbs_ngu_information_at_ngran mbs_ngu_information_at_ngran;
};

struct e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran_location_independent {
  //mbs_ngu_information_at_ngran (M)
  e1ap_mbs_ngu_information_at_ngran mbs_ngu_information_at_ngran;
};

struct e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran_location_dependent {
  //location_dependent_mbs_ngu_information_at_ngran (1..maxnoofMBSAreaSessionIDs)
  std::vector<e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran_location_dependent_item> location_dependent_mbs_ngu_information_at_ngran;
};

/// \brief BC Bearer Context NG-U TNL Info at NG-RAN.
/// \remark See TS 37.483, 9.3.1.116 - BC Bearer Context NG-U TNL Info at NG-RAN.
struct e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran {
  e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran() : choice(e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran_location_dependent{}) {}
  e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran(const e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran_location_independent& val) : choice(val) {}
  e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran(const e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran_location_dependent& val) : choice(val) {}

  bool is_locationdependent() const { return std::holds_alternative<e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran_location_dependent>(choice); }

  e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran_location_independent get_locationindependent() { return std::get<e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran_location_independent>(choice); }
  e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran_location_dependent get_locationdependent() { return std::get<e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran_location_dependent>(choice); }

private:
  std::variant<e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran_location_independent, e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran_location_dependent> choice;
};

/// See TS 37.483 Section 9.3.3.27: BC Bearer Context To Setup Response
struct e1ap_bc_bearer_context_to_setup_response {
  //bc_bearer_context_ngu_tnl_info_at_ng_ran (O)
  std::optional<e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran> bc_bearer_context_ngu_tnl_info_at_ng_ran;
  //bc_mrb_setup_response_list (1..maxnoofMRBs)
  std::vector<e1ap_bc_mrb_setup_response_item> bc_mrb_setup_response_list;
  //bc_mrb_failed_list (0..maxnoofMRBs)
  std::vector<e1ap_bc_mrb_failed_item> bc_mrb_failed_list;
  //available_bc_mrb_configuration (O)
  std::optional<std::vector<e1ap_bc_mrb_setup_config>> available_bc_mrb_configuration;
};

struct e1ap_bc_mrb_to_modify_item {
  //mrb_id (M)
  mrb_id_t mrb_id = mrb_id_t::invalid;
  //bc_bearer_context_f1u_tnl_info_at_du (O)
  std::optional<e1ap_bc_bearer_context_f1u_tnl_info_at_du> bc_bearer_context_f1u_tnl_info_at_du;
  //mbs_pdcp_configuration (O)
  std::optional<e1ap_pdcp_config> mbs_pdcp_cfg;
  //mbs_qos_flows_information_to_be_setup (O)
  std::optional<std::vector<e1ap_qos_flow_qos_param_item>> mbs_qos_flow_info_to_be_setup;
  //mrb_qos (O)
  std::optional<e1ap_qos_flow_level_qos_params> mrb_qos;
  //f1u_tnl_info_to_add_or_modify_list (0..1)
  std::vector<e1ap_f1u_tnl_info_to_add_or_modify_item> f1u_tnl_info_to_add_or_modify_list;
  //f1u_tnl_info_to_release_list (0..1)
  std::vector<e1ap_bc_f1u_context_reference_e1> f1u_tnl_info_to_release_list;
};

struct e1ap_bc_mrb_to_remove_item {
  //mrb_id (M)
  mrb_id_t mrb_id = mrb_id_t::invalid;
};

/// See TS 37.483 Section 9.3.3.28: BC Bearer Context To Modify
struct e1ap_bc_bearer_context_to_modify {
  //bc_bearer_context_ngu_tnl_info_at_5gc_to_setup_or_modify (O)
  std::optional<e1ap_bc_bearer_context_ngu_tnl_info_at_5gc> bc_bearer_context_ngu_tnl_info_at_5gc_to_setup_or_modify;
  //bc_mrb_to_setup_list (O)
  std::optional<std::vector<e1ap_bc_mrb_setup_config>> bc_mrb_to_setup_list;
  //bc_mrb_to_modify_list (0..maxnoofMRBs)
  std::vector<e1ap_bc_mrb_to_modify_item> bc_mrb_to_modify_list;
  //bc_mrb_to_remove_list (0..maxnoofMRBs)
  std::vector<e1ap_bc_mrb_to_remove_item> bc_mrb_to_remove_list;
};

struct e1ap_bc_mrb_setup_or_modify_response_item {
  //mrb_id (M)
  mrb_id_t mrb_id = mrb_id_t::invalid;
  //mbs_qos_flow_setup_list (O)
  std::optional<std::vector<e1ap_qos_flow_item>> mbs_qos_flow_setup_list;
  //mbs_qos_flow_failed_list (O)
  std::optional<std::vector<e1ap_qos_flow_failed_item>> mbs_qos_flow_failed_list;
  //bc_bearer_context_f1u_tnl_info_at_cu (O)
  std::optional<e1ap_bc_bearer_context_f1u_tnl_info_at_cu> bc_bearer_context_f1u_tnl_info_at_cu;
  //f1u_tnl_info_added_or_modified_list (0..1)
  std::vector<e1ap_f1u_tnl_info_added_or_modified_item> f1u_tnl_info_added_or_modified_list;
};

/// See TS 37.483 Section 9.3.3.29: BC Bearer Context To Modify Response
struct e1ap_bc_bearer_context_to_modify_response {
  //bc_bearer_context_ngu_tnl_info_at_ng_ran (O)
  std::optional<e1ap_bc_bearer_context_ngu_tnl_info_at_ng_ran> bc_bearer_context_ngu_tnl_info_at_ng_ran;
  //bc_mrb_setup_or_modify_response_list (1..maxnoofMRBs)
  std::vector<e1ap_bc_mrb_setup_or_modify_response_item> bc_mrb_setup_or_modify_response_list;
  //bc_mrb_failed_list (0..maxnoofMRBs)
  std::vector<e1ap_bc_mrb_failed_item> bc_mrb_failed_list;
  //available_bc_mrb_configuration (O)
  std::optional<std::vector<e1ap_bc_mrb_setup_config>> available_bc_mrb_configuration;
};

} // namespace srsran
