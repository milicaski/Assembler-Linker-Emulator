#pragma once
#include <iostream>
#include <string>
using namespace std;

#define PSW 8
#define PC 7
#define SP 6
#define HALT 0x00
#define IRET 0x20
#define RET 0x40
#define INT 0x10
#define XCHG 0x60
#define ADD 0x70
#define SUB 0x71
#define MUL 0x72
#define DIV 0x73
#define CMP 0x74 
#define NOT 0x80
#define AND 0x81
#define OR 0x82
#define XOR 0x83
#define TEST 0x84
#define SHL 0x90
#define SHR 0x91
#define CALL 0x30
#define JMP 0x50
#define JEQ 0x51
#define JNE 0x52
#define JGT 0x53
#define LDR_POP 0xa0
#define STR_PUSH 0xb0

#define ZF 0x1
#define OF 0x2
#define CF 0x4
#define NF 0x8
#define TR 0x2000
#define TL 0x4000
#define I 0x8000
/*
#define immed 0x00
#define regdir 0x01
#define regdir16 0x05
#define regind 0x02
#define regind16 0x03
#define memory 0x04
#define regindAfterInc2 0x42
#define regindPreDec2 0x12
*/
#define immed 0x0
#define regdir 0x1
#define regdir16 0x5
#define regind 0x2
#define regind16 0x3
#define memory 0x4
/*#define regindAfterInc2 0x42
#define regindPreDec2 0x12*/

void readFile(string file);
void setPcSp();
void emulate();
unsigned char fetch();
short getPCForJumps();
void executeLdr();
void executeStr();
void push(short val);
short pop();
void checkForInterrupts();