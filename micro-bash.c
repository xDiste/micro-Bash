#include "parser.h"

int main(int argc, char **argv){

    char *line = NULL;
    char curr_path[MAXLEN];
    char *cmd[MAXLEN];
    char prompt[MAXLEN];

    do{
        
        fflush(stdin);  // clean up the standard input
        getcwd(curr_path, sizeof(curr_path)); // I insert in "buff" the address that points to the path of the current directory
        
        strcpy(prompt,curr_path);
        strcat(prompt," $ ");

        line = readline(prompt);

        // if the user sends the EOF signal with the key sequence "ctrl + d" or types quit, the micro-bash closes
        if (!line || strcmp("quit",line) == 0) {
            printf("\n");
			exit (EXIT_SUCCESS);
		}

        // if you hit enter with no commands or arguments, return up
        if(strlen(line) == 0)
            continue;
        
        // if something has been inserted in the line, I save it in history
        if(line && *line)
            add_history(line);


        // if there is one or more pipes I execute the command sequence parser
        if(checkPipe(line)){
            parser_seq(line);
        }
        
        else{
            // I run the line parser
            parser_cmd(line,cmd);

            // if the command from the user is "cd" it is the only built-in
            if((strcmp(cmd[0],"cd") == 0)){

                // I execute the auxiliary function created to handle the "cd" command
                runCd(cmd,curr_path);
            }

            // otherwise if the command is not "cd"
            else{
                // I execute the auxiliary function that allows me to execute the command
                runCommand(cmd);
            }
        }

        free(line);
        
    }while(1);

    return 0;
}



