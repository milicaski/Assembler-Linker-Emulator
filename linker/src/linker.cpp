#include "../inc/ImplLinker.h"
#include <stdio.h>
#include <string.h>

using namespace std;
extern int numFiles;
extern bool linkable;
extern int highestAdr;

int main(int argc, char* argv[]) {
    bool option = false;
    string outputFile;
    if(argc < 2){
        cout << "Not enough arguments" << endl;
        return -1;
    }else{
        for(int i = 1; i < argc; i++){
            string arg = argv[i];
            if(arg.compare("-o") == 0){
                outputFile = argv[i + 1];
                i++;
            }else if(arg.find("-") != 0){
                read(argv[i]);
            }else if(arg.compare("-linkable") == 0){
                linkable = true;
                option = true;
            }else if(arg.compare("-hex") == 0){
                linkable = false;
                option = true;
            }
        }
    }
   
    if(!option){
        return 0;
    }

    checkSymbolsAndFillSectionAdr();
 
    if(!linkable){
        for(int i = 1; i < argc; i++){
            string arg = argv[i];
            if(arg.find("-place=") != string::npos){
                string place = argv[i];
                int index =  place.find("@");
                string sec = place.substr(7, index - 7);
                string hex = place.substr(index + 1, place.length() - (index + 1));
                
                setNewStartAdrAndUpdateHighestAdr(sec, stoul(hex, nullptr, 16));//ovo se poziva za sve place opcije
            }
        }
        
        updateStartAdrForSections();
    }
    
    formSectionsInfo();
    formSymbolTable();
    fillContent();
  
    if(linkable){
        string txtFile = outputFile.substr(0, outputFile.length() - 2);
        printST(txtFile.append(".txt"));
        writeToFileLinkable(outputFile);
    }else{
        writeToFileHex(outputFile);
    }
}