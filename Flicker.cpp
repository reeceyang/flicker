#include "Flicker.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include <iostream>
#include <algorithm>
#include <chrono>
using namespace std::chrono;

Flicker::Flicker(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  long long ms = duration_cast< milliseconds >(
      system_clock::now().time_since_epoch()
  ).count();
  srand((unsigned int) ms);

  flickerCountdown = 0;
  GetParam(kFlicker)->InitDouble("Flicker", 0., 0., 100.0, 0.01, "%");
  GetParam(kFlickerOn)->InitBool("Light On", true);
  GetParam(kFlickerLength)->InitMilliseconds("Length", 1., 1., 1000.);

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new IBitmapControl(0, 0, pGraphics->LoadBitmap(OFF_FN, 1)));
    pGraphics->AttachControl(new IBitmapControl(0, 0, pGraphics->LoadBitmap(ON_FN, 1)), kInFlicker);
    pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50).GetHShifted(250), "Flicker", IText(50, COLOR_WHITE)));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-200).GetHShifted(250), kFlickerLength));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100).GetHShifted(250), kFlicker));
    pGraphics->AttachControl(new IVSwitchControl(b.GetCentredInside(100).GetVShifted(-250).GetHShifted(250), kFlickerOn));
  };
#endif
}

#if IPLUG_EDITOR
void Flicker::HideOnLightbulb(bool hide)
{
  if (GetUI()->GetControlWithTag(kInFlicker)->As<IControl>()) {
    GetUI()->GetControlWithTag(kInFlicker)->As<IControl>()->Hide(hide);
  }
}
#endif

#if IPLUG_DSP
void Flicker::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const int randNum = rand() % 10000;
  const int sampleRate = GetSampleRate();
//  std::cout << std::to_string(randNum) << "\n";
//  std::cout << sampleRate << " " << nFrames << "\n";
//  std::cout << flickerTime << " " << flickerEndTime << "\n";
  
  const double flicker = GetParam(kFlicker)->Value() * 100;
  // scale milliseconds to samples
  const int flickerLength = std::round(sampleRate * GetParam(kFlickerLength)->Value() / 1000.);
  // if the light is on, a flicker turns the light off. if the light is off, the flicker turns the light on
  const bool lightOn = GetParam(kFlickerOn)->Value();
  const bool notInFlicker = flickerCountdown <= 0;
  const bool playSound = (notInFlicker && lightOn) || (!notInFlicker && !lightOn);
  
  if (notInFlicker && randNum < flicker) {
    flickerCountdown = flickerLength;
  }
  
  if (GetUI()) {
    HideOnLightbulb(!playSound);
  }
  
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = playSound ? inputs[c][s] : 0;
    }
  }
  
  // subtract number of samples from countdown
  flickerCountdown = std::max((long long) 0, flickerCountdown - nFrames);
}
#endif
