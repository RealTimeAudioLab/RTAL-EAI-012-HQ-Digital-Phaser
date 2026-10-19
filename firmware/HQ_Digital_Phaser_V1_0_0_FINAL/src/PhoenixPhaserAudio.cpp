#include "PhoenixPhaserAudio.h"
#include <math.h>

#pragma GCC optimize ("O3")

#ifndef I2S_COMM_FORMAT_STAND_I2S
  #define PHX_I2S_COMM_FORMAT ((i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB))
#else
  #define PHX_I2S_COMM_FORMAT I2S_COMM_FORMAT_STAND_I2S
#endif

PhoenixPhaserAudio::PhoenixPhaserAudio(uint8_t bclkPin, uint8_t lrckPin, uint8_t doutPin, uint8_t dinPin)
: _bclkPin(bclkPin), _lrckPin(lrckPin), _doutPin(doutPin), _dinPin(dinPin),
  _audioOk(false), _bypass(false), _stages(12), _sweepPercent(50),
  _targetCoeff(0.0f), _centerHz(560), _currentCoeff(0.0f),
  _resonancePercent(0), _drivePercent(0), _driveSmoothed(0.0f), _feedbackL(0.0f), _feedbackR(0.0f), _feedbackGain(0.0f),
  _underruns(0), _audioLoadAvgUs(0), _audioLoadPeakUs(0),
  _audioLoadAccumUs(0), _audioLoadBlocks(0), _audioLoadWindowStartMs(0),
  _audioTaskHandle(nullptr) {}

void PhoenixPhaserAudio::begin() {
  // Precompute sweep coefficient and sine tables outside the audio task.
  for (int i = 0; i <= LUT_SIZE; ++i) {
    const float u = (float)i / (float)LUT_SIZE;
    const float f = 70.0f * powf(2.0f, 6.0f * u);
    const float g = tanf((float)M_PI * f / (float)SAMPLE_RATE);
    _coeffLut[i] = (1.0f - g) / (1.0f + g);
    _sineLut[i] = sinf(2.0f * (float)M_PI * u);
  }
  resetStages();
  setLfoRatePercent(_lfoRatePercent);
  setSweepPercent(_sweepPercent);
  _currentCoeff = _targetCoeff;
  _audioLoadWindowStartMs = millis();
  _audioOk = beginI2S();
  if (_audioOk) {
    xTaskCreatePinnedToCore(audioTaskThunk, "PhaserAudio", 4096, this, 24, &_audioTaskHandle, 1);
  }
}

void PhoenixPhaserAudio::resetStages() {
  for (uint8_t i = 0; i < MAX_STAGES; ++i) {
    _left[i].x1 = _left[i].y1 = 0.0f;
    _right[i].x1 = _right[i].y1 = 0.0f;
    _left[i].a = _right[i].a = 0.0f;
  }
}

void PhoenixPhaserAudio::setStages(uint8_t stages) {
  if (stages != 6 && stages != 12) stages = 12;
  _stages = stages;
}

void PhoenixPhaserAudio::setSweepPercent(uint8_t percent) {
  if (percent > 100) percent = 100;
  _sweepPercent = percent;

  // P001 calibration: exactly six octaves, 70 Hz ... 4480 Hz.
  // This calculation runs only when the user changes SWEEP; never in audio render.
  const float s = (float)percent * 0.01f;
  const float freq = 70.0f * powf(2.0f, 6.0f * s);
  _centerHz = (uint32_t)(freq + 0.5f);

  const float g = tanf((float)M_PI * freq / (float)SAMPLE_RATE);
  float a = (1.0f - g) / (1.0f + g);
  if (a > 0.9995f) a = 0.9995f;
  if (a < -0.9995f) a = -0.9995f;
  _targetCoeff = a;
}

void PhoenixPhaserAudio::setLfoRatePercent(uint8_t percent) {
  if (percent > 100) percent = 100;
  _lfoRatePercent = percent;
  // 0.01 .. 2.5 Hz LOW, 1 .. 250 Hz HIGH; computed on UI thread.
  const float lo = _lfoHighRange ? 1.0f : 0.01f;
  const float hi = _lfoHighRange ? 250.0f : 2.5f;
  const float rate = lo * powf(hi / lo, (float)percent * 0.01f);
  _lfoMilliHz = (uint32_t)(rate * 1000.0f + 0.5f);
  _lfoPhaseInc = rate / (float)SAMPLE_RATE;
}

bool PhoenixPhaserAudio::beginI2S() {
  i2s_config_t cfg = {};
  cfg.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX);
  cfg.sample_rate = SAMPLE_RATE;
  cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
  cfg.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  cfg.communication_format = PHX_I2S_COMM_FORMAT;
  cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  cfg.dma_buf_count = 8;
  cfg.dma_buf_len = BLOCK_FRAMES;
  cfg.use_apll = true;
  cfg.tx_desc_auto_clear = true;
  cfg.fixed_mclk = 0;

  esp_err_t err = i2s_driver_install(I2S_NUM_0, &cfg, 0, nullptr);
  if (err != ESP_OK) {
    Serial.printf("I2S install error: %d\n", (int)err);
    return false;
  }

  i2s_pin_config_t pins = {};
  pins.bck_io_num = _bclkPin;
  pins.ws_io_num = _lrckPin;
  pins.data_out_num = _doutPin;
  pins.data_in_num = _dinPin;

  err = i2s_set_pin(I2S_NUM_0, &pins);
  if (err != ESP_OK) {
    Serial.printf("I2S pin error: %d\n", (int)err);
    i2s_driver_uninstall(I2S_NUM_0);
    return false;
  }

  i2s_zero_dma_buffer(I2S_NUM_0);
  return true;
}

void PhoenixPhaserAudio::audioTaskThunk(void *arg) {
  static_cast<PhoenixPhaserAudio*>(arg)->audioTask();
}

void PhoenixPhaserAudio::audioTask() {
  static constexpr int WORDS = BLOCK_FRAMES * 2;
  int32_t rx[WORDS];
  int32_t tx[WORDS];

  for (;;) {
    size_t bytesRead = 0;
    const esp_err_t r = i2s_read(I2S_NUM_0, rx, sizeof(rx), &bytesRead, portMAX_DELAY);
    if (r != ESP_OK || bytesRead == 0) {
      ++_underruns;
      continue;
    }

    const uint32_t t0 = micros();
    const int wordsRead = (int)(bytesRead / sizeof(int32_t));
    const uint8_t stages = _stages;
    const bool bypass = _bypass;
    const uint8_t outputMode = _outputMode;
    const float stereoOffset = outputMode == 2 ? (float)_stereoPhaseIndex * 0.125f : 0.0f;
    const float matrixTarget=_matrixEnabled?1.0f:0.0f;
    const float matrixStep=(matrixTarget-_matrixBlend)/(float)BLOCK_FRAMES;
    float matrixBlend=_matrixBlend;
    const uint8_t ms1=_mod1Source,mt1=_mod1Target,ms2=_mod2Source,mt2=_mod2Target;
    const float md1=(float)_mod1Depth*0.01f,md2=(float)_mod2Depth*0.01f;
    // 0..100: approximately 0.2..100 ms one-pole smoothing.
    const float smoothSamples=6.4f+32.0f*(float)_modSmooth;
    const float smoothAlpha=1.0f/(1.0f+smoothSamples);
    const float envTarget = _envEnabled ? 1.0f : 0.0f;
    const float envBlendStep = (envTarget - _envBlend) / (float)BLOCK_FRAMES;
    float envBlend = _envBlend;
    // Envelope time mappings: ATTACK 1..200 ms, RELEASE 10..2000 ms.
    const float attackMs = 1.0f + 1.99f * (float)_envAttack;
    const float releaseMs = 10.0f + 19.90f * (float)_envRelease;
    const float attackCoeff = 1.0f / (1.0f + attackMs * 0.001f * SAMPLE_RATE);
    const float releaseCoeff = 1.0f / (1.0f + releaseMs * 0.001f * SAMPLE_RATE);
    const float envGain = 1.0f + 15.0f * (float)_envSensitivity * 0.01f;
    const float envDepth = (float)_envDepth * 0.01f;
    const float analogTarget = _analogMode ? 1.0f : 0.0f;
    const float analogStep = (analogTarget - _analogBlend) / (float)BLOCK_FRAMES;
    float analogBlend = _analogBlend;
    const float variation = (float)_stageVariation * 0.0001f; // +/- 1% frequency offset max
    const float resCharacter = (float)_resonanceCharacter * 0.01f;
    const float color = (float)_analogColor * 0.01f;
    const float targetFeedback = (float)_resonancePercent * 0.0090f;
    const float feedbackStep = (targetFeedback - _feedbackGain) / (float)BLOCK_FRAMES;
    float feedbackGain = _feedbackGain;
    const float driveTarget = (float)_drivePercent * 0.01f;
    const float driveStep = (driveTarget - _driveSmoothed) / (float)BLOCK_FRAMES;
    float drive = _driveSmoothed;

    // Smooth manual coefficient changes over one complete audio block.
    const float sweepTarget = (float)_sweepPercent * 0.01f;
    const float sweepStep = (sweepTarget - _sweepSmooth) / (float)BLOCK_FRAMES;
    float sweep = _sweepSmooth;
    const float amountTarget = (float)_lfoAmountPercent * 0.01f;
    const float amountStep = (amountTarget - _lfoAmountSmooth) / (float)BLOCK_FRAMES;
    float amount = _lfoAmountSmooth;
    const float phaseInc = _syncEnabled ? _syncPhaseInc : _lfoPhaseInc;
    if (_phaseResetRequested) { _lfoPhase = 0.0f; _phaseResetRequested = false; }
    float phase = _lfoPhase;
    float coeff = _currentCoeff;

    for (int i = 0; i + 1 < wordsRead; i += 2) {
      if (bypass) {
        if (outputMode != 2) { tx[i] = rx[i]; tx[i + 1] = rx[i]; }
        else { tx[i] = rx[i]; tx[i + 1] = rx[i + 1]; }
        // Bit-exact I2S pass-through is the reference test mode.
        sweep += sweepStep;
        amount += amountStep;
        phase += phaseInc;
        if (phase >= 1.0f) phase -= 1.0f;
        feedbackGain += feedbackStep;
        drive += driveStep;
        analogBlend += analogStep;
        envBlend += envBlendStep;
        matrixBlend += matrixStep;
        // Keep DSP state tracking live input even while bypassed: clear feedback.
        _feedbackL = _feedbackR = 0.0f;
        continue;
      }

      sweep += sweepStep;
      amount += amountStep;
      phase += phaseInc;
      if (phase >= 1.0f) phase -= 1.0f;
      envBlend += envBlendStep;
      matrixBlend += matrixStep;
      // Envelope followers run independently per input channel. In MONO/AUX,
      // both sides use the first input; TRUE STEREO uses independent inputs.
      const float envIn0 = fabsf((float)rx[i] * (1.0f / 2147483648.0f));
      const float envIn1 = outputMode == 2 ? fabsf((float)rx[i + 1] * (1.0f / 2147483648.0f)) : envIn0;
      _envL += (envIn0 > _envL ? attackCoeff : releaseCoeff) * (envIn0 - _envL);
      _envR += (envIn1 > _envR ? attackCoeff : releaseCoeff) * (envIn1 - _envR);
      const float envModL = envBlend * envDepth * fminf(1.0f, _envL * envGain);
      const float envModR = envBlend * envDepth * fminf(1.0f, _envR * envGain);
      // Per-sample LFO and coefficient modulation; 250 Hz has no control-rate aliasing.
      const float wave = lookup(_sineLut, phase * (float)LUT_SIZE);
      const float rightWaveForMatrix = (stereoOffset>0.0f) ? lookup(_sineLut, (phase+stereoOffset>=1.0f?phase+stereoOffset-1.0f:phase+stereoOffset)*(float)LUT_SIZE) : wave;
      const float envSourceL=fminf(1.0f,_envL*envGain);
      const float envSourceR=fminf(1.0f,_envR*envGain);
      const float raw[2][2]={{ms1==0?wave:envSourceL,ms1==0?rightWaveForMatrix:envSourceR},
                             {ms2==0?wave:envSourceL,ms2==0?rightWaveForMatrix:envSourceR}};
      for(int m=0;m<2;++m)for(int ch=0;ch<2;++ch) _modSmoothed[m][ch]+=smoothAlpha*(raw[m][ch]-_modSmoothed[m][ch]);
      float mxSweepL=0,mxSweepR=0,mxResL=0,mxResR=0,mxDriveL=0,mxDriveR=0;
      const float gains[2]={md1*matrixBlend,md2*matrixBlend};
      const uint8_t targets[2]={mt1,mt2};
      for(int m=0;m<2;++m){
        const float vl=_modSmoothed[m][0]*gains[m],vr=_modSmoothed[m][1]*gains[m];
        if(targets[m]==0){mxSweepL+=vl;mxSweepR+=vr;}
        else if(targets[m]==1){mxResL+=vl;mxResR+=vr;}
        else {mxDriveL+=vl;mxDriveR+=vr;}
      }
      float position = sweep + mxSweepL + 0.5f * amount * wave + envModL;
      if (position < 0.0f) position = 0.0f;
      if (position > 1.0f) position = 1.0f;
      coeff = lookup(_coeffLut, position * (float)LUT_SIZE);
      float rightCoeff = coeff;
      if (stereoOffset > 0.0f) {
        float rightPhase = phase + stereoOffset;
        if (rightPhase >= 1.0f) rightPhase -= 1.0f;
        const float rightWave = lookup(_sineLut, rightPhase * (float)LUT_SIZE);
        float rightPosition = sweep + mxSweepR + 0.5f * amount * rightWave + envModR;
        if (rightPosition < 0.0f) rightPosition = 0.0f;
        if (rightPosition > 1.0f) rightPosition = 1.0f;
        rightCoeff = lookup(_coeffLut, rightPosition * (float)LUT_SIZE);
      }
      if (stereoOffset <= 0.0f && outputMode == 2 && envBlend != 0.0f) {
        float rp = sweep + mxSweepR + 0.5f * amount * wave + envModR;
        if (rp < 0.0f) rp = 0.0f;
        if (rp > 1.0f) rp = 1.0f;
        rightCoeff = lookup(_coeffLut, rp * (float)LUT_SIZE);
      }
      feedbackGain += feedbackStep;
      drive += driveStep;
      analogBlend += analogStep;

      // Phoenix C036 uses RIGHT_LEFT interleaving. We deliberately treat each
      // channel identically in P001; channel naming is irrelevant to the test.
      const float dry0 = (float)rx[i] * (1.0f / 2147483648.0f);
      const float dry1 = (float)rx[i + 1] * (1.0f / 2147483648.0f);
      // Closed-loop allpass chain, with bounded nonlinear feedback injection.
      // Analog character: modest, bounded and smoothly crossfaded with P009 path.
      const float characterGain = 1.0f + analogBlend * resCharacter * 0.20f;
      const float fbGainL=fmaxf(0.0f,fminf(0.95f,feedbackGain+0.45f*mxResL));
      const float fbGainR=fmaxf(0.0f,fminf(0.95f,feedbackGain+0.45f*mxResR));
      const float fbL = _feedbackL * fbGainL * characterGain;
      const float fbR = _feedbackR * fbGainR * characterGain;
      const float feedbackL = feedbackClip(fbL) + analogBlend * resCharacter * 0.12f * (fbL / (1.0f + fabsf(fbL)) - feedbackClip(fbL));
      const float feedbackR = feedbackClip(fbR) + analogBlend * resCharacter * 0.12f * (fbR / (1.0f + fabsf(fbR)) - feedbackClip(fbR));
      const float driveL=fmaxf(0.0f,fminf(1.0f,drive+0.5f*mxDriveL));
      const float driveR=fmaxf(0.0f,fminf(1.0f,drive+0.5f*mxDriveR));
      float wet0 = driveSample(dry0, driveL) + feedbackL;
      float wet1 = driveSample(outputMode == 2 ? dry1 : dry0, driveR) + feedbackR;

      static const float offsets[12] = {-0.84f,0.43f,-0.22f,0.95f,-0.61f,0.18f,0.72f,-0.38f,0.56f,-0.93f,0.27f,-0.11f};
      for (uint8_t s = 0; s < stages; ++s) {
        // Small coefficient offsets approximate fixed analog stage tolerances.
        const float delta = analogBlend * variation * offsets[s] * (1.0f - coeff * coeff);
        const float deltaR = analogBlend * variation * offsets[s] * (1.0f - rightCoeff * rightCoeff);
        wet0 = processAllpass(_left[s], wet0, coeff + delta);
        wet1 = processAllpass(_right[s], wet1, rightCoeff + deltaR);
      }
      if (analogBlend > 0.0f && color > 0.0f) {
        // Subtle soft saturation after the allpass chain; at OFF, exact P009 path.
        const float k = analogBlend * color * 0.22f;
        wet0 = wet0 * (1.0f - k) + k * (wet0 / (1.0f + 0.35f * fabsf(wet0)));
        wet1 = wet1 * (1.0f - k) + k * (wet1 / (1.0f + 0.35f * fabsf(wet1)));
      }

      _feedbackL = wet0;
      _feedbackR = wet1;

      // Fixed equal-voltage dry/wet mix. The phase-dependent cancellations are
      // the actual phaser notches. No limiter/drive is used in P001.
      // Softly bounded wet output protects against feedback-related overs.
      const float out0 = 0.5f * (dry0 + wet0);
      const float out1 = outputMode == 0 ? out0 :
                         (outputMode == 1 ? 0.5f * (dry0 - wet0) : 0.5f * (dry1 + wet1));
      tx[i] = clampI2S(out0);
      tx[i + 1] = clampI2S(out1);
    }

    _analogBlend = analogTarget;
    _envBlend = envTarget;
    _matrixBlend=matrixTarget;
    _currentCoeff = coeff;
    _sweepSmooth = sweepTarget;
    _lfoAmountSmooth = amountTarget;
    _lfoPhase = phase;
    _feedbackGain = targetFeedback;
    _driveSmoothed = driveTarget;

    const uint32_t elapsed = micros() - t0;
    _audioLoadAccumUs += elapsed;
    ++_audioLoadBlocks;
    if (elapsed > _audioLoadPeakUs) _audioLoadPeakUs = elapsed;

    const uint32_t now = millis();
    if (now - _audioLoadWindowStartMs >= 1000UL) {
      _audioLoadAvgUs = _audioLoadBlocks ? (uint32_t)(_audioLoadAccumUs / _audioLoadBlocks) : 0;
      _audioLoadAccumUs = 0;
      _audioLoadBlocks = 0;
      _audioLoadWindowStartMs = now;
    }

    size_t bytesWritten = 0;
    const esp_err_t w = i2s_write(I2S_NUM_0, tx, bytesRead, &bytesWritten, portMAX_DELAY);
    if (w != ESP_OK || bytesWritten != bytesRead) ++_underruns;
  }
}
