# RTAL HQ Digital Phaser

**v1.0.0 FINAL · ESP32-S3 · Stereo Phaser · 128 Presets**

A digital phaser inspired by the classic Moog MF-103, developed by **RealTimeAudioLab** for the Phoenix ESP32-S3 audio platform. Independent project; not affiliated with Moog Music.

## Features

- 6-/12-stage phasing, resonance and drive
- LFO 0.01–250 Hz with internal tempo and MIDI-clock sync
- Mono, Main/Aux and true-stereo output; adjustable stereo LFO phase
- Analog character controls (stage variation, resonance character, color)
- Envelope follower and two-slot modulation matrix
- Encoder-based OLED interface; SD preset browser
- MIDI DIN CC control in two banks and Program Change 0–127
- 128 SD presets (**CREATIVE V2**, 71 with active modulation matrix)

## Hardware

| Component | Connection |
|---|---|
| MCU | ESP32-S3 |
| ADC | PCM1808, I2S input GPIO5 |
| DAC | PCM5102A, I2S output GPIO17 |
| I2S | BCLK GPIO18, LRCK GPIO16, MCLK GPIO0 (per hardware setup) |
| OLED | SSD1309 SPI 128×64: SCK12, MOSI11, CS10, DC6, RST7 |
| Encoder | A1, B2, switch42 |
| SD | SPI SCK12, MOSI11, MISO13, CS9 |
| MIDI DIN | RX40, TX39; 31250 baud |

Audio: 32 kHz, 128-frame processing blocks.

## Installation

1. Back up the existing SD card directory `RTAL_PHASER`, especially `MIDI.CFG` and personal presets.
2. Copy the files from `Soundbank/RTAL_PHASER/` to the SD card directory `/RTAL_PHASER/`. Preserve your existing `MIDI.CFG`.
3. Open `Firmware/RTAL_Phaser_v1_0_0_FINAL/RTAL_Phaser_v1_0_0_FINAL.ino` in Arduino IDE 2. The sketch folder and `.ino` filename must match.
4. Select the appropriate ESP32-S3 board and compile/upload with the installed ESP32 core and required U8g2 library.
5. Use the encoder to select and load presets. MIDI Program Change 0 selects preset 001; PC 127 selects preset 128.

## Documentation

- [German operating guide](docs/RTAL_Phaser_v1.0.0_Handbuch_DE.pdf)
- [MIDI CC mapping](docs/MIDI_CC.md)
- [Preset catalog (CSV)](sdcard_content/PRESET_CATALOG.csv)
- [Release notes](firmware/RELEASE.md)

## Release status

Firmware is based on the hardware-confirmed **P015 FIX5** baseline. The 128 preset files have been structurally validated, but a complete audio listening and endurance test of the final combined package is still pending. This release is supplied as-is; back up your SD data before installation.

## License

No open-source license is asserted in this package. Copyright remains with the respective authors. Add a LICENSE file only after selecting the intended license and checking any third-party code dependencies.

*RealTimeAudioLab · RTAL Embedded Audio Initiative · RTAL-EAI-012*
