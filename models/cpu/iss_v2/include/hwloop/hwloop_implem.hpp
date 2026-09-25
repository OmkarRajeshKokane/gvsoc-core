/*
 * Copyright (C) 2026 ETH Zurich, University of Bologna and EssilorLuxottica SAS
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

/*
 * Authors: Germain Haugou (germain.haugou@gmail.com)
 */

#pragma once

#include <cpu/iss_v2/include/hwloop/hwloop.hpp>


inline iss_reg_t Hwloop::check(iss_reg_t pc, iss_reg_t next_pc)
{
    // Fast path: no loop is active, return the natural next PC.
    if (this->active == 0) return next_pc;

    // Loop 0 has higher priority than loop 1: if both end at the same
    // PC and both want to iterate, loop 0 wins this cycle. Loop 1 will
    // be checked again next time PC reaches the shared end.
    iss_reg_t target = next_pc;
    for (int i = 0; i < CONFIG_GVSOC_ISS_NB_HWLOOP; i++)
    {
        if ((this->active & (1u << i)) && this->end_pc[i] == pc)
        {
            this->count[i]--;
            if (this->count[i] == 0)
            {
                this->active &= ~(1u << i);
            }
            else if (target == next_pc)
            {
                target = this->start_pc[i];
            }
        }
    }
    return target;
}
