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

#include "srsran/support/bit_encoding.h"
#include "srsran/support/math/math_utils.h"

using namespace srsran;

bool bit_encoder::pack(uint64_t val, uint32_t n_bits)
{
  srsran_assert(n_bits <= 64U, "Invalid number of bits={} passed to pack()", n_bits);

  while (n_bits > 0U) {
    if (offset == 0U) {
      if (not writer.append(0U)) {
        return false;
      }
    }

    // Apply mask if required.
    if (n_bits < 64U) {
      // Bitmap of n_bits ones. (UB if n_bits == 64).
      uint64_t mask = (1UL << n_bits) - 1UL;
      val           = val & mask;
    }

    // In case n_bits < number of bits left in current byte (ie, last iteration).
    if (static_cast<uint32_t>(8U - offset) > n_bits) {
      auto shifted_val = static_cast<uint8_t>(val << (8U - offset - n_bits));
      writer.back() += shifted_val;
      offset += n_bits;
      n_bits = 0U;
      continue;
    }

    // In case, space in current byte is lower or equal to n_bits (ie, not the last iteration).
    auto shifted_val = static_cast<uint8_t>(val >> (n_bits - 8U + offset));
    writer.back() += shifted_val;
    n_bits -= 8U - offset;
    offset = 0U;
  }

  return true;
}

bool bit_encoder::pack_bytes(span<const uint8_t> bytes)
{
  if (bytes.empty()) {
    return true;
  }

  // Aligned case.
  if (offset == 0) {
    return writer.append(bytes);
  }

  return std::all_of(bytes.begin(), bytes.end(), [this](auto byte) { return pack(byte, 8U); });
}

bool bit_encoder::pack_bytes(byte_buffer_view bytes)
{
  if (bytes.empty()) {
    return true;
  }

  // Aligned case.
  if (offset == 0) {
    return writer.append(bytes);
  }

  return std::all_of(bytes.begin(), bytes.end(), [this](auto byte) { return pack(byte, 8U); });
}

bool bit_decoder::advance_bits(uint32_t n_bits)
{
  uint32_t extra_bits     = (offset + n_bits) % 8U;
  uint32_t bytes_required = divide_ceil(offset + n_bits, 8U);
  uint32_t bytes_offset   = (offset + n_bits) / 8U;

  if (bytes_required > buffer.end() - it) {
    return false;
  }

  it += bytes_offset;
  offset = extra_bits;

  return true;
}

template <typename T>
bool bit_decoder::unpack(T& val, uint32_t n_bits)
{
  srsran_assert(n_bits <= sizeof(T) * 8U, "unpack_bits() only supports up to {} bits", sizeof(T) * 8U);

  val = 0;
  while (n_bits > 0U) {
    if (it == buffer.end()) {
      return false;
    }

    if (static_cast<uint32_t>(8U - offset) > n_bits) {
      uint8_t mask = static_cast<uint8_t>(1u << (8u - offset)) - static_cast<uint8_t>(1u << (8u - offset - n_bits));
      val += (static_cast<uint8_t>((*it) & mask)) >> (8u - offset - n_bits);
      offset += n_bits;
      n_bits = 0;
      continue;
    }

    auto mask = static_cast<uint8_t>((1u << (8u - offset)) - 1u);
    val += (static_cast<T>((*it) & mask)) << (n_bits - 8 + offset);
    n_bits -= 8 - offset;
    offset = 0;
    ++it;
  }

  return true;
}

template bool bit_decoder::unpack<bool>(bool&, uint32_t n_bits);
template bool bit_decoder::unpack<int8_t>(int8_t&, uint32_t n_bits);
template bool bit_decoder::unpack<uint8_t>(uint8_t&, uint32_t n_bits);
template bool bit_decoder::unpack<int16_t>(int16_t&, uint32_t n_bits);
template bool bit_decoder::unpack<uint16_t>(uint16_t&, uint32_t n_bits);
template bool bit_decoder::unpack<int32_t>(int32_t&, uint32_t n_bits);
template bool bit_decoder::unpack<uint32_t>(uint32_t&, uint32_t n_bits);
template bool bit_decoder::unpack<int64_t>(int64_t&, uint32_t n_bits);
template bool bit_decoder::unpack<uint64_t>(uint64_t&, uint32_t n_bits);

bool bit_decoder::unpack_bytes(span<uint8_t> bytes)
{
  if (bytes.empty()) {
    return true;
  }

  if (static_cast<std::ptrdiff_t>(bytes.size()) > buffer.end() - it) {
    return false;
  }

  // Aligned case
  if (offset == 0) {
    std::copy(it, it + bytes.size(), bytes.begin());
    it += bytes.size();
    return true;
  }

  // Unaligned case.
  for (auto& byte : bytes) {
    if (not unpack(byte, 8)) {
      return false;
    }
  }

  return true;
}

byte_buffer_view bit_decoder::unpack_aligned_bytes(size_t n_bytes)
{
  byte_buffer_view result;
  align_bytes();
  if (n_bytes == 0) {
    return result;
  }

  if (static_cast<std::ptrdiff_t>(n_bytes) > buffer.end() - it) {
    return result;
  }
  result = byte_buffer_view{it, it + n_bytes};
  it += n_bytes;

  return result;
}

void bit_decoder::align_bytes()
{
  if (offset != 0) {
    srsran_sanity_check(it != buffer.end(), "Invalid bit_decoder state");
    ++it;
    offset = 0;
  }
}
