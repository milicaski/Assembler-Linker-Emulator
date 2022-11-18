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



struct firstPassRecord {
    string symbol;
    int section;
    int ofset;
    RelType type;
};



void printST(string file);
int addSection(string name);
void setSize(int section, int size);
void addSymbol(string name, int section, unsigned short ofset);
void setAsGlobal(string name);
void setAsExtern(string name);
void addEquSymbol(string name, unsigned short v);
void fillContent(int section, char byte);
void fillTwoAdr(int section, char firstByte, int rd, int rs);
void changeInstrDesc(int section, int instrDesc, char byte);
void addRecord(int section, int ofset, string symbol, RelType type);
void makeRelRecord(int section, int ofset, RelType type, int symbol);
void secondPass();
void writeToFile(string file);