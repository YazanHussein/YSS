#include <fcntl.h>
#include <sys/stat.h>
#include "builtins.h"

char* findsynonym(char* cmd);
int executetb();
extern volatile int terminate;

int executetb() {

  table * tmpcmd = Head;

  mode_t currmsk = umask(0);
  umask(currmsk);

  int p[2];
  pid_t pid;
  int fd_in = 0, status;

  do {
    arguments * tmparg = tmpcmd -> cmnd.arg;
    char * argv[4 + tmpcmd -> cmnd.args];
    argv[0] = tmpcmd -> cmnd.cmd;
    if (tmpcmd -> cmnd.opt != NULL) {
      argv[1] = tmpcmd -> cmnd.opt;

      if(tmpcmd -> cmnd.opt2 != NULL) {
            argv[2] = tmpcmd -> cmnd.opt2;
            int i = 3;
            while (tmparg != NULL) {
                argv[i] = tmparg -> currArg;
                i += 1;
                tmparg = tmparg -> next;
            }
            argv[i] = NULL;
      }
      else {
            int i = 2;
            while (tmparg != NULL) {
                argv[i] = tmparg -> currArg;
                i += 1;
                tmparg = tmparg -> next;
            }
            argv[i] = NULL;
      }
    }
    else {
      if(tmpcmd->cmnd.opt2 != NULL) {
            argv[1] = tmpcmd -> cmnd.opt2;
            int i = 2;
            while (tmparg != NULL) {
                argv[i] = tmparg -> currArg;
                i += 1;
                tmparg = tmparg -> next;
            }
            argv[i] = NULL;
      }
      else {
            int i = 1;
            while (tmparg != NULL) {
            argv[i] = tmparg -> currArg;
            i += 1;
            tmparg = tmparg -> next;
            }
            argv[i] = NULL;
      }
    }

    char* pointerOfEqualsSign = strstr(argv[0], "="); //getting the address of '='
    if(pointerOfEqualsSign != NULL){
      if(nodes == 1){ //if there is no pipes, the assignment is done
        int i=0;
        while(argv[i] != NULL){
          if(pointerOfEqualsSign != NULL){ //ADDED BY ME
            char * variable[2];

            variable[1] = pointerOfEqualsSign+1; //getting the string after '='
            *pointerOfEqualsSign = '\0'; //null terminate the string on '='
            variable[0] = argv[i]; //getting the string before '=' which is done by the help of the above line
            if( checkNonDigitOrLetter(variable[0]) )
              {
              if(variable[1][0] == '$' && variable[1][1] != '$') {
                variables * tmpvar = localvars;
                int found =0;
                variable[1] = variable[1]+1;
                while(tmpvar != NULL) {
                  if(strcmp(variable[1],tmpvar -> name) == 0) {
                    variable[1] = strdup(tmpvar->value);
                    found = 1;
                    break;
                  }
                  tmpvar = tmpvar -> next;
                }
                if(found){
                  YSS_assignLocalVar(variable);
                }
                else
                  printf("No local variable with the name '%s'\n", variable[1]);
              }
              else{
                if(variable[1][0] == '$' && variable[1][1] == '$'){
                  char* var = variable[1]+2;
                  char *value = getenv(var);
                  if(value != NULL){
                    variable[1] = strdup(value);
                    YSS_assignLocalVar(variable);
                  }
                  else
                    printf("No environment variable with the name '%s'\n", var);
                }
                else
                  YSS_assignLocalVar(variable);
              }
            }
            else
              printf("ERROR: cannot add the variable '%s'\n\tthe left hand side must be 'letter(letter|digit)*'\n", argv[i]);
          }
          else //ADDED BY ME
            printf("ERROR: cannot add the variable '%s': \tmissing '='\n", argv[i]);

          i++;
          if(argv[i] != NULL)
            pointerOfEqualsSign = strstr(argv[i], "=");
        }
        return 1;
      }
      else{
        //if there is piping, this command will be ignored, thus, the output of previous command will become the input of the next command
        printf(BOLD KYEL"WARNING: "KNRM"the assignment \""BOLD KRED"%s"KNRM"\" is ignored\n", tmpcmd->cmnd.cmd);
        tmpcmd = tmpcmd -> next;
        continue;
      }
    }

    if (isBuiltin(argv[0]) != NULL) {
      char * builtnm = isBuiltin(argv[0]);
      return callBuiltin(builtnm, argv, (sizeof(argv) / sizeof(argv[0])));
    }
    else {

      pipe(p);

      if ((pid = fork()) < 0) {
        perror("YSS");
        exit(EXIT_FAILURE);
      } else if (pid == 0) {

        dup2(fd_in, 0); //change the input to get from prev pipe
        // if there is an input from file overwrite pipe if any
        if (tmpcmd -> cmnd.stdIn != NULL) {

          int in ;

          if ((in = open(tmpcmd -> cmnd.stdIn, O_RDONLY)) == -1) { //we can check error in yacc

            printf("Can't read from non-existing file\n");
            return 1;
          } else {

            dup2( in , 0); //overwrite dup for file

          }

        }

        if (tmpcmd -> next != NULL) { //if there is next out to pipe
          dup2(p[1], 1);
          close(p[0]);
        }

        // if there is an output to file
        if (tmpcmd -> cmnd.stdOut != NULL) {

          if (tmpcmd -> cmnd.append == 1) {
            int out;
            if ((out = open(tmpcmd -> cmnd.stdOut, O_RDONLY)) == -1) {
              printf("Can't append on non-existing file\n");
              return 1;
            }
            else {
              if (tmpcmd -> cmnd.error == 1) {
                out = open(tmpcmd -> cmnd.stdOut, O_WRONLY | O_APPEND);
                dup2(out, 2);
                dup2(0, 1);
              } else {
                out = open(tmpcmd -> cmnd.stdOut, O_WRONLY | O_APPEND);
                dup2(out, 1);
              }
            }
          } else {
            int x = open(tmpcmd -> cmnd.stdOut, O_CREAT, (0666 - currmsk)); //create file if it doesnt exist
            close(x);

            if (tmpcmd -> cmnd.error == 1) {
              int out = open(tmpcmd -> cmnd.stdOut, O_WRONLY | O_TRUNC);
              dup2(out, 2);
              dup2(0, 1);
            } else {
              int out = open(tmpcmd -> cmnd.stdOut, O_WRONLY | O_TRUNC); //open file for writing
              dup2(out, 1);

            }
          }
        }

        if (tmpcmd -> cmnd.op != NULL) {
          if (strcmp("2>&1", tmpcmd -> cmnd.op) == 0)
            dup2(1, 2);

          if (strcmp("1>&2", tmpcmd -> cmnd.op) == 0)
            dup2(2, 1);
        }

        status = execvp(argv[0], argv);

        perror("YSS");

        char* synm = findsynonym(argv[0]);
        if(strlen(synm) > 0) {
            printf("Did you mean command: '%s'\n",synm);
        }
        exit(EXIT_FAILURE);
      }
      else {
        if (bg == '1') { //theres background
          printf("[1] %d\n",pid);
          //waitpid(pid, &status, 0); // wait child
          kill(pid, SIGCONT);

          if(status == 0)
            printf("[1]+ Done            %s\n",tmpcmd->cmnd.cmd);

          else {
            perror("YSS");
            EXIT_FAILURE;
          }

          close(p[1]);
          fd_in = p[0]; //save the input for the next command
          tmpcmd = tmpcmd -> next;
        }
        else {
          wait(NULL);
          close(p[1]);
          fd_in = p[0]; //save the input for the next command
          tmpcmd = tmpcmd -> next;

          if(terminate){
          	terminate = 0;
          	return 1;
          }
        }
      }

    }
  } while (tmpcmd != NULL);
}

char* findsynonym(char* cmd) {
    int length = strlen(cmd);
    int done  = 0;
    char* synonym = (char*) malloc(length);
    strcpy(synonym,"");

    for(int i = 0; i < (sizeof(builtins)/sizeof(builtins[0])); i++) {
        if(strlen(builtins[i]) == length) {
            int j=0;
            while(j < length) {
                if(strchr(cmd,builtins[i][j]) != NULL)
                    j++;
                else
                    break;
            }
            if(j == length) {
                strcpy(synonym,builtins[i]);
                done = 1;
                break;
            }
        }
    }

    if(done == 0) {
        FILE* fptr = fopen(commands, "r");
        char* line = NULL;
        size_t len = 0;
        ssize_t read;

        while((read = getline(&line, &len, fptr)) != -1) {
            line = strtok(line,"\n");

            if(strlen(line) == length) {
                int i=0;
                while(i < length) {
                    if(strchr(cmd,line[i]) != NULL)
                        i++;
                    else
                        break;
                }
//                printf("%d after %s\n",i,line);
                if(i == length) {
                    strcpy(synonym,line);
                    done = 1;
                    break;
                }
            }
            line = NULL;
        }
        fclose(fptr);
    }

    return synonym;
}
