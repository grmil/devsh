#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// TODO: Make this dynamic
#define MAX_ARGS 10

/*
 * reads in a dynamically allocated line from stdin
 *
 * line: where to store the string read in
 * returns the number of characters read in or -1 if an error
 */
int read_line(char **line) {
    *line = NULL;
    size_t size = 0;
    ssize_t nread;

    // getline allocates reallocates instead of using a set size
    if ((nread = getline(line, &size, stdin)) == -1) {
      return -1;
    }

    return (int) nread;
  }

/*
 * separates args in a string by space chars
 *
 * line: string that is read in from stdin
 * args: where to store the parsed args for future use
 * does not return anything
 */
void parse_line(char *line, char *args[]) {
  char *token; 

  // give strtok the string to work on in the first call
  token = strtok(line, " \n");

  int i = 0;
  while (token != NULL) {
    args[i] = token;

    // later calls to strtok use the same string as the first
    token = strtok(NULL, " \n");
    i++;
  }

  // add null arg for exec syscall to work
  args[i] = NULL;
}

/*
 * creates and runs the desired process
 *
 * args: takes in an array of args
 * line: passed in to free from child processes
 * returns the error code
 */
int execute_args(char **args, char *line) {
  pid_t pid;

  // create child process
  pid = fork();

  if (pid < 0) {
    return -1;

  } else if (pid > 0) {
    // parent execution
    int wstatus;
    waitpid(pid, &wstatus, 0);

    // macros to decode wstatus
    if (WIFEXITED(wstatus)) {
      return WEXITSTATUS(wstatus);
    }

  } else if (pid == 0) {
    // child execution
    if (execvp(args[0], args) == -1) {
      free(line);
      exit(127);
    }
  }
  return -1;
}

/*
 * main execution loop of the shell
 * no args are needed and it returns nothing
 */
void sh_loop() {
  while (true) {
    char *line = NULL;
    char *args[MAX_ARGS];
    int nread;

    // Output the prompt
    // env can be set up here later for custom prompt
    const char *user = getenv("USER");
    const char *hostname = getenv("HOSTNAME");
    const char *dir = getenv("PWD");
    printf("\n%s@%s %s\n> ", user, hostname, dir);

    // handle reading failures
    // nread returns num chars on success
    if ((nread = read_line(&line)) == -1) {
      free(line);
      break;
    };

    // populating `args` using the line that is read in from stdin
    parse_line(line, args);


    // Skip blank commands
    if (args[0] == NULL) {
      free(line);
      continue;
    } 

    // Run built-in quit command
    if (strcmp(args[0], "quit") == 0) {
      free(line);
      break;
    }
    
    // create and run the desired process (fork-exec)
    int estatus;
    estatus = execute_args(args, line);

    switch (estatus) {
      case -1:
        perror("Execution Error");
        break;

      case 127:
        fprintf(stderr, "%s: command not found\n", args[0]); 
        break;
    }
    free(line);
  }  
}

int main(int argc, char **argv) {

  // Initialization Stuff -- config, default execution
  sh_loop();
  // Cleanup -- kill relevant ps
}
