/************************************************************************
 * Name: Brett Stephenson
 * Description: Small shell
 **********************************************************************/

//Header Files
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>

//put the vars outside the functions for easy access
int quit = 0;
int i;
char line[512];
int background = 0;
int statusCode;
int bgProcess[100];
int numProcess = 0;

/********************************************************************************
 *                  getInput
 * Desc: Gets in the input from the user
 * *****************************************************************************/
void getInput() {
        fflush(stdout);
        fflush(stdin);
        printf(": ");
        fflush(stdin);
        if(fgets(line, sizeof(line), stdin) != NULL) { 
                char *pos;
                pos = strchr(line, '\n'); 
        *pos = '\0';

        //if there is an & at the end track it as background
        if((pos=strchr(line, '&')) != NULL) { 
                *pos = '\0'; 
                background = 1; 
        }
        else {
                background = 0;
        }
    }
        return;
}

/*******************************************************************************
 *                      checkProcesses
 * Desc: Checks what processes have ended
 * I had help with this from a previous student, who suggested I get a list of
 * PIDs and kill thme when it came time
 ******************************************************************************/
void checkProc() {
    int status;
    for(i = 0; i < numProcess; i++) { 
            if(waitpid(bgProcess[i], &status, WNOHANG) > 0) {
              //if ended
              if(WIFEXITED(status)) { 
                printf("Child %d exited with status= %d\n", bgProcess[i], WEXITSTATUS(status));
              }
              //if signaled to end
              if(WIFSIGNALED(status)) { 
                printf("Child %d exited with status= %d\n", bgProcess[i], WEXITSTATUS(status));
              }
            }
    }
}


int main(int argc, char *argv[], char *envp[]) {
        while(quit == 0) {
                //see if any had quit
                checkProc(); 
                getInput(); 

                //if exit
                if(strcmp(line, "exit") == 0) { 
                  quit = 1;
                  return 0;
                }

                //comments
                else if(strstr(line, "#")) { 
                  continue;
                }

                //find status
                else if(strcmp(line, "status") == 0) { 
                        printf("exited with status: %d\n", statusCode);
                        continue;
                }

                //cd
                else if(strncmp("cd", line, strlen("cd")) == 0) { 
                  if(line[2] == ' ') { 
                    char cwd[1024];
                    getcwd(cwd, sizeof(cwd)); 
                    char *path = strstr(line, " ");
                    if(path) { 
                      path += 1; 
                      char *value;
                      value = malloc(strlen(path));
                      memcpy(value, path, strlen(path)); 
                      *(value + strlen(path)) = 0;
                      sprintf(cwd, "%s/%s", cwd, value); 
                      free(value); 
                    }
                    chdir(cwd); 
                  }
                  else { 
                    char *homeDir = getenv("HOME"); 
                    chdir(homeDir); 
                  }
                }
                else { 
                  pid_t pid, ppid;
                  int status;

                  char *command;
                  char *args[512];
                  int numArgs;
                  int redirection = 0;

                  command = strtok(line, " ");
                  if(command == NULL) { //Empty line
                    continue;
                  }
           
                  //Setup Array for execvp
                  args[0] = command;
                  numArgs = 1;
                  args[numArgs] = strtok(NULL, " "); 
                  while(args[numArgs] != NULL) { 
                    numArgs++;
                    args[numArgs] = strtok(NULL, " ");
                  }
                  
                  //background process
                  if(background == 1) { 
                    if((pid = fork()) < 0) {
                      perror("Error while forking");
                      exit(1);
                    }
                    //child process
                    if(pid == 0) { 
                      for(i = 0; i < numArgs; i++) {
                        //find the redirection if there is one
                        if(strcmp(args[i], "<") == 0) { 
                          if(access(args[i+1], R_OK) == -1) { 
                            printf("cannot open %s for input\n", args[i+1]);
                            redirection = 1;
                          }
                          else { 
                            int fd = open(args[i+1], O_RDONLY, 0);
                            dup2(fd, STDIN_FILENO);
                            close(fd);
                            redirection = 1;
                            execvp(command, &command);  
                          }
                        }
                        //if the redirection is the other way
                        if(strcmp(args[i], ">") == 0) { 
                          int fd = creat(args[i+1], 0644); 
                          dup2(fd, STDOUT_FILENO);
                          close(fd);
                          redirection = 1;
                          execvp(command, &command); 
                        }
                      }
                      if(redirection == 0) { 
                        execvp(command, args); 
                      }
                      printf("%s no such file or directory\n", command); 
                      exit(1);
                    }
                    //parent
                    else { 
                      int status;
                      int process;
                      printf("background pid is %d\n", pid);
                      bgProcess[numProcess] = pid; 
                      numProcess++;
                      process = waitpid(pid, &status, WNOHANG);
                      continue;
                    }
                  }

                  //forground
                  else { 
                    if((pid = fork()) < 0) {
                      perror("Error while forking");
                      exit(1);
                    }
                    //child
                    if(pid == 0) { 
                      for(i = 0; i < numArgs; i++) {
                        if(strcmp(args[i], "<") == 0) { 
                          if(access(args[i+1], R_OK) == -1) { 
                            printf("cannot open %s for input\n", args[i+1]);
                            redirection = 1;
                          }
                          //redirect and execute
                          else { 
                            int fd = open(args[i+1], O_RDONLY, 0);
                            dup2(fd, STDIN_FILENO);
                            close(fd);
                            redirection = 1;
                            execvp(command, &command); 
                          }
                        }
                        // if it is the other way
                        if(strcmp(args[i], ">") == 0) { 
                          int fd = creat(args[i+1], 0644); 
                          dup2(fd, STDOUT_FILENO);
                          close(fd);
                          redirection = 1;
                          execvp(command, &command); 
                        }
                      }
                      //otherwise
                      if(redirection == 0) { 
                        execvp(command, args); 
                      }
                      printf("%s no such file or directory\n", command); 
                      exit(0);
                    }
                    //parent waits and makes sure everything works
                    else { 
                      int status;
                      waitpid(pid, &status, 0); 
                      if(WIFEXITED(status)) {
                        statusCode = WEXITSTATUS(status);
                      }
                    }
                  }
                }
                //ignore ^C
                signal(SIGINT, SIG_IGN);  
        }
        return 0;

}
