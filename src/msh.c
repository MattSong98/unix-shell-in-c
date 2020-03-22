/***********************************************
************************************************
** @File name: msh.c
** @Auther: Matt Song
** @Version: 1.0.0
** @Date: 2020-3-21
** @Description: A shell run on Unix shell to provide basic operations 
*************************************************
************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define  DEFAULT_CWD       "/Users/mattsong"
#define  LINE_BUFFER_SIZE               100
#define  DIR_BUFFER_SIZE                100
#define  ARG_BUFFER_SIZE                 50
#define  MAX_ARG_NUM                     20

/***********************************************
*************** declaration ********************
***********************************************/

void                       launch(char *);
char                     *getLine(char *);
void          parseLine(char *, char ***);
void                     execute(char **);
void                          quit(int *); 
void                   cd(char *, char *);     

/*******************************************************
*********************** main ***************************
********************************************************/

int main(int argc, char **argv) {

  char  cwd[DIR_BUFFER_SIZE] = DEFAULT_CWD;
  char                 *line = NULL; 
  char                **args = NULL;
  int              isStopped = 0;

  launch(cwd);

  while (!isStopped) {

    line = getLine(cwd);
    parseLine(line, &args);
    
    if (args[0]) { 
      if (!strcmp(args[0], "quit") || !strcmp(args[0], "exit")) {
        quit(&isStopped);
      } else if (!strcmp(args[0], "cd")) {
        cd(args[1], cwd);
      } else {
        execute(args);
      }
      for (char *p = args[0]; !p; p++) {
        free(p);
      }
    }
    free(args);     
    free(line);
  }
  return 0;
}

/********************************************
***************** msh core ******************
********************************************/

// msh initialization
void launch(char *cwd) {
  
  if (chdir(cwd) != 0) {
    perror("msh");
    exit(EXIT_FAILURE);
  }

}

// read a line from stdin
char *getLine(char *cwd) {

  char *buffer = (char *)malloc(sizeof(char) * LINE_BUFFER_SIZE);
  int bufferSize = LINE_BUFFER_SIZE;
  int position = 0;

  if (!buffer) {
    perror("msh");
    exit(EXIT_FAILURE);
  }

  printf("%s[M] ", cwd);

  while (1) {
    int c = getchar();        
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    }
    buffer[position++] = c;

    if (position == bufferSize) {
      bufferSize += LINE_BUFFER_SIZE;
      buffer = realloc(buffer, bufferSize);
      if (!buffer) {
        perror("msh");
        exit(EXIT_FAILURE);
      }
    }
  }
}

// parse the line and transform it into args & count
void parseLine(char *line, char ***ptrToArgs) {
  
  char **args   =  (char **)malloc(sizeof(char *) * MAX_ARG_NUM);
  char   *ptr   =  line;
  char   *arg   =  NULL;
  int   count   =  0;
  *ptrToArgs    =  args;

  if (!args) {
    perror("msh");
    exit(EXIT_FAILURE);
  }

  while (1) {
    
    if (*ptr == '\0') {
      if (count != 0) 
        *arg = '\0';
      args[count] = NULL;
      return;
    }

    if (*ptr == ' ') {
      ptr++;
      continue;
    }

    if (count == 0 && *ptr != ' ') {
      arg = (char *)malloc(sizeof(char) * ARG_BUFFER_SIZE);
      if (!arg) {
        perror("msh");
        exit(EXIT_FAILURE);
      }
      args[count++] = arg; 
      *(arg++) = *(ptr++);
      continue;
    }
  
    if (*ptr != ' ' && *(ptr-1) != ' ') {
      *(arg++) = *(ptr++);
      continue;
    }
  
    if (*ptr != ' ' && *(ptr-1) == ' ') {
      *arg = '\0';
      arg = (char *)malloc(sizeof(char) * ARG_BUFFER_SIZE);
      if (!arg) {
        perror("msh");
        exit(EXIT_FAILURE);
      }
      args[count++] = arg;
      *(arg++) = *(ptr++);
      continue;
    } 
  }   
}

// execute if not a built-in command 
void execute(char **args) {
  
  pid_t  pid = fork();
  pid_t wpid = 0;
  int status = 0;

  if (pid == 0) {
    // child precess
    if (execvp(args[0], args) == -1) {
      perror("exec");
      exit(EXIT_FAILURE);
    }
  } else if (pid < 0) {
    // parent if failed
    perror("exec");
  } else {
    // parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
}

/***************************************************
*************** msh built-in command ***************
***************************************************/

// quit from msh 
void quit(int *ptr) {

  *ptr = 1;

}

// change directory to dir 
void cd(char *dir, char *cwd) {

  if (!dir) dir = DEFAULT_CWD;
  if (chdir(dir) == 0) {
    if (!getcwd(cwd, DIR_BUFFER_SIZE))
      perror("cd");
  }
  else perror("cd");

}
