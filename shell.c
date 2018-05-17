#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

#define MAX_CMD_LEN 1024
#define MAX_CWD_LEN 1024

// shamelessly copied
char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

char *getCurrentPath(void) {
  char cwd[MAX_CWD_LEN];
  return getcwd(cwd, sizeof(cwd));
}

int parseCommand(char *tcmd) {
  // trim whitespaces
  char *cmd;
  cmd = trimwhitespace(tcmd);

  // TODO check and implement PIPE

  const char delim[2] = " ";
  char *token;
  token = strtok(cmd, delim);

  printf("parse Command: %s\r\n", token); // DEBUG

  int result = 0;
  if (strcmp(token, "exit") == 0) {
    // exit console
    printf("terminating shell...\r\n");
    result = 1;
  } else if (strcmp(token, "wait") == 0) {
    // exit console
    printf("wait\r\n");
    result = 1;
  } else if (strcmp(token, "cd") == 0) {
    // cd command
    
    // get the path (token no. 2)
    token = strtok(NULL, delim);

    printf("cd command to %s\r\n", token); // DEBUG
    
    if (chdir(token) != 0) {
      printf("-shell: cd %s: no such Directory\r\n", token);
    } 
  } else {
    // execute program
    printf("exec\r\n"); // DEBUG

    // create the system exec command
    char exec_cmd[MAX_CMD_LEN] = "./";
    strcat(exec_cmd, token);

    // TODO add the command parameters

    printf("exec %s\r\n", exec_cmd); // DEBUG
          
    int status = system(exec_cmd);
    printf("status %d\r\n", status);

    result = 0;
  }

  return result;
}

int changeDirectory(char *path) {
  return chdir(path);
}

int main(void)
{
  char input[MAX_CMD_LEN];
  char *path;

  int result = 0;
  while(result == 0) {

    path = getCurrentPath();
  
    printf("%s > ", path);

    if (fgets(input, MAX_CMD_LEN, stdin) != NULL) {

      // remove trailing newline
      // find position of new line
      // replace with terminating 0
      char *newLinePos;
      if ((newLinePos=strchr(input, '\n')) != NULL) {
        *newLinePos = '\0';
      }

      if (strcmp(input, "\0") != 0) {
        // TODO tokenize on & (see below)
        // fork processes
        result = parseCommand(input);
      } else {
        result = -1;
      }
  
      // tokenize commands
      // const char delim[2] = "&";
      // char *token;
  
      // token = strtok(input, delim);
  
      // while (token != NULL) { 
      //   printf("%s", token);
      //   printf("\n\r");
      //   token = strtok(NULL, delim);
      // }
  
    }
  }

	return 0;
}
