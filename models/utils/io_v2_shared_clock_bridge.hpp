// SPDX-FileCopyrightText: 2026 ETH Zurich, University of Bologna and EssilorLuxottica SAS
//
// SPDX-License-Identifier: Apache-2.0
//
// Authors: Germain Haugou (germain.haugou@gmail.com)

#pragma once

#include <vp/vp.hpp>
#include <vp/itf/io_v2.hpp>
#include <vp/debug_mem.hpp>

// io_v2 bridge between two clock domains driven by the same clock (one clock
// behind two gates or dividers, no synchronizer on the chip): the two domains
// have every edge in common, so nothing is resynchronized. Requests, responses
// and retries are relayed as they are, in the same cycle, exactly as a binding
// inside one domain; the only thing done is to bring the other domain's engine
// up to date before calling into it, so that what it enqueues lands on the
// right cycle. Not for a real clock domain crossing, which keeps its
// synchronizer latency whatever the phase of the two clocks (see
// IoV2ClockBridge and its cdc kinds).
class IoV2SharedClockBridge : public vp::Component, public vp::DebugMemIf
{
public:
    IoV2SharedClockBridge(vp::ComponentConf &config);
    void start() override;

    vp::DebugMemIf *debug_mem_if() override { return this; }
    int debug_mem_access(uint64_t addr, uint8_t *data, uint64_t size,
        bool is_write) override;
    void debug_mem_regions(std::vector<vp::DebugMemRegion> &regions,
        uint64_t local_base, uint64_t window_size, uint64_t entry_base,
        int depth) override;

private:
    static vp::IoReqStatus in_req_handler(vp::Block *__this, vp::IoReq *req);
    static void            in_resp_retry_handler(vp::Block *__this, vp::IoRetryChannel channel);
    static vp::IoRespAck   out_resp_handler(vp::Block *__this, vp::IoReq *req);
    static void            out_retry_handler(vp::Block *__this, vp::IoRetryChannel channel);

    vp::IoSlave  in{&IoV2SharedClockBridge::in_req_handler,
                    &IoV2SharedClockBridge::in_resp_retry_handler};
    vp::IoMaster out{&IoV2SharedClockBridge::out_retry_handler,
                     &IoV2SharedClockBridge::out_resp_handler};
    vp::Trace trace;

    vp::ClockEngine *master_engine = nullptr;
    vp::ClockEngine *slave_engine  = nullptr;
};
