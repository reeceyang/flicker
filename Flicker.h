#pragma once

#include "IPlug_include_in_plug_hdr.h"

const int kNumPresets = 1;

enum EParams
{
  kFlicker = 0,
  kFlickerLength,
  kInFlicker,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class Flicker final : public Plugin
{
public:
  Flicker(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void HideOnLightbulb(bool hide);
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  
  long long flickerEndTime;
#endif
};
