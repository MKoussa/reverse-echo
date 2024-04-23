#include "userdelfx.h"
//#include "float_math.h"
#include "buffer_ops.h"

#define BUFFER_LEN 48000 * 2
static __sdram float s_delay_ram[BUFFER_LEN];

static uint8_t depth;
static uint8_t repeat;
static uint32_t echoRate;

void DELFX_INIT(uint32_t platform, uint32_t api)
{
  buf_clr_f32(s_delay_ram, BUFFER_LEN);
  repeat = 0;
  depth = 0;
  echoRate = 0;
}

void DELFX_PROCESS(float *xn, uint32_t frames)
{
  for(uint32_t i = 0; i < frames; i++)
  {
    s_delay_ram[echoRate * 2]     = (xn[i * 2] + s_delay_ram[echoRate * 2]) / 2;
    s_delay_ram[echoRate * 2 + 1] = (xn[i * 2 + 1] + s_delay_ram[echoRate * 2 + 1]) / 2;

    xn[i * 2]     = (s_delay_ram[BUFFER_LEN - (echoRate * 2)] + xn[i * 2]) / 2;
    xn[i * 2 + 1] = (s_delay_ram[BUFFER_LEN - (echoRate * 2 + 1)] + xn[i * 2 + 1]) / 2;

    echoRate++;
  }
  if(echoRate > BUFFER_LEN / 2)
  {
    echoRate = 0;
  }
}

void DELFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) {
  case k_user_delfx_param_time:
    echoRate = (valf * (BUFFER_LEN / 2)) - 1;
    break;
  case k_user_delfx_param_depth:
    depth = (uint8_t)(valf * 10);
    break;
  default:
    break;
  }
}
