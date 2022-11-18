//#include <iostream>
#include <stdlib.h>
#include <iterator>
#include <fstream>

#include "../inc/Impl.h"

//using namespace std;
extern int locCnt;
extern int yylineno;

vector<Symbol> symTable;
vector<firstPassRecord> firstPassRelTab;

void printST(string file){
    ofstream txt;
    txt.open(file);

    for(vector<Symbol>::iterator it = symTable.begin(); it != symTable.end(); ++it){
       txt << it->index << "\t" << it->name << "\t" << it->secName << "\t" << it->section << "\t" << it->ofset << "\t" << it->size << "\t" << it->type << endl;
    }
    txt << endl;
    /*cout<<endl;
    for(vector<firstPassRecord>::iterator it = firstPassRelTab.begin(); it != firstPassRelTab.end(); ++it){
       cout<<it->symbol<<" "<<it->section<<" "<<it->ofset<<" "<<it->type<<endl;
    }*/
    //cout<<endl;
    for(vector<Symbol>::iterator it = symTable.begin(); it != symTable.end(); ++it){
       if(it->type == 4){
           txt << "Relocation records for section " << it->name << endl;
           for(vector<RelRecord>::iterator it2 = it->relRecords.begin(); it2 != it->relRecords.end(); ++it2){
               txt << it2->ofset << "\t" << it2->type << "\t" << it2->symbolIndex << "\t" << it2->symbolName << endl;
           }
           txt << endl;
       }
    }

    for(vector<Symbol>::iterator it = symTable.begin(); it != symTable.end(); ++it){
       if(it->type == 4){
           txt << "Content for section " << it->name << endl;
           for(int i = 0; i < it->size; i++){
               txt << hex << ((it->content[i] >> 4) & 0xf) << (it->content[i] & 0xf) << " ";
           }  
           txt << endl;
       }
    }
    txt.close();
}

int addSection(string name){
    for(vector<Symbol>::iterator it = symTable.begin(); it != symTable.end(); ++it){
        if(it->name.compare(name) == 0){
            cout << "1.Multiple definition of a symbol " << it->name << " at line " << yylineno << endl;
            exit(-1);
        }
    }
    Symbol s;
    s.index = ind++;
    s.name = name;
    s.secName = name;
    s.section =  s.index;
    s.ofset = 0;
    s.size = 0;
    s.type = Type::sec;
    symTable.push_back(s);
    return s.index;
}

void setSize(int section, int size){
   for(vector<Symbol>::iterator it = symTable.begin(); it != symTable.end(); ++it){
       if(it->index == section){
           it->size =  size;
           return;
       }
   }
}

void addSymbol(string name, int section, unsigned short ofset){//ako je dva puta def isti simbol greska
   // printST();
    for(vector<Symbol>::iterator it = symTable.begin(); it != symTable.end(); ++it){
        if(it->name.compare(name) == 0 ){
            if(it->type == Type::global){
                it->secName = symTable[section - 1].name;
                it->section = section;
                it->ofset = ofset;
                return;
            }else{
                cout << "2.Multiple definition of a symbol " << it->name << " at line " << yylineno << endl;
                exit(-1);
            }
        }
    }
    Symbol s;
    s.index = ind++;
    s.name = name;
    s.secName = symTable[section - 1].name;
    s.section =  section;
    s.ofset = ofset;
    s.size = 0;
    s.type = Type::local;
    symTable.push_back(s);
}

void setAsGlobal(string name){
    //printST();
    for(vector<Symbol>::iterator it = symTable.begin(); it != symTable.end(); ++it){
        if(it->name.compare(name) == 0){
            it->type =  Type::global;
            return;
        }
    }
    Symbol s;
    s.index = ind++;
    s.name = name;
    s.secName = "";
    s.section =  0;
    s.ofset = 0;
    s.size = 0;
    s.type = Type::global;
    symTable.push_back(s);
}

void setAsExtern(string name){//podrazumevam da necu da definisem simbol i stavim da je extern, znaci sig ga nema u sim tab
    Symbol s;
    s.index = ind++;
    s.name = name;
    s.secName = "";
    s.section =  0;
    s.ofset = 0;
    s.size = 0;
    s.type = Type::und;
    symTable.push_back(s);
}

void addEquSymbol(string name, unsigned short v){
     for(vector<Symbol>::iterator it = symTable.begin(); it != symTable.end(); ++it){
        if(it->name.compare(name) == 0){
            cout << "3.Multiple definition of a symbol " << it->name << " at line " << yylineno << endl;
            exit(-1); 
        }
    }
    Symbol s;
    s.index = ind++;
    s.name = name;
    s.secName = "";
    s.section = 0;
    s.ofset = v;
    s.size = 0;
    s.type = Type::absolut;
    symTable.push_back(s);
}

void fillContent(int section, char byte){//uvek puni otpozadi sadrzaj to znaci da uvek mora da se pozove (nebitno dal moze odmah da se prevede ili ne) i da se makar nulom popuni
    symTable[section - 1].content.push_back(byte);
}

void fillTwoAdr(int section, char firstByte, int rd, int rs){
    fillContent(section, firstByte);
    locCnt++;
    fillContent(section, ((rd << 4) & 0xf0) | (rs & 0x0f));
    locCnt++;
}

void changeInstrDesc(int section, int instrDesc, char byte){
    symTable[section - 1].content[instrDesc] = byte;
}

void addRecord(int section, int ofset, string symbol, RelType type){
    firstPassRecord fps;
    fps.section = section;
    fps.ofset = ofset;
    fps.symbol = symbol;
    fps.type = type;
    firstPassRelTab.push_back(fps);
}

void makeRelRecord(int section, int ofset, RelType type, int symbol, string symbolName){
    RelRecord record;
    record.ofset = ofset;
    record.type = type;
    record.symbolIndex = symbol;
    record.symbolName = symbolName;
    symTable[section - 1].relRecords.push_back(record);
}
void secondPass(){
    for(vector<firstPassRecord>::iterator it = firstPassRelTab.begin(); it != firstPassRelTab.end(); ++it){
        bool found = false;
        for(vector<Symbol>::iterator iter = symTable.begin(); iter != symTable.end(); ++iter){
            if(iter->name.compare(it->symbol) == 0){
                found = true;
                switch (it->type){
                    case RelType::dataAbs: {
                        if(iter->type == Type::absolut){
                            symTable[it->section - 1].content[it->ofset] = iter->ofset & 0xff;
                            symTable[it->section - 1].content[it->ofset + 1] = (iter->ofset >> 8) & 0xff;
                        }else if(iter->type == Type::global){
                            makeRelRecord(it->section, it->ofset, it->type, iter->index, iter->name);
                        }else if(iter->type == Type::local){
                            symTable[it->section - 1].content[it->ofset] = iter->ofset & 0xff;
                            symTable[it->section - 1].content[it->ofset + 1] = (iter->ofset >> 8) & 0xff;
                            //stvor rel zapis za sekciju u kojoj je simbol def
                            makeRelRecord(it->section, it->ofset, it->type, iter->section, symTable[iter->section - 1].name);//pppppp
                        }else if(iter->type == Type::sec){
                            //stvor rel zapis za tu sekciju(ne znam dal ovo uopste sme)
                            makeRelRecord(it->section, it->ofset, it->type, iter->index, iter->name);
                        }else if(iter->type == Type::und){
                            //stvori rel zapis za ovaj simbol
                            makeRelRecord(it->section, it->ofset, it->type, iter->index, iter->name);
                        }
                        break;
                    }
                    case RelType::instrAbs: {
                        if(iter->type == Type::absolut){
                            symTable[it->section - 1].content[it->ofset] = (iter->ofset >> 8) & 0xff;
                            symTable[it->section - 1].content[it->ofset + 1] = iter->ofset & 0xff;
                        }else if(iter->type == Type::global){
                            //stvori rel zapis za taj simbol
                            makeRelRecord(it->section, it->ofset, it->type, iter->index, iter->name);
                        }else if(iter->type == Type::local){
                            symTable[it->section - 1].content[it->ofset] = (iter->ofset >> 8) & 0xff;
                            symTable[it->section - 1].content[it->ofset + 1] = iter->ofset & 0xff;
                            //stvor rel zapis za sekciju u kojoj je simbol def
                            makeRelRecord(it->section, it->ofset, it->type, iter->section, symTable[iter->section - 1].name);//pppp
                        }else if(iter->type == Type::sec){
                            //stvor rel zapis za tu sekciju(ne znam dal ovo uopste sme)
                            makeRelRecord(it->section, it->ofset, it->type, iter->index, iter->name);
                        }else if(iter->type == Type::und){
                            //stvori rel zapis za ovaj simbol
                            makeRelRecord(it->section, it->ofset, it->type, iter->index, iter->name);
                        }
                        break;
                    }
                    case RelType::instrRel: {//ako je u istoj sek provera
                        if(iter->type == Type::absolut){
                            cout << "Syntax error - symbol used wrongly at line " << yylineno << endl; 
                            exit(-1);
                        }else if(iter->type == Type::global){
                            symTable[it->section - 1].content[it->ofset + 1] = 0xfe;
                            //stvori rel zapis za taj simbol
                            makeRelRecord(it->section, it->ofset, it->type, iter->index, iter->name);
                        }else if(iter->type == Type::local){
                            if(iter->section == it->section){
                                int diff = iter->ofset - (it->ofset + 2);
                                symTable[it->section - 1].content[it->ofset] = (diff >> 8) & 0xff;
                                symTable[it->section - 1].content[it->ofset + 1] = diff & 0xff;
                            }else{
                                symTable[it->section - 1].content[it->ofset] = (iter->ofset >> 8) & 0xff;
                                symTable[it->section - 1].content[it->ofset + 1] = (iter->ofset & 0xff) + 0xfe;
                                //stvor rel zapis za sekciju u kojoj je simbol def
                                makeRelRecord(it->section, it->ofset, it->type, iter->section, symTable[iter->section - 1].name);//ppp
                            }
                            
                        }else if(iter->type == Type::sec){
                            symTable[it->section - 1].content[it->ofset + 1] = 0xfe;
                            //stvor rel zapis za tu sekciju(ne znam dal ovo uopste sme)
                            makeRelRecord(it->section, it->ofset, it->type, iter->index, iter->name);
                        }else if(iter->type == Type::und){
                            symTable[it->section - 1].content[it->ofset + 1] = 0xfe;
                            //stvori rel zapis za ovaj simbol
                            makeRelRecord(it->section, it->ofset, it->type, iter->index, iter->name);
                        }
                        break;
                    }
                }
                break;
            }
        } 
        if(!found){
            cout << "Undefined symbol used at line " << yylineno << endl;
            exit(-1);
        }
     
    }   
}

void writeToFile(string file){
    ofstream wf(file, ios::out | ios::binary);
    //int numSymbols = symTable.size();
    int numSymbols = 0;
    for(int i = 0; i < symTable.size(); i++){
        if(symTable[i].type != Type::absolut && symTable[i].type != Type::local){
            numSymbols++;
        }
    }
    wf.write((char *) &numSymbols, sizeof(numSymbols));
    for(vector<Symbol>::iterator it = symTable.begin(); it != symTable.end(); ++it){
        if(it->type == Type::absolut || it->type == Type::local){
            continue;
        }
        int index = it->index;
        wf.write((char *) &index, sizeof(index));

        int len = it->name.length();
        wf.write((char *) &len, sizeof(len));
        for(int i = 0; i < len; i++) {
            char c = it->name[i];
            wf.write((char *) &c, sizeof(c));
        }

        int secLen = it->secName.length();
        wf.write((char *) &secLen, sizeof(secLen));
        for(int i = 0; i < secLen; i++) {
            char c = it->secName[i];
            wf.write((char *) &c, sizeof(c));
        }

        int sec = it->section;
        wf.write((char *) &sec, sizeof(sec));

        unsigned short ofset = it->ofset;
        wf.write((char *) &ofset, sizeof(ofset));

        int size = it->size;
        wf.write((char *) &size, sizeof(size));

        int contLen = it->content.size();
        wf.write((char *) &contLen, sizeof(contLen));
        for(int i = 0; i < contLen; i++){
            char c = it->content[i];
            wf.write((char *) &c, sizeof(c));
        }

        int relLen = it->relRecords.size();
        wf.write((char *) &relLen, sizeof(relLen));
        for(int i = 0; i < relLen; i++){
            int relOf = it->relRecords[i].ofset;
            wf.write((char *) &relOf, sizeof(relOf));

            RelType type = it->relRecords[i].type;
            wf.write((char *) &type, sizeof(type));

            int symInd = it->relRecords[i].symbolIndex;
            wf.write((char *) &symInd, sizeof(symInd));

            int symLen = it->relRecords[i].symbolName.length();
            wf.write((char *) &symLen, sizeof(symLen));
            for(int j = 0; j < symLen; j++) {
                char c = it->relRecords[i].symbolName[j];
                wf.write((char *) &c, sizeof(c));
            }
        }

        Type type = it->type;
        wf.write((char *) &type, sizeof(type));
    }

    wf.close();
}