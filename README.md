# Plantage
Projekt von Denis und Daniel.

## Allgemeines
Das Projekt besteht aus zwei Programmen.
Es gibt ein Programm für den MicroBit und ein PC Programm.


## Anleitung MicroBit
### Allgemeines
Das Programm `MicroBit.hex` kann direkt heruntergeladen und auf den MicroBit übertragen werden.
Der MicroBit arbeitet unabhängig vom PC Programm.
Die erforderliche Hardware ist im Wiki hinterlegt.

### Funktionen
Der MircroBit führt folgende Funktionen selbstständig aus:

* Je nach gemessener Lichtstärke werden die grüne und gelbe LED gedimmt.  
* Je nach gemessener Temperatur wird die rote LED gedimmt und die Ventilatorgeschwindigkeit gesteuert.  
* Je nach gemessener Feuchtigkeit wird die blaue LED ein- und ausgeschaltet.  
* Die gemessenen Werte, der jeweilige Status der LEDs und des Ventilators werden via USB an den PC übertragen.  

Die genauen Grenzwerte können den Projektanforderungen im Wiki entnommen werden.

## Anleitung Programm Plantage
### Allgemeines
Das Programm `Plantage` läuft in einem Linux Terminal.  
Die Voraussetzungen, die erfüllt sein müssen, damit das Programm läuft, befinden sich im Wiki.

* Das Programm zeigt die einzelnen Felder der Plantage an.  
* Für jedes Feld kann der jeweilige Pflanzenname und die Pflanzenhöhe hinterlegt werden.  
* Auf der rechten Seite werden die Daten, die vom MicroBit übertragen werden, angezeigt.

### Bedienung
* Mit den `Pfeiltasten` kann durch die einzelnen Felder navigiert werden.  
* Mit `Enter` öffnet sich das Eingabefenster des jeweiligen Feldes.  
Nach Eingabe der gewünschten Daten kann mit `Enter` das Eingabefeld wieder geschlossen werden.  
* Durch Drücken von `s` können die eingegebenen Daten gespeichert werden.  
Diese stehen nach dem nächsten Programmstart wieder zur Verfügung.  
* Mit der Taste `Esc` wird das Programm beendet.
