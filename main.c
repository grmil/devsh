#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

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
int parse_line(char *line, char ***args_ptr) {
  size_t size = 8;
  char **args = malloc(size * sizeof(char *));
  if (args == NULL) {
    return -1;
  }

  // give strtok the string to work on in the first call
  char *token = strtok(line, " \n");

  size_t i = 0;
  while (token != NULL) {

    if (i >= size) {
      size *= 2;
      
      // Reallocate if more memory is needed
      char **tmp = realloc(args, size * sizeof(char *));
      if (tmp == NULL) {
        free(args);
        return -1;
      }
      args = tmp;
    }

    args[i] = token;

    // later calls to strtok use the same string as the first
    token = strtok(NULL, " \n");
    i++;
  }

  // Reallocate if memory is needed to add NULL arg
  if (i >= size) {
    size += 1;

    char **tmp = realloc(args, size * sizeof(char *));
    if (tmp == NULL) {
      free(args);
      return -1;
    }
    args = tmp;
  }

  // add null arg for exec syscall to work
  args[i] = NULL;
  *args_ptr = args;
  return 0;
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
      free(args);
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
    char **args = NULL;
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
      continue;
    };

    // populating `args` using the line that is read in from stdin
    int status;
    if ((status = parse_line(line, &args)) == -1) {
      perror("Parsing Error");
      free(line);
      free(args);
      continue;
    }


    // Skip blank commands
    if (args[0] == NULL) {
      free(line);
      free(args);
      continue;
    } 

    // Run built-in quit command
    if (strcmp(args[0], "quit") == 0) {
      free(line);
      free(args);
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
    free(args);
  }  
}

int main() {

  // Initialization Stuff -- config, default execution
  sh_loop();
  // Cleanup -- kill relevant ps
}
