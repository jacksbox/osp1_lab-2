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



struct bg_proc_status{
  pid_t pid;
  int status;
  int errorno;
  struct bg_proc_status *next;
};
struct bg_proc_status *bg_proc_status_ptr_first;


void insert_bg_proc(struct bg_proc_status* in) {
  struct bg_proc_status *bgps = bg_proc_status_ptr_first;
  if(bgps == NULL) {
    bgps = in;
    bg_proc_status_ptr_first = bgps;
    return;
  }
  while(bgps->next != NULL){
    bgps = bgps->next;
  }
  bgps->next = in;
}

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

    struct bg_proc_status *new_bg_proc = (struct bg_proc_status*)malloc(sizeof(struct bg_proc_status));
    insert_bg_proc(new_bg_proc);
    new_bg_proc->pid = getpid();
    new_bg_proc->status=0;
    new_bg_proc->errorno=0;
    new_bg_proc->next=NULL;

    argArray = realloc(argArray, sizeof(char*) * ++i);
    argArray[i-1] = cmd;

    token = strtok(args, delim);

    while (token != NULL) {
        argArray = realloc(argArray, sizeof(char*) * ++i);
        argArray[i-1] = token;
        token = strtok(NULL, delim);
    }

    int status = execvp(cmd, argArray);

    new_bg_proc->status=status;
    new_bg_proc->errorno=errno;

  /*  for (int j = 0; j < sizeof(argArray); ++j) {
      fprintf(stderr,"[child]  args: %s\r\n", argArray[j]);
    }*/
    free(argArray);
    exit(-1);
  }
  else
  { /* parent */
    fprintf(stderr,"[parent] process id: %d\n", (int) getpid());
    fprintf(stderr," [child] process id: %d\n", proc_id);
    //pid_t child_id = wait(&status);

    //printf("[parent] child %d returned: %d\n",
    //        child_id, WEXITSTATUS(status));
  }
  return 0;
}





int cmd_wait(char* args){
  char *cptr=args;
  while(*cptr != '\0'){
    while(*cptr == ' '){
      cptr++;
    }
    if(*cptr=='\0') break;
    int wordl=0;
    char *endptr=cptr;
    while(*endptr != ' '){
      wordl++;
      endptr++;
      if(*endptr == '\0')break;
    }
    char * single_arg=(char*)malloc((wordl+1)*sizeof(char));
    strncpy(single_arg,cptr,wordl);
    strcat(single_arg,"\0");
    struct bg_proc_status *bgpst_ptr= bg_proc_status_ptr_first;
    while (bgpst_ptr !=NULL && single_arg != NULL) {
      if(atoi(single_arg) == (int) bgpst_ptr->pid ){
        printf("Process ID  %d\n", bgpst_ptr->pid);
        printf(" Status:   %d\n", bgpst_ptr->status);
        if(bgpst_ptr->errorno>0)
          printf("  Error:  %s\n",strerror(bgpst_ptr->errorno) );
        break;
      }
      bgpst_ptr = bgpst_ptr->next;
    }
    cptr=endptr;
    cptr++;
    printf(" -- ");
  }
  return 0;
}


int parseCommand(char *input) {
  // trim whitespaces
  // and copy command
  char *command;
  command = trimwhitespace(input);

  int inBackground = shouldStartInBackground(command);

  // split in command and arguments
  char *cmd;
  char *args;

  cmd = strtok(command, " ");
  args = strtok(NULL, "");

  //fprintf(stderr,"parse Command: %s\r\n", cmd); // DEBUG
  //fprintf(stderr,"with Arguments: %s\r\n", args); // DEBUG
  //fprintf(stderr,"background process: %d\r\n", inBackground); // DEBUG

  int result = 0;
  if (strcmp(cmd, "exit") == 0) {
    // exit console
    fprintf(stderr,"terminating shell...\r\n");
    result = 1;
  } else if (strcmp(cmd, "wait") == 0) {
    char **argsArr=NULL;
    printf(" Not correctly implemented. :-(");
    result=cmd_wait(args);
    free(argsArr);
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

  //    fprintf(stderr,"exec: %s\r\n", exec_cmd); // DEBUG

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
        //  fprintf(stderr,"\tChild process takes: %s\n",token_arr[--token_index] );
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
        //  fprintf(stderr,"\tparent process parses: %s\n",token_arr[token_index] );
          int status = 0;
        /*  pid_t child_id = */wait(&status);
    //  		printf("[parent] child %d returned: %d\n",
      //		        child_id, WEXITSTATUS(status));
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
  bg_proc_status_ptr_first=NULL;


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
