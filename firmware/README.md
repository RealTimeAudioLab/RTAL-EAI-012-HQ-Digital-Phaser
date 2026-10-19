# HQ Digital Phaser — V1.0.0 FINAL package

RealTimeAudioLab · ESP32-S3 stereo digital phaser · Windows editor and 128-slot MIDI librarian.

**Release basis:** hardware-confirmed firmware **P016 FIX4b** and editor **RC5 FIX3**. The final packaging only updates the visible version/title and Arduino sketch folder/name; DSP, MIDI protocol, SD layout and preset data remain unchanged. **This assembled V1.0.0 package has not yet been compiled/flashed or end-to-end retested.**

## Contents

- `firmware/HQ_Digital_Phaser_V1_0_0_FINAL/` — complete Arduino sketch and sources.
- `editor/HQ_Digital_Phaser_Editor.py` — editor V1.0.0 source.
- `editor/BUILD_WINDOWS_EXE.cmd` — builds the Windows executable locally with Python 3.12 and PyInstaller.
- `docs/` — German manual, MIDI reference and migration documentation.
- `images/` — screenshots of editor and PRO LIBRARIAN.
- `RELEASE_NOTES.md` and `RELEASE_CHECKLIST.md`.

## Windows editor

Run `editor/BUILD_WINDOWS_EXE.cmd` on Windows 10/11 with Python 3.12 installed. The resulting executable is `editor/dist/HQ_Digital_Phaser_Editor.exe`. **No precompiled EXE is included.** Connect MIDI IN and MIDI OUT and press `Verbinden`. The most recently used ports are remembered; SD slot starts at 1.

![HQ Digital Phaser Editor](images/HQ_Digital_Phaser_Editor_1.JPG)

![HQ Digital Phaser PRO LIBRARIAN](images/HQ_Digital_Phaser_Editor_Pro_Librarian.JPG)

## Firmware

Open `firmware/HQ_Digital_Phaser_V1_0_0_FINAL/HQ_Digital_Phaser_V1_0_0_FINAL.ino` in Arduino IDE 2, configure your actual ESP32-S3 board and required libraries, compile, then upload. The Arduino sketch directory and `.ino` filename match. Preserve SD `/RTAL_PHASER/` including `MIDI.CFG` and all existing preset/name files. **Back up SD and the complete 128-slot bank before flashing or restoring.**

## Compatibility

The historic internal SD directory `/RTAL_PHASER/`, `PRESETxx.DAT` and `NAMExx.TXT` naming, binary V5 preset format, and the MIDI SysEx vendor/protocol bytes intentionally retain their original identifiers. This is not a data migration.

## Release qualification

Earlier user hardware tests confirmed full 128-slot MIDI backup and verified restore, single-slot restore, editor RC4, and MIDI-port memory. The rebranded final package requires one more compile/build and smoke test. See `RELEASE_CHECKLIST.md`.

Independent project inspired by classic analog phasers; no affiliation with Moog Music. © 2026 RealTimeAudioLab.
