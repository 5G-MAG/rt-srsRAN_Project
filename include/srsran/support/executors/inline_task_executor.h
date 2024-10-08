/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/support/executors/task_executor.h"

namespace srsran {

class inline_task_executor final : public task_executor
{
public:
  SRSRAN_NODISCARD bool execute(unique_task task) override
  {
    task();
    return true;
  }

  SRSRAN_NODISCARD bool defer(unique_task task) override
  {
    task();
    return true;
  }
};

} // namespace srsran