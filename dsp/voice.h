// Copyright 2016 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Main synthesis voice.

#ifndef PLAITS_DSP_VOICE_H_
#define PLAITS_DSP_VOICE_H_

#include "stmlib/stmlib.h"

#include "stmlib/dsp/filter.h"
#include "stmlib/dsp/limiter.h"
#include "stmlib/utils/buffer_allocator.h"

#include "ArpOn/dsp/engine/additive_engine.h"
#include "ArpOn/dsp/engine/bass_drum_engine.h"
#include "ArpOn/dsp/engine/chord_engine.h"
#include "ArpOn/dsp/engine/engine.h"
#include "ArpOn/dsp/engine/fm_engine.h"
#include "ArpOn/dsp/engine/grain_engine.h"
#include "ArpOn/dsp/engine/hi_hat_engine.h"
#include "ArpOn/dsp/engine/modal_engine.h"
#include "ArpOn/dsp/engine/noise_engine.h"
#include "ArpOn/dsp/engine/particle_engine.h"
#include "ArpOn/dsp/engine/snare_drum_engine.h"
#include "ArpOn/dsp/engine/speech_engine.h"
#include "ArpOn/dsp/engine/string_engine.h"
#include "ArpOn/dsp/engine/swarm_engine.h"
#include "ArpOn/dsp/engine/virtual_analog_engine.h"
#include "ArpOn/dsp/engine/waveshaping_engine.h"
#include "ArpOn/dsp/engine/wavetable_engine.h"

#include "ArpOn/dsp/envelope.h"

#include "ArpOn/dsp/fx/low_pass_gate.h"

namespace plaits {

const int kMaxEngines = 16;
const int kMaxTriggerDelay = 8;
const int kTriggerDelay = 5;

const int kNumArps = 11;
const int kNumArpNotes = 8;
// const int kNumArpVoices = 5;
const float arp_table[kNumArps][kNumArpNotes] = {
  { 0.00f, 0.00f, 12.00f, 12.00f, 24.00f, 24.00f, 36.00f, 36.00f },  // OCT
  { 0.00f, 7.00f,  7.00f, 12.00f, 12.00f, 19.00f, 19.00f, 24.00f },  // 5
  { 0.00f, 5.00f,  7.00f, 12.00f, 12.00f, 17.00f, 19.00f, 24.00f },  // sus4
  { 0.00f, 3.00f,  7.00f, 12.00f, 12.00f, 15.00f, 19.00f, 24.00f },  // m
  { 0.00f, 3.00f,  7.00f, 10.00f, 12.00f, 15.00f, 19.00f, 22.00f },  // m7
  { 0.00f, 3.00f, 10.00f, 14.00f, 17.00f, 24.00f, 27.00f, 31.00f },  // m9
  { 0.00f, 3.00f, 10.00f, 17.00f, 20.00f, 27.00f, 37.00f, 40.00f },  // m11
  { 0.00f, 2.00f,  9.00f, 16.00f, 18.00f, 24.00f, 31.00f, 32.00f },  // 69
  { 0.00f, 4.00f, 11.00f, 14.00f, 18.00f, 25.00f, 28.00f, 32.00f },  // M9
  { 0.00f, 4.00f,  7.00f, 11.00f, 15.00f, 18.00f, 22.00f, 26.00f },  // M7
  { 0.00f, 4.00f,  7.00f, 12.00f, 16.00f, 19.00f, 24.00f, 28.00f },  // M
};

// const int arp_table = 17;
//  const float arpTable[kArpNumChords][kArpNumNotes] = {
//   // Fixed Intervals
//   { 0.00f, 0.00f, 12.00f, 12.00f },  // Octave
//   { 0.00f, 7.00f,  7.00f, 12.00f },  // Fifth
//   // Minor
//   { 0.00f, 3.00f,  7.00f, 12.00f },  // Minor
//   { 0.00f, 3.00f,  7.00f, 10.00f },  // Minor 7th
//   { 0.00f, 3.00f, 10.00f, 14.00f },  // Minor 9th
//   { 0.00f, 3.00f, 10.00f, 17.00f },  // Minor 11th
//   // Major
//   { 0.00f, 4.00f,  7.00f, 12.00f },  // Major
//   { 0.00f, 4.00f,  7.00f, 11.00f },  // Major 7th
//   { 0.00f, 4.00f, 11.00f, 14.00f },  // Major 9th
//   // Colour Chords
//   { 0.00f, 5.00f,  7.00f, 12.00f },  // Sus4
//   { 0.00f, 2.00f,  9.00f, 16.00f },  // 69
//   { 0.00f, 4.00f,  7.00f,  9.00f },  // 6th
//   { 0.00f, 7.00f, 16.00f, 23.00f },  // 10th (Spread maj7)
//   { 0.00f, 4.00f,  7.00f, 10.00f },  // Dominant 7th
//   { 0.00f, 7.00f, 10.00f, 13.00f },  // Dominant 7th (b9)
//   { 0.00f, 3.00f,  6.00f, 10.00f },  // Half Diminished
//   { 0.00f, 3.00f,  6.00f,  9.00f },  // Fully Diminished
// };

const int kNumArpModes = 7;
enum ArpMode {
  ARP_MODE_UP = 1,
  ARP_MODE_DOWN = 2,
  ARP_MODE_INCLUSIVE = 3,
  ARP_MODE_EXCLUSIVE = 4,
  ARP_MODE_RANDOM = 5,
  ARP_MODE_WALK = 6,
  ARP_MODE_WALK_WITH_PAUSE = 7
};

class ChannelPostProcessor {
 public:
  ChannelPostProcessor() { }
  ~ChannelPostProcessor() { }
  
  void Init() {
    lpg_.Init();
    Reset();
  }
  
  void Reset() {
    limiter_.Init();
  }
  
  void Process(
      float gain,
      bool bypass_lpg,
      float low_pass_gate_gain,
      float low_pass_gate_frequency,
      float low_pass_gate_hf_bleed,
      float* in,
      short* out,
      size_t size,
      size_t stride) {
    if (gain < 0.0f) {
      limiter_.Process(-gain, in, size);
    }
    const float post_gain = (gain < 0.0f ? 1.0f : gain) * -32767.0f;
    if (!bypass_lpg) {
      lpg_.Process(
          post_gain * low_pass_gate_gain,
          low_pass_gate_frequency,
          low_pass_gate_hf_bleed,
          in,
          out,
          size,
          stride);
    } else {
      while (size--) {
        *out = stmlib::Clip16(1 + static_cast<int32_t>(*in++ * post_gain));
        out += stride;
      }
    }
  }
  
 private:
  stmlib::Limiter limiter_;
  LowPassGate lpg_;
  
  DISALLOW_COPY_AND_ASSIGN(ChannelPostProcessor);
};

struct Patch {
  float note;
  float harmonics;
  float timbre;
  float morph;
  float frequency_modulation_amount;
  float timbre_modulation_amount;
  float morph_modulation_amount;

  int engine;
  float decay;
  float lpg_colour;
  float arp_steps;
  float arp_index; 
  float arp_mode; 
};

struct Modulations {
  float engine;
  float note;
  float frequency;
  float harmonics;
  float timbre;
  float morph;
  float trigger;
  float level;

  bool frequency_patched;
  bool timbre_patched;
  bool morph_patched;
  bool trigger_patched;
  bool level_patched;
};

class Voice {
 public:
  Voice() { }
  ~Voice() { }
  
  struct Frame {
    short out;
    short aux;
  };

  void Init(stmlib::BufferAllocator* allocator, int* arp_step);
  void DoNextArpStep(int arp_mode, int arp_steps);
  void Render(
      const Patch& patch,
      const Modulations& modulations,
      Frame* frames,
      size_t size);
  
  inline int active_engine() const { return previous_engine_index_; }
    
 private:
  void ComputeDecayParameters(const Patch& settings);
  
  inline float ApplyModulations(
      float base_value,
      float modulation_amount,
      bool use_external_modulation,
      float external_modulation,
      bool use_internal_envelope,
      float envelope,
      float default_internal_modulation,
      float minimum_value,
      float maximum_value) {
    float value = base_value;
    modulation_amount *= std::max(fabsf(modulation_amount) - 0.05f, 0.05f);
    modulation_amount *= 1.05f;
    
    float modulation = use_external_modulation
        ? external_modulation
        : (use_internal_envelope ? envelope : default_internal_modulation);
    value += modulation_amount * modulation;
    CONSTRAIN(value, minimum_value, maximum_value);
    return value;
  }
  
  AdditiveEngine additive_engine_;
  BassDrumEngine bass_drum_engine_;
  ChordEngine chord_engine_;
  FMEngine fm_engine_;
  GrainEngine grain_engine_;
  HiHatEngine hi_hat_engine_;
  ModalEngine modal_engine_;
  NoiseEngine noise_engine_;
  ParticleEngine particle_engine_;
  SnareDrumEngine snare_drum_engine_;
  SpeechEngine speech_engine_;
  StringEngine string_engine_;
  SwarmEngine swarm_engine_;
  VirtualAnalogEngine virtual_analog_engine_;
  WaveshapingEngine waveshaping_engine_;
  WavetableEngine wavetable_engine_;

  stmlib::HysteresisQuantizer engine_quantizer_;
  
  int previous_engine_index_;
  float engine_cv_;
  
  stmlib::HysteresisQuantizer arp_index_quantizer_;
  stmlib::HysteresisQuantizer arp_mode_quantizer_;
  
  float arp_ratios[kNumArps][kNumArpNotes];

  float arp_inversion; 
  int* arp_step;
  bool arp_reverse; 

  float previous_note_;
  bool trigger_state_;
  bool level_state_;
  
  DecayEnvelope decay_envelope_;
  LPGEnvelope lpg_envelope_;
  
  float trigger_delay_line_[kMaxTriggerDelay];
  DelayLine<float, kMaxTriggerDelay> trigger_delay_;
  
  ChannelPostProcessor out_post_processor_;
  ChannelPostProcessor aux_post_processor_;
  
  EngineRegistry<kMaxEngines> engines_;
  
  float out_buffer_[kMaxBlockSize];
  float aux_buffer_[kMaxBlockSize];
  
  DISALLOW_COPY_AND_ASSIGN(Voice);
};

}  // namespace plaits

#endif  // PLAITS_DSP_VOICE_H_
