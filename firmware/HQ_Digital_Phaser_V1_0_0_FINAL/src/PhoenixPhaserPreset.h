#pragma once
#include <stdint.h>
struct PresetData {
  uint32_t magic;
  uint16_t version;
  uint8_t stages, sweep, resonance, drive, bypass;
  uint8_t lfoHigh, lfoRate, lfoAmount, sync, division, output;
  uint16_t bpm;
  uint32_t checksum;
} __attribute__((packed));

// V2 extends V1 with the stereo phase selection; V1 layout is preserved.
struct PresetDataV2 {
  uint32_t magic;
  uint16_t version;
  uint8_t stages, sweep, resonance, drive, bypass;
  uint8_t lfoHigh, lfoRate, lfoAmount, sync, division, output;
  uint16_t bpm;
  uint8_t stereoPhase;
  uint32_t checksum;
} __attribute__((packed));

// V3 adds analog-character controls while retaining V1/V2 readers.
struct PresetDataV3 {
  uint32_t magic; uint16_t version;
  uint8_t stages, sweep, resonance, drive, bypass;
  uint8_t lfoHigh, lfoRate, lfoAmount, sync, division, output;
  uint16_t bpm; uint8_t stereoPhase;
  uint8_t analogMode, stageVariation, resonanceCharacter, analogColor;
  uint32_t checksum;
} __attribute__((packed));

// V4: P011 envelope follower controls. Older packed formats remain unchanged.
struct PresetDataV4 {
  uint32_t magic; uint16_t version;
  uint8_t stages, sweep, resonance, drive, bypass;
  uint8_t lfoHigh, lfoRate, lfoAmount, sync, division, output;
  uint16_t bpm; uint8_t stereoPhase;
  uint8_t analogMode, stageVariation, resonanceCharacter, analogColor;
  uint8_t envFollow, envSensitivity, envAttack, envRelease;
  int8_t envDepth;
  uint32_t checksum;
} __attribute__((packed));

// V5: P012 two-slot modulation matrix; V1..V4 layouts unchanged.
struct PresetDataV5 {
  uint32_t magic; uint16_t version;
  uint8_t stages,sweep,resonance,drive,bypass;
  uint8_t lfoHigh,lfoRate,lfoAmount,sync,division,output;
  uint16_t bpm; uint8_t stereoPhase;
  uint8_t analogMode,stageVariation,resonanceCharacter,analogColor;
  uint8_t envFollow,envSensitivity,envAttack,envRelease; int8_t envDepth;
  uint8_t matrixEnabled,mod1Source,mod1Target,mod2Source,mod2Target,modSmooth;
  int8_t mod1Depth,mod2Depth;
  uint32_t checksum;
} __attribute__((packed));
