/*
* Lex file for YSS v1.3
*
*/

%{

#include <string.h>
#include "trim.h"
#include "y.tab.h"
char* trimQoute(char *str);

%}

%%

\n      {
                return NEWLINE;
        }


[ ]     {
               /* discard spaces */
        }

"#"   {
                yylval.str = strdup(trim(yytext));
                return IO;
        }

[ 1]?">"   {
                yylval.str = strdup(trim(yytext));
                return IO;
        }

"<"[0 ]?     {
                yylval.str = strdup(trim(yytext));
                return IO;
        }

"#>"    {
                yylval.str = strdup(trim(yytext));
                return IO;
        }

[ 1]?">>"    {
                yylval.str = strdup(trim(yytext));
                return IO;
        }

[ 1-2]">&"[1-2 ]    {
                yylval.str = strdup(trim(yytext));
                return IOR;
        }

"|"     {
                yylval.str = strdup(yytext);
                return PIPE;
        }

"&"     {
                return AMPERSAND;
        }

[ ]"-"[a-zA-Z0-9]* {
                yylval.str = strdup(trim(yytext));
                return OPTION;
        }

[ ]"--"[a-zA-Z=a-zA-Z]* {
                yylval.str = strdup(trim(yytext));
                return OPTION2;
}

"\""[\%\=\+"\'\(\)\/\_\-\.\?\*\~a-zA-Z0-9 ]+"\"" {
                yylval.str = strdup(trimQoute(yytext));
                return WORD;
       }


[\%\=\+"\'\(\)\$\/\_\-\.\?\*\~a-zA-Z0-9]+ {
                yylval.str = strdup(yytext);
                return WORD;
       }


.|\n  {
                return NOTOKEN;
      }

%%

int yywrap (void) {
    return 1;
}
