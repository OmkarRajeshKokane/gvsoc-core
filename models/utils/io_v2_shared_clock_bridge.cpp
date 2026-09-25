// SPDX-FileCopyrightText: 2026 ETH Zurich, University of Bologna and EssilorLuxottica SAS
//
// SPDX-License-Identifier: Apache-2.0
//
// Authors: Germain Haugou (germain.haugou@gmail.com)

#include "io_v2_shared_clock_bridge.hpp"


IoV2SharedClockBridge::IoV2SharedClockBridge(vp::ComponentConf &config)
    : vp::Component(config)
{
    this->traces.new_trace("trace", &this->trace, vp::DEBUG);
    this->new_slave_port("input", &this->in);
    this->new_master_port("output", &this->out);
}


void IoV2SharedClockBridge::start()
{
    // The remote PORT owners, not the remote contexts: with a muxed peer port
    // the remote context is the dispatch stub, not the component.
    auto *master_port = this->in.get_remote_port();
    auto *slave_port  = this->out.get_remote_port();
    if (master_port == nullptr || slave_port == nullptr)
    {
        this->trace.fatal("bridge not fully bound (in.bound=%d, out.bound=%d)\n",
                          master_port != nullptr, slave_port != nullptr);
        return;
    }
    this->master_engine = master_port->get_owner()->clock.get_engine();
    this->slave_engine  = slave_port->get_owner()->clock.get_engine();
    this->trace.msg(vp::Trace::LEVEL_INFO, "bridge mode=shared_clock\n");
}


// Everything is relayed 1:1 in the same cycle, the ownership of the requests
// and responses transferring exactly as with a direct binding.

vp::IoReqStatus IoV2SharedClockBridge::in_req_handler(vp::Block *__this, vp::IoReq *req)
{
    IoV2SharedClockBridge *self = static_cast<IoV2SharedClockBridge *>(__this);
    self->slave_engine->sync();
    return self->out.req(req);
}


vp::IoRespAck IoV2SharedClockBridge::out_resp_handler(vp::Block *__this, vp::IoReq *req)
{
    IoV2SharedClockBridge *self = static_cast<IoV2SharedClockBridge *>(__this);
    self->master_engine->sync();
    return self->in.resp(req);
}


void IoV2SharedClockBridge::out_retry_handler(vp::Block *__this, vp::IoRetryChannel channel)
{
    IoV2SharedClockBridge *self = static_cast<IoV2SharedClockBridge *>(__this);
    self->master_engine->sync();
    self->in.retry(channel);
}


void IoV2SharedClockBridge::in_resp_retry_handler(vp::Block *__this, vp::IoRetryChannel channel)
{
    IoV2SharedClockBridge *self = static_cast<IoV2SharedClockBridge *>(__this);
    self->slave_engine->sync();
    if (self->out.is_resp_retry_bound())
    {
        self->out.resp_retry(channel);
    }
}


// ---- Backdoor debug path (DebugMemIf): pass-through into the output --------

static vp::DebugMemIf *output_debug_mem(vp::IoMaster &itf)
{
    std::vector<vp::SlavePort *> finals = itf.get_final_ports();
    if (finals.empty() || finals[0]->get_owner() == nullptr)
    {
        return nullptr;
    }
    return finals[0]->get_owner()->debug_mem_if();
}

int IoV2SharedClockBridge::debug_mem_access(uint64_t addr, uint8_t *data,
    uint64_t size, bool is_write)
{
    vp::DebugMemIf *child = output_debug_mem(this->out);
    if (child == nullptr)
    {
        return -1;
    }
    return child->debug_mem_access(addr, data, size, is_write);
}

void IoV2SharedClockBridge::debug_mem_regions(std::vector<vp::DebugMemRegion> &regions,
    uint64_t local_base, uint64_t window_size, uint64_t entry_base, int depth)
{
    if (depth >= vp::DebugMemIf::MAX_DEPTH)
    {
        return;
    }
    vp::DebugMemIf *child = output_debug_mem(this->out);
    if (child != nullptr)
    {
        child->debug_mem_regions(regions, local_base, window_size, entry_base,
            depth + 1);
    }
}


extern "C" vp::Component *gv_new(vp::ComponentConf &config)
{
    return new IoV2SharedClockBridge(config);
}
