# HQ Digital Phaser – Versionen und Umbenennung

## Historie und Unterscheidung

| Bereich | Bisherige Bezeichnung | Aktueller Dokumentationsname |
|---|---|---|
| Produkt | RTAL Phoenix Phaser / RTAL HQ Digital Phaser | HQ Digital Phaser |
| Frühere Firmware-Veröffentlichung | v1.0.0 FINAL / P015 FIX5 | Historischer Release-Stand |
| Editor-kompatible Firmware | P016 FIX4b | Weiterhin P016 FIX4b (intern unverändert) |
| Windows-Editor | V1.0 RC5 FIX2, später umbenannt | HQ Digital Phaser Editor |
| SD-Verzeichnis | `/RTAL_PHASER/` | **Unverändert** |
| Preset-Dateien | `PRESETxx.DAT`, `NAMExx.TXT` | **Unverändert** |
| MIDI SysEx | bisherige RTAL-Kennung | **Unverändert** |

## Warum interne Namen erhalten bleiben

Ein kosmetisches Umbenennen von Verzeichnissen oder Binärkennungen kann bestehende SD-Karten, Presets und die PC-Kommunikation beschädigen. Daher betrifft diese Dokumentationsaktualisierung Produkttexte, Titel und Dateinamen der Dokumente; sie migriert **keine** binären Daten oder Firmware-Pfade.

## Wichtiger Release-Hinweis

Das alte Firmware-Handbuch trägt „v1.0.0 FINAL“ und verweist auf P015 FIX5. Die Editor-Entwicklung ist separat bis RC5 fortgeschritten. Beim GitHub-Release immer explizit **Firmware-Build, Editor-Build, Soundbank-Version und Teststatus** nennen. Das Wort „FINAL“ im historischen Handbuch darf nicht als aktuelle Freigabe aller Komponenten verstanden werden.

## Geprüft vs. noch zu prüfen

**Aus der Projektentwicklung als erfolgreich gemeldet:** vollständiges 128-Slot-MIDI-Backup und Restore mit Verify, Einzel-Restore, PRO LIBRARIAN, RC4 und MIDI-Port-Memory. **Für diese Dokumentenüberarbeitung nicht erneut geprüft:** GitHub-Dateipfade, aktuell hochgeladene Firmware, Windows-EXE-Build und sämtliche 128 Presets akustisch.
