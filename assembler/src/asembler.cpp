//#include <iostream>
#include "../inc/Impl.h"
//using namespace std;
extern int yylex();
extern int yylineno;
extern char* yytext;
extern FILE *yyin;
extern int yyparse();
//extern void printST();
//extern void secondPass();

int main(int argc, char* argv[]){
    string outputFile = "output.o";
    char* inputFile;
    if(argc < 2){
        cout << "Input file name required" << endl;
        return -1;
    }else if(argc == 3){
        cout << "Error: not enough arguments" << endl;
        return -1;
    }else if(argc == 4){
        inputFile = argv[3];
        outputFile = argv[2];
    }else{
        inputFile = argv[1];
    }

    FILE *f = fopen(inputFile,"r");
    yyin = f;
   
    yyparse();
    fclose(f);

    secondPass();
    
    string txtFile = outputFile.substr(0, outputFile.length() - 2);

    printST(txtFile.append(".txt"));
  
    writeToFile(outputFile);
    return 0;
}