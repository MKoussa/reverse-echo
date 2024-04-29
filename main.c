#include "userdelfx.h"
#include "buffer_ops.h"

#define BUFFER_LEN 48000 * 4
#define BUFFER_LEN_HALF 48000

static __sdram float s_delay_ram[BUFFER_LEN];

static uint8_t timeChange, depthChange; 
static float depth, depthVal, depthDiv, wetDry, wetDryDiv;
static uint32_t echoCount, echoMax, echoMaxVal;

void DELFX_INIT(uint32_t platform, uint32_t api)
{
  buf_clr_f32(s_delay_ram, BUFFER_LEN);

  depth = 0.0f;
  echoCount = 0;
  wetDry = 0.0f;
  echoMax = 0;
  //
  depthDiv = 0.0f;
  wetDryDiv = 0.0f;
  //
  depthVal = 0;
  echoMaxVal = 0;
  //
  timeChange = 0;
  depthChange = 0;
}

void DELFX_PROCESS(float *xn, uint32_t frames)
{
  depthDiv = 0.99f + depth;
  wetDryDiv = 1.0f + wetDry;
  
  for(uint32_t i = 0; i < frames; i++)
  {
    s_delay_ram[echoCount * 2]     = (xn[i * 2]     + (s_delay_ram[echoCount * 2]     * depth)) / (0.99 + depth);   
    s_delay_ram[echoCount * 2 + 1] = (xn[i * 2 + 1] + (s_delay_ram[echoCount * 2 + 1] * depth)) / (0.99 + depth);

    if(wetDry > 0)
    {
      xn[i * 2]     = (xn[i * 2]     + (s_delay_ram[(echoMax * 2)     - (echoCount * 2)]     * wetDry)) / (wetDryDiv);
      xn[i * 2 + 1] = (xn[i * 2 + 1] + (s_delay_ram[(echoMax * 2 + 1) - (echoCount * 2 + 1)] * wetDry)) / (wetDryDiv);
    }
    else
    {
      xn[i * 2]     = xn[i * 2];
      xn[i * 2 + 1] = xn[i * 2 + 1];
    }

    echoCount++;
    if(echoCount > echoMax) { echoCount = 0; }
  }

  if(timeChange == 1)
  {
    echoMax = echoMaxVal;
    timeChange = 0;
  }

  if(depthChange == 1)
  {
    depth = depthVal;
    depthChange = 0;
    echoCount = 0;
  }
}

void DELFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) 
  {
    case k_user_delfx_param_time:
      timeChange = 1;
      echoMaxVal = ((uint32_t)(valf * BUFFER_LEN_HALF)) + 48000;
      break;
    case k_user_delfx_param_depth:
      depthChange = 1;
      depthVal = valf;
      break;
    case k_user_delfx_param_shift_depth:
      wetDry = valf;
      break;
    default:
      break;
  }
}
