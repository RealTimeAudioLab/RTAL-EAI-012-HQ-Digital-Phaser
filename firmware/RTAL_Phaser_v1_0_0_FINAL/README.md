# RTAL HQ Digital Phaser v1.0.0 FINAL

### RealTimeAudioLab | ESP32-S3 Stereo Phaser

**The first official release of RTAL Phoenix Phaser is here!**

RTAL  HQ Digital Phaser is a real-time digital stereo phaser inspired by the legendary **Moog MF-103 12-Stage Phaser**, developed for the ESP32-S3 platform.

The project combines classic analog-inspired phasing with modern digital modulation, MIDI integration, stereo processing, and a comprehensive preset system.

Designed and developed by **RealTimeAudioLab**, the Phoenix Phaser demonstrates the possibilities of real-time audio DSP on affordable microcontroller hardware.

---

## ✨ Main Features

### Analog-Inspired Phaser Engine
- Selectable 6-stage and 12-stage phaser processing
- Adjustable sweep frequency and resonance
- Feedback-based resonance control
- Adjustable drive and analog-inspired saturation
- Stage Variation for a more organic phaser response
- Resonance Character and Analog Color controls
- Real-time DSP processing

### Advanced LFO Engine
- Wide frequency range from 0.01 Hz to 250 Hz
- Adjustable modulation depth
- Internal tempo generation
- MIDI Clock synchronization
- 11 selectable rhythmic divisions
- Adjustable stereo LFO phase

### Stereo Audio Processing
- Mono processing
- Main/Aux output configuration
- True stereo operation
- Independent stereo modulation behavior

### Envelope Follower
- Audio-input-driven modulation
- Adjustable sensitivity
- Attack and release controls
- Positive and negative modulation depth
- Dynamic, performance-responsive phaser effects

### Dual Modulation Matrix
Two independent modulation routes provide additional sound-design possibilities.

Features include:
- LFO and Envelope modulation sources
- Multiple modulation destinations
- Positive and negative modulation depths
- Simultaneous modulation of two parameters
- Adjustable modulation smoothing

The modulation matrix enables everything from subtle analog movement to complex, evolving and experimental phaser textures.

---

## 🎛️ User Interface

The HQ Digital Phaser features an SSD1309 128 × 64 OLED display and an encoder-based user interface.

- Complete parameter editing using a rotary encoder
- Structured parameter menus
- Integrated preset browser
- Preset names displayed on the OLED
- Custom RealTimeAudioLab startup screen
- No additional control buttons required

---

## 🎹 MIDI Integration

Full MIDI control allows integration into hardware synthesizer setups, DAWs, and external MIDI controller systems.

- MIDI Control Change support
- Two MIDI CC parameter banks
- MIDI Program Change support
- MIDI Clock synchronization
- Start, Stop and Continue handling
- Configurable MIDI channel
- Remote control of the modulation matrix

**Program Change 0–127 selects Presets 001–128.**

---

## 💾 128 Presets – Soundbank

This release includes a complete collection of **128 phaser presets**, designed to demonstrate the sonic possibilities of the Phoenix DSP engine.

The soundbank covers:
- Classic vintage phaser sounds
- Warm analog-inspired sweeps
- Wide stereo modulation
- Slow and evolving textures
- Envelope-controlled effects
- Rhythmic modulation
- Resonant and metallic phasing
- Experimental dual-matrix effects

**71 presets feature an active modulation matrix**, providing additional movement and dynamic sound variations.

All presets are stored on an SD card and can be selected directly from the OLED interface or via MIDI Program Change.

---

## 🔧 Hardware Platform

The project is based on the ESP32-S3 microcontroller.

| Component | Hardware |
|---|---|
| Microcontroller | ESP32-S3 |
| Audio ADC | PCM1808 |
| Audio DAC | PCM5102A |
| Display | SSD1309 OLED, 128 × 64 |
| Storage | MicroSD card |
| MIDI | DIN MIDI |
| Controls | Rotary encoder |
| Audio Interface | I2S |
| Sample Rate | 32 kHz |
| Audio Block Size | 128 samples |

The firmware is developed using the Arduino environment.

---

## 📦 Release Contents

The release package includes:
- Complete ESP32-S3 Arduino firmware
- 128-preset  soundbank
- Preset names and parameter catalog
- Hardware and installation documentation
- MIDI CC reference
- Detailed German user manual
- README and release documentation

---

## 🚀 Getting Started

1. Download the release package.
2. Open the Arduino project in Arduino IDE.
3. Configure the ESP32-S3 board.
4. Compile and upload the firmware.
5. Copy the supplied preset soundbank to the SD card.
6. Connect the audio input and output.
7. Power up the device and explore the presets.

For detailed installation instructions, hardware connections and parameter descriptions, please refer to the included documentation.

---

## 🧪 Development Status

**Firmware:** P015 FIX5  
**Release:** v1.0.0 FINAL  
**Soundbank:** 128 Presets

The firmware is based on the latest confirmed Phoenix Phaser development version.

The soundbank has been checked for file structure, parameter ranges and checksums. Final listening tests of the complete CREATIVE V2 collection remain recommended.

---

## 🎵 About RealTimeAudioLab

RealTimeAudioLab explores real-time audio synthesis, digital signal processing, embedded music instruments and audio effects using modern microcontroller platforms.

The Phoenix Phaser is part of a growing collection of DIY audio projects developed around the ESP32-S3 architecture.

**RealTimeAudioLab – Real-Time Audio Engineering**

---

### Thank you for your interest!

Explore the project, experiment with the modulation possibilities, and discover what modern microcontrollers can achieve in real-time audio processing.

**RTAL Phoenix Phaser v1.0.0 FINAL**  
© 2026 RealTimeAudioLab