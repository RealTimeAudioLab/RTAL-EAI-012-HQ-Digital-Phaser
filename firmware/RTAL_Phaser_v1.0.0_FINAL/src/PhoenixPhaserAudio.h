#pragma once
#include <Arduino.h>
#include <math.h>
#include "driver/i2s.h"

class PhoenixPhaserAudio {
public:
  PhoenixPhaserAudio(uint8_t bclkPin, uint8_t lrckPin, uint8_t doutPin, uint8_t dinPin);

  void begin();
  bool ok() const { return _audioOk; }

  void setSweepPercent(uint8_t percent);
  void setStages(uint8_t stages);
  void setResonancePercent(uint8_t percent) { if (percent > 100) percent = 100; _resonancePercent = percent; }
  uint8_t resonancePercent() const { return _resonancePercent; }
  void setDrivePercent(uint8_t percent) { if (percent > 100) percent = 100; _drivePercent = percent; }
  uint8_t drivePercent() const { return _drivePercent; }
  void setLfoRatePercent(uint8_t percent);
  void setLfoAmountPercent(uint8_t percent) { _lfoAmountPercent = percent > 100 ? 100 : percent; }
  void setLfoHighRange(bool high) { _lfoHighRange = high; }
  uint8_t lfoRatePercent() const { return _lfoRatePercent; }
  uint8_t lfoAmountPercent() const { return _lfoAmountPercent; }
  bool lfoHighRange() const { return _lfoHighRange; }
  uint32_t lfoMilliHz() const { return _lfoMilliHz; }
  void setSyncFrequency(float hz) { _syncPhaseInc = hz / (float)SAMPLE_RATE; }
  void setSyncEnabled(bool enabled) { _syncEnabled = enabled; }
  void requestPhaseReset() { _phaseResetRequested = true; }
  void setOutputMode(uint8_t mode) { _outputMode = mode > 2 ? 0 : mode; }
  uint8_t outputMode() const { return _outputMode; }
  void setStereoPhase(uint8_t index) { _stereoPhaseIndex = index > 4 ? 0 : index; }
  void setAnalogMode(bool v) { _analogMode = v; }
  void setStageVariation(uint8_t v) { _stageVariation = v > 100 ? 100 : v; }
  void setResonanceCharacter(uint8_t v) { _resonanceCharacter = v > 100 ? 100 : v; }
  void setAnalogColor(uint8_t v) { _analogColor = v > 100 ? 100 : v; }
  void setEnvelopeEnabled(bool v) { _envEnabled = v; }
  void setEnvelopeSensitivity(uint8_t v) { _envSensitivity = v > 100 ? 100 : v; }
  void setEnvelopeAttack(uint8_t v) { _envAttack = v > 100 ? 100 : v; }
  void setEnvelopeRelease(uint8_t v) { _envRelease = v > 100 ? 100 : v; }
  void setEnvelopeDepth(int8_t v) { _envDepth = v < -100 ? -100 : (v > 100 ? 100 : v); }
  void setMatrix(bool enabled,uint8_t s1,uint8_t t1,int8_t d1,uint8_t s2,uint8_t t2,int8_t d2,uint8_t smooth) {
    _matrixEnabled=enabled;_mod1Source=s1;_mod1Target=t1;_mod1Depth=d1;
    _mod2Source=s2;_mod2Target=t2;_mod2Depth=d2;_modSmooth=smooth;
  }
  void setBypass(bool bypass) { _bypass = bypass; }

  uint8_t stages() const { return _stages; }
  uint8_t sweepPercent() const { return _sweepPercent; }
  uint32_t centerFrequencyHz() const { return _centerHz; }
  uint32_t sampleRate() const { return SAMPLE_RATE; }
  uint16_t blockFrames() const { return BLOCK_FRAMES; }
  uint32_t underruns() const { return _underruns; }
  uint32_t audioLoadAvgUs() const { return _audioLoadAvgUs; }
  uint32_t audioLoadPeakUs() const { return _audioLoadPeakUs; }

private:
  static constexpr uint32_t SAMPLE_RATE = 32000;
  static constexpr uint16_t BLOCK_FRAMES = 128;
  static constexpr uint8_t MAX_STAGES = 12;

  struct AllpassStage {
    float x1;
    float y1;
    float a;
    AllpassStage() : x1(0.0f), y1(0.0f), a(0.0f) {}
  };

  uint8_t _bclkPin, _lrckPin, _doutPin, _dinPin;
  volatile bool _audioOk;
  volatile bool _bypass;
  volatile uint8_t _outputMode = 0;
  volatile uint8_t _stereoPhaseIndex = 0;
  volatile bool _analogMode = false;
  volatile uint8_t _stageVariation = 30, _resonanceCharacter = 30, _analogColor = 20;
  float _analogBlend = 0.0f;
  volatile bool _envEnabled = false;
  volatile uint8_t _envSensitivity = 50, _envAttack = 25, _envRelease = 50;
  volatile int8_t _envDepth = 50;
  float _envL = 0.0f, _envR = 0.0f, _envBlend = 0.0f;

  volatile bool _matrixEnabled=false;
  volatile uint8_t _mod1Source=0,_mod1Target=0,_mod2Source=1,_mod2Target=1,_modSmooth=30;
  volatile int8_t _mod1Depth=0,_mod2Depth=0;
  float _modSmoothed[2][2]={{0,0},{0,0}};
  float _matrixBlend=0.0f;
  volatile uint8_t _stages;
  volatile uint8_t _sweepPercent;
  volatile uint8_t _resonancePercent;
  volatile uint8_t _drivePercent;
  volatile float _targetCoeff;
  volatile uint32_t _centerHz;

  static constexpr int LUT_SIZE = 1024;
  float _coeffLut[LUT_SIZE + 1];
  float _sineLut[LUT_SIZE + 1];
  volatile uint8_t _lfoRatePercent = 30;
  volatile uint8_t _lfoAmountPercent = 0;
  volatile bool _lfoHighRange = false;
  volatile uint32_t _lfoMilliHz = 0;
  volatile float _lfoPhaseInc = 0.0f;
  volatile float _syncPhaseInc = 0.0f;
  volatile bool _syncEnabled = false;
  volatile bool _phaseResetRequested = false;
  float _lfoPhase = 0.0f;
  float _lfoAmountSmooth = 0.0f;
  float _sweepSmooth = 0.5f;
  static inline float lookup(const float *table, float index) {
    int i = (int)index;
    if (i < 0) i = 0;
    if (i >= LUT_SIZE) i = LUT_SIZE - 1;
    const float t = index - (float)i;
    return table[i] + t * (table[i + 1] - table[i]);
  }
  AllpassStage _left[MAX_STAGES];
  AllpassStage _right[MAX_STAGES];
  float _currentCoeff;
  float _feedbackL, _feedbackR, _feedbackGain;
  float _driveSmoothed;
  static inline float driveSample(float x, float drive) {
    // Bounded smooth saturator, unity at drive=0; gain compensation.
    const float gain = 1.0f + 7.0f * drive * drive;
    const float z = x * gain;
    const float sat = z / (1.0f + fabsf(z));
    const float compensation = 1.0f / (0.5f + 0.5f * gain);
    return x * (1.0f - drive) + (sat * compensation) * drive;
  }
  static inline float feedbackClip(float x) {
    // Bounded, smooth, odd saturation, no transcendental operations.
    const float a = fabsf(x);
    return x / (1.0f + a);
  }

  volatile uint32_t _underruns;
  volatile uint32_t _audioLoadAvgUs;
  volatile uint32_t _audioLoadPeakUs;
  uint64_t _audioLoadAccumUs;
  uint32_t _audioLoadBlocks;
  uint32_t _audioLoadWindowStartMs;
  TaskHandle_t _audioTaskHandle;

  bool beginI2S();
  void resetStages();
  void audioTask();
  static void audioTaskThunk(void *arg);

  static inline float processAllpass(AllpassStage &s, float x, float a) {
    const float y = a * x + s.x1 - a * s.y1;
    s.x1 = x;
    s.y1 = y;
    return y;
  }

  static inline int32_t clampI2S(float x) {
    if (x >= 0.999999999f) return 2147483647;
    if (x <= -1.0f) return (int32_t)0x80000000;
    return (int32_t)(x * 2147483647.0f);
  }
};
