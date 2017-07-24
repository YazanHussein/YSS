
int initAlias() {
    if (access(aliases,F_OK) == -1) {
        printf("Error: Environment file non-existing\n");
        return 0;
    }

    FILE* fptr = fopen(aliases, "r");
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    while((read = getline(&line, &len, fptr)) != -1) {
        char* cmnd = strtok(line,"\n");
        cmnd = strstr(line,"=")+1;
        cmnd = trimQoute(cmnd);

        char* alias = strstr(line," ")+1;
        alias = strtok(alias,"=");
        insertAlias(cmnd,alias);
        line = NULL;
    }
    fclose(fptr);

    insertAlias("forbid","rm -rf /*");
    insertAlias("forbid","sudo rm -rf /*");
    return 1;
}

void initEnvVars(){
    char * homedir = getpwuid(getuid())->pw_dir;
    char * savefile = "/.YSS_envars";
    char * path = malloc(strlen(homedir) + 13); //13 for savefile length & null

    strcpy(path, homedir);
    strcat(path, savefile);

    FILE* file = fopen(path, "r");
    char *line;

    if(file != NULL){
        while(fscanf(file, "%s", line) != EOF){
            char* positionOFEqualsSign = strstr(line, "=");

            if(positionOFEqualsSign == NULL)
                continue;
            else{
                *positionOFEqualsSign = '\0';
                setenv(line, positionOFEqualsSign+1, 1);
                assignEnvVar(strdup(line));
            }
        }
        fclose(file);
    }
}


void initHistory(){
    char * homedir = getpwuid(getuid())->pw_dir;
    char * savefile = "/.YSS_history";
    char * path = malloc(strlen(homedir) + 14); //14 for savefile length & null

    strcpy(path, homedir);
    strcat(path, savefile);

    FILE* file = fopen(path, "r");
    char *line;

    if(file != NULL){
        char* line = NULL;
        size_t len = 0;
        ssize_t read;

        while((read = getline(&line, &len, file)) != -1) {
            line[strlen(line) - 1] = '\0';
            addInputToHistory(line);
        }

        fclose(file);
    }
}
