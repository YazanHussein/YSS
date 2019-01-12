#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include "strmode.h"

#define aliases "./.aliases"
#define commands "./.commands"

//struct to save aliases of commands in this shell session
typedef struct aliased{
    char* command;
    char* alias;
    struct aliased *next;
}aliased;

//this struct contains the local variables
typedef struct variables{
    char* name;
    char* value;
    struct variables* next;
}variables;

int YSS_alias(char* argv[], int argnum);
int YSS_unalias(char* argv[], int argnum);
int YSS_cd(char* argv[], int argnum);
int YSS_help();
int YSS_echo(char *argv[], int argnum);
int YSS_exit();
int YSS_export(char *argv[], int argnum);
int YSS_umask(char *argv[], int argnum);
int YSS_unset(char *argv[]);
int YSS_unexport(char *argv[]);
int YSS_up(char *argv[], int argnum);
int YSS_assignLocalVar(char *argv[]);
int YSS_forbid();

void qouteSubstring(char *string, int start, int end);
char* trimQoute(char *str);
void insertAlias(char *command, char* alias);
void deleteAlias(char *als);
char* get_current_dir_name();
int checkNonDigitOrLetter(char * string);
void assignEnvVar(char *name);
int deleteNode(variables *tmpvar, char vartype, char * name);


aliased *head = NULL;
variables *localvars = NULL; //linkedlist of local vars
variables *envvars = NULL;

//array to save the names of the builtin commands
char *builtins[] = {
  "alias",
  "cd",
  "echo",
  "exit",
  "export",
  "forbid",
  "help",
  "unalias",
  "umask",
  "unset",
  "unexport",
  "up"
};

// method to check if this command has an alias or not
char* isAliased(char* cmd) {
    aliased *ptr = head;
    while(ptr != NULL) {
        if(strcmp(ptr -> alias,cmd) == 0) {
            return ptr -> command;
        }
        ptr = ptr -> next;
    }
    return cmd;
}

// method to check if this command is builtin
char* isBuiltin(char *cmd) {
    for (int i = 0; i < (sizeof(builtins)/sizeof(builtins[0])); i++) {
        if (strcmp(cmd, builtins[i]) == 0) {
        return cmd;
        }
    }
    return NULL;
}

// method to call a specific builtin command
int callBuiltin(char* cmd, char* argv[], int argvSize) {
    if (strcmp(cmd, "cd") == 0) {
        return YSS_cd(argv, (argvSize-4));
    }
    else
    if (strcmp(cmd, "up") == 0) {
        return YSS_up(argv,argvSize);
    }
    else
    if(strcmp(cmd, "exit") == 0) {
        return YSS_exit();
    }
    else
    if(strcmp(cmd, "help") == 0) {
        return YSS_help();
    }
    else
    if(strcmp(cmd, "forbid") == 0) {
        return YSS_forbid();
    }
    else
    if(strcmp(cmd, "alias") == 0) {
        return YSS_alias(argv,(argvSize-4));
    }
    else
    if(strcmp(cmd, "unalias") == 0) {
        return YSS_unalias(argv,(argvSize-4));
    }
    else
    if(strcmp(cmd, "export") == 0) {
        return YSS_export(argv,(argvSize-2));
    }
    else
    if(strcmp(cmd, "echo") == 0) {
        return YSS_echo(argv,argvSize-4);
    }
    else
    if(strcmp(cmd, "umask") == 0) {
        return YSS_umask(argv,(argvSize-4));
    }
    else
    if(strcmp(cmd, "unset") == 0) {
        return YSS_unset(argv);
    }
    else
    if(strcmp(cmd, "unexport") == 0) {
        return YSS_unexport(argv);
    }
    return 1;
}

//command to forbid of deleting everything
int YSS_forbid() {
    printf("This command is forbidden, because it will delete everything\n");
    return 1;
}

//delete environment var
int YSS_unexport(char *argv[]){
    if(argv[1] == NULL){
        printf("Usage: unexport [name ...]\n");
        return 1;
    }
    if(strcmp(argv[1], "--help") == 0){
        printf("Usage: unexport [name ...]\n");
        printf("This command can be used to remove one or mutiple environment variables.\n\n");
        printf("Available options: -\n");
        printf("  --help\tTo show the help menu of this command\n");
        return 1;
    }

    int i=1;
    int existingVariable = 0;

    while(argv[i] != NULL){
        if(getenv(argv[i]) == NULL){
            printf("No environment variable with the name '%s'\n", argv[i]);
            argv[i] = NULL;
        }
        else{
            unsetenv(argv[i]);
            if(!existingVariable)
                existingVariable = 1;
        }
        i++;
    }

    if(existingVariable){
        char input[5] = {0};

        printf("Do you want to save changes permanently [y/n]? ");
        fgets(input, 4, stdin);
        char * newline = strstr(input, "\n");
        if(newline != NULL)
            *newline = '\0';
        else
            while(getchar() != '\n');

        if(strcmp(input, "y") == 0 || strcmp(input, "yes") == 0){
            variables *tmpvar = envvars;
            for(int j=1; j<i; j++){
                if(argv[j] != NULL){
                    if(deleteNode(tmpvar, 'e', argv[j]) == 0)
                        printf("No environment variable with the name '%s'\n", argv[j]);
                }
            }
        }
    }

    return 1;
}


int YSS_up(char *argv[], int argnum){

    if(argv[1]==NULL)
    {
        printf("Usage : up 'number' or '-' to go back to previous dir\n");
        printf("For example: up 5\n");
        return 1;
    }
    if(strcmp(argv[1],"--help")==0){
        printf("up moves n directories upwards\n");
        printf("Usage : up 'number' or '-' to go back to previous dir\n");
        printf("For example: up 5\n");
    }
    if(argv[1][0]!='-'){
//        int num=(int)argv[1][0]-'0';
        int num = atoi(argv[1]);
        char* dir ="..";

        //string to hold the old working directory
        char *oldpwd = malloc(2048 * sizeof(char));
        strcpy(oldpwd,"");

        strcpy(oldpwd,getenv("PWD"));
        setenv("OLDPWD",oldpwd,1);

        free(oldpwd);

        while(num != 0){
            //string to hold the current working directory
            char *pwd = malloc(2048 * sizeof(char));
            strcpy(pwd,"");

            // while(num!=0){
            if (chdir(dir) != 0) {
                // if the directory not exist
                perror("Error");
            }

            strcpy(pwd,get_current_dir_name());
            setenv("PWD",pwd,1);

            num--;

            free(pwd);
        }
    }
    else{
        //string to hold the current working directory
        char *pwd = malloc(2048 * sizeof(char));
        strcpy(pwd,"");

        //string to hold the old working directory
        char *oldpwd = malloc(2048 * sizeof(char));
        strcpy(oldpwd,"");

        strcpy(oldpwd,getenv("PWD"));

        //  cd to previous
        chdir(getenv("OLDPWD"));

        setenv("OLDPWD",oldpwd,1);

        strcpy(pwd,get_current_dir_name());
        setenv("PWD",pwd,1);

        free(pwd);
        free(oldpwd);
    }
    return 1;
}

//builtin command to delete local var
int YSS_unset (char *argv[]){
    if(argv[1] == NULL){
        printf("Usage: unset [name ...]\n");
        return 1;
    }
    if(strcmp(argv[1], "--help") == 0){
        printf("Usage: unset [name ...]\n");
        printf("This command can be used to remove one or mutiple local variables.\n\n");
        printf("Available options: -\n");
        printf("  --help\tTo show the help menu of this command\n");
        return 1;
    }

    int i=1;
    while(argv[i] != NULL){
        variables *tmpvar = localvars;
        if(deleteNode(tmpvar, 'l', argv[i]) == 0)
            printf("No local variable with the name '%s'\n", argv[i]);
        i++;
    }
    return 1;
}

//tmpvar: pointer to the linkedlist, vartype: local or env var, name: name of var
int deleteNode(variables *tmpvar, char vartype, char * name){
    if(tmpvar != NULL){ //ADDED BY ME
        //delete it if it is at first, which you have to know if it local or env (by vartype)
        if(strcmp(tmpvar->name, name) == 0){
                if(vartype == 'l')
                    localvars = localvars->next;
                else
                    envvars = envvars->next;
        }
        else{ //delete if it is between the first and the last node
            variables *tmpvarNext = tmpvar->next;

            if(tmpvarNext != NULL){
                short isFound = 0;
                while(tmpvarNext->next != NULL){
                   if(strcmp(tmpvarNext->name, name) == 0){
                        isFound = 1;
                        tmpvar->next = tmpvarNext->next;
                         break;
                    }
                    tmpvar = tmpvar->next;
                    tmpvarNext = tmpvarNext->next;
                }

                if(!isFound)
                    if(strcmp(tmpvarNext->name, name) == 0){ //delete if is the last node
                        tmpvar->next = NULL;
                    }
                    else
                        return 0;
            }
            else
               return 0;
        }
    }
    else //ADDED BY ME
        return 0;

    return 1;
}

//builtin command to make the local variable an environment variable
int YSS_export(char *argv[], int argnum){
    if(argv[1] == NULL){
        printf("Usage: export [name[=value] ...]\n");
        return 1;
    }
    if(strcmp(argv[1], "--help") == 0){ //ADDED BY ME
        printf("Usage: export [name[=value] ...]\n");
        printf("This command can be used to export one or multiple local variable to the environment variables, then by default deletes the "\
            "local variable, unless providing the option -k. Also we can export a new variable (name=value).\n\n");
        printf("Available options: -\n");
        printf("  -k\t\tDo not remove the variable from the local variables (keep)\n");
        printf("  --help\tTo show the help menu of this command\n");
        return 1;
    }
    short keep = 0; //with 0 the var is removed from localvars linkedlist
    int i=1;
    if(argv[1][0] == '-'){
        if(argv[1][1] == 'k'){ //k=keep local var
            keep = 1;
            i=2;
        }
    }

    char *vars[argnum];
    vars[0] = '\0';
    int existingVariable = 0;

    while(argv[i] != NULL){
        if(strstr(argv[i], "=") == NULL){ //exporting existing var means there is no '=' in the argument of export
            variables *tmpvar = localvars;
            short isFound = 0;

            if(!checkNonDigitOrLetter(argv[i])){
                printf("No local variable with the name '%s'\n", argv[i]);
                i++;
                continue;
            }

            //keep searching until the variable is found then add it to the environment vars
            while(tmpvar != NULL){
                if (strcmp(argv[i], tmpvar->name) == 0) {
                    isFound = 1;
                    setenv(tmpvar->name, tmpvar->value, 1);
                    vars[i] = tmpvar->name;

                    if(!existingVariable)//ADDED BY ME
                        existingVariable = 1;

                    if(!keep)
                        YSS_unset((char*[]){NULL, tmpvar->name, NULL}); //delete it from the local vars

                    break;
                }
                tmpvar = tmpvar->next;
            }
            //if the variable name is not found in local vars
            if(!isFound)
                printf("No local variable with the name '%s'\n", argv[i]);
        }
        else{
            char* pointerOfEqualsSign = strstr(argv[i], "=");
            *pointerOfEqualsSign = '\0';
            if(checkNonDigitOrLetter(argv[i])){
                setenv(argv[i], pointerOfEqualsSign+1, 1);
                vars[i] = strdup(argv[i]);

                if(!existingVariable)
                        existingVariable = 1;

            }
            else
                printf("ERROR: cannot add the variable '%s'\n\tthe left hand side must be 'letter(letter|digit)*'\n", argv[i]);
        }
        i++;
    }

    vars[i] = '\0';

    if(existingVariable){//ADDED BY ME

        char input[5] = {0};

        printf("Do you want to save changes permanently [y/n]? ");
        fgets(input, 4, stdin);
        char * newline = strstr(input, "\n");
        if(newline != NULL)
            *newline = '\0';
        else
            while(getchar() != '\n');

        if(strcmp(input, "y") == 0 || strcmp(input, "yes") == 0){
            for(int i=1; i<argnum; i++){
                if(vars[i] != NULL)
                    assignEnvVar(vars[i]);
            }
        }
    }


    return 1;
}


//save permanent env vars to write them to file when exiting YSS
void assignEnvVar(char *name){
    variables *tmpvar = envvars;
    variables *lastNode = NULL;

    //check if it is exist, then edit the value
    while(tmpvar != NULL){
        if(strcmp(tmpvar->name, name) == 0){
            tmpvar->value = getenv(name);
            return;
        }
        if(tmpvar->next == NULL) //saving the last node address
            lastNode = tmpvar;

        tmpvar = tmpvar->next;
    }

    //add the var to the linkedlist
    tmpvar = malloc(sizeof(variables*));
    tmpvar->name = name;
    tmpvar->value = getenv(name);
    tmpvar->next = 0;

    if(envvars == NULL)
        envvars = tmpvar;
    else{
        lastNode->next = tmpvar; //insert at the end of the linkedlist
    }
}

//adding the var to the linked list of local variables
int YSS_assignLocalVar(char *argv[]){
    variables *tmpvar = localvars;

    //check if it is exist, then edit the value
    while(tmpvar != NULL){
        if(strcmp(tmpvar->name, argv[0]) == 0){
            tmpvar->value = argv[1];
            return 1;
        }
        tmpvar = tmpvar->next;
    }

    //add the var to the linkedlist
    tmpvar = malloc(sizeof(variables*));
    tmpvar->name = argv[0];
    tmpvar->value = argv[1];
    tmpvar->next = 0;

    if(localvars == NULL)
        localvars = tmpvar;
    else{ //add first
        tmpvar->next = localvars;
        localvars = tmpvar;
    }

    return 1;
}

//return 0 if the string does not contain numbers or digits
int checkNonDigitOrLetter(char * string){
    if(!isalpha(string[0]))
        return 0;

    int length = (int)strlen(string);
    for(int i=1; i<length; i++){
        if( !isalpha(string[i]) && !isdigit(string[i]) )
            return 0;
    }
    return 1;
}


//builtin command for aliases of any shell command
int YSS_alias(char *argv[], int argnum) {
    char* alias = NULL;
    char* command = NULL;

    if(argnum == 0) {
        if(argv[1] != NULL) {
            if(argv[1][1] == '-') {
                if(strcmp(argv[1],"--help") == 0) {
                    printf("Usage: alias ...\n");
                    printf("This command used to alias shell commands for many reasons.\n");
                    printf("For example to make the use of the command easier,\n");
                    printf(" or if you want special names for the commands\n\n");
                    printf("Available options: -\n");
                    printf("  -l, --list\t\t\tTo list all the aliased commands\n");
                    printf("  --help\t\t\tTo show the help menu of this command\n\n");
                    printf("You can use man command also\n");
                    printf("Please note this command doesn't take any arguments\n");
                    return 1;
                }
                else if(strcmp(argv[1],"--list") == 0) {
                    aliased *ptr = head;
                    while(ptr != NULL) {
                        printf("alias %s='%s'\n",ptr -> alias,ptr -> command);
                        ptr = ptr -> next;
                    }
                    return 1;
                }
                else {
                    printf("alias: Invalid option\n");
                    return 1;
                }
            }
            if(strcmp(argv[1],"-l") == 0) {
                aliased *ptr = head;
                while(ptr != NULL) {
                    printf("alias %s='%s'\n",ptr -> alias,ptr -> command);
                    ptr = ptr -> next;
                }
                return 1;
            }
            else {
                printf("alias: Invalid option\n");
                return 1;
            }
        }
        else {
                FIRST:
                    printf("Please Enter your alias name? ");

                    char* alias = (char*) malloc(50);
                    fgets(alias, 50, stdin);

                    if(strlen(alias) == 1) {
                        printf("You didn't enter anything\n");
                        goto FIRST;
                    }
                    else
                        alias = strtok(alias,"\n");

                    if(strcmp(alias,isAliased(alias)) != 0) {
                        printf("This alias already used\n");
                        printf("Do you want to try another [y/n]? ");

                        char* ans2 = malloc(3);
                        fgets(ans2,2,stdin);

                        char * newline = strstr(ans2, "\n");
                        if(newline != NULL)
                            *newline = '\0';
                        else
                            while(getchar() != '\n');

                        if(strcmp(ans2,"y") == 0) {
                            goto FIRST;
                        }
                        fseek(stdin,0,SEEK_END);
                        return 1;
                    }

                SECOND:
                    printf("Please Enter the desired command? ");

                    char* command = (char*) malloc(1024);
                    fgets(command,1024,stdin);

                    if(strlen(command) == 1) {
                        printf("You didn't enter anything\n");
                        goto SECOND;
                    }
                    else
                        command = strtok(command,"\n");

                    printf("Do you want it permanently [y/n]? ");
                    char* ans3 = (char*) malloc(3);
                    fgets(ans3,2,stdin);

                    char * newline = strstr(ans3, "\n");
                    if(newline != NULL)
                        *newline = '\0';
                    else
                        while(getchar() != '\n');

                    if(strcmp(ans3,"y") == 0) {
                        FILE* fptr = fopen(aliases, "a");
                        fprintf(fptr,"alias %s='%s'\n",alias,command);
                        fclose(fptr);
                    }

                    insertAlias(command,alias);
                    fseek(stdin,0,SEEK_END);

                    return 1;
        }
    }
    else {
        printf("Usage: alias\n");
        printf("Type alias --help for further information\n");
        return 1;
    }
}


//builtin command to change directory
int YSS_cd(char *argv[], int argnum) {
    //string to hold the current working directory
    char *pwd = malloc(2048 * sizeof(char));
    strcpy(pwd,"");

    //string to hold the old working directory
    char *oldpwd = malloc(2048 * sizeof(char));
    strcpy(oldpwd,"");

    // if no arguments entered to cd command
    if (argv[1] == NULL) {
        argv[1] = getpwuid(getuid())->pw_dir;

        strcat(oldpwd,getenv("PWD"));

        chdir(argv[1]);

        setenv("OLDPWD",oldpwd,1);

        strcpy(pwd,get_current_dir_name());
        setenv("PWD",pwd,1);

        free(pwd);
        free(oldpwd);

        return 1;
    }

    // if the user used option with cd command
    if(argv[1][0] == '-') {
        printf("%s: Invalid syntax this command doesn't work with option\n",argv[1]);
    }
    else {
        // string to hold all the path to the new directory
        char *args = malloc(2048 * sizeof(char));
        strcpy(args,"");

        // just one argument (path without spaces)
        strcat(args, argv[1]);

        // the path is with spaces
        if(argnum > 1) {
            // we edit it to be ready for chdir function
            for(int i=2; i <= argnum; i++) {
                strcat(args, " ");
                strcat(args, argv[i]);
            }
        }

        // trim the qoutes from beginning & ending
        char* dir = trimQoute(args);

        // to free from memory the string
        free(args);

        strcat(oldpwd,getenv("PWD"));

        if (chdir(dir) != 0) {
            // if the directory not exist
            printf("%s: This directory does not exist\n",dir);
        }

        setenv("OLDPWD",oldpwd,1);

        strcpy(pwd,get_current_dir_name());
        setenv("PWD",pwd,1);


        free(pwd);
        free(oldpwd);
    }
    return 1;
}

//builtin command to print the argument again or its value
int YSS_echo(char *argv[], int argnum) {
    if(argv[1] == NULL) {
        printf("echo: Nothing to print\n");
    }
    else {
        char str[1024] = {0};
        short done = 0;
        char opt = '\0';
        if(argv[1][0] == '-') {
            if(strcmp(argv[1],"-l") == 0)
                opt = 'l';
            else
            if(strcmp(argv[1],"-v") == 0)
                opt = 'v';
            else
            if(strcmp(argv[1],"-i") == 0)
                opt = 'i';
            else
            if(strcmp(argv[1],"--help") == 0) {
                printf("Usage: echo ...\n");
                printf("This command used to print again the arguments.\n");
                printf("If there is a $ sign it will substitute the value of the variable.\n");
                printf("If there is a $( sign there is a command substitution.\n");
                printf("Available options: -\n");
                printf("  -l\t\t\tTo print the local value of the variables\n");
                printf("  -v\t\t\tTo print the environment value of the variables\n");
                printf("  -i\t\t\tInteractive mode\n");
                printf("  --help\t\tTo show the help menu of this command\n\n");
                printf("You can use man command also\n");
                return 1;
            }
            else {
                printf("echo: Invalid option\n");
                return 1;
            }
        }

        if(opt != '\0') {
            if(opt == 'l') {
                for(int i=2; i < argnum+2; i++) {
                    if(argv[i][0] == '$') {
                        variables * tmpvar = localvars;
                        char* var = strstr(argv[i],"$")+1;
                        while(tmpvar != NULL) {
                            if(strcmp(var,tmpvar -> name) == 0) {
                                strcat(str,tmpvar -> value);
                                strcat(str," ");
                                done = 1;
                            }
                            tmpvar = tmpvar -> next;
                        }
                    }
                    else {
                        strcat(str,argv[i]);
                        strcat(str," ");
                    }
                }
            }

            if(opt == 'v') {
                for(int i=2; i < argnum+2; i++) {
                    if(argv[i][0] == '$') {
                        char* var = strstr(argv[i],"$")+1;
                        char* env = getenv(var);
                        if(env != NULL) {
                            strcat(str,getenv(var));
                            strcat(str," ");
                            done = 1;
                        }
                    }
                    else {
                        strcat(str,argv[i]);
                        strcat(str," ");
                   }
                }
            }

            if(opt == 'i') {
                for(int i=2; i < argnum+2; i++) {
                    if(argv[i][0] == '$') {
                        printf("Do you want local value or environment?\n");
                        printf("\ta: Local value\n");
                        printf("\tb: Environment value\n");
                        printf("Your choice ");

                        char* ans = (char*) malloc(3);
                        fgets(ans,2,stdin);
                        //ans = strtok(ans,"\n");

                        char * newline = strstr(ans, "\n");
                        if(newline != NULL)
                            *newline = '\0';
                        else
                            while(getchar() != '\n');

                        if(strcmp(ans,"a") == 0) {
                            variables * tmpvar = localvars;
                            char* var = strstr(argv[i],"$")+1;
                            while(tmpvar != NULL) {
                                if(strcmp(var,tmpvar -> name) == 0) {
                                    strcat(str,tmpvar -> value);
                                    strcat(str," ");
                                    done = 1;
                                }
                                tmpvar = tmpvar -> next;
                            }
                        }
                        else
                        if(strcmp(ans,"b") == 0) {
                            char* var = strstr(argv[i],"$")+1;
                            char* env = getenv(var);
                            if(env != NULL) {
                                strcat(str,getenv(var));
                                strcat(str," ");
                                done = 1;
                            }
                        }
                        else {
                            printf("Default choice executed (local)\n");
                            variables * tmpvar = localvars;
                            char* var = strstr(argv[i],"$")+1;
                            while(tmpvar != NULL) {
                                if(strcmp(var,tmpvar -> name) == 0) {
                                    strcat(str,tmpvar -> value);
                                    strcat(str," ");
                                    done = 1;
                                }
                                tmpvar = tmpvar -> next;
                            }
                        }
                    }
                    else {
                        strcat(str,argv[i]);
                        strcat(str," ");
                    }
                }
                fseek(stdin,0,SEEK_END);
            }
        }
        else {
            for(int i=1; i < argnum+1; i++) {
                if(argv[i][0] == '$') {
                    variables * tmpvar = localvars;
                    char* var = strstr(argv[i],"$")+1;
                    while(tmpvar != NULL) {
                        if(strcmp(var,tmpvar -> name) == 0) {
                            strcat(str,tmpvar -> value);
                            strcat(str," ");
                            done = 1;
                        }
                        tmpvar = tmpvar -> next;
                    }
                }
                else {
                    strcat(str,argv[i]);
                    strcat(str," ");
                }
            }
        }

        char* line = &str[0];
        printf("%s\n",line);

        return 1;
    }
}

// builtin command to exit the shell
int YSS_exit() {
    return 0;
}

// builting command for help menu
int YSS_help(){
  int i;
  printf("YSS Shell\n");
  printf("Version 1.3\n");
  printf("Use man command for further information\n");

  printf("Our builtin commands are: -\n");
  for (i = 0; i < (sizeof(builtins)/sizeof(builtins[0])); i++) {
    printf("  %s\n", builtins[i]);
  }

  return 1;
}


// builtin command to delete an existing alias
int YSS_unalias(char* argv[], int argnum) {
    if(argnum == 0) {
        if(argv[1] != NULL) {
            if(argv[1][1] == '-') {
                if(strcmp(argv[1],"--help") == 0) {
                    printf("Usage: unalias ...\n");
                    printf("This command used to delete existing aliases.\n");
                    printf("  -l, --list\t\t\tTo list all the aliased commands\n");
                    printf("  --help\t\t\tTo show the help menu of this command\n\n");
                    printf("You can use man command also\n");
                    printf("Please note this command doesn't take any arguments\n");
                    return 1;
                }
                else if(strcmp(argv[1],"--list") == 0) {
                    aliased *ptr = head;
                    while(ptr != NULL) {
                        printf("alias %s='%s'\n",ptr -> alias,ptr -> command);
                        ptr = ptr -> next;
                    }
                    return 1;
                }
                else {
                    printf("unalias: Invalid option\n");
                    return 1;
                }
            }
            if(strcmp(argv[1],"-l") == 0) {
                aliased *ptr = head;
                while(ptr != NULL) {
                    printf("alias %s='%s'\n",ptr -> alias,ptr -> command);
                    ptr = ptr -> next;
                }
                return 1;
            }
            else {
                printf("unalias: Invalid option\n");
                return 1;
            }
        }
        else {
                FIRST:
                    printf("Please Enter alias name to delete it? ");

                    char* alias = (char*) malloc(50);
                    fgets(alias, 50, stdin);

                    if(strlen(alias) == 1) {
                        printf("You didn't enter anything\n");
                        goto FIRST;
                    }
                    else
                        alias = strtok(alias,"\n");

                    if(strcmp(alias,isAliased(alias)) == 0) {
                        printf("This alias doesn't exist\n");

                SECOND:
                        printf("Do you want to try another [y/n]? ");

                        char* ans2 = malloc(3);
                        fgets(ans2,2,stdin);

                        char * newline = strstr(ans2, "\n");
                        if(newline != NULL)
                            *newline = '\0';
                        else
                            while(getchar() != '\n');

                        if(strcmp(ans2,"y") == 0) {
                            goto FIRST;
                        }
                        fseek(stdin,0,SEEK_END);
                        return 1;
                    }

                    if(strcmp(alias,"rm") == 0) {
                        printf("This alias can't be deleted\n");
                        goto SECOND;
                    }

                THIRD:
                    deleteAlias(alias);

                    FILE* fptr = fopen(aliases, "r");
                    char* line = NULL;
                    size_t len = 0;
                    ssize_t read;
                    short exist = 0;

                    char* als = (char*) malloc(5+strlen(alias));
                    char* file = (char*) malloc(4000);
                    file = strcpy(file,"");

                    als = strcpy(als,"alias ");
                    als = strcat(als,alias);
                    als = strcat(als,"\0");

                    while((read = getline(&line, &len, fptr)) != -1) {
                        if(strstr(line,als) == NULL)
                            file = strcat(file,line);
                        else
                            exist = 1;

                    }
                    fclose(fptr);

                    if(exist == 1) {
                        printf("Do you want to delete it permanently [y/n]? ");

                        char* ans3 = malloc(3);
                        fgets(ans3,2,stdin);

                        char * newline = strstr(ans3, "\n");
                        if(newline != NULL)
                            *newline = '\0';
                        else
                            while(getchar() != '\n');

                        if(strcmp(ans3,"y") == 0) {
                            FILE* fptr = fopen(aliases, "w");
                            fprintf(fptr,"%s",file);

                        }
                        fseek(stdin,0,SEEK_END);
                    }
                    free(als);
                    free(file);
                    return 1;
        }
    }
    else {
        printf("Usage: unalias\n");
        printf("Type unalias --help for further information\n");
        return 1;
    }
}

// builtin command for masking permissions of files & directories
int YSS_umask(char *argv[], int argnum) {
    mode_t currmsk = umask(0);
    umask(currmsk);

    char msk[3];
    sprintf(msk,"%o",currmsk);

    short digits = strlen(msk);

    char *mask = malloc((4 - digits) * sizeof(char));
    strcpy(mask,"");

    while(digits < 4) {
        strcat(mask,"0");
        digits+=1;
    }
    strcat(mask,"\0");

    if(argnum < 2) {
        if(argv[1] == NULL) {
            if(currmsk == 0)
                printf("0000\n");
            else {
                printf("%s%s\n",mask,msk);
            }
            return 1;
        }
        else {
            if(argv[1][0] == '-') {
                if(strcmp(argv[1],"-S") == 0) {
                    if(argv[2] == NULL) {
                        char mask[9];
                        int curmsk = 0777 - currmsk;
                        strmode(curmsk,mask);

                        printf("%s\n",mask);
                        return 1;
                    }
                    else {

                    }
                }
                else {
                    printf("umask: Invalid option\n");
                    printf("usage: umask [-S] [mode]\n");
                    return 1;
                }
            }
            else {
                for(int i=0; i < strlen(argv[1]); i++) {
                    if(isalpha(argv[1][i]) != 0) {
                        printf("umask: Please just use numbers 0-7\n");
                        return 1;
                    }
                }

                if(strlen(argv[1]) > 3) {
                    printf("umask: mode must be three digits at most\n");
                    return 1;
                }

                if(strchr(argv[1],'8') != NULL || strchr(argv[1],'9') != NULL) {
                    printf("umask: out of range mode, please use numbers 0-7\n");
                    return 1;
                }

                char* msk = (char*) malloc(4);
                strcpy(msk,argv[1]);
                msk = strtok(msk,"\n");
                char* endp = NULL;


                mode_t newmsk = strtol(msk, &endp, 8);

                umask(newmsk);
                return 1;
            }
        }
        free(mask);
    }
    else {
        printf("umask: Too many arguments\n");
        return 1;
    }
}

// method to clear single & double qoutes from string
char* trimQoute(char *str) {
    char *s = str;
    int len = strlen(s);
    if(s[0] == '\'' || s[0] == '"'){
        s = s+1;
    }
    qouteSubstring(s,0,len);

    return s;
}


void qouteSubstring(char *string, int start, int end){
    char* i = string;
    char* j = string;
    while(*j != 0)
    {
        *i = *j++;
        if(*i != '\'' && *i != '"')
            i++;
    }
    *i = 0;
}

void GetTailNode(
      aliased  *I__listHead,   /* The caller supplied list head pointer. */
      aliased **_O_listTail    /* The function sets the callers pointer to the last node. */)
{
   aliased *curNode = I__listHead;

   /* Iterate through all list nodes until the last node is found. */
   /* The last node's 'next' field, which is always NULL. */
   if(curNode) {
      while(curNode->next)
         curNode=curNode->next;
   }

   /* Set the caller's pointer to point to the last (ie: tail) node. */
   if(_O_listTail)
      *_O_listTail = curNode;
}

void insertAlias(char *cmd, char* als) {

    aliased **IO_head=&head;

    aliased *tailNode;
    aliased *newNode = NULL;
    //Getting a pointer to the last node in the list
    GetTailNode(*IO_head, &tailNode);

    /* Allocate memory for new node (with its payload). */
    newNode=malloc(sizeof(*newNode));
    if(NULL == newNode)
    {
        fprintf(stderr, "malloc() failed.\n");
    }

    // Initializing the new node

    newNode->command=cmd;
    newNode->alias=als;

    /* Link this node into the list as the new tail node. */
    newNode->next = NULL;
    if(tailNode)
        tailNode->next = newNode;
    else
        *IO_head = newNode;
}

void deleteAlias(char *als) {
    aliased **start = &head;
    aliased *ptr = *start;
    aliased *ptr2 = ptr->next;


    if(strcmp(ptr->alias,als) == 0) {
        *start = ptr->next;
    }
    else {
        while(strcmp(ptr2->alias,als) != 0) {
            ptr2 = ptr2->next;
            ptr = ptr->next;
        }


        if(ptr2->next != NULL) {
            ptr->next = ptr2->next;
        }
        else
            ptr->next = NULL;

        free(ptr2);
    }
}
