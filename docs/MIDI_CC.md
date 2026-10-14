# MIDI-CC-Referenz

CC32 wählt die Parameterbank: **0** = Hauptparameter, **1** = Modulationsmatrix. MIDI-Kanal in SYSTEM.

## Bank 0 (CC32=0)

| CC | Parameter |
|---:|---|
| 106 | SWEEP |
| 109 | RESONANCE |
| 110 | DRIVE |
| 111 | STAGES |
| 112 | BYPASS |
| 115 | OUTPUT MODE |
| 118 | LFO RATE |
| 119 | LFO AMOUNT |
| 75 | LFO SYNC |
| 76 | DIVISION |
| 86 | STEREO PHASE |
| 89 | ANALOG MODE |
| 90 | STAGE VARIATION |
| 77 | RESONANCE CHARACTER |
| 78 | ANALOG COLOR |
| 28 | ENVELOPE ON |
| 52 | SENSITIVITY |
| 29 | ATTACK |
| 102 | RELEASE |
| 103 | DEPTH |
| 105 | MATRIX ON |
| 23 | MOD SMOOTH |
| 30 | INTERNAL BPM |

## Bank 1 (CC32=1)

| CC | Parameter |
|---:|---|
| 106 | MOD1 SOURCE |
| 109 | MOD1 TARGET |
| 110 | MOD1 DEPTH |
| 111 | MOD2 SOURCE |
| 112 | MOD2 TARGET |
| 115 | MOD2 DEPTH |

**Program Change:** 0–127 → Preset 001–128. MIDI Clock: F8; Start/Continue/Stop nach Firmware-Implementierung.
