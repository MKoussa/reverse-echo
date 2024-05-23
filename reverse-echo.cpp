#include "userdelfx.h"
#include "buffer_ops.h"
#include <atomic>

#define BUFFER_LEN 48000 * 4
#define BUFFER_LEN_HALF 48000

static __sdram float s_delay_ram[BUFFER_LEN];

static bool depthChange, wetDryChange;

static float depth, depthMath, wetDryMath, depthVal, wetDry, wetDryVal;

static uint32_t echoCount, echoMax;
static std::atomic<uint32_t> echoMaxVal(0);

void DELFX_INIT(uint32_t platform, uint32_t api)
{
  buf_clr_f32(s_delay_ram, BUFFER_LEN);

  depth = 0.0f;
  echoCount = 0;
  wetDry = 0.0f;
  echoMax = 0;
  //
  depthMath = 0.99f;
  wetDryMath = 1.0f;
  //
  depthVal = 0.0f;
  echoMaxVal = 0;
  wetDryVal = 0.0f;
}

void DELFX_PROCESS(float *xn, uint32_t frames)
{  
  for(uint32_t i = 0; i < frames; i++)
  {
    if(echoCount > echoMax)
    { 
      echoCount = 0;        
      echoMax = echoMaxVal;
    }
    s_delay_ram[echoCount * 2]     = (xn[i * 2]     + (s_delay_ram[echoCount * 2]     * depth)) / depthMath;   
    s_delay_ram[echoCount * 2 + 1] = (xn[i * 2 + 1] + (s_delay_ram[echoCount * 2 + 1] * depth)) / depthMath;
    
    if(wetDry > 0)
    {     
      xn[i * 2]     = (xn[i * 2]     * wetDryMath) + (s_delay_ram[(echoMax * 2)     - (echoCount * 2)]     * wetDry);
      xn[i * 2 + 1] = (xn[i * 2 + 1] * wetDryMath) + (s_delay_ram[(echoMax * 2 + 1) - (echoCount * 2 + 1)] * wetDry);
    }
    else
    {
      xn[i * 2]     = xn[i * 2];
      xn[i * 2 + 1] = xn[i * 2 + 1];
    }

    echoCount++;
    if(depthChange)
    {
        depth = depthVal;
        depthMath = 0.99f + depth;
        depthChange = false;
    }

    if(wetDryChange)
    {
        wetDry = wetDryVal;
        wetDryMath = 1.0f - wetDry;
        wetDryChange = false;
    }
  }
}

void DELFX_PARAM(uint8_t index, int32_t value)
{
  const float valf = q31_to_f32(value);
  switch (index) 
  {
    case k_user_delfx_param_time:
      echoMaxVal = ((uint32_t)(valf * BUFFER_LEN_HALF)) + 48000;
      break;
    case k_user_delfx_param_depth:
      depthVal = valf;
      depthChange = true;
      break;
    case k_user_delfx_param_shift_depth:
      wetDryVal = valf;
      wetDryChange = true;
      break;
    default:
      break;
  }
}
