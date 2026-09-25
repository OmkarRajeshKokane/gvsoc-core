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

#include <cpu/iss_v2/include/iss.hpp>
#include <cpu/iss_v2/include/hwloop/hwloop.hpp>


Hwloop::Hwloop(Iss &iss) : iss(iss)
{
    iss.traces.new_trace("hwloop", &this->trace, vp::DEBUG);
}


void Hwloop::reset(bool active)
{
    if (active)
    {
        for (int i = 0; i < CONFIG_GVSOC_ISS_NB_HWLOOP; i++)
        {
            this->start_pc[i] = 0;
            this->end_pc[i] = 0;
            this->count[i] = 0;
        }
        this->active = 0;
    }
}


void Hwloop::set_start(int idx, iss_reg_t pc)
{
    this->trace.msg(vp::Trace::LEVEL_DEBUG,
        "Setting hwloop start (idx: %d, pc: 0x%lx)\n", idx, (unsigned long)pc);
    this->start_pc[idx] = pc;
}


void Hwloop::set_end(int idx, iss_reg_t pc)
{
    this->trace.msg(vp::Trace::LEVEL_DEBUG,
        "Setting hwloop end (idx: %d, pc: 0x%lx)\n", idx, (unsigned long)pc);
    this->end_pc[idx] = pc;
}


void Hwloop::set_count(int idx, iss_reg_t count)
{
    this->trace.msg(vp::Trace::LEVEL_DEBUG,
        "Setting hwloop count (idx: %d, count: %d)\n", idx, (int)count);
    this->count[idx] = count;
    if (count == 0)
    {
        this->active &= ~(1u << idx);
    }
    else
    {
        this->active |= (1u << idx);
    }
}
