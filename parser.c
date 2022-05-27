#include "parser.h"

// auxiliary function that takes care of checking if the pipe is inside the command line entered by the userbool checkPipe(char *line)
bool checkPipe(char *line)
{
        return (strstr(line,"|") != NULL);
}

// auxiliary function that controls which stream you want to redirect
int checkStream(char *line)
{   
    if(line == NULL)
        return -1;
    
    // return 0 for input redirection
    if(strstr(line,"<") != NULL)
        return 0; 
    
    // return 1 for output redirection
    if(strstr(line,">") != NULL)
        return 1;
    else
        return -1;
}

// auxiliary function that redirect the stdin
void stream_read(char *token)
{
    token = strtok(token,"<");

    // If the file name is not specified I point out the error
    if(token == NULL)
    {
        perror("Errore di sintassi: non è specificato il file per la redirezione.");
        return;
    }
    freopen(token,"r",stdin);
}

// auxiliary function that redirect the stdout
void stream_write(char *token)
{
    token = strtok(token,">");

    // If the file name is not specified I point out the error
    if(token == NULL)
    {
        perror("Errore di sintassi: non è specificato il file per la redirezione.");
        return;
    }
    freopen(token,"a",stdout);
   
}

// auxiliary function that controls if the "CD" command is in the line of the commands entered by the user
bool check_cd(char *line)
{
    return (strstr(line,"cd") != NULL);
}

// auxiliary function that takes care of checking if in one of the controls inside the pipe there is a redirduation of the input / output
int parserStreamSeq(char **cmd)
{
    while(*cmd != NULL){
        *cmd++;

        if(checkStream(*cmd) == 1)
            return 1;
        else 
            if(checkStream(*cmd) == 0)
                return 0;
    }
    return -1;
}


// function that takes care of creating pipe and executing commands with redirected input and output descriptor files
void pipeCmds(char *cmds[],int n) 
{
    char *cmd[MAXLEN];
    int newPipe[2], oldPipe[2];
    int stream, status;
    pid_t pid;

    // for each command
    for(int i = 0; i < n; i++){
        // I run the command in the sequence
        parser_cmd(cmds[i],cmd);

        // Control if there are stream redirections in the command
        stream = parserStreamSeq(cmd);
        
        
        // If the first cannot redirect the output
        if(i == 0 && stream == 1)
        {
            perror("Errore di sintassi: output primo comando");
            return;
        }
        // If the last cannot redirect the input
        else if(i == n-1 && stream == 0)
        {
            perror("Errore di sintassi: input ultimo comando");
            return;
        }
        // If the command is among other commands I can't do anything
        else if(i > 0 && i < n-1 && (stream == 0 || stream == 1))
        {
            perror("Errore di sintassi: input/output in uno dei comandi in mezzo alla pipe");
            return;
        }

        // if there are commands to be performed
        if(i < n-1){
            pipe(newPipe);  // I create a new pipe
        }

        pid = fork();   // I run fork

        // child process
        if(pid == 0){
            // If I'm not running the first command
            if(i > 0){
                close(oldPipe[1]);  // I close the descriptor file in pos 1

                dup2(oldPipe[0], STDIN_FILENO);    /* I run the DUP2 that performs the same task of DUP but does not use the smallest descriptor file
                                        Not used but use the descriptor file number specified as a second topic */
                
                close(oldPipe[0]);  // I close the descriptor file in pos 0
            }

            // If there are still commands to be performed
            if(i < n-1){
                close(newPipe[0]);  // I close the fileDescriptor in POS 0
                dup2(newPipe[1], STDOUT_FILENO);    // replaces the descriptor file
                close(newPipe[1]);  // I close the descriptor file in pos 1
            }

            // I run the command and at the same time the return value
            runCommand(cmd);
            exit(1);
        }
        // father process
        else{
            // If I'm not running the first command
            if(i > 0) 
            {   
                // I close the old files descriptor
                close(oldPipe[0]);
                close(oldPipe[1]);
            }

            // If there are other commands to run
            if(i < n-1){
                // update the descriptor files of the old pipe
                oldPipe[0] = newPipe[0];
                oldPipe[1] = newPipe[1];
            }

            // I wait for the child process            
            wait(&status);
            
            if (WIFEXITED(status) == 0)
            {
                printf("Errore: EXIT STATUS %d\n", WEXITSTATUS(status));
            }
            
        }
    }

}

// auxiliary function that takes care of counting pipes
int countPipes(char *line)
{
    int nPipes = 0;
    
    for(int i = 0; line[i] != '\0'; ++i){
        if(line[i] == '|')
            ++nPipes;
    }
    return nPipes;
}

// auxiliary function that controls command is composed only from empty spaces
bool emptyCommand(char *line)
{
    while (*line != '\0') {
        if (!isspace((unsigned char)*line))
            return false;
        *line++;
    }
    return true;
}


// Function that deals with the parsing of the command sequence introduced by the user
void parser_seq(char *line)
{   
    // If "cd" is present in the command sequence -> Syntax error
    if(check_cd(line)){
        perror("Errore di sintassi: cd ");
        return;
    }

    char *buff;
    char *cmd[MAXLEN];
    char *commands[MAXLEN];

    memset(&commands,0, sizeof(commands));

    int countCmds = 0, nPipes = countPipes(line);

    // for the whole sequence
    while(*line != '\0'){

        // extrapolo the tokens and use as a delimiter the pipe represented by "|"
        buff = strtok_r(line, "|", &line);

        // If there is an empty command I point out the syntax error and do not continue parsing
        if(emptyCommand(buff))
        {
            perror("Errore di sintassi: comando vuoto.");
            return;
        }    

        // For each token that ends with the space, I replace the space with a string end
        if(buff[strlen(buff)-1] == ' '){
            buff[strlen(buff)-1] = '\0';
        }

        // I insert the token inside the controls array
        commands[countCmds++] = buff;

    }
    
    // If the number of pipes is different from the (number of commands) -1 means that in some pipe the command is missing
    if(countCmds-1 != nPipes)
    {
        perror("Errore di sintassi: pipe senza comandi");
        return;
    }

    // I call the function to run multi-pipe
    pipeCmds(commands,countCmds);

}


// Function that runs the parsing of a single command entered by the user and returns it in Buff "Ready to run"
void  parser_cmd(char *line, char **buff){
    char *tmp;

    // If the command is empty I do nothing
    if(line == NULL){
        return;
    }

    // for the whole command
    while(*line != '\0'){

        // extrapolo the token using " " as a delimiter
        tmp = strtok_r(line, " ", &line);
        
        // If the token is formed by only spaces I don't consider it
        if(tmp == NULL)
            continue;

        // If the token has the unique operator prefix '$' I am looking for the corresponding environment variable
        if(tmp[0] == '$')
        {
            tmp = strtok(tmp,"$");
            tmp = getenv(tmp);
            *buff++ = tmp;
        }
        
        else
        {
            *buff++ = tmp;
        }
    }

    *buff = NULL;   // At the end I insert a null to signal that I finished
}


// auxiliary function performing a single command with and without argument
void runCommand(char **buff)
{
    pid_t pid;
    int status;
    // I create a child process to execute all other non built-in commands
    pid = fork();

    // in the child process
    if(pid == 0){
        
        int i = 0;
        
        char **aux = buff;

        // for the whole command
        while(*aux != NULL){
            
            *aux++;    // I move forward
        
            // If I find a redirdation of the output
            if(checkStream(*aux) == 1)
            {
                char *tmp;
                tmp = strstr(*aux,">");    // I move to the file name
                stream_write(tmp);  // I call the auxiliary_write function with the name of the file to be generated
                *aux = NULL;   // I take off the file name to execute it from the command topics
            }

            // If I find a redirduation of the input
            else if(checkStream(*aux) == 0)
            {
                char *tmp;
                tmp = strstr(*aux,"<");    // I move to the file name
                stream_read(tmp);   // I call the auliary strem_read function with the name of the file to read
                *aux = NULL;   // I take off the name of the file to execute it to execute it
            }   
        }

       // I run the command exec giving the content of the cmd and the address of cmd and if it is not successful I return a mistake
        if(execvp(*buff,buff) < 0){
            perror("Comando non trovato ");
        }
    }

    wait(&status); // The father process remains on hold
    
    if (WIFEXITED(status) == 0)
    {
        printf("EXIT STATUS %d\n", WEXITSTATUS(status));
    }
}

// function that takes care of executing the "CD" command 
void runCd(char **cmd_cd, char *curr_path)
{   
    if(cmd_cd[2] != NULL)
    {
        perror("Errore di sintassi: cd ha troppi argomenti");
        return;
    }
    // If the topic is not there or it is "~" then I move to the Home folder
    if(cmd_cd[1] == NULL  || (strcmp(cmd_cd[1], "~") == 0) )
    {
        chdir(getenv("HOME"));
    }
    else
    {
        // I put the directory to which I want to log in in curl_path
        strcat(curr_path,"/");
        strcat(curr_path,cmd_cd[1]);

        // Use ChDir to change the Working Directory
        chdir(curr_path);
    }
}
