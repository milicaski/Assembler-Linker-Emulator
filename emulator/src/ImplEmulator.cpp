#include "../inc/ImplEmulator.h"
#include <fstream>
#include <unistd.h>
#include <time.h> 

char mem[65536];
short reg[9];
clock_t myTime;
int lookup[8];
unsigned short term_in = 0xff02;
unsigned short term_out = 0xff00;
unsigned short tim_cfg = 0xff10;
int interrupt = 0;

void readFile(string file){
    lookup[0] = 500;
    lookup[1] = 1000;
    lookup[2] = 1500;
    lookup[3] = 2000;
    lookup[4] = 5000;
    lookup[5] = 10000;
    lookup[6] = 30000;
    lookup[7] = 60000;
    myTime = clock();

    ifstream rf(file, ios::out | ios::binary);

    int num;
    rf.read((char *) &num, sizeof(num));

    for(int i = 0; i < num; i++){
        int size;
        rf.read((char *) &size, sizeof(size));

        unsigned short start;
        rf.read((char *) &start, sizeof(start));

        //printf("%x: ",start);
        for(int i = 0; i < size; i++){
            char c;
            rf.read((char *) &c, sizeof(c));
            //printf("%x%x ", (c >> 4) & 0xf, c & 0xf);
            mem[(unsigned short)start] = c;
            start++;
        }
        //cout<<endl;
        
    }
}

void setPcSp(){
    reg[PC] = (mem[1] << 8) | mem[0];
    reg[SP] = 20000;
    reg[PSW] = 0;
}

unsigned char fetch(){
    unsigned char byte = mem[(unsigned short)reg[PC]];
    reg[PC]++;
    return byte;
}

short getPCForJumps(){
    short retVal;
    char reg_desc = fetch();
    char addr_mode = fetch();
    int up = (addr_mode >> 4) & 0xf;
    switch (addr_mode & 0xf){
        case immed:{
            char high_payload = fetch();
            char low_payload = fetch();
            unsigned short newPc = (high_payload << 8 | (low_payload & 0xff));
            retVal = newPc;
            break;
        }
        case regdir:{
            int src = reg_desc & 0xf;
            retVal = reg[src];
            break;
        }
        case regdir16:{
            char high_payload = fetch();
            char low_payload = fetch();
            short payload = (high_payload << 8 | (low_payload & 0xff));
            int src = reg_desc & 0xf;
            retVal = reg[src] + payload;
            break;
        }
        case regind:{
            int src = reg_desc & 0xf;
            if(up == 1){
                reg[src] -= 2;
            }
            if(up == 2){
                reg[src] += 2;
            }
            retVal = ((mem[(unsigned short)(reg[src] + 1)] << 8) | (mem[(unsigned short)reg[src]] & 0xff));
            if(up == 3){
                reg[src] -= 2;
            }
            if(up == 4){
                reg[src] += 2;
            }
            break;
        }
        case regind16:{
            char high_payload = fetch();
            char low_payload = fetch();
            short payload = (high_payload << 8 | (low_payload & 0xff));
            int src = reg_desc & 0xf;
            if(up == 1){
                reg[src] -= 2;
            }
            if(up == 2){
                reg[src] += 2;
            }
            retVal = (mem[(unsigned short)(reg[src] + payload + 1)] << 8) | (mem[(unsigned short)(reg[src] + payload)] & 0xff);
            if(up == 3){
                reg[src] -= 2;
            }
            if(up == 4){
                reg[src] += 2;
            }
            break;
        }
        case memory:{
            char high_payload = fetch();
            char low_payload = fetch();
            short payload = (high_payload << 8 | (low_payload & 0xff));
            retVal = (mem[(unsigned short)(payload + 1)]) | (mem[(unsigned short)payload] & 0xff);
            break;
        }
        default:{
            //pogresno adresiranje
            cout<<"doslo do greske1"<<endl;
            push(reg[PC]);
            push(reg[PSW]);
            reg[PSW] &= (~I);
            reg[PC] = (mem[3] << 8) | (mem[2] & 0xff);
            retVal = reg[PC];
            break;
        }
    }
    
    return retVal;
}

void executeLdr(){
    char reg_desc = fetch();
    char addr_mode = fetch();
    int up = (addr_mode >> 4) & 0xf;
    switch (addr_mode & 0xf){
        case immed:{
            char high_payload = fetch();
            char low_payload = fetch();
            short payload = ((high_payload << 8) | (low_payload & 0xff));
            int dest = (reg_desc >> 4) & 0xf;
            reg[dest] = payload;
            break;
        }
        case regdir:{
            int src = reg_desc & 0xf;
            int dest = (reg_desc >> 4) & 0xf;
            reg[dest] = reg[src];
            break;
        }
       case regdir16:{
            char high_payload = fetch();
            char low_payload = fetch();
            short payload = (high_payload << 8 | low_payload);
            int src = reg_desc & 0xf;
            int dest = (reg_desc >> 4) & 0xf;
            reg[dest] = reg[src] + payload;
            break;
        }
        case regind:{
            int src = reg_desc & 0xf;
            int dest = (reg_desc >> 4) & 0xf;
            if(up == 1){
                reg[src] -= 2;
            }
            if(up == 2){
                reg[src] += 2;
            }
            reg[dest] = (mem[(unsigned short)(reg[src] + 1)] << 8) | (mem[(unsigned short)reg[src]] & 0xff);
            if(up == 3){
                reg[src] -= 2;
            }
            if(up == 4){
                reg[src] += 2;
            }
            break;
        }
        case regind16:{
            char high_payload = fetch();
            char low_payload = fetch();
            short payload = (high_payload << 8 | low_payload);
            int src = reg_desc & 0xf;
            int dest = (reg_desc >> 4) & 0xf;
            if(up == 1){
                reg[src] -= 2;
            }
            if(up == 2){
                reg[src] += 2;
            }
            reg[dest] = (mem[(unsigned short)(reg[src] + payload + 1)] << 8) | (mem[(unsigned short)(reg[src] + payload)] & 0xff);
            if(up == 3){
                reg[src] -= 2;
            }
            if(up == 4){
                reg[src] += 2;
            }
            break;
        }
        case memory:{
            char high_payload = fetch();
            char low_payload = fetch();
            short payload = (high_payload << 8 | low_payload);
            int dest = (reg_desc >> 4) & 0xf;
            reg[dest] = (mem[(unsigned short)(payload + 1)] << 8) | (mem[(unsigned short)(payload)] & 0xff);
            break;
        }
        /*case regindAfterInc2:{//pop
            int src = reg_desc & 0xf;
            int dest = (reg_desc >> 4) & 0xf;
            reg[dest] = (mem[(unsigned short)(reg[src] + 1)] << 8) | (mem[(unsigned short)reg[src]] & 0xff);//ucitaj sa steka
            reg[src] += 2;//sp+=2
            break;
        }*/
        default:{
            //pogresno adresiranje
            cout<<"doslo do greske2"<<endl;
            push(reg[PC]);
            push(reg[PSW]);
            reg[PSW] &= (~I);
            reg[PC] = (mem[3] << 8) | (mem[2] & 0xff);
            break;
        }
    }
}

void executeStr(){
    char reg_desc = fetch();
    char addr_mode = fetch();
    int up = (addr_mode >> 4) & 0xf;
    switch (addr_mode & 0xf){
        case regdir:{
            int src = reg_desc & 0xf;
            int dest = (reg_desc >> 4) & 0xf;
            reg[src] = reg[dest];
            break;
        }
       case regdir16:{
            char high_payload = fetch();
            char low_payload = fetch();
            short payload = (high_payload << 8 | low_payload);
            int src = reg_desc & 0xf;
            int dest = (reg_desc >> 4) & 0xf;
            reg[src] = reg[dest] + payload;
            break;
        }
        case regind:{
            int src = reg_desc & 0xf;
            int dest = (reg_desc >> 4) & 0xf;
            if(up == 1){
                reg[src] -= 2;
            }
            if(up == 2){
                reg[src] += 2;
            }
            mem[(unsigned short)reg[src]] = reg[dest] & 0xff;
            mem[(unsigned short)(reg[src] + 1)] = (reg[dest] >> 8) & 0xff;
            if(up == 3){
                reg[src] -= 2;
            }
            if(up == 4){
                reg[src] += 2;
            }
            break;
        }
        case regind16:{
            char high_payload = fetch();
            char low_payload = fetch();
            short payload = (high_payload << 8 | low_payload);
            int src = reg_desc & 0xf;
            int dest = (reg_desc >> 4) & 0xf;
            if(up == 1){
                reg[src] -= 2;
            }
            if(up == 2){
                reg[src] += 2;
            }
            mem[(unsigned short)(reg[src] + payload)] = reg[dest] & 0xff;
            mem[(unsigned short)(reg[src] + payload + 1)] = (reg[dest] >> 8) & 0xff;
            if(up == 3){
                reg[src] -= 2;
            }
            if(up == 4){
                reg[src] += 2;
            }
            break;
        }
        case memory:{
            char high_payload = fetch();
            char low_payload = fetch();
            short payload = (high_payload << 8 | low_payload);
            int dest = (reg_desc >> 4) & 0xf;
            mem[(unsigned short)(payload)] = reg[dest] & 0xff;
            mem[(unsigned short)(payload + 1)] = (reg[dest] >> 8) & 0xff;
            break;
        }
        /*case regindPreDec2:{//push
            int src = reg_desc & 0xf;
            int dest = (reg_desc >> 4) & 0xf;
            reg[src] -=2;
            //upisi na stek
            mem[(unsigned short)reg[src]] = reg[dest] & 0xff;
            mem[(unsigned short)(reg[src] + 1)] = (reg[dest] >> 8) & 0xff;
            break;
        }*/
        default:{
            //pogresno adresiranje
            cout<<"doslo do greske3"<<endl;
            push(reg[PC]);
            push(reg[PSW]);
            reg[PSW] &= (~I);
            reg[PC] = (mem[3] << 8) | (mem[2] & 0xff);
            break;
        }
    }
}

void push(short val){
    reg[SP] -=2;
    mem[(unsigned short)reg[SP]] = val & 0xff;
    mem[(unsigned short)(reg[SP] + 1)] = (val >> 8) & 0xff;
}

short pop(){
    short val;
    val = (mem[(unsigned short)(reg[SP] + 1)] << 8) | (mem[(unsigned short)reg[SP]] & 0xff);//ucitaj sa steka
    reg[SP] += 2;
    return val;
}

void checkForInterrupts(){
    char c;
    if(read(STDIN_FILENO, &c, 1) == 1){
        //printf("uso %c", c);
        //cout<<"pritisnut"<<endl;
        mem[term_in] = c;
        mem[term_in + 1] = 0x00;
        interrupt = 1;
    }
    //printf("term_out je: %c\n", mem[term_out]);
    if(mem[term_out] != 0x00){
        //printf("%c ", mem[term_out]);
        write(STDOUT_FILENO, &mem[term_out], 1);
        mem[term_out] = 0x00;
        mem[term_out + 1] = 0x00;
    }
    if(!(reg[PSW] & I)){
        if(!(reg[PSW] & TL)){
            if(interrupt){
                interrupt = 0;
                push(reg[PC]);
                push(reg[PSW]);
                reg[PSW] &= (~I);
                reg[PC] = (mem[7] << 8) | (mem[6] & 0xff);
                return;
            }
        }
        if(!(reg[PSW] & TR)){
            //cout<<"uso timer"<<endl;
            clock_t t = clock();
            clock_t diff = t - myTime;
            int mills = lookup[mem[tim_cfg]];
            if(((float)diff) / CLOCKS_PER_SEC * 1000 >= mills){
                myTime = t;
                push(reg[PC]);
                push(reg[PSW]);
                reg[PSW] &= (~I);
                reg[PC] = (mem[5] << 8) | (mem[4] & 0xff);
                return;
            }
        }
    }
}

void emulate(){
    bool over = false;
    while(!over){
        unsigned char g;
        switch (g = fetch()){
            case HALT:{
                over = true;
                break;
            }
            case IRET:{
                reg[PSW] = pop();
                reg[PC] = pop();
               // cout<<"iret"<<endl;
                break;
            }
            case RET:{
                reg[PC] = pop();
                break;
            }
            case INT:{
                char reg_desc = fetch();
                int dest = (reg_desc >> 4) & 0xf;
                push(reg[PC]);
                push(reg[PSW]);
                reg[PC] = (mem[(unsigned short)((reg[dest] % 8) * 2 + 1)] << 8) | (mem[(unsigned short)((reg[dest] % 8) * 2)] & 0xff);
                reg[PSW] &= (~I);
                break;
            }
            case ADD:{
                char byte = fetch();
                int dest = (byte >> 4) & 0xf;
                int src = byte & 0xf;
                reg[dest] = reg[dest] + reg[src];
                break;
            }
            case SUB:{
                char byte = fetch();
                int dest = (byte >> 4) & 0xf;
                int src = byte & 0xf;
                reg[dest] = reg[dest] - reg[src];
                break;
            }
            case MUL:{
                char byte = fetch();
                int dest = (byte >> 4) & 0xf;
                int src = byte & 0xf;
                reg[dest] = reg[dest] * reg[src];
                break;
            }
            case DIV:{
                char byte = fetch();
                int dest = (byte >> 4) & 0xf;
                int src = byte & 0xf;
                reg[dest] = reg[dest] / reg[src];
                break;
            }
            case CMP:{
                char byte = fetch();
                int dest = (byte >> 4) & 0xf;
                int src = byte & 0xf;
                int tmp = reg[dest] - reg[src];
                //updt psw
                if(tmp == 0){
                    reg[PSW] |= ZF;
                }else{
                   reg[PSW] &= ~ZF; 
                }
                if(tmp < 0){
                    reg[PSW] |= NF;
                }else{
                   reg[PSW] &= ~NF; 
                }
                if((reg[dest] > 0 && reg[src] < 0 && tmp < 0) ||
                    (reg[dest] < 0 && reg[src] > 0 && tmp > 0)){
                    reg[PSW] |= OF;
                }else{
                   reg[PSW] &= ~OF; 
                }
                if(reg[dest] < reg[src]){
                    reg[PSW] |= CF;
                }else{
                   reg[PSW] &= ~CF; 
                }
                break;
            }
            case NOT:{
                char byte = fetch();
                int dest = (byte >> 4) & 0xf;
                reg[dest] = ~reg[dest];
                break;
            }
            case AND:{
                char byte = fetch();
                int dest = (byte >> 4) & 0xf;
                int src = byte & 0xf;
                reg[dest] = reg[dest] & reg[src];
                break;
            }
            case OR:{
                char byte = fetch();
                int dest = (byte >> 4) & 0xf;
                int src = byte & 0xf;
                reg[dest] = reg[dest] | reg[src];
                break;
            }
            case XOR:{
                char byte = fetch();
                int dest = (byte >> 4) & 0xf;
                int src = byte & 0xf;
                reg[dest] = reg[dest] ^ reg[src];
                break;
            }
            case TEST:{
                char byte = fetch();
                int dest = (byte >> 4) & 0xf;
                int src = byte & 0xf;
                int tmp = reg[dest] & reg[src];
                //updt psw
                if(tmp == 0){
                    reg[PSW] |= ZF;
                }else{
                   reg[PSW] &= ~ZF; 
                }
                if(tmp < 0){
                    reg[PSW] |= NF;
                }else{
                   reg[PSW] &= ~NF; 
                }
                break;
            }
            case SHL:{
                char byte = fetch();
                int dest = (byte >> 4) & 0xf;
                int src = byte & 0xf;
                reg[dest] = reg[dest] << (reg[src] - 1);
                //updt psw
                if(reg[dest] & 0x8000){
                    reg[PSW] |= CF;
                }else{
                   reg[PSW] &= ~CF; 
                }
                reg[dest] = reg[dest] << 1;
                
                if(reg[dest] == 0){
                    reg[PSW] |= ZF;
                }else{
                   reg[PSW] &= ~ZF; 
                }
                if(reg[dest] < 0){
                    reg[PSW] |= NF;
                }else{
                   reg[PSW] &= ~NF; 
                }
                break;
            }
            case SHR:{
                char byte = fetch();
                int dest = (byte >> 4) & 0xf;
                int src = byte & 0xf;
                reg[dest] = (reg[dest] - 1) >> reg[src];
                //updt psw
                if(reg[dest] & 0x0001){
                    reg[PSW] |= CF;
                }else{
                   reg[PSW] &= ~CF; 
                }
                reg[dest] = reg[dest] >> 1;

                if(reg[dest] == 0){
                    reg[PSW] |= ZF;
                }else{
                   reg[PSW] &= ~ZF; 
                }
                if(reg[dest] < 0){
                    reg[PSW] |= NF;
                }else{
                   reg[PSW] &= ~NF; 
                }
                break;
            }
            case CALL:{
                push(reg[PC]);
                reg[PC] = getPCForJumps();
                break;
            }
            case JMP:{
                reg[PC] = getPCForJumps();
                break;
            }
            case JEQ:{
                short retVal = getPCForJumps();
                if(reg[PSW] & ZF){
                   reg[PC] = retVal;
                }
                break;
            }   
            case JNE:{
                short retVal = getPCForJumps();
                if(!(reg[PSW] & ZF)){
                    reg[PC] = retVal;
                }
                break;
            }
            case JGT:{
                short retVal = getPCForJumps();
                if( !((reg[PSW] & NF) ^ (reg[PSW] & OF) & !(reg[PSW] & ZF))){
                    reg[PC] = retVal; 
                }
                break;
            }
            case LDR_POP:{
                executeLdr();
                //cout<<"ldr_pop"<<endl;
                break;
            }
            case STR_PUSH:{
                executeStr();
                //cout<<"str_push"<<endl;
                break;
            }
            default:{
                //pogresan op code
                //printf("doslo do greske4 %x\n", g);
                
                push(reg[PC]);
                push(reg[PSW]);
                reg[PSW] &= (~I);
                reg[PC] = (mem[3] << 8) | (mem[2] & 0xff);
                //printf("doslo do greske4 %x\n", reg[PC]);
                break;
            }
                
        }
        //prekidi
        checkForInterrupts();
    }
}
