#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
/*
sudo add-apt-repository "deb http://archive.ubuntu.com/ubuntu $(lsb_release -sc) main universe"
sudo apt-get update
sudo apt-get install libncurses5-dev
gcc program.c -lncurses

*/
void printusage();
void changeper(int arr[3][3],char* argv);


int main(int argc, char *argv[]) {

  if(argc < 2)//should have at least 2 arguments
    {
        printusage();
        return 1;//return with error
    }
    //get file permissions
   // printf("%s",argv[1]);
    if(strcmp(argv[1],"--help")==0){
        //if(strcmp(argv[2],"help")==0){
            printf("chper allows you to change file permisions\n");
            printusage();
            return 0;
        //}
    }

    //

    WINDOW *w;
    char list[3][8] = { "Read", "Write", "Execute"};
    char item[7];
    int ch, i = 0, width = 7;


    initscr(); // initialize Ncurses

    int yMax,xMax;//screen x and y size
      //  getmaxyx(stdscr,yMax,xMax);
//new size
    struct winsize ww;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ww);

   // printf ("lines %d\n", ww.ws_row);
 //   printf ("columns %d\n", ww.ws_col);
    yMax=ww.ws_row;
    xMax=ww.ws_col;
    //new size

    w = newwin( 5, 30, ((yMax/2)-2),((xMax/2)-14)); // create a new window
    box( w, 0, 0 ); // sets default borders for the window

    //create border to hold title
    WINDOW *w2;
    w2 = newwin(2, 30, ((yMax/2)-4),(xMax/2-14)); // create a new window to put names in

    WINDOW *w3;
    w3 = newwin(3,32, ((yMax/2)+6),((xMax/2)-16));

    //  box( w2, 0, 0 ); // sets default borders for the window
    start_color();
    init_pair(2, COLOR_BLACK, COLOR_GREEN);
    attron(COLOR_PAIR(2));
    mvwprintw( w2, 1, 2, "%s", "User\t   Group    Other" );
    refresh();


    //  print all the menu items and highlight the first one
    int perm[3][3];//save permissions in here

    //initialize matrix for current permissions on first file
    char* fn=argv[1];
    FILE *stream;
    struct stat info;
    stat(fn, &info);
    mode_t t_mode=info.st_mode;//get current file info

    //user info
    if(t_mode&S_IRUSR){
        perm[0][0]=1;
    }
    else{
        perm[0][0]=0;
    }
    if(t_mode & S_IWUSR){
        perm[1][0]=1;
    }
    else{
        perm[1][0]=0;
    }
     if( t_mode & S_IXUSR){
        perm[2][0]=1;
    }
    else{
        perm[2][0]=0;
    }

    //group info
     if(t_mode&S_IRGRP){
        perm[0][1]=1;
    }
    else
    {
        perm[0][1]=0;
    }
    if(t_mode & S_IWGRP){
        perm[1][1]=1;
    }
    else
    {
        perm[1][1]=0;
    }
    if( t_mode & S_IXGRP){
        perm[2][1]=1;
    }
    else
    {
        perm[2][1]=0;
    }

    //others info
    if(t_mode&S_IROTH){
        perm[0][2]=1;
    }
    else
    {
        perm[0][2]=0;
    }
    if(t_mode & S_IWOTH){
        perm[1][2]=1;
    }
    else
    {
        perm[1][2]=0;
    }
    if( t_mode & S_IXOTH){
        perm[2][2]=1;
    }
     else
    {
        perm[2][2]=0;
    }

    init_pair(3,COLOR_BLACK, COLOR_GREEN   );


    for( i=0; i<3; i++ ) {
        if( i == 0 )
            wattron( w, A_STANDOUT ); // highlights the first item.
        else{
            wattroff( w, A_STANDOUT );
            if(perm[i][0]==1)//if matrix bit on set wattron dont print now and set off anyways after print.
                wattron( w, COLOR_PAIR(3) );
            sprintf(item, "%-7s",  list[i]);
            }
        sprintf(item, "%-7s",  list[i]);//sprintf used to control the format
        mvwprintw( w, i+1, 2, "%s", item );
        wattroff( w, COLOR_PAIR(3) );
    }
    //print second column
    for( i=0; i<3; i++ ) {
        wattroff( w, A_STANDOUT );
        sprintf(item, "%-7s",  list[i]);
        if(perm[i][1]==1){//if matrix bit on set wattron dont print now and set off anyways after print.
            wattron( w, COLOR_PAIR(3) );
            sprintf(item, "%-7s",  list[i]);
        }
        mvwprintw( w, i+1, 7+2+2, "%s", item );
        wattroff( w, COLOR_PAIR(3) );
    }
    //print third column
    for( i=0; i<3; i++ ) {
        wattroff( w, A_STANDOUT );
        sprintf(item, "%-7s",  list[i]);
        if(perm[i][2]==1){//if matrix bit on set wattron dont print now and set off anyways after print.
            wattron( w, COLOR_PAIR(3) );
            sprintf(item, "%-7s",  list[i]);
        }
        mvwprintw( w, i+1, 14+4+2, "%s", item );
        wattroff( w, COLOR_PAIR(3) );
    }

    mvwprintw(w3,1,2,"\tPress 's' to save\n");
    mvwprintw(w3,2,2,"Press 'q' to quit without save\n");

                  //  " Execute  Execute  Execute  │"
    wrefresh( w ); // update the terminal screen
    wrefresh( w2 ); // update the terminal screen
    wrefresh( w3 );



    i = 0;
    int j=2;
    noecho(); // disable echoing of characters on the screen
    keypad( w, TRUE ); // enable keyboard input for the window.
    curs_set( 0 ); // hide the default screen cursor.



    //initialize perm
    // get the input
    start_color();
    init_pair(3,COLOR_BLACK, COLOR_GREEN   );
    while(( ch = wgetch(w)) != 'q'){
        // right pad with spaces to make the items appear with even width.
        sprintf(item, "%-7s",  list[i]);
        if( perm[i][((j-2)/9)]==0)
            mvwprintw( w, i+1, j, "%s", item );
        else{
            wattron( w, COLOR_PAIR(3) );
            sprintf(item, "%-7s",  list[i]);
            mvwprintw( w, i+1, j, "%s", item);
            wattroff( w, COLOR_PAIR(3) );
        }
        // use a variable to increment or decrement the value based on the input.
        switch( ch ) {
            case KEY_UP:
                i--;
                i = ( i<0 ) ? 2 : i;
            break;
            case KEY_DOWN:
                i++;
                i = ( i>2 ) ? 0 : i;
            break;
            case KEY_LEFT:
                j-=9;
                j = ( j<0 ) ? 20 : j;
            break;
            case KEY_RIGHT:
                j+=9;
                j = ( j>27 ) ? 2 : j;
            break;
            case 's':
                //execute call changeper fun;
                for(int i=1;i<argc;i++){
                    changeper(perm,argv[i]);
                }
                delwin( w );
                endwin();
                return 0;
            break;
            case 10://enter is pressed get position to know which option.
                printw("i:%d  j:%d",i,j);
                if(perm[i][((j-2)/9)]==0){
                    perm[i][((j-2)/9)]=1;
                }
                else {
                    perm[i][((j-2)/9)]=0;
                }
            break;
        }
        // now highlight the next item in the list.
        JUMP: ;     //label to jump to
        wattron( w, A_STANDOUT );
        sprintf(item, "%-7s",  list[i]);
        mvwprintw( w, i+1, j, "%s", item);
        wattroff( w, A_STANDOUT );
    }

    delwin( w );
    endwin();
    return 0;
}

int getChmod(const char *path){
    //int chmod(const char *path, mode_t mode);
    // int stat(const char *restrict path, struct stat *restrict buf);
    struct stat ret;

    if (stat(path, &ret) == -1) {
        return -1;
    }
        //st_mode Mode of file
        // read permission of owner ;write permission owner ; Execute permission owner
    return (ret.st_mode & S_IRUSR)|(ret.st_mode & S_IWUSR)|(ret.st_mode & S_IXUSR)|/*owner*/
    (ret.st_mode & S_IRGRP)|(ret.st_mode & S_IWGRP)|(ret.st_mode & S_IXGRP)|/*group*/
    (ret.st_mode & S_IROTH)|(ret.st_mode & S_IWOTH)|(ret.st_mode & S_IXOTH);/*other*/
}

 void printusage(){
    printf("Usage : chper [FILENAME]..");
    printf("\nFor example: chper myfile etc/usr/file1 file2\n");

}

void changeper(int arr[3][3],char* args){


    mode_t mode;
    //get file permissions
    char* fn=args;
    FILE *stream;
    struct stat info;
    //     (stream = +(fn, "r")) == NULL
    if (access( args, F_OK ) == -1 ){
        perror("YSS: ERROR file not found or corrupted ");
        printf("YSS: Error file %s not found or corrupted\n", args);
        fprintf(stderr,"YSS: Error file %s not found or corrupted\n", args);
    }
    else {
        //fclose(stream);
        stat(fn, &info);
    //mode=info.st_mode;//get current file info
    /*
S_IRUSR
sets file owner permission to read.
S_IWUSR
sets file owner permission to write.
S_IXUSR
sets file owner permission to execute

S_IRGRP
sets group permission to read.
S_IWGRP
sets group permission to write.
S_IXGRP
sets group permission to execute.

S_IROTH
sets general permission to read.
S_IWOTH
sets general permission to write.
S_IXOTH
sets general permission to execute.


    */
    //first walk through read permissions
        if(arr[0][0]==1){//read user S_IRUSR
         mode=mode|S_IRUSR;
        }
        if(arr[0][1]==1){//read group
            mode=mode|S_IRGRP;

        }
        if(arr[0][2]==1){//read other
            mode=mode|S_IROTH;
        }

    //walk through write permissions

        if(arr[1][0]==1){//write user
            mode=mode|S_IWUSR;

        }
        if(arr[1][1]==1){//write group
            mode=mode|S_IWGRP;

        }
        if(arr[1][2]==1){//write other
           mode=mode|S_IWOTH;
        }

//walk through execute permissions


        if(arr[2][0]==1){//execute user
            mode=mode|S_IXUSR;

        }
        if(arr[2][1]==1){//execute group
            mode=mode|S_IXGRP;

        }
        if(arr[2][2]==1){//execute other
            mode=mode|S_IXOTH;

        }

        if(chmod(fn,mode)!= 0){
            perror("Failed to change permissions for file/s\n ");
        }
    }
}

