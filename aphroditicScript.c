#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <pwd.h>

#define MAX_COMMAND_LENGTH 1024 // Constant value for the length of input command
#define MAX_ARGS 64		// Constant value for the number of arguments in the input command
char initialDir[1024];		// Variable that holds the initial directory, used for getcwd()

// Using struct to hold the parsed input command
struct ShellCommand {
	char* args[MAX_ARGS];
	int in;
	int out;
	char* in_file;
	char* out_file;
};

// Functions to be implemented:
char* CommandPrompt(); // Getting the user input
struct ShellCommand ParseCommandLine(char* input); // Processing the user input (As a shell command)
void ExecuteCommand(struct ShellCommand command); //Executing the shell command
void UpdatePrompt(); // Prints the current working directory

char* CommandPrompt()
{
	// Dynamically allocating the memory
	char* userInput = malloc(MAX_COMMAND_LENGTH * sizeof(char));
	
	// Error handling
	if(userInput == NULL){
		perror("malloc failure");
		exit(EXIT_FAILURE);
	}
	
	// Getting the user input
  	fgets(userInput, MAX_COMMAND_LENGTH, stdin);
	
	// Removing the newline character
	userInput[strcspn(userInput, "\n")] = 0;
	
	return userInput;
}

struct ShellCommand ParseCommandLine(char* input)
{
	struct ShellCommand command;
	char *token;
	int i = 0;
	
	// Initilizing the command struct
	memset(&command, 0, sizeof(struct ShellCommand));

	// Tokenizing the command
	token = strtok(input, " ");

	// Looping until token is NULL or there is one spot left in args[]
	// Last spot for args[] will be kept NULL, so execvp can funtion properly
	while (token != NULL && i < MAX_ARGS - 1) {
		if (strcmp(token, "<") == 0) {
			//Input redirection
			command.in = 1;
			token = strtok(NULL, " ");
			command.in_file = token;
		}
		else if (strcmp(token, ">") == 0) {
			command.out = 1;
			token = strtok(NULL, " ");
                        command.out_file = token;
		}
		else {
			command.args[i++] = token;
		}

		token = strtok(NULL, " ");
	}
	command.args[i] = NULL; // Ending args[] with NULL for execvp compatibility
	return command;
}


void ExecuteCommand(struct ShellCommand command) 
{	
	// Checking if the user inputed the cd command
        if(strcmp(command.args[0], "cd") == 0){
              // Changing the directory if the command is cd
	      // The second argument (after cd) is passed into chdir()
              if( chdir(command.args[1]) != 0){
                perror("cd failed");
                }
              return;
             }
	
	// Forking a child
	pid_t pid = fork();
	
	if(pid < 0){
		perror("fork failed");
		exit(EXIT_FAILURE);
	}
	
	// Child process
	else if (pid == 0)
	{
		// Handling input redirections
		// command.in being TRUE (1) indicates input redirection > was found in the command 
		if (command.in) {
			int fd = open(command.in_file, O_RDONLY);
			if(fd < 0){
				perror("open input file failed");
				exit(EXIT_FAILURE);
			}
			dup2(fd, STDIN_FILENO);
			close(fd);
		}	
		// Handling output redirections
		// command.out being TRUE (1) indicates output redirection < was found in the command
		if (command.out) {
			int fd = open(command.out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		        if( fd< 0){
				perror("open output file failed.");
				exit(EXIT_FAILURE);
			}	
			dup2(fd, STDOUT_FILENO);
			close(fd);		
		}

		execvp(command.args[0], command.args);
		perror("execvp failed");
		exit(EXIT_FAILURE);
	}
	else {
		// Parent process
		int status;
		//  Waiting for the child process to complete
		waitpid(pid, &status, 0);
	}
}

// Getting and printing the current working directory
void UpdatePrompt(){

	char cwd[1024]; // Used to store the current working directory obtained from getcwd()
	// Getting the current working directory
	if(getcwd(cwd, sizeof(cwd)) != NULL) {
		// Printing the current working directory
		// Coloring the $ sign using ANSI escape characters
		// \033[94m - blue, \033[0m - white
		printf("%s\033[94m$\033[0m ", cwd);
	}else{
		perror("getcwd() error");
                exit(EXIT_FAILURE);
	}
}

int main() {
	char* input;
	struct ShellCommand command;
		
	if (getcwd(initialDir, sizeof(initialDir)) == NULL) {
                perror("getcwd() error");
		exit(EXIT_FAILURE);
        }
        
	// Repeatedly prompt the user for input
	for (;;)
	{
		// Printing out the prompt
		UpdatePrompt();

		// Getting user input
		input = CommandPrompt();

		// Checking if the user typed exit as a command
		if(strcmp(input, "exit") == 0){
			free(input); // Freeing the dynamilcally allocated memory
			break; // Breaking from the loop, exiting the program
		}

		// Parsing the command line
		command = ParseCommandLine(input);
		
		// Executing the command
		ExecuteCommand(command);

      		// Freeing dynamically allocated memory for the input
       		free(input);
	}
	return 0;
}




