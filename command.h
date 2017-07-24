#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <regex.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

int wldcrd(char* arg);
int CmdSub(char* arg);

//list of arguments
typedef struct arguments {
    char* currArg;
    struct arguments *next;
}arguments;

//command contents
typedef struct command {
    char *cmd;
    char *opt;
    char *opt2;
    arguments *arg;
    int args;
    char *stdIn;
    char *stdOut;
    char *stdeErr;
    char *op;//operator
    short append;//shows if append or not
    short error; // show if error redirect or not
    char *descr;
}command;

//list of commands "separated by pipes
typedef struct table {
    command cmnd;

    struct table *next;
}table;

//initializing table head
table *Head = NULL;

//number of nodes "commands in table"
int nodes=0;

command cmad;

arguments argm;

char bg = '0';

void GetTableTailNode(
      table  *I__listHead,   /* The caller supplied list head pointer. */
      table **_O_listTail    /* The function sets the callers pointer to the last node. */)
{
   table *curNode = I__listHead;

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

   //insert node at
int InsertNode() {
  char *cmd=cmad.cmd;
  char *opt=cmad.opt;
  char *opt2=cmad.opt2;
  char *op=cmad.op;
  arguments *arg=cmad.arg;
  int args=cmad.args;
  char *stdIn=cmad.stdIn;
  char *stdOut=cmad.stdOut;
  char *stdeErr=cmad.stdeErr;
  short append=cmad.append;
  short error=cmad.error;
  char *descr=cmad.descr;

   table **IO_head=&Head;

   int rCode=0;
   table *tailNode;
   table *newNode = NULL;
   //Getting a pointer to the last node in the list
   GetTableTailNode(*IO_head, &tailNode);
   if(rCode) {
      fprintf(stderr, "Error in linked list %d\n", rCode);
      goto CLEANUP;
   }

   /* Allocate memory for new node (with its payload). */
   newNode=malloc(sizeof(*newNode));
   if(NULL == newNode)
    {
      rCode=ENOMEM;   /* ENOMEM is defined in errno.h */
      fprintf(stderr, "malloc() failed.\n");
      goto CLEANUP;
    }

   // Initializing the new node
     newNode->cmnd.cmd=cmd;
     newNode->cmnd.opt=opt;
     newNode->cmnd.opt2=opt2;
     newNode->cmnd.op=op;
     newNode->cmnd.arg=arg;
     newNode->cmnd.args=args;
     newNode->cmnd.stdIn=stdIn;
     newNode->cmnd.stdOut=stdOut;
     newNode->cmnd.stdeErr=stdeErr;
     newNode->cmnd.append=append;
     newNode->cmnd.error=error;
     newNode->cmnd.descr=descr;

   /* Link this node into the list as the new tail node. */
   newNode->next = NULL;
   if(tailNode)
      tailNode->next = newNode;
   else
      *IO_head = newNode;

    nodes+=1;
    cmad.arg=NULL;
    cmad.args=0;
CLEANUP:

   return(rCode);
}

//emptying struct cmad to be used for another command
void clrcont(){
   cmad.cmd='\0';
   cmad.opt='\0';
   cmad.opt2='\0';
   cmad.op='\0';
   cmad.stdeErr='\0';
   cmad.stdIn='\0';
   cmad.stdOut='\0';
   cmad.append=0;
   cmad.error=0;
}

//method to empty the command table (linked list)
void clrtbl() {
    Head = NULL;
    nodes=0;
    bg = '0';
}

void GetArgTailNode(
      arguments  *I__listHead,   /* The caller supplied list head pointer. */
      arguments **_O_listTail    /* The function sets the callers pointer to the last node. */)
{
   arguments *curNode = I__listHead;

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

void insertArgNode(char* currArg) {

    char *arg=currArg;
    int cont = wldcrd(arg);
    int cont2=CmdSub(arg);
    //printf("SENDING %s",arg);
    if(cont2==1){
    if(cont == 1) {
        arguments **IO_head=&cmad.arg;

        arguments *tailNode;
        arguments *newNode = NULL;
        //Getting a pointer to the last node in the list
        GetArgTailNode(*IO_head, &tailNode);

        /* Allocate memory for new node (with its payload). */
        newNode=malloc(sizeof(*newNode));
        if(NULL == newNode)
        {
            fprintf(stderr, "malloc() failed.\n");
        }

        // Initializing the new node

        newNode->currArg=arg;
        cmad.args+=1;
        /* Link this node into the list as the new tail node. */
        newNode->next = NULL;
        if(tailNode)
            tailNode->next = newNode;
        else
            *IO_head = newNode;
    }
}}

//method to decide what is the current I/O redirection
void io_red(char* str) {
    if(strcmp(">",cmad.op)==0){
        cmad.stdOut=str;
    }
    else if(strcmp("<",cmad.op)==0){
        cmad.stdIn=str;
    }
    else if(strcmp(">>",cmad.op)==0){
        cmad.stdOut=str;//out to FILE
        cmad.append= 1;      //and append
    }
    else if(strcmp("#",cmad.op)==0) {
        cmad.stdOut=str;
        cmad.error=1;
    }
    else if(strcmp("#>",cmad.op)==0) {
        cmad.stdOut=str;
        cmad.append=1;
        cmad.error=1;
    }
}

//execute command substitution if any
int CmdSub(char* arg) {
    if(strstr(arg,"$(") == NULL ) {
        return 1;
    }

   // printf("WOW %s,",strstr(arg,"$("));

        //contains $(
    int tmpin=dup(0);//save in and out to restore after execution
    int tmpout=dup(1);


    int file = open(".fd322", O_CREAT|O_WRONLY, 0777);
    int file1 = open("/dev/null", O_CREAT|O_WRONLY, 0777);
    pid_t pid;
    int p[2];
    pipe(p);

    pid= fork();
    if(pid==0){
        //child process
        dup2(file1, 1);
    //dup2(p[1], 1);
   // close(p[0]);
   // printf("executing %s\n",arg);
        printf("hhorray\n");
        system("echo hi >.fd322");
   // printf("ls");

        close(file);
        dup2(file1, 1);
    //return 0;
    }
    else{
        //parent process
       //

        //wait(NULL);

    }
    //restore default in/out


    //read file

    FILE* fptr = fopen(".fd322", "r");
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    while((read = getline(&line, &len, fptr)) != -1) {
        char* curlne = strtok(line,"\n");
        insertArgNode(curlne);
        line = NULL;
    }

    return 0;
}


//method to expand the wildcards if exist
int wldcrd(char* arg) {
    if(strchr(arg,'*') == NULL &&  strchr(arg,'?') == NULL) {
        return 1;
    }

    char* reg = (char*) malloc(2*strlen(arg)+10);
    strcpy(reg,"");

    char* reg2 = (char*) malloc(2*strlen(arg)+10);
    strcpy(reg2,"");

    char* a =arg;
    char* r = reg;
    char* r2 = reg2;
    *r = '^';
    r++;

    while(*a) {
        if(*a == '*') {
            *r = '.';
            r++;
            *r2 = '.';
            r2++;
            *r = '*';
            r++;
            *r2 = '*';
            r2++;
        }
        else
        if(*a == '?') {
            *r = '.';
            r++;
            *r2 = '.';
            r2++;
        }
        else
        if(*a == '.') {
            *r = '\\';
            r++;
            *r = '.';
            r++;
            *r2 = '\\';
            r2++;
            *r2 = '.';
            r2++;
        }
        else {
            *r = *a;
            r++;
            *r2 = *a;
            r2++;
        }
        a++;
    }
    *r = '$';
    r++;
    *r = 0;
    *r2 = 0;

    regex_t reggg;

    if(regcomp(&reggg,reg,REG_EXTENDED) != 0) {
        perror("regcomp");
        return 0;
    }

    /*char* direct = (char*) malloc(strlen(arg));

    if(strchr(reg2,'/') != NULL) {
        char* token;
        token = strtok(reg2,"/");

        while(token != NULL) {
            if(strchr(token,'*') == NULL &&  strchr(token,'?') == NULL) {
                strcat(direct,"/");
                strcat(direct,token);
            }
            token = strtok(NULL,"/");
        }
//        strcat(direct,"\0");
    }*/

    DIR * dir;

    //string to hold the old working directory
    char *oldpwd = malloc(2048 * sizeof(char));
    strcpy(oldpwd,"");

    int changed = 0;

    /*if(strlen(direct) > 0) {
        strcat(oldpwd,getenv("PWD"));
        chdir(direct);
        changed = 1;
    }*/

    dir = opendir(".");


    struct dirent * ent;
    int chngwld = 0;

    while((ent = readdir(dir)) != NULL) {
//        char* wild = (char*) malloc(150);
//        if(strlen(direct) > 0) {
//            //printf("%s\n",direct);
//            strcpy(wild,direct);
//            strcat(wild,"/");
//            strcat(wild,strdup(ent->d_name));
//            insertArgNode(wild);
////            printf("%s\n",wild);
//            chngwld = 1;
//        }
//        else {
        if(regexec(&reggg, ent->d_name, (size_t)0, NULL, 0 ) == 0) {
            if(ent->d_name[0] == '.') {
                if(arg[0] == '.'){
                    insertArgNode(strdup(ent->d_name));
                    chngwld = 1;
                }
            }
            else {
                insertArgNode(strdup(ent->d_name));
                chngwld = 1;
            }
        }
//        free(wild);
    }

    closedir(dir);
    free(reg);
    free(reg2);
    //free(direct);
    regfree(&reggg);

    /*if(changed == 1) {
        chdir(oldpwd);
    }*/

    if(chngwld == 1) {
        return 0;
    }
    return 1;
}
