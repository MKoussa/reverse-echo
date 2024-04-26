#include "userdelfx.h"
#include "buffer_ops.h"

#define BUFFER_LEN 48000 * 4
#define BUFFER_LEN_HALF 48000

static __sdram float s_delay_ram[BUFFER_LEN];

static float depth, wetDry;
static uint32_t echoRate, echoMax;

void DELFX_INIT(uint32_t platform, uint32_t api)
{
  buf_clr_f32(s_delay_ram, BUFFER_LEN);
  depth = 0.0f;
  echoRate = 0;
  wetDry = 0.0f;
  echoMax = 0;
}

void DELFX_PROCESS(float *xn, uint32_t frames)
{
  for(uint32_t i = 0; i < frames; i++)
  {
    s_delay_ram[echoRate * 2]     = (xn[i * 2]     + (s_delay_ram[echoRate * 2]     * depth)) / (0.99 + depth);   
    s_delay_ram[echoRate * 2 + 1] = (xn[i * 2 + 1] + (s_delay_ram[echoRate * 2 + 1] * depth)) / (0.99 + depth);

    if(wetDry > 0)
    {
      xn[i * 2]     = (xn[i * 2]     + (s_delay_ram[(echoMax * 2)     - (echoRate * 2)]     * wetDry)) / (1 + wetDry);
      xn[i * 2 + 1] = (xn[i * 2 + 1] + (s_delay_ram[(echoMax * 2 + 1) - (echoRate * 2 + 1)] * wetDry)) / (1 + wetDry);
    }
    else
    {
      xn[i * 2]     = xn[i * 2];
      xn[i * 2 + 1] = xn[i * 2 + 1];
    }

    echoRate++;
    if(echoRate > echoMax) { echoRate = 0; }
  }
}

void DELFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) 
  {
    case k_user_delfx_param_time:
      echoMax = (valf * BUFFER_LEN_HALF) + 48000;
      break;
    case k_user_delfx_param_depth:
      echoRate = 0;
      depth = valf;
      break;
    case k_user_delfx_param_shift_depth:
      wetDry = valf;
      break;
    default:
      break;
  }
}
