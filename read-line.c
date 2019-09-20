/*
 * CS252: Systems Programming
 * Purdue University
 * Example that shows how to read one line with simple editing
 * using raw terminal.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER_LINE 2048
extern void tty_raw_mode(void);

// Buffer where line is stored
int line_length;
char line_buffer[MAX_BUFFER_LINE];

// Simple history array
// This history does not change. 
// Yours have to be updated.
int history_index = 0;
char * history [128];
//int history_length = sizeof(history)/sizeof(char *);
int history_length = 0;

void read_line_print_usage()
{
  char * usage = "\n"
    " ctrl-?       Print usage\n"
    " Backspace    Deletes last character\n"
    " up arrow     See last command in the history\n";

  write(1, usage, strlen(usage));
}

/* 
 * Input a line with some basic editing.
 */
char * read_line() {

  // Set terminal in raw mode
  tty_raw_mode();

  line_length = 0;
  int line_loc = line_length;

  // Read one line until enter is typed
  while (1) {

    // Read one character in raw mode.
    char ch;
    read(0, &ch, 1);

    if (ch>=32) {
    
        //backspace
        if(ch == 127)
        {
            //if there is something typed. otherwise don't do anything
            if (line_length > 0 && line_loc > 0)
            {
                ch = 8;
                write(1,&ch,1);
                //write space to erase last character
                ch = ' ';
                write(1, &ch, 1);

                ch  = 8;
                write(1, &ch, 1);
                line_length--;
                line_loc--;
            }
            continue;

        }
      // It is a printable character. 

      // Do echo
      write(1,&ch,1);

      // If max number of character reached return.
      if (line_length==MAX_BUFFER_LINE-2) break; 

      // add char to buffer.
      line_buffer[line_length]=ch;
      if (line_loc == line_length)
      {
            line_length++;
      }
      line_loc++;
    }
    else if (ch==10) {
      // <Enter> was typed. Return line
      
      // Print newline
      write(1,&ch,1);

      break;
    }
    else if (ch == 31)
    {
        read_line_print_usage();
        line_buffer[0]=0;
        break;
    }
    else if (ch == 8) {
      // <backspace> was typed. Remove previous character read.
        if(line_length > 0 && line_loc > 0) {
            for ( int i =0; i < line_loc; i++)
            {
                ch = 27;
                write(1, &ch, 1);
                ch = 91;
                write(1, &ch, 1);
                ch = 68;
                write(1, &ch, 1);
            }
            line_loc--;

            char new_line_buffer[sizeof(line_buffer)];
            int i =0;
            for (int j = 0; j < line_length; j++)
            {
                if ( j != line_loc)
                {
                    new_line_buffer[i] = line_buffer[j];
                    write(1, &new_line_buffer[i],1);
                    i++;
                }
            }
            ch = ' ';
            write(1, &ch, 1);

            for (int i = 0; i < line_length-line_loc;i++)
            {
                ch = 27;
                write(1, &ch, 1);
                ch = 91;
                write(1, &ch, 1);
                ch = 68;
                write(1, &ch, 1);
            }
            line_length--;
            strncpy(line_buffer, new_line_buffer, line_length);
        }
    }
    else if (ch == 4)
    {
        if ( line_length > 0 && line_loc < line_length)
        {
            for (int i = 0; i < line_loc; i++)
            {
                ch = 27;
                write(1, &ch, 1);
                ch = 91;
                write(1, &ch, 1);
                ch = 68;
                write(1, &ch, 1);
            }
            char new_line_buffer[sizeof(line_buffer)];
            int i =0;
            for (int j = 0; j < line_length; j++)
            {
                if (j != line_loc)
                {
                    new_line_buffer[i] = line_buffer[j];
                    write(1, &new_line_buffer[i], 1);
                    i++;
                }
            }
            ch = ' ';
            write(1, &ch, 1);
        
            for ( int i =0; i < line_length-line_loc; i++)
            {
                ch = 27;
                write(1, &ch, 1);
                ch = 91;
                write(1, &ch, 1);
                ch = 68;
                write(1, &ch, 1);
            }
            line_length--;
            strncpy(line_buffer, new_line_buffer, line_length);

        }
    }
    //ctrl-A
    else if (ch == 1)
    {
        while (line_loc > 0)
        {
            ch = 8;
            write(1, &ch, 1);
            line_loc--;
        }
    }
    //ctrl-E
    else if (ch == 5)
    {
        while(line_loc != line_length)
        {
            ch = 27;
            write(1, &ch, 1);
            ch = 91;
            write(1, &ch, 1);
            ch = 67;
            write(1, &ch, 1);
            line_loc++;
        }
    }
    if (ch==27) {
      // Escape sequence. Read two chars more
      //
      // HINT: Use the program "keyboard-example" to
      // see the ascii code for the different chars typed.
      //
      char ch1; 
      char ch2;
      read(0, &ch1, 1);
      read(0, &ch2, 1);
    //up
    if (ch1==91 && ch2==65) {
	    int i = 0;
	    for (i =0; i < line_length; i++) {
	        ch = 8;
	        write(1,&ch,1);
	    }

	    // Print spaces on top
	    for (i =0; i < line_length; i++) {
	        ch = ' ';
	        write(1,&ch,1);
	    }

	    // Print backspaces
	    for (i =0; i < line_length; i++) {
	        ch = 8;
	        write(1,&ch,1);
	    }   	

	    // Copy line from history
        if (history_length > 0 && history_index >= 0)
        {
            //printf("up: %d\n", history_index);
	        strcpy(line_buffer, history[history_index--]);
	        history_index=(history_index)%history_length;
            if (history_index == -1)
            {
                history_index = history_length - 1;
            }
         line_length = strlen(line_buffer);
        }
	    // echo line
	    write(1, line_buffer, line_length);
        line_loc = line_length;
    }
    //down
    else if (ch1 == 91 && ch2 == 66)
    {
        //erase old line
        //print backspaces
        for (int i =0; i < line_length; i++)
        {
            ch = 8;
            write(1, &ch ,1);
        }
    
        for (int i = 0; i < line_length; i++)
        {
            ch = ' ';
            write(1, &ch, 1);
        }
        for (int i = 0; i < line_length; i++)
        {
            ch = 8;
            write(1, &ch, 1);
        }
        //
        if (history_length > 0 && history_index <= history_length-1)
        {
            strcpy(line_buffer, history[history_index++]);
        }
        else if (history_index == history_length)
        {
            history_index = history_length - 1;
            strcpy(line_buffer, "");
        }
        line_length = strlen(line_buffer);
        write(1, line_buffer, line_length);
        line_loc = line_length;
    }
    //left
    else if (ch1 == 91 && ch2 == 68)
    {
        if (line_loc > 0 && line_length > 0)
        {
            ch = 27;
            write(1, &ch, 1);
            ch = 91;
            write(1, &ch, 1);
            ch = 68;
            write(1, &ch, 1);
            line_loc--;
            
        }
    }
    //right
    else if (ch1 == 91 && ch2 == 67)
    {
        if (line_loc < line_length)
        {
            ch = 27;
            write(1, &ch, 1);
            ch = 91;
            write(1, &ch, 1);
            ch = 67;
            write(1, &ch, 1);
            line_loc++;	
        }
    }
        
    }

  }

  // Add eol and null char at the end of string
  line_buffer[line_length]=10;
  line_length++;
  line_buffer[line_length]=0;
    
  //update history
  history[history_length] = (char *)malloc(strlen(line_buffer) * sizeof(char) +1);
  strcpy(history[history_length++], line_buffer);
  history[history_length-1][strlen(line_buffer)-1] = '\0';
  history_index = history_length-1;
  //tty_raw_mode(); 
  return line_buffer;
}

