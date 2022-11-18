%{
#include "parser.tab.h"
%}
%option yylineno
%%  
#.*                              	;
\.global                            return GLOBAL;
\.extern                            return EXTERN;
\.section                           return SECTION;
\.word                              return WORD;
\.skip                              return SKIP;
\.equ                               return EQU;
\.end                               return END;
halt                                return HALT;
int                                 return INT;
iret                                return IRET;
call                                return CALL;
ret                                 return RET;
jmp                                 return JMP;
jeq                                 return JEQ;
jne                                 return JNE;
jgt                                 return JGT;
push                                return PUSH;
pop                                 return POP;
xchg                                return XCHG;
add                                 return ADD;
sub                                 return SUB;
mul                                 return MUL;
div                                 return DIV;
cmp                                 return CMP;
not                                 return NOT;
and                                 return AND;
or                                  return OR;
xor                                 return XOR;
test                                return TEST;
shl                                 return SHL;
shr                                 return SHR;
ldr                                 return LDR;
str                                 return STR;
(r[0-7])|(psw)|(pc)|(sp)            {char * r = strdup(yytext); 
									if(strcmp(r,"psw") == 0){
										yylval.num = 8;
									}else if(strcmp(r,"pc") == 0){
										yylval.num = 7;
									}else if(strcmp(r,"sp") == 0){
										yylval.num = 6;
									}else{
										yylval.num = atoi(&(r[1]));
									} 
									return REG;
									}
\$                                  return DOLLAR;
%                                   return PERC;
\[                                  return OPEN_P;
\]                                  return CLOSED_P;
\+                                  return PLUS;
\*                                  return STAR;
,                                   return COMMA;
:									return COLON;
[a-zA-Z][_a-zA-Z0-9]*               {yylval.name = strdup(yytext); return IDENTIFIER;}
0[xX][0-9A-Fa-f]+                   {yylval.num = strtol(strdup(yytext), nullptr, 16); return HEX_NUM;}
([0-9])|([1-9][0-9]*)               {yylval.num = atoi(strdup(yytext)); return DEC_NUM;}
\n                                  return NEW_LINE;
[ \t]                               ;
.					                return SYNTAX_ERROR;
%%

int yywrap(void)
{
	return 1;
}