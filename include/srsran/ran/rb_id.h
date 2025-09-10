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

#include "srsran/ran/logical_channel/lcid.h"
#include "fmt/format.h"

namespace srsran {

constexpr std::size_t MAX_NOF_SRBS = 4;

enum class srb_id_t : uint8_t { srb0 = 0, srb1, srb2, srb3, nulltype };

constexpr inline std::underlying_type_t<srb_id_t> srb_id_to_uint(srb_id_t id)
{
  return static_cast<std::underlying_type_t<srb_id_t>>(id);
}

inline srb_id_t int_to_srb_id(std::underlying_type_t<srb_id_t> val)
{
  return static_cast<srb_id_t>(val);
}

inline srb_id_t to_srb_id(lcid_t lcid)
{
  return is_srb(lcid) ? static_cast<srb_id_t>(lcid) : srb_id_t::nulltype;
}

/// Convert SRB ID to LCID.
inline lcid_t srb_id_to_lcid(srb_id_t srb_id)
{
  return static_cast<lcid_t>(srb_id);
}

inline const char* srb_id_to_string(srb_id_t srb_id)
{
  static constexpr const char* names[] = {"SRB0", "SRB1", "SRB2", "SRB3", "invalid"};
  return names[srb_id_to_uint(srb_id < srb_id_t::nulltype ? srb_id : srb_id_t::nulltype)];
}

enum class drb_id_t : uint8_t {
  drb1 = 1,
  drb2,
  drb3,
  drb4,
  drb5,
  drb6,
  drb7,
  drb8,
  drb9,
  drb10,
  drb11,
  drb12,
  drb13,
  drb14,
  drb15,
  drb16,
  drb17,
  drb18,
  drb19,
  drb20,
  drb21,
  drb22,
  drb23,
  drb24,
  drb25,
  drb26,
  drb27,
  drb28,
  drb29,
  invalid
};

static constexpr std::size_t MAX_NOF_DRBS = 29;

constexpr inline uint8_t drb_id_to_uint(drb_id_t id)
{
  return static_cast<uint8_t>(id);
}

constexpr inline drb_id_t uint_to_drb_id(uint8_t id)
{
  return static_cast<drb_id_t>(id);
}

enum class mrb_id_t : uint16_t {
  mrb1 = 1, mrb2, mrb3, mrb4, mrb5, mrb6, mrb7, mrb8, mrb9, mrb10, mrb11, mrb12, mrb13, mrb14, mrb15, mrb16,
  mrb17, mrb18, mrb19, mrb20, mrb21, mrb22, mrb23, mrb24, mrb25, mrb26, mrb27, mrb28, mrb29, mrb30, mrb31, mrb32,
  mrb33, mrb34, mrb35, mrb36, mrb37, mrb38, mrb39, mrb40, mrb41, mrb42, mrb43, mrb44, mrb45, mrb46, mrb47, mrb48,
  mrb49, mrb50, mrb51, mrb52, mrb53, mrb54, mrb55, mrb56, mrb57, mrb58, mrb59, mrb60, mrb61, mrb62, mrb63, mrb64,
  mrb65, mrb66, mrb67, mrb68, mrb69, mrb70, mrb71, mrb72, mrb73, mrb74, mrb75, mrb76, mrb77, mrb78, mrb79, mrb80,
  mrb81, mrb82, mrb83, mrb84, mrb85, mrb86, mrb87, mrb88, mrb89, mrb90, mrb91, mrb92, mrb93, mrb94, mrb95, mrb96,
  mrb97, mrb98, mrb99, mrb100, mrb101, mrb102, mrb103, mrb104, mrb105, mrb106, mrb107, mrb108, mrb109, mrb110, mrb111, mrb112,
  mrb113, mrb114, mrb115, mrb116, mrb117, mrb118, mrb119, mrb120, mrb121, mrb122, mrb123, mrb124, mrb125, mrb126, mrb127, mrb128,
  mrb129, mrb130, mrb131, mrb132, mrb133, mrb134, mrb135, mrb136, mrb137, mrb138, mrb139, mrb140, mrb141, mrb142, mrb143, mrb144,
  mrb145, mrb146, mrb147, mrb148, mrb149, mrb150, mrb151, mrb152, mrb153, mrb154, mrb155, mrb156, mrb157, mrb158, mrb159, mrb160,
  mrb161, mrb162, mrb163, mrb164, mrb165, mrb166, mrb167, mrb168, mrb169, mrb170, mrb171, mrb172, mrb173, mrb174, mrb175, mrb176,
  mrb177, mrb178, mrb179, mrb180, mrb181, mrb182, mrb183, mrb184, mrb185, mrb186, mrb187, mrb188, mrb189, mrb190, mrb191, mrb192,
  mrb193, mrb194, mrb195, mrb196, mrb197, mrb198, mrb199, mrb200, mrb201, mrb202, mrb203, mrb204, mrb205, mrb206, mrb207, mrb208,
  mrb209, mrb210, mrb211, mrb212, mrb213, mrb214, mrb215, mrb216, mrb217, mrb218, mrb219, mrb220, mrb221, mrb222, mrb223, mrb224,
  mrb225, mrb226, mrb227, mrb228, mrb229, mrb230, mrb231, mrb232, mrb233, mrb234, mrb235, mrb236, mrb237, mrb238, mrb239, mrb240,
  mrb241, mrb242, mrb243, mrb244, mrb245, mrb246, mrb247, mrb248, mrb249, mrb250, mrb251, mrb252, mrb253, mrb254, mrb255, mrb256,
  mrb257, mrb258, mrb259, mrb260, mrb261, mrb262, mrb263, mrb264, mrb265, mrb266, mrb267, mrb268, mrb269, mrb270, mrb271, mrb272,
  mrb273, mrb274, mrb275, mrb276, mrb277, mrb278, mrb279, mrb280, mrb281, mrb282, mrb283, mrb284, mrb285, mrb286, mrb287, mrb288,
  mrb289, mrb290, mrb291, mrb292, mrb293, mrb294, mrb295, mrb296, mrb297, mrb298, mrb299, mrb300, mrb301, mrb302, mrb303, mrb304,
  mrb305, mrb306, mrb307, mrb308, mrb309, mrb310, mrb311, mrb312, mrb313, mrb314, mrb315, mrb316, mrb317, mrb318, mrb319, mrb320,
  mrb321, mrb322, mrb323, mrb324, mrb325, mrb326, mrb327, mrb328, mrb329, mrb330, mrb331, mrb332, mrb333, mrb334, mrb335, mrb336,
  mrb337, mrb338, mrb339, mrb340, mrb341, mrb342, mrb343, mrb344, mrb345, mrb346, mrb347, mrb348, mrb349, mrb350, mrb351, mrb352,
  mrb353, mrb354, mrb355, mrb356, mrb357, mrb358, mrb359, mrb360, mrb361, mrb362, mrb363, mrb364, mrb365, mrb366, mrb367, mrb368,
  mrb369, mrb370, mrb371, mrb372, mrb373, mrb374, mrb375, mrb376, mrb377, mrb378, mrb379, mrb380, mrb381, mrb382, mrb383, mrb384,
  mrb385, mrb386, mrb387, mrb388, mrb389, mrb390, mrb391, mrb392, mrb393, mrb394, mrb395, mrb396, mrb397, mrb398, mrb399, mrb400,
  mrb401, mrb402, mrb403, mrb404, mrb405, mrb406, mrb407, mrb408, mrb409, mrb410, mrb411, mrb412, mrb413, mrb414, mrb415, mrb416,
  mrb417, mrb418, mrb419, mrb420, mrb421, mrb422, mrb423, mrb424, mrb425, mrb426, mrb427, mrb428, mrb429, mrb430, mrb431, mrb432,
  mrb433, mrb434, mrb435, mrb436, mrb437, mrb438, mrb439, mrb440, mrb441, mrb442, mrb443, mrb444, mrb445, mrb446, mrb447, mrb448,
  mrb449, mrb450, mrb451, mrb452, mrb453, mrb454, mrb455, mrb456, mrb457, mrb458, mrb459, mrb460, mrb461, mrb462, mrb463, mrb464,
  mrb465, mrb466, mrb467, mrb468, mrb469, mrb470, mrb471, mrb472, mrb473, mrb474, mrb475, mrb476, mrb477, mrb478, mrb479, mrb480,
  mrb481, mrb482, mrb483, mrb484, mrb485, mrb486, mrb487, mrb488, mrb489, mrb490, mrb491, mrb492, mrb493, mrb494, mrb495, mrb496,
  mrb497, mrb498, mrb499, mrb500, mrb501, mrb502, mrb503, mrb504, mrb505, mrb506, mrb507, mrb508, mrb509, mrb510, mrb511, mrb512,
  invalid
};

static constexpr std::size_t MAX_NOF_MRBS = 512;

constexpr inline uint16_t mrb_id_to_uint(mrb_id_t id)
{
  return static_cast<uint16_t>(id);
}

constexpr inline mrb_id_t uint_to_mrb_id(uint16_t id)
{
  return static_cast<mrb_id_t>(id);
}

/// Radio Bearer type, either SRB or DRB.
enum class rb_type_t { srb, drb, mrb };

/// Radio Bearer Identity, e.g. SRB1, DRB1, DRB2,...
class rb_id_t
{
public:
  rb_id_t() = default;
  rb_id_t(srb_id_t srb_id_) : rb_type(rb_type_t::srb), srb_id(srb_id_) {}
  rb_id_t(drb_id_t drb_id_) : rb_type(rb_type_t::drb), drb_id(drb_id_) {}
  rb_id_t(mrb_id_t mrb_id_) : rb_type(rb_type_t::mrb), mrb_id(mrb_id_) {}

  // Copy constructor
  rb_id_t(const rb_id_t& other) : rb_type(other.rb_type) {
    switch (rb_type) {
      case rb_type_t::srb: srb_id = other.srb_id; break;
      case rb_type_t::drb: drb_id = other.drb_id; break;
      case rb_type_t::mrb: mrb_id = other.mrb_id; break;
    }
  }

  // Assignment operator
  rb_id_t& operator=(const rb_id_t& other) {
    if (this != &other) {
      rb_type = other.rb_type;
      switch (rb_type) {
        case rb_type_t::srb: srb_id = other.srb_id; break;
        case rb_type_t::drb: drb_id = other.drb_id; break;
        case rb_type_t::mrb: mrb_id = other.mrb_id; break;
      }
    }
    return *this;
  }

  // Equality operator
  bool operator==(const rb_id_t& other) const {
    if (rb_type != other.rb_type) return false;
    switch (rb_type) {
      case rb_type_t::srb: return srb_id == other.srb_id;
      case rb_type_t::drb: return drb_id == other.drb_id;
      case rb_type_t::mrb: return mrb_id == other.mrb_id;
    }
    return false;
  }

  bool is_srb() const { return rb_type == rb_type_t::srb; }
  bool is_drb() const { return rb_type == rb_type_t::drb; }
  bool is_mrb() const { return rb_type == rb_type_t::mrb; }

  srb_id_t get_srb_id() const { return srb_id; }
  drb_id_t get_drb_id() const { return drb_id; }
  mrb_id_t get_mrb_id() const { return mrb_id; }

private:
  rb_type_t rb_type;
  union {
    srb_id_t srb_id;
    drb_id_t drb_id;
    mrb_id_t mrb_id;
  };
};

} // namespace srsran

namespace fmt {

// drb_id_t formatter
template <>
struct formatter<srsran::drb_id_t> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::drb_id_t o, FormatContext& ctx) const
  {
    switch (o) {
      case srsran::drb_id_t::invalid:
        return format_to(ctx.out(), "invalid DRB");
      default:
        return format_to(ctx.out(), "DRB{}", drb_id_to_uint(o));
    }
  }
};

// mrb_id_t formatter
template <>
struct formatter<srsran::mrb_id_t> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::mrb_id_t o, FormatContext& ctx) const
  {
    switch (o) {
      case srsran::mrb_id_t::invalid:
        return format_to(ctx.out(), "invalid MRB");
      default:
        return format_to(ctx.out(), "MRB{}", mrb_id_to_uint(o));
    }
  }
};

// srb_id_t formatter
template <>
struct formatter<srsran::srb_id_t> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::srb_id_t o, FormatContext& ctx) const
  {
    return format_to(ctx.out(), "SRB{}", srb_id_to_uint(o));
  }
};

// rb_id_t formatter
template <>
struct formatter<srsran::rb_id_t> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx)
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::rb_id_t o, FormatContext& ctx) const
  {
    if (o.is_drb()) {
      return format_to(ctx.out(), "{}", o.get_drb_id());
    }
    if (o.is_srb()) {
      return format_to(ctx.out(), "{}", o.get_srb_id());
    }
    if (o.is_mrb()) {
      return format_to(ctx.out(), "{}", o.get_mrb_id());
    }
    return format_to(ctx.out(), "Invalid");
  }
};

} // namespace fmt

// rb_id_t hash function
namespace std {
template <>
struct hash<srsran::rb_id_t> {
  std::size_t operator()(const srsran::rb_id_t& rb) const noexcept {
    using srsran::rb_type_t;
    if (rb.is_srb()) {
      return std::hash<int>()(static_cast<int>(rb_type_t::srb) ^ static_cast<int>(rb.get_srb_id()));
    }
    if (rb.is_drb()) {
      return std::hash<int>()(static_cast<int>(rb_type_t::drb) ^ static_cast<int>(rb.get_drb_id()));
    }
    if (rb.is_mrb()) {
      return std::hash<int>()(static_cast<int>(rb_type_t::mrb) ^ static_cast<int>(rb.get_mrb_id()));
    }
    return 0;
  }
};

} // namespace std
