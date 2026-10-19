# HQ Digital Phaser – Firmware und Editor

**RealTimeAudioLab | Technische Dokumentation | Oktober 2026**

## Projektidentität und Status

- **Neuer Projektname:** HQ Digital Phaser. Frühere Bezeichnungen „RTAL Phoenix Phaser“ und „RTAL HQ Digital Phaser“ sind historische Namen.
- **Historische Firmware-Veröffentlichung:** v1.0.0 FINAL auf Basis **P015 FIX5** (so in den gelieferten Altunterlagen bezeichnet).
- **Spätere bestätigte Editor-Hardwarebasis:** **P016 FIX4b**. Damit wurden Backup/Restore aller 128 Presets und Editorfunktionen getestet.
- **Editor-Entwicklung:** V0.9 PRO LIBRARIAN → V1.0 RC1–RC5; RC5 FIX1 merkt sich MIDI-Ports, RC5 FIX2 setzt den SD-Startslot auf 1. Die reine Produktumbenennung erfolgte anschließend.
- Die Versionsnummer **v1.0.0 FINAL** im alten Firmware-README ist **keine** Bestätigung, dass der neue Windows-Editor bereits als V1.0.0 FINAL veröffentlicht wurde.

## Funktionsarchitektur

Das Eingangssignal gelangt über PCM1808/I²S in den ESP32-S3. Die Effektverarbeitung umfasst 6/12 Allpass-Stufen mit Sweep, Resonance und Drive. LFO und Envelope Follower steuern den Phaser direkt und über zwei Modulationsrouten. Der PCM5102A gibt das Ergebnis aus. OLED, Encoder und SD bilden die lokale Bedien- und Preset-Ebene; DIN-MIDI steuert Parameter, Presets und Clock.

## Pinbelegung

| Baugruppe | GPIO / Parameter |
|---|---|
| PCM1808 | DIN 5, BCLK 18, LRCK 16, MCLK 0 |
| PCM5102A | DOUT 17, BCLK 18, LRCK 16 |
| OLED SSD1309 SPI | SCK 12, MOSI 11, CS 10, DC 6, RST 7 |
| SD SPI | SCK 12, MISO 13, MOSI 11, CS 9 |
| Encoder | A 1, B 2, Switch 42 |
| MIDI DIN | RX 40, TX 39, 31250 Baud |

**Audio-Basis der historischen Firmware:** 32 kHz, 128 Frames pro Block. Diese Angabe bei neuen Firmware-Releases anhand des jeweiligen Sketches erneut verifizieren.

## Installation der Firmware

1. SD-Karte komplett sichern, insbesondere `/RTAL_PHASER/` und `MIDI.CFG`.
2. Arduino IDE 2 und passende ESP32-S3-Core-Version installieren; die verwendete Core-Version im Release dokumentieren.
3. U8g2 und sonstige vom Sketch eingebundene Bibliotheken installieren.
4. Die **tatsächlich ausgelieferte** `.ino` öffnen. Sketch-Ordner und `.ino` müssen gleich heißen.
5. Board-, Flash- und PSRAM-Einstellungen anhand des eingesetzten ESP32-S3-Boards setzen; Firmware kompilieren und flashen.
6. Seriellen Monitor bei 115200 Baud zur Diagnose öffnen (so in den Altunterlagen beschrieben).
7. OLED, Encoder, ADC/DAC, MIDI und SD testen.

**Achtung:** Der historische Pfad `Firmware/RTAL_Phaser_v1_0_0_FINAL/...` beschreibt nur das alte Release-Layout und ist kein garantierter Pfad im aktuellen GitHub-Repository.

## Bedienung am Gerät

| Ansicht | Encoder drehen | Kurz drücken |
|---|---|---|
| HOME | Sweep ändern | Hauptmenü |
| MAIN MENU | Menügruppe wählen | Gruppe öffnen |
| PAGE | Parameter auswählen | Editieren |
| EDIT | Wert verändern | Übernehmen |
| PRESET BROWSER | Slot 001–128 wählen | LOAD/SAVE-Auswahl |
| PRESET ACTION | LOAD/SAVE wählen | Ausführen |

Langer Druck führt zur HOME-Anzeige. Schnelles Drehen kann im EDIT-Modus beschleunigt werden.

## SD-Speicherformat – unverändert

- Ordner: `/RTAL_PHASER/`
- Presets: `PRESET01.DAT` bis `PRESET128.DAT` (historisches V5-Format, **41 Bytes**, mit Prüfsumme).
- Namen: `NAME01.TXT` bis `NAME128.TXT`.
- MIDI-Einstellungen: `MIDI.CFG`.
- Vorhandene Presets **nicht** bloß wegen der Produktumbenennung verschieben oder umbenennen.
- Bei `INVALID PRESET` Struktur, Prüfsumme, Wertebereich und Dateiherkunft kontrollieren.

## MIDI – Überblick

DIN-MIDI 31250 Baud. `CC32=0` wählt Hauptparameter, `CC32=1` Modulationsmatrix. Program Change 0–127 lädt die Presets 001–128. MIDI Clock ist `F8`; Start/Continue/Stop sind von der jeweiligen Firmware-Implementierung abhängig. Vollständige Tabelle: [MIDI_CC.md](MIDI_CC.md).

**MIDI-SysEx:** Die Windows-Editor-Kommunikation benutzt ein proprietäres SysEx-Protokoll. Alle MIDI-Datenbytes müssen 0–127 sein. Für **SD-Slot 128** wurde in der getesteten P016-FIX4b-Editor-Kombination eine besondere 7-Bit-kompatible Codierung eingeführt. Keine Protokolländerungen allein wegen des neuen Produktnamens vornehmen.

## Windows-Editor – Funktionen und Arbeitsweise

Der Editor bietet LIVE-Parametersteuerung, lesbare Enum-Auswahlfelder, Einheitendarstellung und Tooltips, Undo/Redo, A/B-Klangzustände, zwei PC-Banken mit Drag & Drop, Kategorien/Favoriten, Preset-Vergleich sowie MIDI-Backup und verifizierten Restore.

**MIDI-Port-Speicherung:** Nach erfolgreichem Verbinden werden die letzten MIDI IN/OUT-Ports in einer lokalen JSON-Datei gespeichert und beim nächsten Start vorausgewählt. Eine automatische Verbindung beim Start war in RC5 FIX1 noch nicht Bestandteil der Funktion.

**Backup:** 128 Presets einschließlich Namen vom Phaser lesen; anschließend Bankordner am PC speichern.

**Restore:** Nach vorheriger Sicherung zunächst Einzel-Slot-Restore + Verify durchführen; Vollbank-Restore überschreibt alle 128 Slots. Der Editor liest die Daten zur Verifikation zurück. Ein Abbruch stellt bereits überschriebene Slots nicht automatisch wieder her.

**A/B:** Die Zustände werden im Editor gespeichert. Um einen Zustand sicher am Gerät zu hören, **Zum Phaser senden** verwenden.

## Datensicherheit und Fehlerdiagnose

- Vor Firmware-Updates, SD-Operationen und Vollbank-Restores ein vollständiges Backup anlegen.
- Nach dem Speichern im Editor kontrollieren, ob die Bankdateien wirklich im Zielordner liegen.
- RC5 enthält `.bak`-Sicherungen beim Überschreiben vorhandener PC-Dateien, temporäre Schreibdateien und ein Transferprotokoll; dies ersetzt kein externes Backup.
- Keine MIDI-Reaktion: MIDI-Portauswahl, IN/OUT-Richtung, DIN-Verdrahtung, CC32-Bank, MIDI-Kanal und USB-MIDI-Interface prüfen.
- Keine Clock-Synchronisation: LFO SYNC, sendende MIDI-Clock und Statusanzeige prüfen.
- Audiofehler: Eingangspegel, Output Mode, I²S-Pins, DSP AVG/PEAK und UNDERRUNS prüfen.

## Release-Checkliste

- Firmwareversion, Arduino-Core, Boardparameter und Buildprotokoll festhalten.
- Alle 128 Presets inklusive Namen laden; kritische Resonanz-/Matrix-Sounds abhören.
- CC32-Bank 0 und 1, Program Change 0 und 127 sowie MIDI Clock testen.
- Editor LIVE, Undo/Redo, A/B, zwei Banken, Slot 128, MIDI-Port-Speicherung testen.
- Einzel- und Vollbank-Restore mit Rücklesekontrolle testen.
- EXE unter Windows auf einem zweiten PC prüfen; README, Handbuch, Lizenz und Release Notes aktualisieren.
