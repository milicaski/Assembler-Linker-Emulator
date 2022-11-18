#include "../inc/ImplLinker.h"
#include <iostream>
#include <fstream>
#include <map>
#include <list>

bool linkable;
int highestAdr = 0;
map<string, vector<Symbol>> symTables;
vector<SectionInfo> sectionsFromFilesInfo;
vector<SectionAdr> sectionsAdr;
vector<Symbol> symTable;

void writeToFileHex(string file){
    ofstream wf(file, ios::out | ios::binary);

    int num = 0;
    for(int i = 0; i < symTable.size(); i++){
        if(symTable[i].type == Type::sec && symTable[i].size > 0){
            num++;
        }
    }
    wf.write((char *) &num, sizeof(num));
    for(int i = 0; i < symTable.size(); i++){
        if(symTable[i].type == Type::sec && symTable[i].size > 0){
            int size = symTable[i].size;
            wf.write((char *) &size, sizeof(size));

            unsigned short start = symTable[i].ofset;
            wf.write((char *) &start, sizeof(start));

            for(int j = 0; j < size; j++){
                char c = symTable[i].content[j];
                wf.write((char *) &c, sizeof(c));
            }
        }
    }
    wf.close();
}

void writeToFileLinkable(string file){
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

void printST(string file){
    /*cout<<"Nove vred sekcija"<<endl;
    for(int i = 0;i<sectionsFromFilesInfo.size(); i++){
        cout<<sectionsFromFilesInfo[i].name <<" "<<sectionsFromFilesInfo[i].file <<" "<< sectionsFromFilesInfo[i].newValue<<endl;
    }

    cout<<"Nova tabela simbola"<<endl;
    for(int i =0;i<symTable.size();i++){
        cout<<symTable[i].index<<" "<<symTable[i].name<<" "<<symTable[i].secName<<" "<<symTable[i].section<<" "<<symTable[i].ofset<<" "<<symTable[i].size<<" "<<symTable[i].type<<endl;
    }
    for(vector<Symbol>::iterator it = symTable.begin(); it != symTable.end(); ++it){
       if(it->type == 4){
           for(int i=0;i<it->size;i++){
               printf("%x%x ", (it->content[i] >> 4) & 0xf, it->content[i] & 0xf);
           }  
           cout<<endl;
       }
    }
    cout<<endl;
    for(vector<Symbol>::iterator it = symTable.begin(); it != symTable.end(); ++it){
       if(it->type == 4){
           for(int i = 0; i < it->relRecords.size(); i++){
               cout<<it->relRecords[i].ofset<<" "<<it->relRecords[i].type<<" "<<it->relRecords[i].symbolIndex<<" "<<it->relRecords[i].symbolName<<endl;
           }
           cout<<endl;
       }
    }*/
    ofstream txt;
    txt.open(file);

    for(vector<Symbol>::iterator it = symTable.begin(); it != symTable.end(); ++it){
       txt << it->index << "\t" << it->name << "\t" << it->secName << "\t" << it->section << "\t" << it->ofset << "\t" << it->size << "\t" << it->type << endl;
    }
    txt << endl;

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

void read(string file){
    ifstream rf(file, ios::out | ios::binary);
    vector<Symbol> symTable;

    int numSymbols;
    rf.read((char *) &numSymbols, sizeof(numSymbols));
    for(int i = 0; i < numSymbols; i++){
        Symbol symbol;

        int index;
        rf.read((char *) &index, sizeof(index));
        symbol.index = index;

        int len;
        rf.read((char *) &len, sizeof(len));
        string name;
        for(int i = 0; i < len; i++) {
            char c;
            rf.read((char *) &c, sizeof(c));
            name.push_back(c);
        }
        symbol.name = name;

        int secLen;
        rf.read((char *) &secLen, sizeof(secLen));
        string secName;
        for(int i = 0; i < secLen; i++) {
            char c;
            rf.read((char *) &c, sizeof(c));
            secName.push_back(c);
        }
        symbol.secName = secName;

        int sec;
        rf.read((char *) &sec, sizeof(sec));
        symbol.section = sec;

        unsigned short ofset;
        rf.read((char *) &ofset, sizeof(ofset));
        symbol.ofset = ofset;

        int size;
        rf.read((char *) &size, sizeof(size));
        symbol.size = size;

        vector<char> content;
        int contLen;
        rf.read((char *) &contLen, sizeof(contLen));
        for(int j = 0; j < contLen; j++){
            char c;
            rf.read((char *) &c, sizeof(c));
            content.push_back(c);
        }
        symbol.content = content;
        
        vector<RelRecord> relRecords;
        int relLen;
        rf.read((char *) &relLen, sizeof(relLen));
        for(int j = 0; j < relLen; j++){
            RelRecord record;

            int relOf;
            rf.read((char *) &relOf, sizeof(relOf));
            record.ofset = relOf;

            RelType type;
            rf.read((char *) &type, sizeof(type));
            record.type = type;

            int symInd;
            rf.read((char *) &symInd, sizeof(symInd));
            record.symbolIndex = symInd;

            int symLen;
            rf.read((char *) &symLen, sizeof(symLen));
            string symName;
            for(int k = 0; k < symLen; k++) {
                char c;
                rf.read((char *) &c, sizeof(c));
                symName.push_back(c);
            }
            record.symbolName = symName;

            relRecords.push_back(record);
        }

        symbol.relRecords = relRecords;

        Type type;
        rf.read((char *) &type, sizeof(type));
        symbol.type = type;

        symTable.push_back(symbol);
    }
    symTables.insert(pair<string, vector<Symbol>>(file, symTable));
    rf.close();
}

bool stringContainsSym(vector<string> vec, string name){
    for(int i = 0; i < vec.size(); i++){
        if(vec[i].compare(name) == 0){
            return true;
        }
    }
    return false;
}

bool symbolContainsSym(vector<Symbol> vec, string name){
    for(int i = 0; i < vec.size(); i++){
        if(vec[i].name.compare(name) == 0){
            return true;
        }
    }
    return false;
}

void removeSym(vector<string> vec, string name){
    for(vector<string>::iterator it = vec.begin(); it != vec.end(); ++it){
        if(name.compare(*it) == 0){
            vec.erase(it);
            break;
        }
    }
}

void addSectionAdr(string secName, int size){
    for(int i = 0; i < sectionsAdr.size(); i++){
        if(sectionsAdr[i].name.compare(secName) == 0){
            sectionsAdr[i].size += size;
            return;
        }
    }
    SectionAdr secAdr;
    secAdr.name = secName;
    secAdr.startAdr = 0;
    secAdr.nextFreeAdr = 0;
    secAdr.size = size;
    secAdr.isSetForHex = false;
    sectionsAdr.push_back(secAdr);
}

void checkSymbolsAndFillSectionAdr(){
    vector<string> defined;
    vector<string> undefined;
    for(map<string, vector<Symbol>>::iterator iter = symTables.begin(); iter != symTables.end(); ++iter){
        for(vector<Symbol>::iterator it = iter->second.begin(); it != iter->second.end(); ++it){
            if(it->type == Type::global){
                bool contains = stringContainsSym(defined, it->name);
                if(contains){
                    cout << "Multiple definition of symbol " << it->name << endl;
                    exit(-1);
                }else{
                    defined.push_back(it->name);
                    removeSym(undefined, it->name);
                }
            }else if(it->type == Type::und){
                bool contains = stringContainsSym(defined, it->name);
                if(!contains){
                    contains = stringContainsSym(undefined, it->name);
                    if(!contains){
                        undefined.push_back(it->name);
                    }
                }
            }else if(it->type == Type::sec){
               addSectionAdr(it->name, it->size);
            }
        }
    }

    /*cout<<"defined"<<endl;;
    if(undefined.size() > 0 && !linkable){
        cout << "Symbol undefined" << endl;
        exit(-1); 
    }
    for(int i =0;i<defined.size();i++){
        cout<<defined[i]<<" ";
    }
    cout<<endl;
    for(int i =0;i<undefined.size();i++){
        cout<<undefined[i]<<" ";
    }
    cout<<endl;*/
}

void setNewStartAdrAndUpdateHighestAdr(string secName, int startAdr){
    int secIndex = -1;
    int endAdr;
    for(int i = 0; i < sectionsAdr.size(); i++){
        if(sectionsAdr[i].name.compare(secName) == 0){
            if(sectionsAdr[i].isSetForHex){
                cout << "Error: A start address has already been set for this section" << endl;
                exit(-1);
            }
            secIndex = i;
            endAdr = startAdr + sectionsAdr[i].size - 1;;
            break;
        }
    }
    if(secIndex < 0){
        cout << "Error: Section " << secName << " doesn't exist" << endl;
        exit(-1);
    }
   
    for(int i = 0; i < sectionsAdr.size(), i != secIndex; i++){
        if(sectionsAdr[i].isSetForHex){
            int s = sectionsAdr[i].startAdr;
            int e = sectionsAdr[i].startAdr + sectionsAdr[i].size - 1;
            if( (startAdr >= s && startAdr <= e) ||
                (endAdr >= s && endAdr <= e) ||
                (s >= startAdr && s <= endAdr) ||
                (e >= startAdr && e <= endAdr)){
                cout << "Error: Section " << secName << " overlaps with section " << sectionsAdr[i].name << endl;
                exit(-1);
            }
        }
    }

    sectionsAdr[secIndex].isSetForHex = true;
    sectionsAdr[secIndex].startAdr = startAdr;
    sectionsAdr[secIndex].nextFreeAdr = startAdr;
    if(startAdr + sectionsAdr[secIndex].size > highestAdr){
        highestAdr = startAdr + sectionsAdr[secIndex].size;
    }
}

void updateStartAdrForSections(){
    for(int i = 0; i < sectionsAdr.size(); i++){
        if(!sectionsAdr[i].isSetForHex){
            sectionsAdr[i].isSetForHex =  true;
            sectionsAdr[i].startAdr = highestAdr;
            sectionsAdr[i].nextFreeAdr = highestAdr;
            highestAdr += sectionsAdr[i].size;
        }
    }
}

void formSectionsInfo(){
     for(map<string, vector<Symbol>>::iterator iter = symTables.begin(); iter != symTables.end(); ++iter){
        for(vector<Symbol>::iterator it = iter->second.begin(); it != iter->second.end(); ++it){
            if(it->type == Type::sec){
                SectionInfo secInfo;
                secInfo.file = iter->first;
                secInfo.name = it->name;
                for(int i = 0; i < sectionsAdr.size(); i++){
                    if(sectionsAdr[i].name.compare(it->name) == 0){
                        secInfo.newValue = sectionsAdr[i].nextFreeAdr;
                        sectionsAdr[i].nextFreeAdr += it->size;
                        break;
                    }
                }
                sectionsFromFilesInfo.push_back(secInfo);
            }
        }
    }
}

int findSectionIndex(string secName){
    for(int i = 0; i < symTable.size(); i++){
        if(symTable[i].name.compare(secName) == 0){
            return symTable[i].index;
        }
    }
    return -1;
}

unsigned short findOfset(string file, string secName){
    for(int i = 0; i < sectionsFromFilesInfo.size(); i++){
        if(sectionsFromFilesInfo[i].name.compare(secName) == 0 && sectionsFromFilesInfo[i].file.compare(file) == 0){
            return sectionsFromFilesInfo[i].newValue;
        }
    }
    return 0;
}

void formSymbolTable(){
    //ubaci sekcije
    for(int i = 0; i < sectionsAdr.size(); i++){
        Symbol s;
        s.index = ind++;
        s.name = sectionsAdr[i].name;
        s.secName = s.name;
        s.section = s.index;
        s.ofset = sectionsAdr[i].startAdr;
        s.size = sectionsAdr[i].size;
        s.type = Type::sec;
        s.content.resize(s.size, 0x00);
        symTable.push_back(s);
    }

    for(map<string, vector<Symbol>>::iterator iter = symTables.begin(); iter != symTables.end(); ++iter){
        for(vector<Symbol>::iterator it = iter->second.begin(); it != iter->second.end(); ++it){
            if(it->type == Type::global){
                Symbol s;
                s.index = ind++;
                s.name = it->name;
                s.secName = it->secName;
                s.section = findSectionIndex(it->secName);
                s.ofset = findOfset(iter->first, it->secName) + it->ofset;
                s.size = 0;
                s.type = Type::global;
                symTable.push_back(s);
            }else if(it->type == Type::und && linkable){
                //proveri jel postoji
                bool contains = symbolContainsSym(symTable, it->name);
                if(!contains){
                    Symbol s;
                    s.index = ind++;
                    s.name = it->name;
                    s.secName = "";
                    s.section =  0;
                    s.ofset = 0;
                    s.size = 0;
                    s.type = Type::und; 
                    symTable.push_back(s);
                }
            }
        }
    }
}

int findSymIndex(string symbol){
    for(int i = 0; i < symTable.size(); i++){
        if(symTable[i].name.compare(symbol) == 0){
            return symTable[i].index;
        }
    }
    return -1;
}

void makeRelRecord(int section, int ofset, RelType type, int symbol, string symbolName){
    RelRecord record;
    record.ofset = ofset;
    record.type = type;
    record.symbolIndex = symbol;
    record.symbolName = symbolName;
    symTable[section - 1].relRecords.push_back(record);
}

void aggregate(string file, string secName, vector<char> content, int size){
    int secIndex = findSectionIndex(secName);
    int writeIndex = findOfset(file, secName) - symTable[secIndex - 1].ofset;
   // cout<<file<<" "<<secName<<" "<<writeIndex<<endl;
    for(int i = 0; i < size; i++){
        symTable[secIndex - 1].content[writeIndex] = content[i];
        writeIndex++;
    }
}

void fillContent(){
    for(map<string, vector<Symbol>>::iterator iter = symTables.begin(); iter != symTables.end(); ++iter){
        for(vector<Symbol>::iterator it = iter->second.begin(); it != iter->second.end(); ++it){
            if(it->type == Type::sec){// za tu sekciju iz iter->first fajla razresavamo rel zapise
                for(vector<RelRecord>::iterator rec = it->relRecords.begin(); rec != it->relRecords.end(); ++rec){//idemo po rel zapisima
                    int refSymIndex = findSymIndex(rec->symbolName);//ind simb iz nove tab simb na kog se trenutni zapis referise
                    int sectionRemake = findSymIndex(it->name);//ind sekcije iz nove tab simb koju trenutno prepravljamo
                    switch (rec->type){
                        case RelType::dataAbs: {
                            if(symTable[refSymIndex - 1].type == Type::global){
                                int num = (it->content[rec->ofset + 1] << 8) | (it->content[rec->ofset]);
                                num += symTable[refSymIndex - 1].ofset; 
                                it->content[rec->ofset] = (num & 0xff);
                                it->content[rec->ofset + 1] = ((num >> 8) & 0xff);
                                if(linkable){
                                    //ostavi rel zapis u sekciji koju tr prepravljamo i to u novoj tabeli simbola;

                                    //mesto na kom prepravljamo u glavnom sadrzaju gde je sve spojeno je pomeraj sekcije koju prepravljamo iz tog fajla 
                                    //+ pomeraj mesta kog prepravljamo unutar te sekc kad je sekc bila na 0 adr 
                                    int ofset = findOfset(iter->first, it->name) + rec->ofset;//ofset u sekciji iz nove tab simb na kom prepravljamo
                                    makeRelRecord(sectionRemake, ofset, RelType::dataAbs, symTable[refSymIndex - 1].section, symTable[refSymIndex -1].secName);
                                }
                            }else if(symTable[refSymIndex - 1].type == Type::sec){
                                int newVal = (it->content[rec->ofset + 1] << 8) | (it->content[rec->ofset]);
                                newVal += findOfset(iter->first, symTable[refSymIndex - 1].secName);
                                it->content[rec->ofset] = newVal & 0xff;
                                it->content[rec->ofset + 1] = ((newVal >> 8) & 0xff);
                            
                                if(linkable){
                                    int ofset = findOfset(iter->first, it->name) + rec->ofset;
                                    makeRelRecord(sectionRemake, ofset, RelType::dataAbs, symTable[refSymIndex - 1].index, symTable[refSymIndex - 1].name);
                                }
                            }else if(symTable[refSymIndex - 1].type == Type::und && linkable){//svakako ako je und je linkable
                                int ofset = findOfset(iter->first, it->name) + rec->ofset;
                                makeRelRecord(sectionRemake, ofset, RelType::dataAbs, symTable[refSymIndex - 1].index, symTable[refSymIndex - 1].name);
                            }
                            break;
                        }
                        case RelType::instrAbs: {
                            if(symTable[refSymIndex - 1].type == Type::global){
                                int num = (it->content[rec->ofset] << 8) | (it->content[rec->ofset + 1]);
                                num += symTable[refSymIndex - 1].ofset;
                                it->content[rec->ofset] = ((num >> 8) & 0xff);
                                it->content[rec->ofset + 1] = (num & 0xff);
                                if(linkable){
                                    //ostavi rel zapis u sekciji koju tr prepravljamo i to u novoj tabeli simbola;

                                    //mesto na kom prepravljamo u glavnom sadrzaju gde je sve spojeno je pomeraj sekcije koju prepravljamo iz tog fajla 
                                    //+ pomeraj mesta kog prepravljamo unutar te sekc kad je sekc bila na 0 adr 
                                    int ofset = findOfset(iter->first, it->name) + rec->ofset;//ofset u sekciji iz nove tab simb na kom prepravljamo
                                    makeRelRecord(sectionRemake, ofset, RelType::instrAbs, symTable[refSymIndex - 1].section, symTable[refSymIndex -1].secName);
                                }
                            }else if(symTable[refSymIndex - 1].type == Type::sec){
                                int newVal = (it->content[rec->ofset] << 8) | (it->content[rec->ofset + 1]); 
                                newVal += findOfset(iter->first, symTable[refSymIndex - 1].secName);
                                it->content[rec->ofset] = ((newVal >> 8) & 0xff);
                                it->content[rec->ofset + 1] = newVal & 0xff;
                            
                                if(linkable){
                                    int ofset = findOfset(iter->first, it->name) + rec->ofset;
                                    makeRelRecord(sectionRemake, ofset, RelType::instrAbs, symTable[refSymIndex - 1].index, symTable[refSymIndex - 1].name);
                                }
                            }else if(symTable[refSymIndex - 1].type == Type::und && linkable){//svakako ako je und je linkable
                                int ofset = findOfset(iter->first, it->name) + rec->ofset;
                                makeRelRecord(sectionRemake, ofset, RelType::instrAbs, symTable[refSymIndex - 1].index, symTable[refSymIndex - 1].name);
                            }
                            break;
                        }   
                        case RelType::instrRel: {//ako je u istoj sek provera
                            if(symTable[refSymIndex - 1].type == Type::global){
                                int ofset = findOfset(iter->first, it->name) + rec->ofset;
                                if(linkable){
                                    short val = (it->content[rec->ofset] << 8) | (it->content[rec->ofset + 1]);
                                    val += symTable[refSymIndex - 1].ofset;
                                    it->content[rec->ofset] = ((val >> 8) & 0xff);
                                    it->content[rec->ofset + 1] = (val & 0xff); 

                                    if(sectionRemake != symTable[refSymIndex - 1].section){
                                        makeRelRecord(sectionRemake, ofset, RelType::instrRel, symTable[refSymIndex - 1].section, symTable[refSymIndex - 1].secName);
                                    }else{
                                        short num = (it->content[rec->ofset] << 8) | (it->content[rec->ofset + 1]);
                                        num += ofset;
                                        it->content[rec->ofset] = ((num >> 8) & 0xff);
                                        it->content[rec->ofset + 1] = (num & 0xff); 
                                    }
                                }else{
                                    short val = (it->content[rec->ofset] << 8) | (it->content[rec->ofset + 1]);
                                    val += symTable[refSymIndex - 1].ofset - ofset;
                                    it->content[rec->ofset] = ((val >> 8) & 0xff);
                                    it->content[rec->ofset + 1] = (val & 0xff); 
                                }
                            }else if(symTable[refSymIndex - 1].type == Type::sec){
                                int ofset = findOfset(iter->first, it->name) + rec->ofset;
                                if(linkable){
                                    short val = (it->content[rec->ofset] << 8) | (it->content[rec->ofset + 1]);
                                    val += findOfset(iter->first, symTable[refSymIndex - 1].name);
                                    it->content[rec->ofset] = ((val >> 8) & 0xff);
                                    it->content[rec->ofset + 1] = (val & 0xff); 

                                    if(sectionRemake != symTable[refSymIndex - 1].index){
                                        makeRelRecord(sectionRemake, ofset, RelType::instrRel, symTable[refSymIndex - 1].index, symTable[refSymIndex - 1].name);
                                    }else{
                                        //mislim da ovde nema sta da se uradi jer ne moze da se desi tj uvek treba rel zapis
                                        //jer to bi znacilo da smo loc simbol ref koji je def u istoj ovoj sekc koju prepravljamo i da smo napravi rel zapis, a to nismo uradili jer ne treba
                                    }
                                }else{
                                    short val = (it->content[rec->ofset] << 8) | (it->content[rec->ofset + 1]);
                                    val += findOfset(iter->first, symTable[refSymIndex - 1].name) - ofset;
                                    it->content[rec->ofset] = ((val >> 8) & 0xff);
                                    it->content[rec->ofset + 1] = (val & 0xff); 
                                }    
                            }else if(symTable[refSymIndex - 1].type == Type::und && linkable){//svakako ako je und je linkable
                                int ofset = findOfset(iter->first, it->name) + rec->ofset;
                                makeRelRecord(sectionRemake, ofset, RelType::instrRel, symTable[refSymIndex - 1].index, symTable[refSymIndex - 1].name);
                            }
                            break;
                        }
                    }
                }
                aggregate(iter->first, it->name, it->content, it->size);
            }
        }
    }
}