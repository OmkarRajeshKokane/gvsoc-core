#include <vector>
#include <cstdint>

void DIMC::stats()
{
    printf("======================= DIMC STATS ===========================\n");

    printf("Ci                : %u\n", Ci);
    printf("vm (mask)          : %d\n", vm);

    printf("partial_sums       : %d\n", partial_sums);
    printf("bias               : %d\n", bias);

    printf("klc_row_count      : %d\n", kmc_row_count);
    printf("klc_block_count    : %d\n", kmc_block_count);
    printf("flc_block_count    : %d\n", fmc_block_count);

    printf("dps_count          : %d\n", dps_count);
    printf("Burst_computing    : %d\n", burst_mode_compute);


    printf("\n--- FB (Feature Buffer) ---\n");
    printf("FB = ");

    for (int i = 0; i < FB_EW; i++) {
        printf(" %u",FB[i]);
    }
    printf("\n");
    printf("\n--- KB (Kernel Buffer) ---\n");
    for (int r = 0; r < KB_LEN; r++) {
        printf("KB[%d] : ", r);
        for (int c = 0; c < KB_EW; c++) {
            printf("%u ", KB[r][c]);
        }
        printf("\n");
    }

    printf("======================================================================\n");
}

void DIMC::compute_PP()
{
    uint64_t comp_result = 0;
    uint8_t k_val =0;
    uint8_t f_val=0;
    this->dps_count+=1;
    switch (Ci)
    {
        // 1-bit Mode: XNOR + Popcount (ROW_WIDTH bits)
        case 0:
        {
            // masked_* are bit arrays (0/1 per entry) of length ROW_WIDTH
            for (uint32_t i = 0; i < KB_EW; ++i)
            {
                k_val = this->KB[this->row_sel][i];
                f_val = this->FB[i];

                for (int i = 0; i < 8; i++) {
                    bool k = (k_val >> i) & 1;
                    bool f = (f_val >> i) & 1;
                comp_result += k & f;
                }
            }
            break;
        }

        // 2-bit Mode: vector multiplication (512 elements)
        case 1:
        {

            for (uint32_t i = 0; i < KB_EW; ++i)
            {
                 k_val = this->KB[this->row_sel][i] ;
                 f_val = this->FB[i] & 0x3;
                comp_result += (k_val& 0x3) * (f_val & 0x3) + (((k_val& 0x12) * (f_val & 0x12))>>2 )+ (((k_val& 0x48) * (f_val & 0x48)) >> 4) + (((k_val& 0xC0) * (f_val & 0xC0)) >> 6);
            }
            break;
        }

        // 4-bit Mode: vector multiplication (256 elements)
        case 2:
        {

            for (uint32_t i = 0; i < KB_EW; ++i)
            {
                uint32_t k_val = this->KB[this->row_sel][i] & 0xF;
                uint32_t f_val = this->FB[i] & 0xF;
                comp_result = comp_result + ((k_val& 0x0F) * (f_val & 0x0F))  + (((k_val& 0xF0) * (f_val & 0xF0))>>4) ;
                //printf("the comp_rest is = %d, the input 1 is = %d , the input 2 is = %d, with kval = %d and fval = %d \n",comp_result,(k_val& 0x0F * f_val & 0x0F), (k_val& 0xF0 * f_val & 0xF0),k_val,f_val );
            }
            break;
        }

        // Default: 8-bit Mode: vector multiplication (ROW_WIDTH/8 elements)
        default:
        {

            for (uint32_t i = 0; i < KB_EW; ++i)
            {
                uint32_t k_val = this->KB[this->row_sel][i];       // 0..255
                uint32_t f_val = this->FB[i];
                comp_result += k_val * f_val;
                //printf("the comp_rest is = %d, with kval = %d and fval = %d \n",comp_result,k_val,f_val );

            }
            break;
        }

    }


    this->partial_sums=comp_result;
    this->OP_buffer[this->row_sel%8]=this->partial_sums;
    //stats();
}


DIMC::DIMC(Iss &iss)
{
}

void DIMC::build()
{
}

void DIMC::reset(bool active)
{
    if (active)  // frees the dcim memory
    {

            for (int j = 0; j < KB_EW; j++){
                this->FB[j] = 0;
                for (int i =0; i< KB_LEN;i++)
                    this->KB[i][j] = 0;
            }
            this->kmc_row_count=0;
            this->fmc_block_count=0;

    }
}



void DIMC::move_KB(){
	this->kmc_block_count=+1;
    if (this->kmc_block_count>=4)
    {
        this->kmc_block_count=0;
        this->kmc_row_count += 1;
    }
    //stats();
}

void DIMC::move_FB(){
	this->fmc_block_count+=1;
    //stats();
}

void DIMC::final_compute(){

		int64_t psum = this->partial_sums + bias;

		  //printf("\n The partial sum with bias included is :- %d \n",psum);
		  if (psum < 0)
		      psum= 0;           // clamp negative to 0
		  if (psum > 15)
		      psum= 15;

		//velem_set_value(&iss, vd_reg, 0, sewb, this->final_value);
}
