%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"

int files_count;
int lines;
int pos_start;
int pos_end;

void pass_node(char *text){
     yylval.node = new_syntax_tree_node(text);
}
%}

 /***************
 TO STUDENTS: Copy your Lab1 here. Make adjustments if necessary.

 Note: don't modify the prologue unless you know what you are doing.
 ***************/

%%

 /******************TODO*********************/
 /****请在此补全所有flex的模式与动作  start******/
 //STUDENT TO DO
\+  {pos_start = pos_end; pos_end += 1; pass_node(yytext); return ADD;}
\-  {pos_start = pos_end; pos_end += 1; pass_node(yytext); return SUB;}
\*  {pos_start = pos_end; pos_end += 1; pass_node(yytext); return MUL;}
\/  {pos_start = pos_end; pos_end += 1; pass_node(yytext); return DIV;}
\<  {pos_start = pos_end; pos_end += 1; pass_node(yytext); return LT;}
\<= {pos_start = pos_end; pos_end += 2; pass_node(yytext); return LTE;}
\>  {pos_start = pos_end; pos_end += 1; pass_node(yytext); return GT;}
\>= {pos_start = pos_end; pos_end += 2; pass_node(yytext); return GTE;}
==  {pos_start = pos_end; pos_end += 2; pass_node(yytext); return EQ;}
!=  {pos_start = pos_end; pos_end += 2; pass_node(yytext); return NEQ;}
=   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return ASSIN;}
;   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return SEMICOLON;}
,   {pos_start = pos_end; pos_end += 1; pass_node(yytext); return COMMA;}
\(  {pos_start = pos_end; pos_end += 1; pass_node(yytext); return LPARENTHESE;}
\)  {pos_start = pos_end; pos_end += 1; pass_node(yytext); return RPARENTHESE;}
\{  {pos_start = pos_end; pos_end += 1; pass_node(yytext); return LBRACE;}
\}  {pos_start = pos_end; pos_end += 1; pass_node(yytext); return RBRACE;}
\[  {pos_start = pos_end; pos_end += 1; pass_node(yytext); return LBRACKET;}
\]  {pos_start = pos_end; pos_end += 1; pass_node(yytext); return RBRACKET;}
else    {pos_start = pos_end; pos_end += 4; pass_node(yytext); return ELSE;}
if  {pos_start = pos_end; pos_end += 2; pass_node(yytext); return IF;}
int {pos_start = pos_end; pos_end += 3; pass_node(yytext); return INT;}
float   {pos_start = pos_end; pos_end += 5; pass_node(yytext); return FLOAT;}
return  {pos_start = pos_end; pos_end += 6; pass_node(yytext);  return RETURN;}
void    {pos_start = pos_end; pos_end += 4; pass_node(yytext); return VOID;}
while   {pos_start = pos_end; pos_end += 5; pass_node(yytext); return WHILE;}
[a-zA-Z]+   {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return IDENTIFIER;}
[0-9]+  {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return INTEGER;}
([0-9]*\.[0-9]+|[0-9]+\.)   {pos_start = pos_end; pos_end += strlen(yytext); pass_node(yytext); return FLOATPOINT;}
\[\]    {pos_start = pos_end; pos_end += 2; pass_node(yytext); return ARRAY;}
\n  {lines++; pos_start = 1; pos_end = 1;}
[ \t]   {pos_start = pos_end; pos_end += 1;}
\/\*([^\*]|(\*)*[^\*/])*(\*)*\*\/   {
                                        pos_start = pos_end;
                                        int num = count_num_enter(yytext);  //check if the commnet is the cross-comment
                                        if ( num != 0 ) {
                                            lines += num;           //cross comment, update lines and pos_end
                                            pos_end = strlen(strrchr(yytext, '\n'));
                                        } else {                    //one line comment, only update pos_end
                                            pos_end += strlen(yytext);
                                        }
                                    }

. {return ERROR;}





 /****请在此补全所有flex的模式与动作  end******/
%%
/****************C代码 start*************/


int count_num_enter(char *string) {
    int ret = 0;
    int index = 0;
    while(string[index] != '\0') {
        if(string[index] == '\n')
            ret++;
        index++;
    }
    pass_node(yytext); return ret;
}


/* Example for you :-) */
// \+  { pos_start = pos_end; pos_end += 1; pass_node(yytext); pass_node(yytext); return ADD; }
