#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define COMMAND_NOT_FOUND "%s: command not found\n"

char* commands[] = {
  "exit",
  "echo",
  "type",
  "pwd"
};

int getParams(char *input, char **params) {
  int i = 0;
  char *token = strtok(input, " ");
  while (token != NULL) {
    params[i] = token;
    token = strtok(NULL, " ");
    i++;
  }
  params[i] = NULL;
  return i;
}


int commandExistInPath(char *command, char *outputPath) {
  char *cmd = strtok(command, "\n");
  char *path = getenv("PATH");
  char *pathCopy = strdup(path);
  char *pathToken = strtok(pathCopy, ":");

  while (pathToken != NULL) {
    sprintf(outputPath, "%s/%s", pathToken, cmd);

    if (access(outputPath, F_OK) == 0) {
      free(pathCopy);
      return 0;
    }
    pathToken = strtok(NULL, ":");
  }

  free(pathCopy);
  return -1;
}

int main() {
  int exitCode = 0;
  
  while (1) {
    printf("$ ");
    fflush(stdout);

    // Wait for user input
    char input[100];
    fgets(input, 100, stdin);

    char* command = strtok(input, " ");
    command[strcspn(command, "\n")] = 0;

    // Check input
    if(strcmp(command, "exit") == 0) {
      char* exitParams[2];
      getParams(input, exitParams);
      exitCode = exitParams[0] != NULL ? atoi(exitParams[0]) : 0;
      break;
    }

    if(strcmp(command, "echo") == 0){
      char* echoStr = strtok(NULL, "\n");
      printf("%s\n", echoStr);
      continue;
    }

    if(strcmp(command, "type") == 0) {
      char* typeVal = strtok(NULL, "\n");

      if(typeVal == NULL) {
        printf("type: missing argument\n");
        continue;
      }

      int isBuiltin = 0;
      for(int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        if(strcmp(typeVal, commands[i]) == 0) {
          printf("%s is a shell builtin\n", typeVal);
          isBuiltin = 1;
          break;
        }
      }

      if(isBuiltin == 1) {
        continue;
      }

      char* libPath = malloc(100);
      if(commandExistInPath(typeVal, libPath) == 0) {
        printf("%s is %s\n", typeVal, libPath);
        free(libPath);
        continue;
      }
      
      free(libPath);
      printf("%s: not found\n", typeVal);

      continue;
    }


    if(strcmp(command, "pwd") == 0) {
      char cwd[1024];
      getcwd(cwd, sizeof(cwd));
      printf("%s\n", cwd);
      continue;
    }

    if(strcmp(command, "cd") == 0) {
      char *cdPath = strtok(NULL, "\n");
      char cwd[1024];
      getcwd(cwd, sizeof(cwd));

      if(cdPath == NULL) {
        continue;
      }

      if(cdPath[0] == '.') {
        strcat(cwd, "/");
        strcat(cwd, cdPath);
        cdPath = cwd;
      }

      if(cdPath[0] == '~') {
        char *home = getenv("HOME");
        strcat(home, "/");
        strcat(home, cdPath + 2);
        cdPath = home;
      }

      if(chdir(cdPath) == -1) {
        printf("cd: %s: No such file or directory\n", cdPath);
      }
      continue;
    }

    char* programPath = malloc(1024);
    if(commandExistInPath(command, programPath) == 0) {
      char* paramString = command + strlen(command) + 1;

      if(paramString == NULL) {
        paramString = "";
      }

      char executablePath[1024];
      sprintf(executablePath, "%s %s", programPath, paramString);
      system(executablePath);

      continue;
    }

    free(programPath);

    char *displayInput = strtok(input, "\n");
    printf(COMMAND_NOT_FOUND, displayInput);
  }

  return exitCode;
}
