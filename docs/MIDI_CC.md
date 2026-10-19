# HQ Digital Phaser – MIDI-CC-Referenz

**Gültig für die dokumentierte Phaser-Parameterzuordnung.** Der Produktname wurde geändert; MIDI-CC-Nummern und interne Protokollkennungen bleiben kompatibel. DIN MIDI: 31250 Baud; MIDI-Kanal über SYSTEM/OMNI.

## Auswahl der Parameterbank

Vor einem Parameter-CC zunächst **CC32** senden: Wert **0** = Hauptparameter, Wert **1** = Modulationsmatrix. Die folgenden Controller-Nummern sind bankabhängig; ohne korrekte Bankwahl kann derselbe CC einen anderen Parameter steuern.

## Bank 0 – CC32 = 0

| CC | Parameter | Bedeutung |
|---:|---|---|
| 106 | SWEEP | Grundposition der Phaser-Kerben |
| 109 | RESONANCE | Rückkopplung / Kerbintensität |
| 110 | DRIVE | Klangfärbung / Sättigung |
| 111 | STAGES | 6 oder 12 Allpass-Stufen |
| 112 | BYPASS | Effektbearbeitung ein/aus |
| 115 | OUTPUT MODE | MONO / MAIN-AUX / STEREO |
| 118 | LFO RATE | LFO-Geschwindigkeit |
| 119 | LFO AMOUNT | LFO-Modulationstiefe |
| 75 | LFO SYNC | FREE / MIDI CLOCK |
| 76 | DIVISION | Rhythmische Unterteilung |
| 86 | STEREO PHASE | Stereo-LFO-Phasenverschiebung |
| 89 | ANALOG MODE | Analoge Klangfärbung ein/aus |
| 90 | STAGE VARIATION | Unterschiede zwischen Stufen |
| 77 | RESONANCE CHARACTER | Resonanzcharakter |
| 78 | ANALOG COLOR | Analoge Färbung |
| 28 | ENVELOPE ON | Envelope Follower ein/aus |
| 52 | SENSITIVITY | Envelope-Empfindlichkeit |
| 29 | ATTACK | Envelope-Ansprechzeit |
| 102 | RELEASE | Envelope-Abklingzeit |
| 103 | DEPTH | Bipolare Envelope-Wirkung |
| 105 | MATRIX ON | Modulationsmatrix ein/aus |
| 23 | MOD SMOOTH | Glättung der Modulation |
| 30 | INTERNAL BPM | Internes Tempo |

## Bank 1 – CC32 = 1

| CC | Parameter | Bedeutung |
|---:|---|---|
| 106 | MOD1 SOURCE | LFO oder ENVELOPE |
| 109 | MOD1 TARGET | SWEEP / RESONANCE / DRIVE |
| 110 | MOD1 DEPTH | Bipolare Tiefe der ersten Route |
| 111 | MOD2 SOURCE | LFO oder ENVELOPE |
| 112 | MOD2 TARGET | SWEEP / RESONANCE / DRIVE |
| 115 | MOD2 DEPTH | Bipolare Tiefe der zweiten Route |

## Program Change und Clock

- **Program Change 0–127 → Preset 001–128.** Beispiel: PC 0 = Slot 1; PC 127 = Slot 128.
- **MIDI Clock:** Echtzeitnachricht `F8`. Start `FA`, Continue `FB`, Stop `FC` sind MIDI-Standardnachrichten; ob und wie sie wirken, hängt von der Firmware ab.
- **Werte 0–127:** MIDI-CC-Werte werden intern je nach Parameter skaliert. Aus dieser Referenz allein lässt sich **keine** exakte Schwellentabelle für jeden Enum-Parameter ableiten. Für präzise Werte den verwendeten Firmware-Sketch prüfen.

## Praxisbeispiele

**Sweep ändern:** CC32=0, danach CC106 mit gewünschtem Wert 0–127 senden. **MOD1 DEPTH ändern:** CC32=1, danach CC110 senden. Anschließend für den Sweep unbedingt wieder CC32=0 wählen.

**Modulation testen:** MATRIX ON über Bank 0 / CC105 aktivieren; Bank 1 auswählen, MOD1 SOURCE, TARGET und DEPTH setzen; mit einem langsamen LFO beginnen und bei starken Resonanzwerten den Ausgangspegel kontrollieren.

## SysEx und Slot 128

Der HQ Digital Phaser Editor verwendet für Preset-Dumps und Bankübertragung zusätzlich SysEx. **Nicht mit MIDI-CC verwechseln.** MIDI-SysEx-Datenbytes dürfen nur 0–127 enthalten. Die getestete P016-FIX4b-Kommunikation codiert den logischen Slot 128 im entsprechenden Slotfeld als 0. Die interne Protokollkennung bleibt trotz Projektumbenennung unverändert.
