%{
	#include "syntax.tab.h"
	#include "create_gramm_tree.h"
	int is_right = 1;
%}

%option yylineno

digit [0-9]
INT [+-]?([1-9]{digit}*)|0
FLOAT [+-]?({digit}+\.{digit}+)
letter_ [_a-zA-Z]
ID {letter_}({letter_}|{digit})*
SEMI ;
COMMA ,
ASSIGNOP =
RELOP >|<|>=|<=|==|!=
PLUS +
MINUS -
STAR *
DIV /
AND \&\&
OR \|\|
DOT \.
NOT \!
TYPE int|float
LP \(
RP \)
LB \[
RB \]
LC \{
RC \}
STRUCT struct
RETURN return
IF if
ELSE else
WHILE while
ANNOTATION \/\/
ANNOTATIONS \/\*
delim [ \t\r\n]
ws {delim}+
%%
{ws} {}
"+" {yylval.node = create_node("PLUS",0,yylineno); return PLUS;}
"-" {yylval.node = create_node("MINUS",0,yylineno); return MINUS;}
"*" {yylval.node = create_node("STAR",0,yylineno); return STAR;}
"/" {yylval.node = create_node("DIV",0,yylineno); return DIV;}
{INT} {yylval.node = create_node("INT",0,yylineno); return INT;}
{FLOAT} {yylval.node = create_node("FLOAT",0,yylineno); return FLOAT;}
{TYPE} {yylval.node = create_node("TYPE",0,yylineno);return TYPE;}
{LP} {yylval.node = create_node("LP",0,yylineno); return LP;}
{RP} {yylval.node = create_node("RP",0,yylineno); return RP;}
{LB} {yylval.node = create_node("LB",0,yylineno); return LB;}
{RB} {yylval.node = create_node("RB",0,yylineno); return RB;}
{LC} {yylval.node = create_node("LC",0,yylineno); return LC;}
{RC} {yylval.node = create_node("RC",0,yylineno); return RC;}
{ASSIGNOP} {yylval.node = create_node("ASSIGNOP",0,yylineno); return ASSIGNOP;}
{RELOP} {yylval.node = create_node("RELOP",0,yylineno); return RELOP;}
{AND} {yylval.node = create_node("AND",0,yylineno); return AND;}
{OR} {yylval.node = create_node("OR",0,yylineno); return OR;}
{DOT} {yylval.node = create_node("DOT",0,yylineno); return DOT;}
{NOT} {yylval.node = create_node("NOT",0,yylineno); return NOT;}
{STRUCT} {yylval.node = create_node("STRUCT",0,yylineno); return STRUCT;}
{RETURN} {yylval.node = create_node("RETURN",0,yylineno); return RETURN;}
{WHILE} {yylval.node = create_node("WHILE",0,yylineno); return WHILE;}
{SEMI} {yylval.node = create_node("SEMI",0,yylineno); return SEMI;}
{COMMA} {yylval.node = create_node("COMMA",0,yylineno); return COMMA;}
{IF} {yylval.node = create_node("IF",0,yylineno); return IF;}
{ELSE} {yylval.node = create_node("ELSE",0,yylineno); return ELSE;}
{ID} {yylval.node = create_node("ID",0,yylineno); return ID;}
{ANNOTATION} {char ch = input(); while(ch != '\n') ch = input();}
{ANNOTATIONS} {char ch1 = input();while(1){while(ch1 != '*') ch1 = input();ch1 = input();if(ch1 == '/') break;else ch1 = input();}}
. {is_right = 0;printf("Error type A at Line %d: Undefined Character '%s'\n",yylineno,yytext);}

%%