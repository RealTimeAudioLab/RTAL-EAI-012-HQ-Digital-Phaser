# HQ Digital Phaser — V1.0.0 FINAL package

RealTimeAudioLab · ESP32-S3 stereo digital phaser · Windows editor and 128-slot MIDI librarian.

## Contents

- `firmware/HQ_Digital_Phaser_V1_0_0_FINAL/` — complete Arduino sketch and sources.
- `editor/HQ_Digital_Phaser_Editor.py` — editor V1.0.0 source.
- `editor/BUILD_WINDOWS_EXE.cmd` — builds the Windows executable locally with Python 3.12 and PyInstaller.
- `docs/` — German manual, MIDI reference and migration documentation.
- `images/` — screenshots of editor and PRO LIBRARIAN.
- `RELEASE_NOTES.md` and `RELEASE_CHECKLIST.md`.

## Windows editor

## Firmware

Open `firmware/HQ_Digital_Phaser_V1_0_0_FINAL/HQ_Digital_Phaser_V1_0_0_FINAL.ino` in Arduino IDE 2, configure your actual ESP32-S3 board and required libraries, compile, then upload. The Arduino sketch directory and `.ino` filename match. Preserve SD `/RTAL_PHASER/` including `MIDI.CFG` and all existing preset/name files. **Back up SD and the complete 128-slot bank before flashing or restoring.**

## Compatibility

## Release qualification

Earlier user hardware tests confirmed full 128-slot MIDI backup and verified restore, single-slot restore, editor, and MIDI-port memory. The rebranded final package requires one more compile/build and smoke test.

Independent project inspired by classic analog phasers; no affiliation with Moog Music. © 2026 RealTimeAudioLab.
