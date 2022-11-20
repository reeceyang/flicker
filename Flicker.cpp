#include "Flicker.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include <iostream>
#include <chrono>
using namespace std::chrono;

Flicker::Flicker(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  flickerEndTime = duration_cast< milliseconds >(
                     system_clock::now().time_since_epoch()
                   ).count();
  GetParam(kFlicker)->InitDouble("Flicker", 0., 0., 100.0, 0.01, "%");
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
  long long ms = duration_cast< milliseconds >(
      system_clock::now().time_since_epoch()
  ).count();
  srand((unsigned int) ms);
  int randNum = rand() % 100;
//  std::cout << std::to_string(randNum) << "\n";
  
  const double flicker = GetParam(kFlicker)->Value();
  const int flickerLength = GetParam(kFlickerLength)->Value();
  bool playSound = ms > flickerEndTime;
  
  if (playSound && randNum < flicker) {
    flickerEndTime = ms + flickerLength;
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
}
#endif
