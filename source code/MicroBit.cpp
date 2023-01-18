#include "MicroBit.h"

MicroBit uBit;

void shutter (int value);
void light (int value);
void ventilator (int value);
void heating (int value);
void irrigation (int value);

int main() {

    //MicroBit initialisieren
    uBit.init();

    //Display ausschalten um alle Analogpins benutzen zu koennen.
    uBit.display.disable();

    //Deckenlamellen, gruene LED
    uBit.io.P0.isInput();
    uBit.io.P0.isAnalog();

    //Ventilator
    uBit.io.P1.isOutput();
    uBit.io.P1.isAnalog();

    //Feuchtigkeitssensor
    uBit.io.P2.isInput();
    uBit.io.P2.isAnalog();

    //Lichtsensor
    uBit.io.P3.isOutput();
    uBit.io.P3.isAnalog();

    //Licht, gelbe LED
    uBit.io.P4.isOutput();
    uBit.io.P4.isAnalog();

    //Heizung, rote LED
    uBit.io.P10.isOutput();
    uBit.io.P10.isAnalog();

    //Bewaesserung, blaue LED
    uBit.io.P12.isOutput();
    uBit.io.P12.isDigital();
    
    //Werte fuer Deckenlamellen und Licht initialisieren
    int lightIntensity = 0;
    int lightLimitMax = 816;
    int lightLimitHigh = 612;
    int lightLimitLow = 408;
    int lightLimitMin = 204;
    int shutterClosed = 1023;
    int shutterHalf = 256;
    int shutterOpen = 0;
    int lightOn = 1023;
    int lightHalf = 256;
    int lightOff = 0;
    
    //Werte fuer Ventilator und Heizung initialisieren
    int temperature = 0;
    int tempLimitMax = 30;
    int tempLimitHigh = 20;
    int tempLimitLow = 10;
    int tempLimitMin = 0;
    int heatingFull = 1023;
    int heatingHalf = 512;
    int heatingOff = 0;
    int ventilatorFull = 1023;
    int ventilatorHalf = 512;
    int ventilatorOff = 0;

    //Werte fuer Bewaesserung initialisieren
    int moisture = 0;
    int moisture60 = 613;
    int moisture80 = 650;

    //Zaehler und Zeitstempel initialisieren
    int i = 0;
    unsigned long timestamp = 0;

    //String fuer Status der einzelnen Funktionen initialisieren
    ManagedString status("");

    //Endlosschleife fuer Programmablauf
    while(1){

        //Status string zuruecksetzen
        status = "";

        //Lichstaerke messen und Status string aktualisieren
        lightIntensity = uBit.io.P3.getAnalogValue();
        status = status + lightIntensity + "!";
        //Abhaengig der Lichtstaerke Deckenlamellen und Licht steuern
        //Je nach Zustand Status string aktualisieren
        if(lightIntensity > lightLimitMax){
            shutter(shutterClosed);
            light(lightOff);
            status = status + "100%!0%!";
        } else if(lightIntensity <= lightLimitMax && lightIntensity > lightLimitHigh){
            shutter(shutterHalf);
            light(lightOff);
            status = status + "50%!0%!";
        } else if(lightIntensity < lightLimitLow && lightIntensity >= lightLimitMin){
            shutter(shutterOpen);
            light(lightHalf);
            status = status + "0%!50%!";
        } else if(lightIntensity < lightLimitMin){
            shutter(shutterOpen);
            light(lightOn);
            status = status + "0%!100%!";
        } else {
            shutter(shutterOpen);
            light(lightOff);
            status = status + "0%!0%!";
        }

        //Temperatur messen und Status string aktualisieren
        temperature = uBit.thermometer.getTemperature();
        status = status + temperature + "C!";
        //Abhaengig der Temperatur Ventilator und Heizung steuern
        //Je nach Zustand Status string aktualisieren
        if(temperature > tempLimitMax){
            ventilator(ventilatorFull);
            heating(heatingOff);
            status = status + "100%!0%!";
        } else if(temperature <= tempLimitMax && temperature > tempLimitHigh){
            ventilator(ventilatorHalf);
            heating(heatingOff);
            status = status + "50%!0%!";
        } else if(temperature < tempLimitLow && temperature >= tempLimitMin){
            ventilator(ventilatorOff);
            heating(heatingHalf);
            status = status + "0%!50%!";
        } else if(temperature < tempLimitMin){
            ventilator(ventilatorOff);
            heating(heatingFull);
            status = status + "0%!100%!";
        } else {
            ventilator(ventilatorOff);
            heating(heatingOff);
            status = status + "0%!0%!";
        }

        //Feuchtigkeit messen und Status string aktualisieren
        moisture = uBit.io.P2.getAnalogValue();
        status = status + moisture + "!";
        //Wenn Feuchtigkeit zwischen 60% und 79% Bewaesserung einschalten.
        //Zeitstempel setzen und Zaehler auf 1 fuer Fallunterscheidung.
        if(moisture >= moisture60 && moisture < moisture80 && i == 0){
            irrigation(1);
            timestamp = uBit.systemTime();
            i = 1;
        } 
        //Wenn Feuchtigkeit kleiner 60% Bewaesserung einschalten.
        //Zeitstempel setzen und Zaehler auf 2 fuer Fallunterscheidung.
        if(moisture < moisture60 && i == 0){
            irrigation(1);
            timestamp = uBit.systemTime();
            i = 2;
        }
        //Wenn Fall 1 aktiv und mind. 2 Sek. vergangen sind Bewaesserung ausschalten
        if(i == 1 && uBit.systemTime() - timestamp >= 2000){
            irrigation(0);
            i = 0;
        }
        //Wenn Fall 2 aktiv und mind. 4 Sek. vergangen sind Bewaesserung ausschalten
        if(i == 2 && uBit.systemTime() - timestamp >= 4000){
            irrigation(0);
            i = 0;
        }
        //Zustand der Bewaesserung in Status string aktualisieren
        if(i > 0){
            status = status + "Ein!";
        } else {
            status = status + "Aus!";
        }

        //Ende fuer Status string signalisieren und an PC schicken.
        status = status + " !\n";
        uBit.serial.send(status);
        uBit.sleep(50);
    }
}

//Deckenlamellen schalten
void shutter (int value){
    uBit.io.P0.setAnalogValue(value);
}

//Licht schalten
void light (int value){
    uBit.io.P4.setAnalogValue(value);
}

//Ventilator schalten
void ventilator (int value){
    uBit.io.P1.setAnalogValue(value);
}

//Heizung schalten
void heating (int value){
    uBit.io.P10.setAnalogValue(value);
}

//Bewaesserung schalten
void irrigation (int value){
    uBit.io.P12.setDigitalValue(value);
}