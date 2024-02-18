//Aphrodite Script - Linux Shell in C
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
//#include <sys/type.h>
#include <wait.h>
#include <ctype.h> //For isspace function

//Displaying the current working directory and taking user input
char* CommandPrompt()
{
    //Using buffer to store the current working directory
    char cwd[1024];
    
    //Rereieving the current working directory
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        //Print the current working directory with $ ahead
        printf("%s $ ", cwd);  
    } else {
        perror("getcwd() error");
    }

    //Getting user input
    char* input = malloc(256);
    if (fgets(input, 256, stdin) != NULL) {
        size_t inputLength = strlen(input);
        //Removing newline character from end of the input
        if (inputLength > 0 && (input[inputLength -1] == '\n')) {
            input[inputLength -1] = '\0';
        }
        //Returning the input
        return input;
    }
    else {
        perror("fgets() error"); // error handling 
    }
    free (input);
    return NULL;
}

//Storing the shell command 
struct ShellCommand { 
    char* command;
}; 

//Processing the user input
struct ShellCommand ParseCommandLine(char *input) {
    struct ShellCommand command;
    command.command = input;
    return command;
}

char* remove_space(char* str) {
    //Skipping leading white spaces
    //Finding the first occurence of ' ', '\t' and '\n', using strpbrk
    str = strpbrk(str, " \t\n"); 
    //str being FALSE means there is an empty string, we return the string
    if (!str) return str;      
    //strchr is used to find the first occurence of a character, this case '\0'
    char* end = strchr(str, '\0');

    //Removing trailing white spaces
    //isspace checks whether a character has a white space    
    while (end > str && isspace(end[-1])){
	    end--;
    }
    //Adding null character in the end
    *end = '\0';                
    return str;
}

//Executing the shell command
void ExecuteCommand(struct ShellCommand command){
    //Initiating a child process
    pid_t PID = fork(); 
    
    if (PID == -1) {
        perror("fork error"); // error handling 
        exit(1);
    }

    //Child process
    if (PID == 0 ) {
    	//Exiting the shell, if user types exit
	if (strcmp(command.command, "exit") == 0) {
            exit(0);
        }	
    	
	//Implementing the change directory (cd) command
    	//Changing current directory to the previous directory 
        if (strcmp(command.command, "cd ..") == 0) {
           //chdir() changes the directory to the path passed as argument
	   //'..' means parent directory
	   //chdir() returning -1 indicates failure, 0 indicates success 
	   if (chdir("..") == -1) { 
	       perror("cd .. error"); // error handling
               exit(1);
            } 
	}
	
	//Implemting cd to change directory to the specified path
        else if (strncmp(command.command, "cd ", 3) == 0) {
	    //Taking into account everything after "cd "
            char* directory = command.command + 3; 
            //Changing the directory
	    if (chdir(directory) == -1) {  
                perror("cd error"); // error handling 
                exit(1);
            }
        }

	//Implementing pwd command, printing out the current working directory 
        else if (strcmp(command.command, "pwd") == 0) {
            char* args_list[] = {"pwd", NULL};
            if (execvp("pwd", args_list) == -1) {
                perror("execvp error");
                exit(1);
            }
        }

	//Impementing the ls command, along with -l, -s, -h
        else if (strncmp(command.command, "ls", 2) == 0) {
            char* args[4];
	    //The first argument will be ls
            args[0] = "ls"; 

            //Tokenzing the input in case there are multiple arguments
	    //e.g., ls -l
	    char* tokens[4];
            int counter = 0;
            char* token = strtok(command.command, " ");
            //Calculating the number of arguments through counter
	    while(token != NULL) {
                tokens[counter] = token;
                token = strtok(NULL, "");
                counter++;
            }

            //Populating the args array if there are more than one arguments
            if (counter > 1) {
                for (int i = 0; i < counter; i++) {
                    args[i] = tokens[i];
                }
                //Indicating end of the array by setting last spot to NULL
		//Putting in NULL will ensure execvp runs without errors  
                args[counter] = NULL;
            }
            else {
                args[1] = NULL;
            }

            //Executing the shell command using execvp
            if (execvp("ls", args) == -1) {
                perror("execvp error"); // error handling 
                exit(1);
            }
        }
    } else {
	//Waiting for child process to terminate
	waitpid(PID, NULL, 0);
    }
}


int main() {
        char* input;
	struct ShellCommand command;
        // struct ShellCommand command;
        // repeatedly prompt the user for input
        for (;;)
        {
                input = CommandPrompt();
                // parse the command line
                command = ParseCommandLine(input);
                // execute the command
                ExecuteCommand(command);
        }
        exit(0);
}

	

