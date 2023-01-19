#include <stdio.h> 
#include <string.h> 
#include <curses.h> 
#include <stdlib.h> 

#include <fcntl.h> 
#include <errno.h> 
#include <termios.h> 
#include <unistd.h> 

#define ENTER 10
#define ESC 27
#define SPACE 32
#define KEY_S 115
#define BKSP 263

WINDOW *win;
FILE *data;

void drawTitle();
void drawPlotGridLines();
void printPlotNames();
void printSelectors();
void printHelp();
void drawStatusGridLines();
void printStatusNames();
int boundaryReached(int y, int x);
int printUserInput();
int checkKey(int key);
void saveData(char* dataStr);
void restoreData();
void createWindow();
void destroyWindow();

int main() {

  int i;
  int x = 12;
  int y = 18;
  int key, select;

  char userInput[17];
  userInput[16] = '\0';
  char dataStr[17];
  userInput[16] = '\0';
  char readBuffer[50];
  char * token;

  struct termios tty;

  //Serielle Schnittstelle oeffnen.
  int serial_port = open("/dev/ttyACM0", O_RDWR);

  //Vorhandene Einstellungen auslesen.
  if(tcgetattr(serial_port, &tty) != 0) {
      printf("Fehler %i von tcgetattr: %s\n", errno, strerror(errno));
      printf("micro:bit ist nicht verbunden.\n");
      return 1;
  }

  //Neue Einstellungen fuer serielle Schnittstelle festlegen.
  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;
  tty.c_cflag &= ~CRTSCTS;
  tty.c_cflag |= CREAD | CLOCAL;
  tty.c_lflag &= ~ECHO;
  tty.c_lflag &= ~ECHOE;
  tty.c_lflag &= ~ECHONL;
  tty.c_lflag &= ~ISIG;
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
  tty.c_oflag &= ~OPOST;
  tty.c_oflag &= ~ONLCR;
  tty.c_cc[VTIME] = 10;    
  tty.c_cc[VMIN] = 0;
  cfsetispeed(&tty, B115200);
  cfsetospeed(&tty, B115200);

  //Einstellungen speichern.
  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
      printf("Fehler %i von tcsetattr: %s\n", errno, strerror(errno));
      return 1;
  }

  //ncurses initialisieren.
  initscr();
  raw();
  curs_set(3);
  noecho();
  timeout(40);
  set_escdelay(0);
  keypad(stdscr, TRUE);

  //Farben definieren.
  start_color();
  init_color(COLOR_BLUE, 51, 204, 255);
  init_color(COLOR_BLACK, 208, 208, 225);
  init_pair(1, COLOR_WHITE, COLOR_BLUE);
  init_pair(2, COLOR_WHITE, COLOR_BLACK);
  bkgd(COLOR_PAIR(1));

  //UI zeichnen.
  box(stdscr, 0, 0);
  drawTitle();
  drawPlotGridLines();
  printPlotNames();
  printSelectors();
  printHelp();
  drawStatusGridLines();
  printStatusNames();

  //Gespeicherte Daten aus Datei einlesen
  restoreData();

  //Programmschleife. Programm kann mit ESC beendet werden.
  while(key != ESC){
    
    //Schleife fuer Cursor und Status.
    while(key != ENTER && key != ESC){
      
      /*Lesespeicher leeren und empfangene Werte von MicroBit einlesen.
        Teile des UI erneut zeichnen, da sie evtl ueberschrieben werden.*/
        memset(&readBuffer, '\0', sizeof(readBuffer));
        read(serial_port, &readBuffer, sizeof(readBuffer));
        box(stdscr, 0, 0);
        drawStatusGridLines();
        printHelp();
      
      //Gesendete Daten von MicroBit aufteilen und ausgeben.
      token = strtok(readBuffer, "!");
      while(token != NULL){
        for(i = 20; i < 57; i = i + 4){
          if(i < 49){
            mvprintw(i, 166, "%s    ", token);
            token = strtok(NULL, "!");
          } else {
            mvprintw(i, 166, "%s", token);
            token = strtok(NULL, "!");
          }
        }
      }
      refresh();
      move(y, x);
      
      //Tastendruck einlesen.
      key = getch();

      //Cursor in verschiedene Richtungen bewegen.
      switch(key){

        //Cursor nach oben bewegen.
        case KEY_UP:
          y = y - 2;
          if(boundaryReached(y, x) == 0){
            if(select == 0){
              y = y - 3;
              move(y, x);
              select++;
            } else {
              move(y, x);
              select--;
            }
          } else {
            y = y + 2;
          } 
          break;

        //Cursor nach unten bewegen.
        case KEY_DOWN:
          y = y + 2;
          if(boundaryReached(y, x) == 0){
            if(select == 0){
              move(y, x);
              select++;
            } else {
              y = y + 3;
              move(y, x);
              select--;
            }
          } else {
            y = y - 2;
          } 
          break;

        //Cursor nach links bewegen.
        case KEY_LEFT:
          x = x - 28;
          if(boundaryReached(y, x) == 0){
            move(y, x);
          } else {
            x = x + 28;
          }
          break;

        //Cursor nach rechts bewegen.
        case KEY_RIGHT:
          x = x + 28;
          if(boundaryReached(y, x) == 0){
            move(y, x);
          } else {
            x = x - 28;
          }
          break;

        //Daten der Plantage in Datei schreiben wenn s gedrueckt wird.
        case KEY_S:
        
          //Daten in Datei schreiben
          saveData(dataStr);

          //Neues Fenster erzeugen
          createWindow();

          //Mitteilung, dass Daten gespeichert wurden
          mvwprintw(win, 9, 36, "Daten wurden gespeichert.");
          wrefresh(win);
          sleep(2);

          //Fenster loeschen
          destroyWindow();
          move(y, x);
          break;
      }
    }

    //Wenn Cursor und Status Schleife mit Enter verlassen wurde hier weitermachen.
    if(key == ENTER){
      
      //Neues Fenster erzeugen.
      createWindow();

      //Auf Fenster schreiben. Abhaengig von Auswahl.
      if(select == 0){
        mvwprintw(win, 6, 6, "Bitte geben Sie den Pflanzennamen ein:");
        mvwprintw(win, 9, 6, "Maximal koennen 16 Zeichen eingegeben werden. "
                             "Es duerfen keine Umlaute verwendet werden.");
        mvwprintw(win, 12, 6, "Eingabe mit Enter bestaetigen.");
      } else {
        mvwprintw(win, 6, 6, "Bitte geben Sie die Pflanzenhoehe ein:");
        mvwprintw(win, 9, 6, "Maximal koennen 16 Zeichen eingegeben werden. "
                             "Es duerfen keine Umlaute verwendet werden.");
        mvwprintw(win, 12, 6, "Eingabe mit Enter bestaetigen.");
      }

      //Benutzereingabe einlesen und auf Fenster schreiben. Taste zuruecksetzen.
      key = printUserInput();

      //Benutzereingabe von Fenster lesen und auf Standardbildschirm schreiben.
      for(i = 0; i < 16; i++){
        userInput[i] = mvwinch(win, 6, 45 + i);
      }
      mvaddstr(y, x + 9, userInput);
      memset(&userInput, '\0', sizeof(userInput));
      move(y, x);

      //Fenster loeschen.
      destroyWindow();
    } 
  }
  //ncurses und Programm beenden.
  endwin();
  return 0; 
}

//Titel auf Standardbildschirm schreiben
void drawTitle(){

  //Offset initialisieren
  int offsetx = 50;
  int offsety = 1;

  //Grosse Buchstaben initialisieren
  chtype p[9][9] =
  {
    {ACS_ULCORNER, ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE, 
    ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_URCORNER},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ACS_VLINE},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ACS_VLINE},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ACS_VLINE},
    {ACS_LTEE, ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE, 
    ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_LRCORNER},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_LRCORNER, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
  };

  chtype l[9][9] =
  {
    {ACS_URCORNER, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_LLCORNER, ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE, 
    ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE},
  };

  chtype a[9][9] =
  {
    {ACS_ULCORNER, ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE, 
    ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_URCORNER},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ACS_VLINE},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ACS_VLINE},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ACS_VLINE},
    {ACS_LTEE, ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE, 
    ACS_HLINE, ACS_HLINE, ACS_RTEE},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ACS_VLINE},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ACS_VLINE},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ACS_VLINE},
    {ACS_LRCORNER, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ACS_LLCORNER},
  };

  chtype n[9][9] =
  {
    {ACS_URCORNER, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ACS_ULCORNER},
    {ACS_VLINE, '\\', ' ', ' ', ' ', ' ', ' ', ' ', ACS_VLINE},
    {ACS_VLINE, ' ', '\\', ' ', ' ', ' ', ' ', ' ', ACS_VLINE},
    {ACS_VLINE, ' ', ' ', '\\', ' ', ' ', ' ', ' ', ACS_VLINE},
    {ACS_VLINE, ' ', ' ', ' ', '\\', ' ', ' ', ' ', ACS_VLINE},
    {ACS_VLINE, ' ', ' ', ' ', ' ', '\\', ' ', ' ', ACS_VLINE},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', '\\', ' ', ACS_VLINE},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', '\\', ACS_VLINE},
    {ACS_LRCORNER, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ACS_LLCORNER},
  };

  chtype t[9][9] =
  {
    {ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_TTEE, 
    ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE},
    {' ', ' ', ' ', ' ', ACS_VLINE, ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ACS_VLINE, ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ACS_VLINE, ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ACS_VLINE, ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ACS_VLINE, ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ACS_VLINE, ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ACS_VLINE, ' ', ' ', ' ', ' '},
    {' ', ' ', ' ', ' ', ACS_BTEE, ' ', ' ', ' ', ' '},
  };

  chtype g[9][9] =
  {
    {ACS_ULCORNER, ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE, 
    ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ACS_HLINE, ACS_HLINE, ACS_HLINE, 
    ACS_HLINE, ACS_HLINE, ACS_URCORNER},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ACS_VLINE},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ACS_VLINE},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ACS_VLINE},
    {ACS_LLCORNER, ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE, 
    ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_LRCORNER},
  };

  chtype e[9][9] =
  {
    {ACS_ULCORNER, ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE, 
    ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE, 
    ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_VLINE, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
    {ACS_LLCORNER, ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE, 
    ACS_HLINE, ACS_HLINE, ACS_HLINE, ACS_HLINE},
  };

  //"Plantage" in gross schreiben
  for(int y = 0; y < 9; y++){
    for(int x = 0; x < 9; x++){
      mvaddch(y + offsety, x + offsetx, p[y][x]);
      offsetx = offsetx + 12;
      mvaddch(y + offsety, x + offsetx, l[y][x]);
      offsetx = offsetx + 12;
      mvaddch(y + offsety, x + offsetx, a[y][x]);
      offsetx = offsetx + 12;
      mvaddch(y + offsety, x + offsetx, n[y][x]);
      offsetx = offsetx + 12;
      mvaddch(y + offsety, x + offsetx, t[y][x]);
      offsetx = offsetx + 12;
      mvaddch(y + offsety, x + offsetx, a[y][x]);
      offsetx = offsetx + 12;
      mvaddch(y + offsety, x + offsetx, g[y][x]);
      offsetx = offsetx + 12;
      mvaddch(y + offsety, x + offsetx, e[y][x]);
      offsetx = 50;
    }
  }

  //Linie unter Titel zeichnen
  mvhline(11, 1, ACS_HLINE, 201);

  //Verbinder fuer Linie unter Titel zeichnen
  mvaddch(11, 0, ACS_LTEE);
  mvaddch(11, 202, ACS_RTEE);

}

//Linien fuer Pflanzenfelder zeichnen
void drawPlotGridLines(){

  int x, y;

  //Horiontale Linien fuer Pflanzenfelder zeichnen
  for(y = 15; y < 51; y = y + 7){
    mvhline(y, 11, ACS_HLINE, 139);
  }

  //Vertikale Linien fuer Pflanzenfelder zeichnen
  for(x = 10; x < 151; x = x + 28){
    mvvline(16, x, ACS_VLINE, 34);
  }

  //Ecken fuer Pflanzenfelder zeichnen
  mvaddch(15, 10, ACS_ULCORNER);
  mvaddch(15, 150, ACS_URCORNER);
  mvaddch(50, 10, ACS_LLCORNER);
  mvaddch(50, 150, ACS_LRCORNER);

  //Verbinder fuer Pflanzenfelder zeichnen
  for(x = 38; x < 123; x = x + 28){
    mvaddch(15, x, ACS_TTEE);
  }

  for(x = 38; x < 123; x = x + 28){
    mvaddch(50, x, ACS_BTEE);
  }

  for(y = 22; y < 44; y = y + 7){
    mvaddch(y, 10, ACS_LTEE);
  }

  for(y = 22; y < 44; y = y + 7){
    mvaddch(y, 150, ACS_RTEE);
  }

  for(y = 22; y < 44; y = y + 7){
    for(x = 38; x < 123; x = x + 28){
      mvaddch(y, x, ACS_PLUS);
    }
  }
}


//Namen fuer Pflanzenfelder schreiben
void printPlotNames(){

  int plotIndex = 1;

  for(int y = 16; y < 51; y = y + 7){
    for(int x = 12; x < 152; x = x + 28){
      mvprintw(y, x, "Feld %d", plotIndex);
      plotIndex++;
    }
  } 
}

//Auswahloption in Pflanzenfelder schreiben
void printSelectors(){

  for(int y = 18; y < 53; y = y + 7){
    for(int x = 12; x < 152; x = x + 28){
      mvprintw(y, x, "> Name :");
      mvprintw(y + 2, x, "> Hoehe:");
    }
  }
}

//An unterem Bildschirmrand schreiben was jede Taste macht.
void printHelp(){

  int x = 13;
  int y = 52;

  mvaddch(y, x, ACS_LARROW);
  x = x + 2;
  mvaddch(y, x, ACS_RARROW);
  x = x + 2;
  mvaddch(y, x, ACS_UARROW);
  x = x + 2;
  mvaddch(y, x, ACS_DARROW);
  x = x + 2;
  mvprintw(y, x, "= Navigation");
  x = x + 44;
  mvprintw(y, x, "Enter = Auswahl/Bestaetigen");
  x = x + 59;
  mvprintw(y, x, "S = Speichern");
  x = x + 43;
  mvprintw(y, x, "ESC = Beenden");
}

//Linien fuer Statusfeld zeichnen
void drawStatusGridLines(){

  int x, y;

  //Horiontale Linien fuer Statusfeld zeichnen
  mvhline(15, 165, ACS_HLINE, 21);
  for(y = 18; y < 51; y = y + 4){
    mvhline(y, 165, ACS_HLINE, 21);
  }

  //Vertikale Linien fuer Statusfeld zeichnen
  for(x = 164; x < 187; x = x + 22){
    mvvline(16, x, ACS_VLINE, 34);
  }

  //Ecken fuer Statusfeld zeichnen
  mvaddch(15, 164, ACS_ULCORNER);
  mvaddch(15, 186, ACS_URCORNER);
  mvaddch(50, 164, ACS_LLCORNER);
  mvaddch(50, 186, ACS_LRCORNER);

  //Verbinder fuer Statusfeld zeichnen
  for(y = 18; y < 47; y = y + 4){
    mvaddch(y, 164, ACS_LTEE);
  }

  for(y = 18; y < 47; y = y + 4){
    mvaddch(y, 186, ACS_RTEE);
  }

}

//Namen in Statusfelder schreiben
void printStatusNames(){

  int x = 166;
  int y = 16;

  mvprintw(y, x, "Status der Plantage");
  y = y + 3;
  mvprintw(y, x, "Lichtstaerke:");
  y = y + 4;
  mvprintw(y, x, "Deckenlamellen:");
  y = y + 4;
  mvprintw(y, x, "Licht:");
  y = y + 4;
  mvprintw(y, x, "Temperatur:");
  y = y + 4;
  mvprintw(y, x, "Ventilator:");
  y = y + 4;
  mvprintw(y, x, "Heizung:");
  y = y + 4;
  mvprintw(y, x, "Feuchtigkeit:");
  y = y + 4;
  mvprintw(y, x, "Bewaesserung:");
}

//Ueberpruefen ob Cursor den Rand erreicht hat.
int boundaryReached(int y, int x){

  int boundaryUpper = 18;
  int boundaryLower = 49;
  int boundaryLeft = 12;
  int boundaryRight = 124;

  if(x < boundaryLeft || x > boundaryRight || 
     y < boundaryUpper || y > boundaryLower){
    return 1;
  } else {
    return 0;
  }
}

//Benutzereingabe einlesen und auf Fenster schreiben.
int printUserInput(){

  int x = 45;
  int y = 6;
  int key;

  //Cursor setzen
  wmove(win, y, x);
  wrefresh(win);

  //Schreibe oder loesche Benutzereingabe.
  while(key != ENTER){
    key = getch();
    if(checkKey(key) == 1 && x < 61){
      wprintw(win, "%c", key);
      wrefresh(win);
      x++;
    }
    if(key == BKSP && x > 45){
      x--;
      mvwprintw(win, y, x, " ");
      wmove(win, y, x);
      wrefresh(win);
    }
  }
  return 0;
}

//Tasteneingabe ueberpruefen.
int checkKey(int key){

  //Wenn Taste Buchstabe, Nummer, Leerzeichen oder ".,-/" : return 1 else: return 0
  if((key > 64 && key < 91) || (key > 96 && key < 123) || 
  (key > 47 && key < 58) || key == SPACE || (key > 43 && key < 48)){
    return 1;
  } else {
    return 0;
  }

}

//Daten der Pflanzenfelder in Textdatei schreiben.
void saveData(char* dataStr){
       
  //Datei oeffnen
  data = fopen("/home/daniel/Dokumente/Programme/Plantage/data.txt", "w");
  if(data == NULL){
    printw("Fehler beim oeffnen der Datei");
  }

  //Daten von Pflanzenfeldern einlesen und in Datei schreiben.
  for(int i = 0; i < 29; i = i + 7){
    for(int j = 0; j < 113; j = j + 28){
      for(int k = 0; k < 3; k = k + 2){
        for(int l = 0; l < 16; l++){
          dataStr[l] = mvwinch(stdscr, 18 + i + k, 21 + j + l);
        }
        fprintf(data, "%s\n", dataStr);
      }
    }
  }
  
  fclose(data);
}

//Daten aus Textdatei lesen und in Pflanzenfelder schreiben.
void restoreData(){

  int character;

  //Datei oeffnen
  data = fopen("/home/daniel/Dokumente/Programme/Plantage/data.txt", "r");
  if(data == NULL){
    printw("Fehler beim oeffnen der Datei");
  }

  //Daten von Datei auslesen und in Pflanzenfelder schreiben.
  for(int i = 0; i < 29; i = i + 7){
    for(int j = 0; j < 113; j = j + 28){
      for(int k = 0; k < 3; k = k + 2){
        for(int l = 0; l < 16; l++){
          character = getc(data);
          if(character != 10){
            mvaddch(18 + i + k, 21 + j + l, character);
          } else {
            l--;
          }
        }
      }
    }
  }

  fclose(data);
}

//Fenster erzeugen
void createWindow(){

  win = newwin(20, 100, 20, 30);
  wbkgd(win, COLOR_PAIR(2));
  box(win, 0, 0);
  wrefresh(win);
}

//Fenster loeschen
void destroyWindow(){

  delwin(win);
  redrawwin(stdscr);
  refresh();
}