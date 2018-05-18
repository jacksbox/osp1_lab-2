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

// finds position of newline in string and replaces it with terminating 0
void removeTrailingNewLine(char *input) {
  char *newLinePos;
  if ((newLinePos=strchr(input, '\n')) != NULL) {
    *newLinePos = '\0';
  }
}

char *getCurrentPath(void) {
  char cwd[MAX_CWD_LEN];
  return getcwd(cwd, sizeof(cwd));
}

int parseCommand(char *input) {
  // trim whitespaces
  char *command;

  command = trimwhitespace(input);

  // TODO check and implement PIPE

  char *cmd;
  char *args;
  const char delim[2] = " ";
  // split in command and arguments
  cmd = strtok(command, delim);
  args = strtok(NULL, "");

  printf("parse Command: %s\r\n", cmd); // DEBUG
  printf("with Arguments: %s\r\n", args); // DEBUG

  int result = 0;
  if (strcmp(cmd, "exit") == 0) {
    // exit console
    printf("terminating shell...\r\n");
    result = 1;
  } else if (strcmp(cmd, "wait") == 0) {
    // exit console
    printf("wait\r\n");
    result = 1;
  } else if (strcmp(cmd, "cd") == 0) {
    // cd command

    printf("cd command to %s\r\n", args); // DEBUG

    if (chdir(args) != 0) {
      printf("-shell: cd %s: no such Directory\r\n", args);
    }
  } else {
    // execute program
    printf("exec\r\n"); // DEBUG

    // create the system exec command
    char exec_cmd[MAX_CMD_LEN] = "./";
    strcat(exec_cmd, cmd);
    // add arguments (if any) to the exec command
    if (args && strcmp(args, "\0") != 0) {
      strcat(exec_cmd, " ");
      strcat(exec_cmd, args);
    }

    printf("exec %s\r\n", exec_cmd); // DEBUG

    int status = system(exec_cmd);
    printf("status %d\r\n", status); // DEBUG

    result = 0;
  }

  return result;
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

      removeTrailingNewLine(input);
      trimwhitespace(input);

      // just parse when its not an empty string
      if (strcmp(input, "\0") != 0) {
        // TODO tokenize on & (see below)
        // fork processes
        result = parseCommand(input);
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
