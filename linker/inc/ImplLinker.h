#pragma once
#include <iostream>
#include <string>
#include <vector>
using namespace std;

enum Type{
    local, global, absolut, und, sec
};

enum RelType{
    dataAbs, instrAbs, instrRel
};

struct RelRecord{
    int ofset;
    RelType type;
    int symbolIndex;
    string symbolName;
};

static int ind = 1;
struct Symbol{
    int index;
    string name;
    string secName;
    int section;
    unsigned short ofset;
    int size;//for sections
    vector<char> content;//only for sections, hidden field(won't be visible in .o file)
    vector<RelRecord> relRecords;//only for sections, hidden field(won't be visible in .o file)
    Type type;
};

struct SectionInfo
{
    string name;
    string file;
    int newValue;
};

struct SectionAdr{
    string name;
    int startAdr;
    int nextFreeAdr;
    int size;
    bool isSetForHex;
};

void printST(string file);
void read(string file);
void checkSymbolsAndFillSectionAdr();
bool stringContainsSym(vector<string> vec, string name);
bool symbolContainsSym(vector<Symbol> vec, string name);
void removeSym(vector<string> vec, string name);
void addSectionAdr(string secName, int size);
void setNewStartAdrAndUpdateHighestAdr(string secName, int startAdr);
void updateStartAdrForSections();
void formSectionsInfo();
void formSymbolTable();
int findSectionIndex(string secName);
unsigned short findOfset(string file, string secName);
void fillContent();
int findSymIndex(string symbol);
void makeRelRecord(int section, int ofset, RelType type, int symbol, string symbolName);
void aggregate(string file, string secName, vector<char> content, int size);
void writeToFileLinkable(string file);
void writeToFileHex(string file);