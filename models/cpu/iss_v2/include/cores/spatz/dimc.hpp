#pragma once

#include <cstdint>
#include <vector>

#define KB_LEN 32
#define KB_EW 256*4/8
#define VRF_size 256
#define FB_LEN 1
#define FB_EW 256*4/8
#define OP_LEN 32/4 // reduced due to bandwidth 
  
class DIMC
{
public:
    DIMC(Iss &iss);

    void build();
    void reset(bool reset);
    
    void move_KB();
    void move_FB();
    void dimc_compute_row();
    void compute_PP();
    void final_compute();
    void stats();

    //   CSRs

    uint8_t Ci =0;
    bool vm= 0;  // can be used for masking in future
    int32_t partial_sums=0;
    int bias =0;
    int kmc_row_count=0;
    int kmc_block_count=0;
    int fmc_block_count=0;

    bool Kernel_load = 0; 
    int Move_delay = 0; 
    bool Feature_reuse = 0;
    bool Feature_load_flag=0;    

    int instrucn_call=0;
    int row_sel=0;
    bool burst_mode_compute=0;
    //bool FLC [FB_EW/VRF_size]; // Feature load Filled/ Complete
    //bool KLC [KB_LEN][KB_EW/VRF_size]; // Kernal load Filled/ Complete

    int  dps_count=0;

    uint8_t FB[FB_EW];   //vs2

    uint8_t KB[KB_LEN][KB_EW];  //vs1

    uint32_t OP_buffer[OP_LEN]; // vd

};
