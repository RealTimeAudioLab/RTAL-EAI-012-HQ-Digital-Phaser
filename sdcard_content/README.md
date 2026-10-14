# RTAL HQ Digital Phaser – 128 Sound Design Presets

128 eigenstaendig abgestimmte Presets in 16 Klangfamilien zu je 8 Sounds. Die Daten sind im nativen P014-V5-Format (41 Bytes, packed, FNV-1a-Pruefsumme). Keine Presets wurden von anderen Herstellern kopiert.

## Installation

1. Die bestehende SD-Karte und insbesondere den Ordner `/RTAL_PHASER` sichern.
2. Den Inhalt des enthaltenen Ordners `RTAL_PHASER` in den bestehenden SD-Ordner `/RTAL_PHASER` kopieren. Vorhandene PRESET*.DAT und NAME*.TXT werden dabei ersetzt. MIDI.CFG nicht loeschen.
3. Encoder > PRESETS > Browser, Programm 001–127 auswaehlen und LOAD.
4. MIDI Program Change 0 laedt Preset 001; PC 126 laedt Preset 127. PC 127 ist mit unveraenderter P014 nicht implementiert.

## Soundfamilien

- 001–008: **VINTAGE 6** – Soft, vintage six-stage modulation
- 009–016: **CLASSIC 12** – Deep twelve-stage sweeping
- 017–024: **STEREO MOTION** – True stereo spatial phasing
- 025–032: **SLOW AMBIENT** – Very slow ambient motion
- 033–040: **FUNK ENVELOPE** – Attack-driven positive envelope phasing
- 041–048: **REVERSE ENVELOPE** – Negative envelope sweep and percussive decay
- 049–056: **MIDI CLOCK** – Tempo-synchronized musical phasing
- 057–064: **ANALOG COLOR** – Analog character and warm drive
- 065–072: **HIGH RESONANCE** – Intense resonance with controlled drive
- 073–080: **FAST MODULATION** – High-speed LFO and metallic textures
- 081–088: **MATRIX SWEEP** – LFO-to-sweep matrix experiments
- 089–096: **MATRIX DYNAMICS** – Envelope-to-resonance/drive routing
- 097–104: **DUAL MATRIX** – Two simultaneously active modulation routes
- 105–112: **KEYS AND PADS** – Subtle musical presets for keys and synth pads
- 113–120: **GUITAR AND BASS** – Guitar/bass-focused phase character
- 121–128: **EXPERIMENTAL** – Extreme and unusual phaser configurations

## Hinweise zum Klang

- Stereo-Effekte benoetigen True Stereo und eine geeignete Stereo-Verkabelung.
- MIDI-CLOCK-Presets benoetigen fuer externe Synchronisation eine laufende MIDI Clock; sonst greift das interne Tempo.
- Envelope-Sounds reagieren auf Eingangspegel; Empfindlichkeit ggf. an die Quelle anpassen.
- Fast-Modulation und Resonance/Experimental-Sounds koennen laut oder schrill werden: mit geringem Abhoerpegel starten.
- Die Werte wurden technisch anhand der Firmware-Feldbereiche und Pruefsummen geprueft; eine klangliche Abhoerpruefung auf echter Hardware fand nicht statt.
- Die Bezeichnungen sind Sound-Design-Namen, keine Versprechen, dass ein bestimmtes Fremdgeraet exakt emuliert wird.

## Dateiformat

`PRESET01.DAT` ... `PRESET128.DAT`: je 41 Byte. `NAME01.TXT` ... `NAME128.TXT`: kurze Namen fuer OLED-Browser. Die Tabelle `PRESET_CATALOG.csv` enthaelt alle 128 Datensaetze und Parameter.
