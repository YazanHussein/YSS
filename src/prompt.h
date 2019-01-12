#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <pwd.h>
#include <unistd.h>
#include "colors.h"
char *repl(char *str, char *orig, char *rep)
{
   char buffer[4096];
  char *p;

  if(!(p = strstr(str, orig)))  // if orig is not in str stop
    return str;

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  str=buffer;

  return str;
}

int lengthhh;

void prompt() {
    char cwd[1024];
    char UserName[64];
    struct utsname unameData;

    uname(&unameData);
    getcwd(cwd,sizeof(cwd));
    struct passwd *pw =getpwuid(getuid());//or by env var HOME homedir=getenv("HOME")
    const char *homedir =pw->pw_dir;
    const char *username=pw->pw_name;
    const char *usershell=pw->pw_shell;

    char *result = malloc(strlen("/home/")+strlen(username)+1);//+1 for the zero-terminator

    strcpy(result, "/home/");
    strcat(result, username);

    char * asd = malloc(200);
    strcpy(asd, username);
    strcat(asd, "@");
    strcat(asd, unameData.nodename);
    strcat(asd, ":");
    strcat(asd, repl(cwd,result,"~"));
    strcat(asd, "$ ");
    lengthhh = (int)strlen(asd);

    paint();
    printf(BOLD "%s%s@%s"KYEL":%s"KNRM"$ ",color,username, unameData.nodename,repl(cwd,result,"~"));
}
