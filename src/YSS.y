%{
//C declerations used in actions
int yylex();
void yyerror(char const *s);
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE yy_scan_string( const char* yy_str );
void yy_switch_to_buffer( YY_BUFFER_STATE new_buffer );
void yy_delete_buffer( YY_BUFFER_STATE b );
void removeSubstring(char *s,const char *toremove);

#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include "command.h"
#include "prompt.h"
#include "execute.h"
#include "input.h"
#include "init.h"
#include "save.h"

%}

//Yacc definitions
%define parse.error verbose
%start  begin
%token  PIPE "|"
%token  NOTOKEN "this token"
%token  IO
%token  IOR
%token  NEWLINE
%token  WORD
%token  AMPERSAND "&"
%token  OPTION
%token  OPTION2

%union
{
    int intValue;
    char *str;
}

%% //Productions Rules

begin: q00 begin | /*empty*/

q00: NEWLINE {return 0;} | cmd q1 q0 | error;

q0: NEWLINE {return 1;} | PIPE q00 {clrcont;};

q1: option q2 | option option q2 | arg_list q3 | io_modifier q4 | background q5 | io_descr q3 | /*empty*/ {InsertNode(); clrcont();};

q2: arg_list q3 | io_modifier q4 | io_descr q3 | background q5 | /*empty*/ {InsertNode(); clrcont();};

q3: io_modifier q4 | io_descr q3 | background q5 | /*empty*/ {InsertNode(); clrcont();};

q4: file q3 ;

cmd: WORD {cmad.cmd = yylval.str;};

arg_list: arg | arg arg_list;

arg: WORD {insertArgNode(yylval.str);};

file: WORD {io_red(yylval.str);};

io_modifier: IO {cmad.op=yylval.str;};

io_descr: IOR {cmad.op=yylval.str;};

option: OPTION {cmad.opt = yylval.str;} | OPTION2 {cmad.opt2 = yylval.str;};

background: AMPERSAND {bg = '1';};

q5: /*empty*/{InsertNode(); clrcont();};

%%

volatile int terminate =0;

void sig_int(int sig){
    printf("\n");

    terminate = 1;
}

//C Code here
int main(void){
    //reads the tokens from yylex()
    char* str = (char *)malloc(1024);
    int status=1;
    status = initAlias();
    initEnvVars();
    initHistory();

    while(status){
        system("/bin/stty intr ^-");

        prompt();

        if( !readInput(str) ){
            break;
        }

        system("/bin/stty intr ^C");
        signal(SIGINT, sig_int);
        short done = 1;

        str = strtok(str,"\n");

        char line[1024] = {0};
        char* token = strtok(str, " ");

        while( token != NULL) {
            if(strcmp(token,isAliased(token)) != 0 && done == 1) {
                strcat(line,isAliased(token));
                strcat(line," ");
            }
            else {
                strcat(line,token);
                strcat(line," ");
            }

            token = strtok(NULL, " ");
            done = 0;
        }

        if(line[0] != 0)
            strncpy(str, line, 1024);

        if(str == NULL)
            str = "\n";
        else
            strncat(str,"\n", 2);

        YY_BUFFER_STATE b = yy_scan_string(str);
        yy_switch_to_buffer(b);

        if(yyparse()) {
            status = executetb();
            clrtbl();
        }

        yy_delete_buffer(b);

        str = realloc(NULL, 1024);
        for(int i=0; i<1024; i++)
            str[i] = 0;

    }

    saveVars();
    saveHistory();

    system("/bin/stty sane");

    return 0;
}

void yyerror (char const *s) {
    char err[64];
    strncpy(err,s,64);

    if(strstr(err,"expecting") != NULL) {
        strcpy(err,strstr(err,"expecting"));
        removeSubstring(err,"$end or");
    }
    else
        strcpy(err,"");

    printf("Syntax error: unexpected %s %s\n",yylval.str,err);
}

void removeSubstring(char *s,const char *toremove) {
  while( s=strstr(s,toremove) )
    memmove(s,s+strlen(toremove),1+strlen(s+strlen(toremove)));
}
