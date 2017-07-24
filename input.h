#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>

extern int lengthhh; //the length of the prompt

typedef struct history{
	char *line;
	struct history *next;
	struct history *previous;
}history;

history *his=NULL; //history linkedlist

void addInputToHistory(char *input){
	int length = (int)strlen(input)+1; //preserve an index for '\0'
	history *tmp = malloc(sizeof(history*));

  tmp->line = (char *)malloc(length);
	strncpy(tmp->line, input, length);
	tmp->line[length-1] = '\0';
	tmp->next = NULL;
	tmp->previous = NULL;

	if(his == NULL){
		his = tmp;
	}
	else{ //add the new node at first
		his->previous = tmp;
		tmp->next = his;

		his = tmp;
	}
}

int readInput(char * str){
  struct termios old_term, new_term; //used for terminal settings
  history *currentLine = NULL; //points to the history linkedlist
  char c; //used to read input
  char lastCommandUp='0'; //when the up arrow reaches the first command, it will become 1
  char lastCommandDown='1'; //when the down arrow reaches the input line, which by default is true
  int i=0; //to save the char entered in the i index in str, and i = the length of the string
  int curx =0; //cursor on the X axis, it will be changed when the right & left arrows are pressed
  int up = 0; //number of lines the user reached above the last line in the input

  //NOTE: i always points to the index after the last char in str

  struct winsize w; //this var will hold the width and the height of the terminal using the func 'ioctl'

  /* Get old terminal settings for further restoration */
  tcgetattr(0, &old_term);

  /* Copy the settings to the new value */
  new_term = old_term;

  /* Disable echo of the character and line buffering */
  new_term.c_lflag &= (~ICANON && ~ECHO);

  /* Set new settings to the terminal */
  tcsetattr(0, TCSANOW, &new_term);

  //printf("%c7", 27); //save current cursor position
  while (1) {
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    c = getchar();

    //while loop is used to check if sequence of arrows are entered
    while(c == 27){
      c = getchar();
      if(c=='['){
		    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        switch(getchar()){
          case 'A': //up arrow
                  curx =0; //when moving up it will set the cursor at the end of the line
                  //if lastCommandUp is reached, then there is nothing to show
                  if(lastCommandUp != '1'){
                    if(his == NULL){ //no search will be made
                      break;
                    }
                    else if(currentLine == NULL){ //at the beginning, currentLine will point at his
                      currentLine = his;
                    }
                    else{
                      //if the next node is NULL, then there no more suggestion to show
                      if(currentLine->next != NULL)
                        currentLine = currentLine->next;
                      else{
                        lastCommandUp = '1';
                        break;
                      }
                    }

                    /**
                      number of lines .. e.g. lengthhh+i = 81 ==> numberOfLines=2
                      up is used to reduce the result by the number of lines that the user reached
                    */
                    int numberOfLines = 1 + ((lengthhh+i - 1) / w.ws_col) - up;

                    //if the length equals the width then the cursot is at the next line ==> move it up
                    if((i+lengthhh)%w.ws_col == 0) //ADDED 7/5/2017
                      printf("%c[1A", 27);

                    printf("\r"); //return cursor to the beginning of the screen
					          //because we want to display an input from history, we must return the cursor to the first line iff (numberOfLines > 1)
                    while(numberOfLines > 1){
                      printf("%c[1A", 27); //move cursor up
                      numberOfLines--;
                    }

                    printf("%c[J", 27); //clear screen from cursor to the end
                    prompt();

                    i = (int)strlen(currentLine->line); //i equals the length of the new input that is got from history
                    strncpy(str, currentLine->line, i+1); //the 1 for '\0', str equals the input from history
                    printf("%s", str); //print the string

                    //if the length equals the width then the cursot is at the right edge ==> move it down
                    if((i+lengthhh)%w.ws_col == 0) //ADDED 7/5/2017
                      printf("\n");

                    //if we moved up then, we can move down, which means that lastCommandDown is not reached
                    if(lastCommandDown == '1')
                      lastCommandDown = '0';

                  	up =0; //reset up to 0, because the cursor position will be at the end of the input
                  }

                  break;
          case 'B': //down arrow
                  curx =0; //when moving down the cursor position will be at the end of the input
                  //if lastCommandDown is reached (lastCommandDown=1), then there is nothing to show
                  if(lastCommandDown != '1'){
                    /**
                      number of lines .. e.g. lengthhh+i = 81 ==> numberOfLines=2
                      up is used to reduce the result by the number of lines that the user reached
                    */
                    int numberOfLines = 1 + ((lengthhh+i - 1) / w.ws_col) - up;

                    //if the length equals the width then the cursot is at the next line ==> move it up
                    if((i+lengthhh)%w.ws_col == 0) //ADDED 7/5/2017
                      printf("%c[1A", 27);

                    printf("\r"); //return cursor to the left edge of the terminal
                    //because we want to display an input from history, we must return the cursor to the first line iff (numberOfLines > 1)
                    while(numberOfLines > 1){
                      printf("%c[1A", 27); //move cursor up
                      numberOfLines--;
                    }

                    //if the previous node is not NULL, then get the previous history
                    if(currentLine->previous != NULL){
                      currentLine = currentLine->previous;
                      i = (int)strlen(currentLine->line);
                      strncpy(str, currentLine->line, i+1); //the 1 for '\0'

                      //if we moved down then, we can move up, which means that lastCommandUp is not reached
                      if(lastCommandUp == '1')
                        lastCommandUp = '0';
                    }
                    else{
                      //if the previous node is NULL ==> no more commands to show
                      lastCommandDown = '1';
                      currentLine = NULL;
                      str[0] = '\0';
                      i=0;
                    }

                    printf("%c[J", 27); //clear screen from cursor to the end
                    prompt();
                    printf("%s", str);

                    //if the length equals the width then the cursot is at the right edge ==> move it down
                    if((i+lengthhh)%w.ws_col == 0) //ADDED 7/5/2017
                      printf("\n");

                    up =0; //reset 'up' to 0, because the cursor position will be at the end of the input
                  }

                  break;
          case 'D': //left arrow
                  /**
                    curx starts from 0, when the right arrow is pressed, curx is increased by 1
                    the cursor must be allowed to reach first char of the input ==> curx must be smaller than i
                    if curx == i then the cursor will be at the first char in str, which allows to insert chars at the beginning of the input
                  */
                  if(curx < i){ //denying the cursor from reaching positions before the beginning of input
                    int offset = i+lengthhh - curx; //get cursor position in any line

                    //if current cursor position at the LEFT edge of the terminal, then, it will be placed at the RIGHT edge of the terminal on the PREVIOUS line
					          if(  offset%w.ws_col == 0){
                      printf("%c[1A", 27); //move cursor up
                      printf("%c[%dC", 27, w.ws_col); //move cursor to the right edge of the terminal
                      curx++;
                      up++; //cursor moved up ==> up is increased
                    }
                    else{
                      printf("%c[1D", 27); //cursor is moved one char to the left
                      curx++;
                    }
                  }

                  break;
          case 'C': //right arrow
				          //when curx > 0 thats mean that the cursor was moved
                  if(curx > 0){
                    int offset = i+lengthhh - --curx; //get cursor position in any line
					          //if current cursor position at the RIGHT edge of the terminal, then, it will be placed at the LEFT edge of the terminal on the NEXT line
                    if(  offset%w.ws_col == 0){
                      printf("%c[1B\r", 27); //move cursor down
                      if(up != 0)
                        up--; //cursor moved down ==> up is decreased
                    }
                    else{
                      printf("%c[1C", 27); //cursor is moved one char to the right
                    }
                  }

                  break;
        }
        c = getchar();
      }
    }

    /**
      if the cursor is moved (curx > 0), then the input must be modified depending on
      the position of the cursor and the characters that are being entered
    */
    if(curx > 0){
      /**
        current cursor position after moving it
        i indicates the str length, curx indicates how chars the cursor moved to the left ==>
        e.g. i=10, curx=1 ==> the cursor must be placed at the last char(9), because i is the length of the str
        and a space.
      */
      int curretCursorPosition = i - curx;

      char *strAppend = (char *)malloc(curx+1); //this will hold the chars from the cursor position to the end
      strncpy(strAppend, str+curretCursorPosition, curx+1); //copy the chars from the cursor position to the end

      if(c == '\177' ){ //backspace
        if(curretCursorPosition > 0){
		      //if current cursor position at the LEFT edge of the terminal, then, it will be placed at the RIGHT edge of the terminal on the PREVIOUS line
          if( ((i+lengthhh - curx)%w.ws_col) == 0){
            printf("\r%c[1A", 27); //move cursor up
            printf("%c[%dC", 27, w.ws_col); //move cursor to the right edge of the terminal
            up++; //cursor moved up ==> up is increased

            printf("%c[J", 27); //clear screen from cursor to the end

            //on cursor position print the string starting from currentCursorPosition
			      printf("%s", str+curretCursorPosition);

            str[--curretCursorPosition] = '\0'; //reset the value before the cursor position
            strcat(str, strAppend); //append the string from currentCursorPosition to str
            str[--i] = '\0'; //the char at str[i-1] must be reset, and i is decreased, becase of removing a char from the input

      			/**
      			  up is decreased iff the remainder of the new length of input+ length of prompt equals zero
      			  which means the number of lines is decreased and so the 'up' value must be decreased
            */
      			if((i+lengthhh )%w.ws_col == 0)
              up--;

      			/**
      			  the next lines will move cursor to the right position.
      			  by moving cursor up until reaching the line that was reached by the user.
      			  move the cursor to the right edge of the terminal because we are deleting the char
      			  on the line above the current line at the edge of terminal
            */
      			int upp = up;
            while(upp-- != 0)
      				printf("%c[1A", 27); //move cursor up 'upp' times.

            printf("%c[%dC", 27, w.ws_col); //move cursor to the right edge of the terminal
          }
          else{
            /**
              number of lines .. e.g. lengthhh+i = 81 ==> numberOfLines=2
              up is used to reduce the result by the number of lines that the user reached
            */
  			    int numberOfLines = 1 + ((lengthhh+i - 1) / w.ws_col) - up;

            //return cursor to the first line and to the left edge
            printf("\r");
            while(numberOfLines > 1){
              printf("%c[1A", 27); //move cursor up
              numberOfLines--;
            }

            printf("%c[J", 27); //clear screen from cursor to the end
            prompt();

            str[--curretCursorPosition] = '\0';
            strcat(str, strAppend);
            str[--i] = '\0';
            printf("%s", str);

            /**
              up is decreased iff the remainder of the new length of input+ length of prompt equals zero
              which means the number of lines is decreased and so the 'up' value must be decreased
            */
            if((i+lengthhh )%w.ws_col == 0)
              up--;

            /**
              the next lines will move cursor to the right position.
              by moving cursor up until reaching the line that was reached by the user.
              move the cursor to the right edge of the terminal because we are deleting the char
              on the line above the current line at the edge of terminal
            */
            int upp = up;
            while(upp != 0){ //move cursor up 'upp' times.
              printf("%c[1A", 27);
              upp--;
            }

            //place the cursor at right position on the current line depending on 'curx'
            int positionOfCursor = (i+lengthhh-curx) % w.ws_col;
            if( positionOfCursor > 0)
              printf("\r%c[%dC", 27, positionOfCursor );
        	  else
        		  printf("\r");
          }
          //no need to continue the rest of the while(1) ==> wait for new char
          continue;
        }
      }
      else if(c == 9){ //tab
        //do nothing
      }
      else if(c == '\003'){ //^C
        printf("^C");
        //because of pressing ^C, the cursor must be set at the last line //ADDED 7/5/2017
        while(up != 0){
          printf("%c[1B", 27); //move cursor down
          up--;
        }
        printf("\n");
        str[0] = '\n';
        str[1] = '\0';
        return 1;
      }
      else if(c == '\004'){ //^D
        //do nothing
      }
      else if(c != '\b'){ //Ctrl+backspace
        if(c == '\n'){
          if(str[0] != '\0') //if input is not newline, then add it to history
            addInputToHistory(str);

          //because of pressing enter, the cursor must be set at the last line
          while(up != 0){
            printf("%c[1B", 27); //move cursor down
            up--;
          }

          str[i++] = c; //add the char entered
          printf("%c", c); //print the char entered

          break; //exit while(1)
        }
        //add char entered at the cursor position and increment position
        str[curretCursorPosition] = c;
        //null terminate the string to add the other part of the string (strAppend) to str
        str[curretCursorPosition+1] = '\0';
        int l = (int)strlen(str); //length of str until the new char

        /**
          number of lines .. e.g. lengthhh+i = 81 ==> numberOfLines=2
          up is used to reduce the result by the number of lines that the user reached
        */
        int numberOfLines = 1 + ((lengthhh+i - 1) / w.ws_col) - up;

        //return cursor to the first line and to the left edge //ADDED 7/5/2017
        printf("\r");
        while(numberOfLines > 1){
          printf("%c[1A", 27); //move cursor up
          numberOfLines--;
        }

        printf("%c[J", 27); //clear screen from cursor to the end
        prompt();

        printf("%s", str);

        if((l + lengthhh)%w.ws_col == 0){
          printf("\n"); //to go down one line because of reaching the end of the line
          if(up != 0) //because we went down we must decrease up if the user is went up
            up--;
        }else{
          //calculating the difference before and after adding new char then add the difference to up
          int numberOfLinesBefore = (i-1+lengthhh)/w.ws_col;
          int numberOfLinesAfter = (i+lengthhh)/w.ws_col;
          up = up + (numberOfLinesAfter - numberOfLinesBefore);
        }
        //ADDED END

        strcat(str, strAppend);
        str[++i] = '\0';

        //print the rest of the string
        printf("%s", strAppend);

        //moving cursor up until reaching the line that was reached by the user.
        int upp = up;
        while(upp > 0){
            printf("%c[1A", 27);
            upp--;
          }

        //place the cursor at right position on the current line depending on 'curx' //ADDED 7/5/2017
        int positionOfCursor = (i+lengthhh-curx) % w.ws_col;
        if( positionOfCursor > 0)
          printf("\r%c[%dC", 27, positionOfCursor );
        else
          printf("\r");
      }
    }
    else{
      if(c == '\177' ){ //backspace
        if(str[0] != '\0'){
          int numberOfLines = 1 + ((lengthhh+i - 1) / w.ws_col);

          //if the length equals the width then the cursot is at the next line ==> move it up
          if((i+lengthhh)%w.ws_col == 0) //ADDED 7/5/2017
            printf("\r%c[1A", 27);

          //return cursor to the first line and to the left edge because of pressing enter
          printf("\r");
          while(numberOfLines > 1){
            printf("%c[1A", 27); //move cursor up
            numberOfLines--;
          }

          printf("%c[J", 27); //clear screen from cursor to the end
          prompt();

          str[--i] = '\0'; //remove char and decrement i
          printf("%s", str);

          //if the length equals the width then the cursot is at the right edge ==> move it down
          if((i+lengthhh)%w.ws_col == 0) //ADDED 7/5/2017
            printf("\n");

          continue;
        }
      }
      else if(c == 9){ //tab
        //do nothing
      }
      else if(c == '\003'){ //^C
        printf("^C\n");
        str[0] = '\n';
        str[1] = '\0';
        return 1;
      }
      else if(c == '\004'){ //^D
        if(i == 0){
          printf("exit\n");
          return 0;
        }
      }
      else if(c != '\b'){ //Ctrl+backspace
    	  if(c == '\n'){
          if(str[0] != '\0')
    		    addInputToHistory(str); //if input is not newline, then add it to history

    		  str[i++] = c; //add the char entered
          printf("%c", c); //print the char entered
    		  break; //exit while(1)
    	  }
        str[i++] = c; //add the char entered
        printf("%c", c); //print the char entered

        //if the length equals the width then the cursot is at the right edge ==> move it down
        if((i+lengthhh)%w.ws_col == 0) //ADDED 7/5/2017
          printf("\n");
      }
    }
  }
  str[i++] = '\0';

  /* Restore old settings */
  tcsetattr(0, TCSANOW, &old_term);

  return 1;
}
