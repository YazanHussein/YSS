void saveVars(){
	char * homedir = getpwuid(getuid())->pw_dir;
	char * savefile = "/.YSS_envars";
	char * path = malloc(strlen(homedir) + 13); //14 for savefile length & null
	strcpy(path, homedir);
	strcat(path, savefile);

	int file = open(path, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);

	if(envvars != NULL){
	    while(envvars != NULL){
	    	write(file, envvars->name, strlen(envvars->name));
	    	write(file, "=", 1);
	    	write(file, envvars->value, strlen(envvars->value));
	    	write(file, "\n", 1);
	        envvars = envvars->next;
		}
	}
}

void saveHistory(){
	char * homedir = getpwuid(getuid())->pw_dir;
	char * savefile = "/.YSS_history";
	char * path = malloc(strlen(homedir) + 14); //14 for savefile length & null
	strcpy(path, homedir);
	strcat(path, savefile);

	if(his != NULL){
		while(his->next != NULL)
			his = his->next;

		int file = open(path, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);

		while(his->previous != NULL){
			write(file, his->line, strlen(his->line));
			write(file, "\n", 1);
			his = his->previous;
		}
		write(file, his->line, strlen(his->line));
		write(file, "\n", 1);

		free(path);
	}
}
