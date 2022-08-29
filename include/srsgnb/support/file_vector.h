/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsgnb/support/error_handling.h"
#include <fstream>
#include <vector>

namespace srsgnb {

template <typename T>
class file_vector
{
private:
  std::string file_name;

public:
  // Constructs object. It does not perform any file operation.
  file_vector(std::string filename) : file_name(std::move(filename)) {}

  /// Get file name.
  const std::string& get_file_name() const { return file_name; }

  // Open the file and appends in a \c std::vector<T> all the file entries.
  std::vector<T> read() const
  {
    std::ifstream binary_file(file_name, std::ios::in | std::ios::binary);

    report_fatal_error_if_not(binary_file.is_open(), "Error opening file '{}'. {}.", file_name, strerror(errno));
    std::vector<T> read_data;

    T read_value;
    while (binary_file.read(reinterpret_cast<char*>(&read_value), sizeof(T))) {
      read_data.push_back(read_value);
    }

    return read_data;
  }
};

} // namespace srsgnb
