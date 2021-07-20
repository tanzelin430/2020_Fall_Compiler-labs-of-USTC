%option noyywrap
%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>

#include "lexical_analyzer.h"

#define TRUE 1
#define FALSE 0

#define MAXRESERVED 6 //关键字最大程度
#define MAXTOKENLEN 40 //标识符最大长度
int lines;
int pos_start;
int pos_end;

/*****************声明和选项设置  end*****************/

%}

blank			[ ]
whitespace      {blank}+
table			[\t]
EOL				[\n]
uc_letter       [A-Z]
lc_letter       [a-z]
letter          {lc_letter}|{uc_letter}
digit           [0-9]
ID				({letter}|\_)({letter}|\_|{digit})*
INTEGER         {digit}+
FLOAT			({digit}+"\."|{digit}*"\."{digit}+)
symble			[+-*<>=!;,()[]{}]
over			[/] /* 因为有可能和注释混淆 */
other			{letter}|{digit}|{symble}|{over}
other_1			[^\*]
other_2			[^/\*]
star			[\*]
backslash		[/]
other_3			{star}+{other_1}{other_2}*
COMMENT			{backslash}{star}{other_1}*{other_3}*{star}+{backslash}
%%

 /******************TODO*********************/
 /****请在此补全所有flex的模式与动作  start******/
 //STUDENT TO DO

(\+)			{ return ADD; }
(\-)			{ return SUB; }
(\*)				{ return MUL; }
(\/)			{ return DIV; }
(\<)			{ return LT; }
(\<=)			{ return LTE; }
(\>)			{ return GT; }
(\>=)			{ return GTE; }
(==)			{ return EQ; }
(!=)			{ return NEQ; }
(=)				{ return ASSIN; }
(;)				{ return SEMICOLON; }
(,)				{ return COMMA; }
(\()			{ return LPARENTHESE; }
(\))			{ return RPARENTHESE; }
(\[)			{ return LBRACKET; }
(\])			{ return RBRACKET; }
(\[\])			{ return ARRAY; }
(\{)			{ return LBRACE; }
(\})			{ return RBRACE; }
("else")		{ return ELSE; }
(if)			{ return IF; }
("int")			{ return INT; }
("float")		{ return FLOAT; }
(return)		{ return RETURN; }
(void)			{ return VOID; }
(while)			{ return WHILE; }
{blank}			{ return BLANK; }
{table}			{ return BLANK; }
{EOL}			{ return EOL; }
{INTEGER}		{ return INTEGER; }
{FLOAT}			{ return FLOATPOINT; }
{COMMENT}		{ return COMMENT; }
{ID}			{ return IDENTIFIER; }
. {return ERROR;}




 /****请在此补全所有flex的模式与动作  end******/
%%

/****************C代码 start*************/

/// \brief analysize a *.cminus file
///
/// \param input_file, 需要分析的文件路径
/// \param token_stream, Token_Node结构体数组，用于存储分析结果，具体定义参考lexical_analyer.h

void analyzer(char* input_file, Token_Node* token_stream){
    lines = 1;
    pos_start = 1;
    pos_end = 1;
    if(!(yyin = fopen(input_file,"r"))){
        printf("[ERR] No input file\n");
        exit(1);
    }
    printf("[START]: Read from: %s\n", input_file);

    int token;
    int index = 0;

	while(token = yylex()){
		switch(token){
			case COMMENT:
				//STUDENT TO DO
				for (int cnt = 0; cnt < yyleng; cnt++){
					if ((yytext[cnt] == '\n')){
						pos_start = 1;
						pos_end = 1;
						lines++;
					}
					else{
						pos_end++;
						pos_start = pos_end;
					}
				}
				break;
			case BLANK:
				//STUDENT TO DO
				pos_end += yyleng;
				pos_start = pos_end;
				break;
			case EOL:
				//STUDENT TO DO
				lines++;
				pos_start = 1;
				pos_end = 1;
				break;
			case ERROR:
				printf("[ERR]: unable to analysize %s at %d lines, from %d to %d\n", yytext, lines, pos_start, pos_end);
			default :
				if (token == ERROR){
					sprintf(token_stream[index].text, "[ERR]: unable to analysize %s at %d lines, from %d to %d", yytext, lines, pos_start, pos_end);
				} else {
					strcpy(token_stream[index].text, yytext);
				}
				pos_end = pos_start + yyleng;
				token_stream[index].token = token;
				token_stream[index].lines = lines;
				token_stream[index].pos_start = pos_start;
				token_stream[index].pos_end = pos_end;
				index++;
				if (index >= MAX_NUM_TOKEN_NODE){
					printf("%d has too many tokens (> %d)", input_file, MAX_NUM_TOKEN_NODE);
					exit(1);
				} else {
				}
				pos_start = pos_end;
		}
	}
	printf("[END]: Analysis completed.\n");
	return;
}



/****************C代码 end*************/
