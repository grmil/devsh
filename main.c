#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// TODO: Make this dynamic
#define MAX_ARGS 10

int read_line(char **line) {
    *line = NULL;
    size_t size = 0;
    ssize_t nread;

    if ((nread = getline(line, &size, stdin)) == -1) {
      return -1;
    }

    return (int) nread;
  }

  void parse_line(char *line, char *args[]) {
    char *token; 

    token = strtok(line, " \n");

    int i = 0;
    while (token != NULL) {
      args[i] = token;
      token = strtok(NULL, " \n");
      i++;
    }
    args[i] = NULL;
  }

void execute_args(char **args, char *line) {
  pid_t pid;

  pid = fork();

  if (pid < 0) {
    perror("Fork Failure");

  } else if (pid > 0) {
    int wstatus;
    waitpid(pid, &wstatus, 0);

  } else if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      printf("%s: command not found\n", args[0]); 
      free(line);
      exit(127);
    }
  }
}

void sh_loop() {
  while (true) {
    char *line = NULL;
    char *args[MAX_ARGS];
    int nread;

    // Output the prompt, can set this up as a custom string later
    const char *hostname = getenv("HOSTNAME");
    const char *user = getenv("USER");
    const char *dir = getenv("PWD");
    printf("\n%s@%s %s\n> ", user, hostname, dir);

    if ((nread = read_line(&line)) == -1) {
      free(line);
      break;
    };

    parse_line(line, args);

    // Skip blank reads
    if (args[0] == NULL) {
      free(line);
      continue;
    } 

    // Run built-ins
    if (strcmp(args[0], "quit") == 0) {
      free(line);
      break;
    }
    
    execute_args(args, line);

    free(line);
  }  
}

int main(int argc, char **argv) {

  // Initialization Stuff -- config, default execution
  sh_loop();
  // Cleanup -- kill relevant ps
}
