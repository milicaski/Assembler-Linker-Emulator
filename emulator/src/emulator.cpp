#include "../inc/ImplEmulator.h"
#include <unistd.h>
#include <termios.h>

int main(int argc, char* argv[]){
   if(argc < 2){
      cout << "Not enough arguments" << endl;
      return -1;
   }
   static struct termios oldtio, newtio;
   tcgetattr(0, &oldtio);
   newtio = oldtio;
   newtio.c_lflag &= ~ICANON;
   newtio.c_lflag &= ~ECHO;
   newtio.c_cc[VTIME] = 0;
   newtio.c_cc[VMIN] = 0;
   tcsetattr(0, TCSANOW, &newtio);

   string file = argv[1];
   readFile(file);
   setPcSp();
   emulate();
  
   tcsetattr(0, TCSANOW, &oldtio);
}