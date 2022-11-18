%{
#include <stdio.h> 
#include "./inc/Impl.h"
void yyerror (const char *s);
extern int yylex();
extern int yylineno;

int section = 0;
int locCnt = 0;
int instrDesc = 0;
%}

%union{
    char* name;
    unsigned short num;
}
%token SYNTAX_ERROR 
%token COMMENT 
%token COLON 
%token GLOBAL 
%token EXTERN 
%token SECTION 
%token WORD 
%token SKIP 
%token EQU 
%token END
%token COMMA
%token NEW_LINE
%token<num> HEX_NUM
%token<num> DEC_NUM
%token<name> IDENTIFIER
%token HALT
%token INT
%token IRET
%token CALL
%token RET
%token JMP
%token JEQ
%token JNE
%token JGT
%token PUSH
%token POP
%token XCHG
%token ADD
%token SUB
%token MUL
%token DIV
%token CMP
%token NOT
%token AND
%token OR
%token XOR
%token TEST
%token SHL
%token SHR
%token LDR
%token STR
%token<num> REG
%token DOLLAR
%token PERC
%token OPEN_P
%token CLOSED_P
%token PLUS
%token STAR
%start program
%type <name> symListGlobal
%type <name> symListExtern
%type <num> number
%%
program: line 
        | line program
        ;

line: IDENTIFIER COLON {addSymbol($1, section, locCnt);}
    | directive NEW_LINE
    | command NEW_LINE
    | NEW_LINE
   ;

directive: GLOBAL symListGlobal
        | EXTERN symListExtern
        | SECTION IDENTIFIER {
            int s = addSection($2);
            if(section > 0){
                setSize(section, locCnt);
            }
            locCnt = 0;
            section = s;
         }
        | WORD list
        | SKIP number {
            for(int i = 0; i < $2; i++){
                fillContent(section, 0x0);
                locCnt++;
            }
        }
        | EQU IDENTIFIER COMMA number {addEquSymbol($2, $4);}
        | END {
            if(section > 0){
                setSize(section, locCnt);
            } 
            return 0;
        }
        ;

symListGlobal: IDENTIFIER {setAsGlobal($1);}
    | symListGlobal COMMA IDENTIFIER {setAsGlobal($3);}
    ;

symListExtern: IDENTIFIER {setAsExtern($1);}
    | symListExtern COMMA IDENTIFIER {setAsExtern($3);}
    ;

list: listType 
    | list COMMA listType 
    ;

listType: IDENTIFIER {
            addRecord(section, locCnt, $1, RelType::dataAbs);
            fillContent(section, 0x00);
            locCnt++;
            fillContent(section, 0x00);
            locCnt++;
        }
        | number {
            fillContent(section, $1 & 0xff);
            locCnt++;
            fillContent(section, ($1>>8) & 0xff);
            locCnt++;
        }
        ;

number: HEX_NUM 
        | DEC_NUM 
        ;

command: one_adr {locCnt++;}
        | two_adr
        | more_adr
        ;

one_adr: HALT {fillContent(section, 0x00);}
        | IRET {fillContent(section, 0x20);}
        | RET {fillContent(section, 0x40);}
        ;

two_adr: INT REG {
            fillContent(section, 0x10);
            locCnt++;
            fillContent(section, (($2 << 4) & 0xf0) | 0xf);
            locCnt++;
        }
        | XCHG REG COMMA REG {
            fillContent(section, 0x60);
            locCnt++;
            fillContent(section, (($2 << 4) & 0xf0) | ($4 & 0x0f));
            locCnt++;
        }
        | arit_op
        | log_op
        | move_op
        ;

arit_op: ADD REG COMMA REG {fillTwoAdr(section, 0x70, $2, $4);}
        | SUB REG COMMA REG {fillTwoAdr(section, 0x71, $2, $4);}
        | MUL REG COMMA REG {fillTwoAdr(section, 0x72, $2, $4);}
        | DIV REG COMMA REG {fillTwoAdr(section, 0x73, $2, $4);}
        | CMP REG COMMA REG {fillTwoAdr(section, 0x74, $2, $4);}
        ;

log_op: NOT REG {fillTwoAdr(section, 0x80, $2, 0);}
        | AND REG COMMA REG {fillTwoAdr(section, 0x81, $2, $4);}
        | OR REG COMMA REG {fillTwoAdr(section, 0x82, $2, $4);}
        | XOR REG COMMA REG {fillTwoAdr(section, 0x83, $2, $4);}
        | TEST REG COMMA REG {fillTwoAdr(section, 0x84, $2, $4);}
        ;

move_op: SHL REG COMMA REG {fillTwoAdr(section, 0x90, $2, $4);}
        | SHR REG COMMA REG {fillTwoAdr(section, 0x91, $2, $4);}

more_adr: CALL operandJump {changeInstrDesc(section, instrDesc, 0x30);}
        | jump_op 
        | LDR operandData1 {changeInstrDesc(section, instrDesc, 0xa0);}
        | LDR operandData2 {changeInstrDesc(section, instrDesc, 0xa0);}
        | PUSH REG {
            fillContent(section, 0xb0);
            locCnt++;
            fillContent(section, (($2 << 4) & 0xf0) | 0x8);
            locCnt++;
            fillContent(section, 0x12);
            locCnt++;
        }
        | POP REG {
            fillContent(section, 0xa0);
            locCnt++;
            fillContent(section, (($2 << 4) & 0xf0) | 0x8);
            locCnt++;
            fillContent(section, 0x42);
            locCnt++;
        }
        | STR operandData2 {changeInstrDesc(section, instrDesc, 0xb0);}
        ;
    
jump_op: JMP operandJump {changeInstrDesc(section, instrDesc, 0x50);}
        | JEQ operandJump {changeInstrDesc(section, instrDesc, 0x51);}
        | JNE operandJump {changeInstrDesc(section, instrDesc, 0x52);}
        | JGT operandJump {changeInstrDesc(section, instrDesc, 0x53);}
        ;

operandJump: number {
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, 0xf0);
                locCnt++;
                fillContent(section, 0x00);
                locCnt++;
                fillContent(section, ($1 >> 8) & 0xff);
                locCnt++;
                fillContent(section, $1 & 0xff);
                locCnt++;
            }
            | IDENTIFIER {
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, 0xf0);
                locCnt++;
                fillContent(section, 0x00);
                locCnt++;
                addRecord(section, locCnt, $1, RelType::instrAbs);
                fillContent(section, 0x00);
                locCnt++;
                fillContent(section, 0x00);
                locCnt++;
            }
            | PERC IDENTIFIER{
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, 0xf7);
                locCnt++;
                fillContent(section, 0x05);
                locCnt++;
                addRecord(section, locCnt, $2, RelType::instrRel);
                fillContent(section, 0x00);
                locCnt++;
                fillContent(section, 0x00);
                locCnt++;
            }
            | STAR number{
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, 0xf0);
                locCnt++;
                fillContent(section, 0x04);
                locCnt++;
                fillContent(section, ($2 >> 8) & 0xff);
                locCnt++;
                fillContent(section, $2 & 0xff);
                locCnt++;
            }
            | STAR IDENTIFIER{
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, 0xf0);
                locCnt++;
                fillContent(section, 0x04);
                locCnt++;
                addRecord(section, locCnt, $2, RelType::instrAbs);
                fillContent(section, 0x00);
                locCnt++;
                fillContent(section, 0x00);
                locCnt++;
            }
            | STAR REG{
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, 0xf0 | ($2 & 0xf));
                locCnt++;
                fillContent(section, 0x01);
                locCnt++;
            }
            | STAR OPEN_P REG CLOSED_P{
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, 0xf0 | ($3 & 0xf));
                locCnt++;
                fillContent(section, 0x02);
                locCnt++;

            }
            | STAR OPEN_P REG PLUS number CLOSED_P{
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, 0xf0 | ($3 & 0xf));
                locCnt++;
                fillContent(section, 0x03);
                locCnt++;
                fillContent(section, ($5 >> 8) & 0xff);
                locCnt++;
                fillContent(section, $5 & 0xff);
                locCnt++;
            }
            | STAR OPEN_P REG PLUS IDENTIFIER CLOSED_P{
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, 0xf0 | ($3 & 0xf));
                locCnt++;
                fillContent(section, 0x03);
                locCnt++;
                addRecord(section, locCnt, $5, RelType::instrAbs);
                fillContent(section, 0x00);
                locCnt++;
                fillContent(section, 0x00);
                locCnt++;
            }
            ;

operandData1: REG COMMA DOLLAR number {
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, (($1 << 4) & 0xf0));
                locCnt++;
                fillContent(section, 0x00);
                locCnt++;
                fillContent(section, ($4 >> 8) & 0xff);
                locCnt++;
                fillContent(section, $4 & 0xff);
                locCnt++;
            }
            | REG COMMA DOLLAR IDENTIFIER {
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, (($1 << 4) & 0xf0));
                locCnt++;
                fillContent(section, 0x00);
                locCnt++;
                addRecord(section, locCnt, $4, RelType::instrAbs);
                fillContent(section, 0x00);
                locCnt++;
                fillContent(section, 0x00);
                locCnt++;
            }
            ;

operandData2: REG COMMA number {
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, (($1 << 4) & 0xf0));
                locCnt++;
                fillContent(section, 0x04);
                locCnt++;
                fillContent(section, ($3 >> 8) & 0xff);
                locCnt++;
                fillContent(section, $3 & 0xff);
                locCnt++;
            }
            | REG COMMA IDENTIFIER {
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, (($1 << 4) & 0xf0));
                locCnt++;
                fillContent(section, 0x04);
                locCnt++;
                addRecord(section, locCnt, $3, RelType::instrAbs);
                fillContent(section, 0x00);
                locCnt++;
                fillContent(section, 0x00);
                locCnt++;
            }
            | REG COMMA PERC IDENTIFIER {
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, (($1 << 4) & 0xf0) | 0x7);
                locCnt++;
                fillContent(section, 0x03);
                locCnt++;
                addRecord(section, locCnt, $4, RelType::instrRel);
                fillContent(section, 0x00);
                locCnt++;
                fillContent(section, 0x00);
                locCnt++;
            }
            | REG COMMA REG {
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, (($1 << 4) & 0xf0) | ($3 & 0x0f));
                locCnt++;
                fillContent(section, 0x01);
                instrDesc = locCnt++;
            }
            | REG COMMA OPEN_P REG CLOSED_P {
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, (($1 << 4) & 0xf0) | ($4 & 0x0f));
                locCnt++;
                fillContent(section, 0x02);
                instrDesc = locCnt++;
            }
            | REG COMMA OPEN_P REG PLUS number CLOSED_P {
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, (($1 << 4) & 0xf0) | ($4 & 0x0f));
                locCnt++;
                fillContent(section, 0x03);
                locCnt++;
                fillContent(section, ($6 >> 8) & 0xff);
                locCnt++;
                fillContent(section, $6 & 0xff);
                locCnt++;
            }
            | REG COMMA OPEN_P REG PLUS IDENTIFIER CLOSED_P {
                fillContent(section, 0x00);
                instrDesc = locCnt++;
                fillContent(section, (($1 << 4) & 0xf0) | ($4 & 0x0f));
                locCnt++;
                fillContent(section, 0x03);
                locCnt++;
                addRecord(section, locCnt, $6, RelType::instrAbs);
                fillContent(section, 0x00);
                locCnt++;
                fillContent(section, 0x00);
                locCnt++;
            }
            ;
%%

void yyerror(const char* s) {
	fprintf(stderr, "Parse error: %s %d\n", s, yylineno);
	exit(1);
}