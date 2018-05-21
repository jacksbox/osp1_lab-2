#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>

#define MAX_CMD_LEN 1024
#define MAX_CWD_LEN 1024
#define MAX_ARGS 100


const char delim[2] = "|";

char *trimwhitespace(char *str)
{
  char *end;

  // move pointer to first non-space char
  while (isspace((unsigned char)*str)) {
    str++;
  }

  // only whitespaces found
  if (*str == 0) {
    return str;
  }

  // find first non-space back-to-front
  // set endpointer position
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end)) {
    end--;
  }
  // add terminator after endpointer
  *(end+1) = 0;

  return str;
}

// finds position of newline in string and replaces it with terminating 0
void removeTrailingNewLine(char *input) {
  char *end;
  if ((end=strchr(input, '\n')) != NULL) {
    *end = 0;
  }
}

int shouldStartInBackground(char *cmd) {
  char *end;
  end = cmd + strlen(cmd) - 1;
  if(strcmp(end, "&") == 0) {
    // replace the & operator
    *end = 0;
    return 1;
  }
  return 0;
}

void makeArgArray(char *cmd, char *args, char *argArray) {

}

pid_t bg_pid[8];
pid_t pid_index;

int forkProcess(char *cmd, char *args) {
  pid_t proc_id;
//  int status = 0;


  proc_id = fork();

  if (proc_id < 0)
  {
    fprintf(stderr, "fork error\n");
    fflush(stderr);
    return EXIT_FAILURE;
  }

  if (proc_id == 0)
  { /* child process */
    close(1);
    //fprintf(stderr,"[child]  process id: %d\n", (int) getpid());
    //fprintf(stderr,"[child]  arg_string: %s\r\n", args);

    char **argArray = NULL;
    // makeArgArray(cmd, args, argArray);

    const char delim[2] = " ";
    char *token;
    int i = 0;

    argArray = realloc(argArray, sizeof(char*) * ++i);
    argArray[i-1] = cmd;

    token = strtok(args, delim);

    while (token != NULL) {
        argArray = realloc(argArray, sizeof(char*) * ++i);
        argArray[i-1] = token;
        token = strtok(NULL, delim);
    }

    execvp(cmd, argArray);
  //  fprintf(stderr,"[child]  command: %s\r\n", cmd);
    for (int j = 0; j < sizeof(argArray); ++j) {
      printf("[child]  args: %s\r\n", argArray[j]);
    }
    free(argArray);
    exit(-1);
  }
  else
  { /* parent */
    fprintf(stderr,"[parent] process id: %d\n", (int) getpid());
    fprintf(stderr," [child] process id: %d\n", proc_id);
    bg_pid[pid_index]=proc_id;
    pid_index = pid_index==8? 0 : pid_index+1;
    //pid_t child_id = wait(&status);

    //printf("[parent] child %d returned: %d\n",
    //        child_id, WEXITSTATUS(status));
  }
  return 0;
}

int parseCommand(char *input) {
  // trim whitespaces
  // and copy command
  char *command;
  command = trimwhitespace(input);

  int inBackground = shouldStartInBackground(command);

  // TODO check and implement PIPE
  // TODO check and implement BACKGROUND PROCESS &

  // split in command and arguments
  char *cmd;
  char *args;

  cmd = strtok(command, " ");
  args = strtok(NULL, "");

  fprintf(stderr,"parse Command: %s\r\n", cmd); // DEBUG
  fprintf(stderr,"with Arguments: %s\r\n", args); // DEBUG
  fprintf(stderr,"background process: %d\r\n", inBackground); // DEBUG

  int result = 0;
  if (strcmp(cmd, "exit") == 0) {
    // exit console
    fprintf(stderr,"terminating shell...\r\n");
    result = 1;
  } else if (strcmp(cmd, "wait") == 0) {
    // exit console
    fprintf(stderr,"wait\r\n");
    result = 1;
  } else if (strcmp(cmd, "cd") == 0) {
    // cd command

    printf("cd command to %s\r\n", args); // DEBUG

    if (chdir(args) != 0) {
      fprintf(stderr,"-shell: cd %s: no such Directory\r\n", args);
    }
  } else {
    // execute program
    fprintf(stderr,"exec\r\n"); // DEBUG

    // create the system exec command
    char exec_cmd[MAX_CMD_LEN] = "./";
    strcat(exec_cmd, cmd);

    if(inBackground == 1) {
      return forkProcess(exec_cmd, args);
    } else {
      // add arguments (if any) to the exec command
      if (args && strcmp(args, "\0") != 0) {
        strcat(exec_cmd, " ");
        strcat(exec_cmd, args);
      }

      fprintf(stderr,"exec: %s\r\n", exec_cmd); // DEBUG

      int status = system(exec_cmd);
      fprintf(stderr,"status: %d\r\n", status); // DEBUG

      result = 0;
    }
  }

  return result;
}

int pipenize(char* input, int pipe_count){

  //int result=0;
  int token_count = pipe_count+1;
  char *token_arr[token_count];

  token_arr[0] = strtok(input, "|");
  int pipefd_arr[pipe_count][2];
  for (int i=1;i<pipe_count+1;i++) {
      token_arr[i] = strtok(NULL, "|");
      if(pipe(pipefd_arr[i]) <0) {
        fprintf(stderr, "could not create pipe :-(\n");
        return -1;
      }
  }


    pid_t cpid;
    int pipe_index=pipe_count-1;
    int token_index = token_count-1;
    while(pipe_index>=0){
      cpid = fork();
      if(cpid==-1){
        perror("fork");
        exit(EXIT_FAILURE);
      }
      if(cpid == 0) {// child PROCESS
          dup2(pipefd_arr[pipe_index][1],1);
          fprintf(stderr,"\tChild process takes: %s\n",token_arr[--token_index] );
          if(pipe_index >0){
            dup2(pipefd_arr[--pipe_index][0],0);
            continue;
          } else {
            parseCommand(token_arr[--token_index]);
            break;
          }

      }
      else {
          dup2(pipefd_arr[pipe_index][0],0);
          fprintf(stderr,"\tparent process parses: %s\n",token_arr[token_index] );
          int status = 0;
          pid_t child_id = wait(&status);
      		printf("[parent] child %d returned: %d\n",
      		        child_id, WEXITSTATUS(status));
          parseCommand(token_arr[token_index]);
          close(pipefd_arr[pipe_index][0]);
          break;
      }
    }





  return 0;
}

int main(void)
{
  char input[MAX_CMD_LEN];
  char cwd[MAX_CWD_LEN];
  pid_index=0;

  memset(bg_pid,0,8*sizeof(pid_t));

  int result = 0;
  while(result == 0) {

    if(getcwd(cwd, sizeof(cwd))==NULL) {
		printf("%s\n",strerror(errno));
		return -1;
	}

    printf("%s > ", cwd);

    if (fgets(input, MAX_CMD_LEN, stdin) != NULL) {

      removeTrailingNewLine(input);
      trimwhitespace(input);


      // just parse when its not an empty string
      if (strcmp(input, "\0") != 0) {
        // first check if there is at least one pipe symbol in input
        char *input_ptr=input;
        int pipe_count=0;
        while (*input_ptr != '\0') {
              if (*input_ptr == '|') {
                pipe_count++;
              }
              input_ptr++;
        }
        // if pipe symbol was found we use pipenize
        if(pipe_count>0){
          result = pipenize(input,pipe_count);
        }

        else result = parseCommand(input);
      }

    }
  }

	return 0;
}
