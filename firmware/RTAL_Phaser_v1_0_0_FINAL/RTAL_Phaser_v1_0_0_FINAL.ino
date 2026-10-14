/*
  RTAL Phoenix Phaser v1.0.0 FINAL
  Static 6/12-stage all-pass phaser test for the proven Project Phoenix hardware.

  Base hardware: 
  Arduino IDE 1.8.19 / ESP32 Arduino core 2.0.16
  ESP32-S3 / PCM1808 ADC / PCM5102A DAC / SSD1309 OLED
*/

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <U8g2lib.h>
#include "src/PhoenixPhaserAudio.h"
#include "src/PhoenixPhaserInput.h"
#include "src/PhoenixPhaserPreset.h"
#include "src/PhaserSplash.h"

#define OLED_SCK   12
#define OLED_MOSI  11
#define OLED_CS    10
#define OLED_DC     6
#define OLED_RST    7

static const uint8_t BTN_PINS[8] = {21,47,45,38,4,15,3,14};
#define ENC_A_PIN   1
#define ENC_B_PIN   2
#define ENC_SW_PIN 42

#define I2S_BCLK_PIN 18
#define I2S_LRCK_PIN 16
#define I2S_DOUT_PIN 17
#define I2S_DIN_PIN   5

U8G2_SSD1309_128X64_NONAME0_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC, OLED_RST);
PhoenixPhaserAudio audio(I2S_BCLK_PIN, I2S_LRCK_PIN, I2S_DOUT_PIN, I2S_DIN_PIN);
PhoenixPhaserInput input(BTN_PINS, ENC_A_PIN, ENC_B_PIN, ENC_SW_PIN);

static uint8_t sweepPct = 50;
static uint8_t resonancePct = 0;
static uint8_t editParam = 0; // 0=SWEEP, 1=RES, 2=DRIVE, 3=LFO RATE, 4=LFO AMOUNT
static uint8_t lfoRatePct = 30;
static uint8_t lfoAmountPct = 0;
static bool lfoHigh = false;
static uint8_t drivePct = 0;
static bool twelveStage = true;
static bool bypass = false;
static uint8_t outputMode = 0; // 0 MONO, 1 MAIN/AUX, 2 TRUE STEREO
static uint32_t lastDrawMs = 0;
static HardwareSerial midiSerial(2);
static bool syncEnabled = false;
static uint8_t division = 3; // quarter note
static uint16_t internalBpm = 120;
static float measuredBpm = 120.0f;
static uint32_t lastClockUs = 0, lastClockMs = 0;
static uint8_t clockTicks = 0;
static uint32_t clockWindowStartUs = 0;
static uint16_t displayBpm = 120;
static uint32_t lastDisplayBpmMs = 0;
static bool externalClock = false;
static const float cyclesPerBeat[] = {0.25f,0.5f,1.0f,2.0f,4.0f,0.6666667f,1.3333333f,1.5f,3.0f,6.0f,2.6666667f};
static const char* divisionName[] = {"1/1","1/2","1/4","1/8","1/16","1/4D","1/8D","1/4T","1/8T","1/16T","1/16D"};
static void updateSync() {
  const float bpm = externalClock ? measuredBpm : (float)internalBpm;
  audio.setSyncFrequency((bpm / 60.0f) * cyclesPerBeat[division]);
  audio.setSyncEnabled(syncEnabled);
}
static void serviceMidiClockByte(uint8_t b) {
    if (b == 0xF8) {
      const uint32_t nowUs = micros();
      if (lastClockUs) {
        const uint32_t dt = nowUs - lastClockUs;
        if (dt >= 1000 && dt <= 250000) {
          if (!externalClock || clockWindowStartUs == 0) {
            clockWindowStartUs = nowUs;
            clockTicks = 0;
          } else if (++clockTicks >= 24) {
            const uint32_t windowUs = nowUs - clockWindowStartUs;
            if (windowUs > 0) {
              const float candidate = 60000000.0f / (float)windowUs;
              if (candidate >= 30.0f && candidate <= 240.0f) {
                // Fast convergence on a genuine tempo change, quiet at steady tempo.
                const float delta = candidate - measuredBpm;
                const float alpha = (fabsf(delta) > 4.0f) ? 0.35f : 0.05f;
                measuredBpm += alpha * delta;
              }
            }
            clockWindowStartUs = nowUs;
            clockTicks = 0;
          }
          externalClock = true;
        } else {
          clockWindowStartUs = nowUs;
          clockTicks = 0;
        }
      }
      lastClockUs = nowUs;
      lastClockMs = millis();
    } else if (b == 0xFA) { audio.requestPhaseReset(); }
    // FB and FC leave modulation running, as in P006.
}
static void serviceMidiClockTimeout() {
  const uint32_t nowMs = millis();
  if (externalClock && nowMs - lastClockMs > 300) {
    externalClock = false;
    lastClockUs = 0;
    clockWindowStartUs = 0;
    clockTicks = 0;
  }
  if (nowMs - lastDisplayBpmMs >= 250) {
    lastDisplayBpmMs = nowMs;
    if (!externalClock) displayBpm = internalBpm;
    else {
      // Integer display with 0.7 BPM hysteresis around the shown value.
      if (measuredBpm > (float)displayBpm + 0.7f ||
          measuredBpm < (float)displayBpm - 0.7f)
        displayBpm = (uint16_t)(measuredBpm + 0.5f);
    }
  }
  updateSync();
}

// Encoder-only menu: HOME -> MENU -> PAGE -> EDIT.
enum UiPage { HOME, MENU, PAGE, EDIT, BROWSER, BROWSER_ACTION };
static UiPage uiPage = HOME;
static uint8_t menuIndex = 0, itemIndex = 0;
static uint32_t lastInteraction = 0, lastEncoderMs = 0;
// SD storage uses shared SPI bus; never touch SD in audio task.
#define SD_CS_PIN 9
static bool sdReady = false;
static uint8_t activePreset = 0;
static uint8_t browserPreset = 1;
static uint8_t browserAction = 0; // 0=LOAD, 1=SAVE
static const uint8_t PRESET_COUNT = 128;
static uint8_t midiChannel=1; // 0=OMNI, 1..16
static uint8_t midiBank=0;
static uint8_t midiRunning=0, midiFirst=0, midiCount=0;
static bool analogMode = false;
static uint8_t stageVariation = 30, resonanceCharacter = 30, analogColor = 20;
static bool envFollow = false;
static uint8_t envSensitivity = 50, envAttack = 25, envRelease = 50;
static int8_t envDepth = 50;
static bool matrixEnabled=false;
static uint8_t mod1Source=0,mod1Target=0,mod2Source=1,mod2Target=1,modSmooth=30;
static int8_t mod1Depth=0,mod2Depth=0;
static uint8_t stereoPhase = 0; // 0,45,90,135,180 degrees
static char notice[24] = "";
static uint32_t noticeUntil = 0;
static void showNotice(const char *msg) {
  snprintf(notice,sizeof(notice),"%s",msg);
  noticeUntil = millis()+1100;
}
static uint32_t presetChecksum(const PresetData &p) {
  const uint8_t *b = reinterpret_cast<const uint8_t*>(&p);
  uint32_t h=2166136261UL;
  for(size_t i=0;i<sizeof(PresetData)-sizeof(uint32_t);++i) { h ^= b[i]; h *= 16777619UL; }
  return h;
}
static uint32_t presetChecksumV2(const PresetDataV2 &p) {
  const uint8_t *b = reinterpret_cast<const uint8_t*>(&p);
  uint32_t h=2166136261UL;
  for(size_t i=0;i<sizeof(PresetDataV2)-sizeof(uint32_t);++i) {h ^= b[i];h *= 16777619UL;}
  return h;
}
static uint32_t presetChecksumV3(const PresetDataV3 &p) {
  const uint8_t *b = reinterpret_cast<const uint8_t*>(&p);
  uint32_t h=2166136261UL;
  for(size_t i=0;i<sizeof(PresetDataV3)-sizeof(uint32_t);++i) {h ^= b[i];h *= 16777619UL;}
  return h;
}
static uint32_t presetChecksumV4(const PresetDataV4 &p) {
  const uint8_t *b = reinterpret_cast<const uint8_t*>(&p);
  uint32_t h = 2166136261UL;
  for(size_t i=0; i<sizeof(PresetDataV4)-sizeof(uint32_t); ++i) {h ^= b[i]; h *= 16777619UL;}
  return h;
}
static uint32_t presetChecksumV5(const PresetDataV5 &p) {
  const uint8_t *b=(const uint8_t*)&p; uint32_t h=2166136261UL;
  for(size_t i=0;i<sizeof(p)-sizeof(uint32_t);++i){h^=b[i];h*=16777619UL;}
  return h;
}
static void applyMatrix(){audio.setMatrix(matrixEnabled,mod1Source,mod1Target,mod1Depth,mod2Source,mod2Target,mod2Depth,modSmooth);}
static void applyEnvelope() {
  audio.setEnvelopeEnabled(envFollow);
  audio.setEnvelopeSensitivity(envSensitivity);
  audio.setEnvelopeAttack(envAttack);
  audio.setEnvelopeRelease(envRelease);
  audio.setEnvelopeDepth(envDepth);
}
static void applyAnalog() {
  audio.setAnalogMode(analogMode);
  audio.setStageVariation(stageVariation);
  audio.setResonanceCharacter(resonanceCharacter);
  audio.setAnalogColor(analogColor);
}
static void presetPath(char *path, size_t n, uint8_t slot) {
  snprintf(path,n,"/RTAL_PHASER/PRESET%02u.DAT",slot);
}
static bool validPreset(const PresetData &p) {
  return p.magic==0x52545048UL && p.version==1 && p.checksum==presetChecksum(p) &&
    (p.stages==6 || p.stages==12) && p.sweep<=100 && p.resonance<=100 &&
    p.drive<=100 && p.bypass<=1 && p.lfoHigh<=1 && p.lfoRate<=100 &&
    p.lfoAmount<=100 && p.sync<=1 && p.division<11 && p.output<=2 &&
    p.bpm>=30 && p.bpm<=240;
}
static void applyPreset(const PresetData &p) {
  twelveStage=p.stages==12; sweepPct=p.sweep; resonancePct=p.resonance;
  drivePct=p.drive; bypass=p.bypass!=0; lfoHigh=p.lfoHigh!=0;
  lfoRatePct=p.lfoRate; lfoAmountPct=p.lfoAmount; syncEnabled=p.sync!=0;
  division=p.division; outputMode=p.output; internalBpm=p.bpm;
  audio.setStages(p.stages);
  audio.setSweepPercent(sweepPct);
  audio.setResonancePercent(resonancePct);
  audio.setDrivePercent(drivePct);
  audio.setBypass(bypass);
  audio.setLfoHighRange(lfoHigh);
  audio.setLfoRatePercent(lfoRatePct);
  audio.setLfoAmountPercent(lfoAmountPct);
  audio.setOutputMode(outputMode);
  stereoPhase=0; audio.setStereoPhase(0);
  updateSync();
}
static void savePreset(uint8_t slot) {
  if(slot<1 || slot>PRESET_COUNT) {showNotice("INVALID SLOT");return;}
  if (!sdReady) { showNotice("SD NOT READY"); return; }
  if (!SD.exists("/RTAL_PHASER") && !SD.mkdir("/RTAL_PHASER")) {
    showNotice("SD MKDIR ERROR"); return;
  }
  PresetDataV5 p = {};
  p.magic=0x52545048UL; p.version=5; p.stages=twelveStage?12:6;
  p.sweep=sweepPct; p.resonance=resonancePct; p.drive=drivePct;
  p.bypass=bypass; p.lfoHigh=lfoHigh; p.lfoRate=lfoRatePct;
  p.lfoAmount=lfoAmountPct; p.sync=syncEnabled; p.division=division;
  p.output=outputMode; p.bpm=internalBpm; p.stereoPhase=stereoPhase;
  p.analogMode=analogMode; p.stageVariation=stageVariation;
  p.resonanceCharacter=resonanceCharacter; p.analogColor=analogColor;
  p.envFollow=envFollow; p.envSensitivity=envSensitivity;
  p.envAttack=envAttack; p.envRelease=envRelease; p.envDepth=envDepth;
  p.matrixEnabled=matrixEnabled;p.mod1Source=mod1Source;p.mod1Target=mod1Target;p.mod1Depth=mod1Depth;
  p.mod2Source=mod2Source;p.mod2Target=mod2Target;p.mod2Depth=mod2Depth;p.modSmooth=modSmooth;
  p.checksum=presetChecksumV5(p);
  char path[48], temp[48]; presetPath(path,sizeof(path),slot);
  snprintf(temp,sizeof(temp),"/RTAL_PHASER/TEMP%02u.DAT",slot);
  SD.remove(temp);
  File f=SD.open(temp,FILE_WRITE);
  if (!f) {showNotice("SD WRITE ERROR");return;}
  const size_t n=f.write((const uint8_t*)&p,sizeof(p));
  f.flush(); f.close();
  if (n!=sizeof(p)) { SD.remove(temp);showNotice("SD WRITE ERROR");return; }
  // Keep old preset until a complete temp file has been written.
  SD.remove(path);
  if (!SD.rename(temp,path)) {showNotice("SD RENAME ERROR");return;}
  activePreset=slot;
  char msg[24];snprintf(msg,sizeof(msg),"PRESET %u SAVED",slot);showNotice(msg);
  Serial.printf("[PRESET] saved %u\n",slot);
}
static void loadPreset(uint8_t slot) {
  if(slot<1 || slot>PRESET_COUNT) {showNotice("INVALID SLOT");return;}
  if (!sdReady) {showNotice("SD NOT READY");return;}
  char path[48];presetPath(path,sizeof(path),slot);
  File f=SD.open(path,FILE_READ);
  if (!f) {showNotice("PRESET EMPTY");return;}
  const size_t fileSize=f.size();
  // Reset new fields only after successful validation (see end of load).
  bool newEnvFollow=false; uint8_t newEnvSensitivity=50, newEnvAttack=25, newEnvRelease=50; int8_t newEnvDepth=50;
  bool newAnalogMode=false; uint8_t newStageVariation=30,newResonanceCharacter=30,newAnalogColor=20;
  bool newMatrixEnabled=false; uint8_t newM1S=0,newM1T=0,newM2S=1,newM2T=1,newMSmooth=30;
  int8_t newM1D=0,newM2D=0;
  if (fileSize==sizeof(PresetData)) {
    PresetData p={};
    const size_t n=f.read((uint8_t*)&p,sizeof(p)); f.close();
    if(n!=sizeof(p) || !validPreset(p)) {showNotice("INVALID PRESET");return;}
    applyPreset(p); // Legacy V1 presets default to 0 degrees.
  } else if(fileSize==sizeof(PresetDataV2)) {
    PresetDataV2 p={};
    const size_t n=f.read((uint8_t*)&p,sizeof(p)); f.close();
    if(n!=sizeof(p) || p.magic!=0x52545048UL || p.version!=2 ||
       p.checksum!=presetChecksumV2(p) || p.stereoPhase>4 ||
       (p.stages!=6 && p.stages!=12) || p.sweep>100 || p.resonance>100 ||
       p.drive>100 || p.bypass>1 || p.lfoHigh>1 || p.lfoRate>100 ||
       p.lfoAmount>100 || p.sync>1 || p.division>=11 || p.output>2 ||
       p.bpm<30 || p.bpm>240) {showNotice("INVALID PRESET");return;}
    PresetData old={};
    old.stages=p.stages;old.sweep=p.sweep;old.resonance=p.resonance;
    old.drive=p.drive;old.bypass=p.bypass;old.lfoHigh=p.lfoHigh;
    old.lfoRate=p.lfoRate;old.lfoAmount=p.lfoAmount;old.sync=p.sync;
    old.division=p.division;old.output=p.output;old.bpm=p.bpm;
    applyPreset(old);
    stereoPhase=p.stereoPhase;audio.setStereoPhase(stereoPhase);
  } else if(fileSize==sizeof(PresetDataV3)) {
    PresetDataV3 p={};
    const size_t n=f.read((uint8_t*)&p,sizeof(p)); f.close();
    if(n!=sizeof(p) || p.magic!=0x52545048UL || p.version!=3 ||
       p.checksum!=presetChecksumV3(p) || p.stereoPhase>4 || p.analogMode>1 ||
       p.stageVariation>100 || p.resonanceCharacter>100 || p.analogColor>100 ||
       (p.stages!=6 && p.stages!=12) || p.sweep>100 || p.resonance>100 ||
       p.drive>100 || p.bypass>1 || p.lfoHigh>1 || p.lfoRate>100 ||
       p.lfoAmount>100 || p.sync>1 || p.division>=11 || p.output>2 ||
       p.bpm<30 || p.bpm>240) {showNotice("INVALID PRESET");return;}
    PresetData old={};
    old.stages=p.stages;old.sweep=p.sweep;old.resonance=p.resonance;
    old.drive=p.drive;old.bypass=p.bypass;old.lfoHigh=p.lfoHigh;
    old.lfoRate=p.lfoRate;old.lfoAmount=p.lfoAmount;old.sync=p.sync;
    old.division=p.division;old.output=p.output;old.bpm=p.bpm;
    applyPreset(old);
    stereoPhase=p.stereoPhase;audio.setStereoPhase(stereoPhase);
    newAnalogMode=p.analogMode;newStageVariation=p.stageVariation;
    newResonanceCharacter=p.resonanceCharacter;newAnalogColor=p.analogColor;
  } else if(fileSize==sizeof(PresetDataV4)) {
    PresetDataV4 p={};
    const size_t n=f.read((uint8_t*)&p,sizeof(p)); f.close();
    if(n!=sizeof(p) || p.magic!=0x52545048UL || p.version!=4 ||
       p.checksum!=presetChecksumV4(p) || p.stereoPhase>4 || p.analogMode>1 ||
       p.stageVariation>100 || p.resonanceCharacter>100 || p.analogColor>100 ||
       p.envFollow>1 || p.envSensitivity>100 || p.envAttack>100 ||
       p.envRelease>100 || p.envDepth< -100 || p.envDepth>100 ||
       (p.stages!=6 && p.stages!=12) || p.sweep>100 || p.resonance>100 ||
       p.drive>100 || p.bypass>1 || p.lfoHigh>1 || p.lfoRate>100 ||
       p.lfoAmount>100 || p.sync>1 || p.division>=11 || p.output>2 ||
       p.bpm<30 || p.bpm>240) {showNotice("INVALID PRESET");return;}
    PresetData old={};
    old.stages=p.stages;old.sweep=p.sweep;old.resonance=p.resonance;
    old.drive=p.drive;old.bypass=p.bypass;old.lfoHigh=p.lfoHigh;
    old.lfoRate=p.lfoRate;old.lfoAmount=p.lfoAmount;old.sync=p.sync;
    old.division=p.division;old.output=p.output;old.bpm=p.bpm;
    applyPreset(old);
    stereoPhase=p.stereoPhase;audio.setStereoPhase(stereoPhase);
    newAnalogMode=p.analogMode;newStageVariation=p.stageVariation;
    newResonanceCharacter=p.resonanceCharacter;newAnalogColor=p.analogColor;
    newEnvFollow=p.envFollow;newEnvSensitivity=p.envSensitivity;
    newEnvAttack=p.envAttack;newEnvRelease=p.envRelease;newEnvDepth=p.envDepth;
  } else if(fileSize==sizeof(PresetDataV5)) {
    PresetDataV5 p={};const size_t n=f.read((uint8_t*)&p,sizeof(p));f.close();
    if(n!=sizeof(p)||p.magic!=0x52545048UL||p.version!=5||p.checksum!=presetChecksumV5(p)||
       p.matrixEnabled>1||p.mod1Source>1||p.mod2Source>1||p.mod1Target>2||p.mod2Target>2||
       p.mod1Depth< -100||p.mod1Depth>100||p.mod2Depth< -100||p.mod2Depth>100||p.modSmooth>100||
       p.stereoPhase>4||p.analogMode>1||p.stageVariation>100||p.resonanceCharacter>100||p.analogColor>100||
       p.envFollow>1||p.envSensitivity>100||p.envAttack>100||p.envRelease>100||p.envDepth< -100||p.envDepth>100||
       (p.stages!=6&&p.stages!=12)||p.sweep>100||p.resonance>100||p.drive>100||p.bypass>1||
       p.lfoHigh>1||p.lfoRate>100||p.lfoAmount>100||p.sync>1||p.division>=11||p.output>2||p.bpm<30||p.bpm>240){
       showNotice("INVALID PRESET");return;
    }
    PresetData old={};old.stages=p.stages;old.sweep=p.sweep;old.resonance=p.resonance;
    old.drive=p.drive;old.bypass=p.bypass;old.lfoHigh=p.lfoHigh;old.lfoRate=p.lfoRate;
    old.lfoAmount=p.lfoAmount;old.sync=p.sync;old.division=p.division;old.output=p.output;old.bpm=p.bpm;
    applyPreset(old);stereoPhase=p.stereoPhase;audio.setStereoPhase(stereoPhase);
    newAnalogMode=p.analogMode;newStageVariation=p.stageVariation;newResonanceCharacter=p.resonanceCharacter;newAnalogColor=p.analogColor;
    newEnvFollow=p.envFollow;newEnvSensitivity=p.envSensitivity;newEnvAttack=p.envAttack;newEnvRelease=p.envRelease;newEnvDepth=p.envDepth;
    newMatrixEnabled=p.matrixEnabled;newM1S=p.mod1Source;newM1T=p.mod1Target;newM1D=p.mod1Depth;
    newM2S=p.mod2Source;newM2T=p.mod2Target;newM2D=p.mod2Depth;newMSmooth=p.modSmooth;
  } else {f.close();showNotice("INVALID PRESET");return;}
  analogMode=newAnalogMode;stageVariation=newStageVariation;
  resonanceCharacter=newResonanceCharacter;analogColor=newAnalogColor;
  envFollow=newEnvFollow;envSensitivity=newEnvSensitivity;
  envAttack=newEnvAttack;envRelease=newEnvRelease;envDepth=newEnvDepth;
  matrixEnabled=newMatrixEnabled;mod1Source=newM1S;mod1Target=newM1T;mod1Depth=newM1D;
  mod2Source=newM2S;mod2Target=newM2T;mod2Depth=newM2D;modSmooth=newMSmooth;
  applyAnalog(); applyEnvelope(); applyMatrix();
  activePreset=slot;
  char msg[24];snprintf(msg,sizeof(msg),"PRESET %u LOADED",slot);showNotice(msg);
  Serial.printf("[PRESET] loaded %u\n",slot);
}
static const char* menuNames[] = {"PHASER", "LFO", "ENVELOPE", "MOD MATRIX", "MIDI CLOCK", "OUTPUT", "SYSTEM", "PRESETS"};
static const uint8_t itemCounts[] = {9,6,5,8,3,1,3,0};
static const char* itemName(uint8_t group, uint8_t idx) {
  static const char* labels[7][9] = {
    {"STAGES","SWEEP","RESONANCE","DRIVE","BYPASS","ANALOG MODE","STAGE VAR","RES CHARACTER","ANALOG COLOR"},
    {"RANGE","RATE","AMOUNT","SYNC","DIVISION","STEREO PHASE"},
    {"ENV FOLLOW","ENV SENS","ENV ATTACK","ENV RELEASE","ENV DEPTH"},
    {"MATRIX","MOD1 SOURCE","MOD1 TARGET","MOD1 DEPTH","MOD2 SOURCE","MOD2 TARGET","MOD2 DEPTH","MOD SMOOTH"},
    {"INTERNAL BPM","CLOCK STATUS","MIDI CHANNEL"},
    {"OUTPUT MODE"},
    {"DSP AVG","DSP PEAK","UNDERRUNS"}
  };
  return labels[group][idx];
}
static bool editable() { return !(menuIndex == 4 && itemIndex == 1) && menuIndex != 6; }
static void itemValue(char* buf, size_t n) {
  if (menuIndex == 0) {
    switch(itemIndex) {
      case 0: snprintf(buf,n,"%u",twelveStage?12:6); break;
      case 1: snprintf(buf,n,"%u%%",sweepPct); break;
      case 2: snprintf(buf,n,"%u%%",resonancePct); break;
      case 3: snprintf(buf,n,"%u%%",drivePct); break;
      case 4: snprintf(buf,n,"%s",bypass?"ON":"OFF"); break;
      case 5: snprintf(buf,n,"%s",analogMode?"ON":"OFF"); break;
      case 6: snprintf(buf,n,"%u%%",stageVariation); break;
      case 7: snprintf(buf,n,"%u%%",resonanceCharacter); break;
      default: snprintf(buf,n,"%u%%",analogColor);
    }
  } else if (menuIndex == 1) {
    switch(itemIndex) {
      case 0: snprintf(buf,n,"%s",lfoHigh?"HIGH":"LOW"); break;
      case 1: snprintf(buf,n,"%u%%",lfoRatePct); break;
      case 2: snprintf(buf,n,"%u%%",lfoAmountPct); break;
      case 3: snprintf(buf,n,"%s",syncEnabled?"MIDI":"FREE"); break;
      case 4: snprintf(buf,n,"%s",divisionName[division]); break;
      default: snprintf(buf,n,"%u DEG",(unsigned)stereoPhase*45);
    }
  } else if (menuIndex == 2) {
    switch(itemIndex) {
      case 0: snprintf(buf,n,"%s",envFollow?"ON":"OFF"); break;
      case 1: snprintf(buf,n,"%u%%",envSensitivity); break;
      case 2: snprintf(buf,n,"%u ms",1+199*envAttack/100); break;
      case 3: snprintf(buf,n,"%u ms",10+1990*envRelease/100); break;
      default: snprintf(buf,n,"%+d%%",(int)envDepth); break;
    }
  } else if (menuIndex == 3) {
    switch(itemIndex){
      case 0:snprintf(buf,n,"%s",matrixEnabled?"ON":"OFF");break;
      case 1:case 4:snprintf(buf,n,"%s",(itemIndex==1?mod1Source:mod2Source)==0?"LFO":"ENVELOPE");break;
      case 2:case 5:{uint8_t t=itemIndex==2?mod1Target:mod2Target;snprintf(buf,n,"%s",t==0?"SWEEP":t==1?"RESONANCE":"DRIVE");break;}
      case 3:case 6:snprintf(buf,n,"%+d%%",(int)(itemIndex==3?mod1Depth:mod2Depth));break;
      default:snprintf(buf,n,"%u%%",modSmooth);break;
    }
  } else if (menuIndex == 4) {
    if(itemIndex==0) snprintf(buf,n,"%u",internalBpm);
    else if(itemIndex==1) snprintf(buf,n,"%s %u BPM",externalClock?"EXT":"INT",displayBpm);
    else if(midiChannel==0) snprintf(buf,n,"OMNI");
    else snprintf(buf,n,"CH %u",midiChannel);
  } else if (menuIndex == 5) snprintf(buf,n,"%s",outputMode==0?"MONO":outputMode==1?"MAIN/AUX":"STEREO");
  else snprintf(buf,n,"%lu",(unsigned long)(itemIndex==0?audio.audioLoadAvgUs():itemIndex==1?audio.audioLoadPeakUs():audio.underruns()));
}
static int clampValue(int v,int lo,int hi) {return v<lo?lo:(v>hi?hi:v);}
static void saveMidiConfig();
static void changeValue(int step) {
  if(!step || !editable()) return;
  if(menuIndex==0) {
    if(itemIndex==0) {twelveStage=step>0;audio.setStages(twelveStage?12:6);}
    if(itemIndex==1) {sweepPct=clampValue((int)sweepPct+step,0,100);audio.setSweepPercent(sweepPct);}
    if(itemIndex==2) {resonancePct=clampValue((int)resonancePct+step,0,100);audio.setResonancePercent(resonancePct);}
    if(itemIndex==3) {drivePct=clampValue((int)drivePct+step,0,100);audio.setDrivePercent(drivePct);}
    if(itemIndex==4) {bypass=step>0;audio.setBypass(bypass);}
    if(itemIndex==5) analogMode=step>0;
    if(itemIndex==6) stageVariation=clampValue((int)stageVariation+step,0,100);
    if(itemIndex==7) resonanceCharacter=clampValue((int)resonanceCharacter+step,0,100);
    if(itemIndex==8) analogColor=clampValue((int)analogColor+step,0,100);
    applyAnalog();
  } else if(menuIndex==1) {
    if(itemIndex==0) {lfoHigh=step>0;audio.setLfoHighRange(lfoHigh);audio.setLfoRatePercent(lfoRatePct);}
    if(itemIndex==1) {lfoRatePct=clampValue((int)lfoRatePct+step,0,100);audio.setLfoRatePercent(lfoRatePct);}
    if(itemIndex==2) {lfoAmountPct=clampValue((int)lfoAmountPct+step,0,100);audio.setLfoAmountPercent(lfoAmountPct);}
    if(itemIndex==3) syncEnabled=step>0;
    if(itemIndex==4) division=clampValue((int)division+step,0,10);
    if(itemIndex==5) {stereoPhase=clampValue((int)stereoPhase+step,0,4);audio.setStereoPhase(stereoPhase);}
  } else if(menuIndex==2) {
    if(itemIndex==0) envFollow=step>0;
    if(itemIndex==1) envSensitivity=clampValue((int)envSensitivity+step,0,100);
    if(itemIndex==2) envAttack=clampValue((int)envAttack+step,0,100);
    if(itemIndex==3) envRelease=clampValue((int)envRelease+step,0,100);
    if(itemIndex==4) envDepth=clampValue((int)envDepth+step,-100,100);
    applyEnvelope();
  } else if(menuIndex==3) {
    if(itemIndex==0)matrixEnabled=step>0;
    if(itemIndex==1)mod1Source=clampValue((int)mod1Source+step,0,1);
    if(itemIndex==2)mod1Target=clampValue((int)mod1Target+step,0,2);
    if(itemIndex==3)mod1Depth=clampValue((int)mod1Depth+step,-100,100);
    if(itemIndex==4)mod2Source=clampValue((int)mod2Source+step,0,1);
    if(itemIndex==5)mod2Target=clampValue((int)mod2Target+step,0,2);
    if(itemIndex==6)mod2Depth=clampValue((int)mod2Depth+step,-100,100);
    if(itemIndex==7)modSmooth=clampValue((int)modSmooth+step,0,100);
    applyMatrix();
  } else if(menuIndex==4 && itemIndex==0) internalBpm=clampValue((int)internalBpm+step,30,240);
  else if(menuIndex==4 && itemIndex==2) { midiChannel=clampValue((int)midiChannel+step,0,16); saveMidiConfig(); }
  else if(menuIndex==5) {outputMode=clampValue((int)outputMode+step,0,2);audio.setOutputMode(outputMode);}
}

// MIDI channel is device-global and stored independently of sound presets.
static void saveMidiConfig() {
  if(!sdReady) return;
  if(!SD.exists("/RTAL_PHASER")) SD.mkdir("/RTAL_PHASER");
  File f=SD.open("/RTAL_PHASER/MIDI.CFG",FILE_WRITE);
  if(f){f.seek(0);uint8_t bytes[3]={'M','C',midiChannel};f.write(bytes,3);f.close();}
}
static void loadMidiConfig() {
  if(!sdReady) return;
  File f=SD.open("/RTAL_PHASER/MIDI.CFG",FILE_READ);
  if(!f) return;
  if(f.size()==3 && f.read()=='M' && f.read()=='C'){
    int ch=f.read(); if(ch>=0 && ch<=16) midiChannel=(uint8_t)ch;
  }
  f.close();
}
static const uint8_t ccOrder[23]={106,109,110,111,112,115,118,119,75,76,86,89,90,77,78,28,52,29,102,103,105,23,30};
static uint8_t midiScale(uint8_t v,uint8_t max){return ((uint16_t)v*max+63)/127;}
static int midiBipolar(uint8_t v){return ((int)v*200+63)/127-100;}
static void midiSetParam(uint8_t group,uint8_t idx,int target) {
  uint8_t prevGroup=menuIndex,prevItem=itemIndex;
  menuIndex=group;itemIndex=idx;
  int current=0;
  if(group==0){switch(idx){case 0:current=twelveStage?1:0;break;case 1:current=sweepPct;break;case 2:current=resonancePct;break;case 3:current=drivePct;break;case 4:current=bypass;break;case 5:current=analogMode;break;case 6:current=stageVariation;break;case 7:current=resonanceCharacter;break;case 8:current=analogColor;break;}}
  if(group==1){switch(idx){case 0:current=lfoHigh;break;case 1:current=lfoRatePct;break;case 2:current=lfoAmountPct;break;case 3:current=syncEnabled;break;case 4:current=division;break;case 5:current=stereoPhase;break;}}
  if(group==2){switch(idx){case 0:current=envFollow;break;case 1:current=envSensitivity;break;case 2:current=envAttack;break;case 3:current=envRelease;break;case 4:current=envDepth;break;}}
  if(group==3){switch(idx){case 0:current=matrixEnabled;break;case 1:current=mod1Source;break;case 2:current=mod1Target;break;case 3:current=mod1Depth;break;case 4:current=mod2Source;break;case 5:current=mod2Target;break;case 6:current=mod2Depth;break;case 7:current=modSmooth;break;}}
  if(group==4)current=internalBpm;
  if(group==5)current=outputMode;
  if(current!=target)changeValue(target-current);
  menuIndex=prevGroup;itemIndex=prevItem;
}
static void midiControl(uint8_t cc,uint8_t value) {
  if(cc==32){midiBank=value==1?1:0;return;}
  int pos=-1;for(uint8_t i=0;i<23;i++)if(ccOrder[i]==cc){pos=i;break;}
  if(pos<0)return;
  if(midiBank==1){
    if(pos>=6)return;
    static const uint8_t ix[6]={1,2,3,4,5,6};
    int target=(pos==0||pos==3)?midiScale(value,1):
               (pos==1||pos==4)?midiScale(value,2):midiBipolar(value);
    midiSetParam(3,ix[pos],target);return;
  }
  static const uint8_t groups[23]={0,0,0,0,0,5,1,1,1,1,1,0,0,0,0,2,2,2,2,2,3,3,4};
  static const uint8_t items[23]={1,2,3,0,4,0,1,2,3,4,5,5,6,7,8,0,1,2,3,4,0,7,0};
  int target=0;
  if(pos==3||pos==4||pos==8||pos==11||pos==15||pos==20)target=midiScale(value,1);
  else if(pos==5)target=midiScale(value,2);
  else if(pos==9)target=midiScale(value,10);
  else if(pos==10)target=midiScale(value,4);
  else if(pos==19)target=midiBipolar(value);
  else if(pos==22)target=30+((uint16_t)value*210+63)/127;
  else target=midiScale(value,100);
  midiSetParam(groups[pos],items[pos],target);
}
static void serviceMidi() {
  while(midiSerial.available()) {
    uint8_t b=(uint8_t)midiSerial.read();
    if(b>=0xF8){serviceMidiClockByte(b);continue;} // realtime may interleave data bytes
    if(b&0x80){
      if(b==0xF0||b==0xF7||b==0xF1||b==0xF2||b==0xF3||b==0xF6){midiRunning=0;midiCount=0;continue;}
      if(b>=0xF0){midiRunning=0;midiCount=0;continue;}
      midiRunning=b;midiCount=0;continue;
    }
    if(!midiRunning)continue;
    uint8_t type=midiRunning&0xF0;
    uint8_t need=(type==0xC0||type==0xD0)?1:2;
    if(need==2 && midiCount==0){midiFirst=b;midiCount=1;continue;}
    uint8_t ch=(midiRunning&0x0F)+1;
    if(midiChannel==0||midiChannel==ch){
      if(type==0xB0)midiControl(midiFirst,b);
      else if(type==0xC0 && b<PRESET_COUNT) {loadPreset(b+1);browserPreset=b+1;}
    }
    midiCount=0;
  }
  serviceMidiClockTimeout();
}

// Optional human-readable names on SD: /RTAL_PHASER/NAME01.TXT ... NAME128.TXT.
// One short ASCII/UTF-8 line, max 19 bytes shown; missing names use slot number.
static void browserName(uint8_t slot, char *dest, size_t n) {
  snprintf(dest,n,"PRESET %03u",slot);
  if(!sdReady) return;
  char path[48];snprintf(path,sizeof(path),"/RTAL_PHASER/NAME%02u.TXT",slot);
  File f=SD.open(path,FILE_READ);
  if(!f) return;
  char tmp[24];size_t i=0;
  while(f.available() && i<sizeof(tmp)-1) {
    char c=(char)f.read();if(c=='\r'||c=='\n')break;
    if((uint8_t)c>=32)tmp[i++]=c;
  }
  f.close();tmp[i]=0;
  if(i)snprintf(dest,n,"%s",tmp);
}

static void drawUI() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tf);
  char line[40];
  if(uiPage==HOME) {
    snprintf(line,sizeof(line),"RTAL V015 P%03u %uST",activePreset,twelveStage?12:6);u8g2.drawStr(0,7,line);
    u8g2.drawHLine(0,12,128);
    // Lightweight animated sweep indication; display work stays on Core 0.
    const int pos=8+(int)((uint32_t)sweepPct*110/100);
    for(int x=0;x<128;x+=3) {
      int d=abs(x-pos); int y=21+(d%24);
      u8g2.drawPixel(x,y);
    }
    u8g2.drawVLine(pos,13,31);
    snprintf(line,sizeof(line),"SWP %3u%% RES %3u%%",sweepPct,resonancePct);u8g2.drawStr(0,51,line);
    snprintf(line,sizeof(line),"%s %s %u BPM",syncEnabled?"SYNC":"FREE",divisionName[division],displayBpm);u8g2.drawStr(0,62,line);
  } else if(uiPage==BROWSER || uiPage==BROWSER_ACTION) {
    u8g2.drawStr(0,7,uiPage==BROWSER?"PRESET BROWSER 1-128":"PRESET ACTION");
    u8g2.drawHLine(0,12,128);
    if(uiPage==BROWSER) {
      const int start=max(1,min(124,(int)browserPreset-2));
      for(int j=0;j<5;++j) {
        uint8_t slot=(uint8_t)(start+j);
        char name[24];browserName(slot,name,sizeof(name));
        snprintf(line,sizeof(line),"%c%03u %-18.18s",slot==browserPreset?'>':' ',slot,name);
        u8g2.drawStr(0,22+j*8,line);
      }
      u8g2.drawStr(0,63,"PRESS: ACTION  HOLD: HOME");
    } else {
      char name[24];browserName(browserPreset,name,sizeof(name));
      snprintf(line,sizeof(line),"%03u %-19.19s",browserPreset,name);
      u8g2.drawStr(0,22,line);
      snprintf(line,sizeof(line),"%c LOAD",browserAction==0?'>':' ');u8g2.drawStr(0,36,line);
      snprintf(line,sizeof(line),"%c SAVE (OVERWRITE)",browserAction==1?'>':' ');u8g2.drawStr(0,47,line);
      u8g2.drawStr(0,63,"PRESS: EXECUTE");
    }
  } else if(uiPage==MENU) {
    u8g2.drawStr(0,7,"MAIN MENU");u8g2.drawHLine(0,12,128);
    for(uint8_t i=(menuIndex>=5?3:0);i<(menuIndex>=5?8:5);i++) {snprintf(line,sizeof(line),"%c %s",i==menuIndex?'>':' ',menuNames[i]);u8g2.drawStr(0,22+(i-(menuIndex>=5?3:0))*8,line);}
  } else {
    snprintf(line,sizeof(line),"%s %s",menuNames[menuIndex],uiPage==EDIT?"EDIT":"SELECT");u8g2.drawStr(0,7,line);
    u8g2.drawHLine(0,12,128);
    for(uint8_t row=0;row<4;row++) {
      int idx=(int)itemIndex-1+(int)row;
      if(idx<0 || idx>=itemCounts[menuIndex]) continue;
      uint8_t old=itemIndex;itemIndex=idx;char val[32];itemValue(val,sizeof(val));itemIndex=old;
      snprintf(line,sizeof(line),"%c%-13.13s %s",idx==itemIndex?'>':' ',itemName(menuIndex,idx),val);
      u8g2.drawStr(0,23+row*9,line);
    }
    u8g2.drawStr(0,63,uiPage==EDIT?"PRESS: SAVE":"PRESS: EDIT / BACK");
  }
  if ((int32_t)(noticeUntil-millis()) > 0) {
    u8g2.setDrawColor(0); u8g2.drawBox(0,24,128,18);
    u8g2.setDrawColor(1); u8g2.drawFrame(0,24,128,18);
    u8g2.drawStr(5,36,notice);
  }
  u8g2.sendBuffer();
}

static void showStartupSplash() {
  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 0, 128, 64, RTAL_PHASER_SPLASH);
  u8g2.sendBuffer();
}

void setup() {
  Serial.begin(115200);
  delay(150);
  Serial.println();
  Serial.println("RTAL Phoenix Phaser v1.0.0 FINAL (P015 FIX5 base)");

  SPI.begin(OLED_SCK, -1, OLED_MOSI, OLED_CS);
  u8g2.begin();
  u8g2.setContrast(220);

  input.begin(); // T1..T8 ignored by application; encoder is sole UI control.
  pinMode(SD_CS_PIN,OUTPUT);
  digitalWrite(SD_CS_PIN,HIGH);
  sdReady=SD.begin(SD_CS_PIN,SPI,8000000);
  Serial.printf("[SD] %s\n",sdReady?"READY":"NOT FOUND");
  loadMidiConfig();
  midiSerial.begin(31250, SERIAL_8N1, 40, 39);
  audio.setStages(12);
  audio.setSweepPercent(sweepPct);
  audio.setResonancePercent(resonancePct);
  audio.setDrivePercent(drivePct);
  audio.setBypass(false);
  audio.setOutputMode(outputMode);
  audio.setLfoHighRange(lfoHigh);
  audio.setLfoRatePercent(lfoRatePct);
  audio.setLfoAmountPercent(lfoAmountPct);
  audio.begin();

  showStartupSplash();
  delay(1800); // Audio task runs independently; startup image only
  drawUI();
  Serial.printf("Audio: %s, fs=%u, block=%u, stages=%u, sweep=%u%%, center=%u Hz\n",
                audio.ok() ? "OK" : "ERROR",
                (unsigned)audio.sampleRate(),
                (unsigned)audio.blockFrames(),
                (unsigned)audio.stages(),
                (unsigned)sweepPct,
                (unsigned)audio.centerFrequencyHz());
}

void loop() {
  PhoenixPhaserInputState st;
  input.update(st);
  serviceMidi();
  uint32_t now=millis();
  bool changed=false;
  if(st.encoderLongPress) {uiPage=HOME;changed=true;lastInteraction=now;}
  else {
    if(st.encoderPress) {
      lastInteraction=now;changed=true;
      if(uiPage==HOME) uiPage=MENU;
      else if(uiPage==BROWSER) {uiPage=BROWSER_ACTION;browserAction=0;}
      else if(uiPage==BROWSER_ACTION) {
        if(browserAction==0)loadPreset(browserPreset);
        else savePreset(browserPreset);
        uiPage=BROWSER;
      }
      else if(uiPage==MENU) {
        if(menuIndex==7)uiPage=BROWSER;
        else {uiPage=PAGE;itemIndex=0;}
      }
      else if(uiPage==PAGE) {
        if(editable()) uiPage=EDIT;
        else if(menuIndex==6 && itemIndex==2) uiPage=MENU;
      } else uiPage=PAGE;
    }
    if(st.encoderStep) {
      lastInteraction=now;changed=true;
      int step=st.encoderStep;
      // Acceleration only when editing; preserve precise single-detent control.
      if(uiPage==EDIT && now-lastEncoderMs<75 && abs(step)>1) step*=3;
      lastEncoderMs=now;
      if(uiPage==HOME) {sweepPct=clampValue((int)sweepPct+step,0,100);audio.setSweepPercent(sweepPct);}
      else if(uiPage==MENU) menuIndex=clampValue((int)menuIndex+step,0,7);
      else if(uiPage==BROWSER) browserPreset=clampValue((int)browserPreset+step,1,PRESET_COUNT);
      else if(uiPage==BROWSER_ACTION) browserAction=clampValue((int)browserAction+step,0,1);
      else if(uiPage==PAGE) itemIndex=clampValue((int)itemIndex+step,0,itemCounts[menuIndex]-1);
      else changeValue(step);
    }
  }
  // Short press on the last menu item? Back navigation is available by long press.
  if(uiPage!=HOME && uiPage!=EDIT && uiPage!=BROWSER && uiPage!=BROWSER_ACTION && now-lastInteraction>10000) {uiPage=HOME;changed=true;}
  updateSync();
  if(changed || now-lastDrawMs>=200) {lastDrawMs=now;drawUI();}
  delay(2);
}
