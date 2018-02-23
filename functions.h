#ifndef __Function__H__
#define __Function__H__
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fstream>
#include <string>
#include <map>
#include <iostream>
#include <vector>

/************** return the current path ***********/
std::string getCurrentPath(){
  char currPath[FILENAME_MAX];
  getcwd(currPath, FILENAME_MAX);
  std::string myPath(currPath);
  return myPath;
}

/******* check whether the programName has '/' inside *******/
bool checkSlash(std::string commName){
  std::string programName = commName;
  std::string str = "/";
  std::size_t found = programName.find(str);
  if (found != std::string::npos){
    return true;
  }
  else{
    return false;
  }
}

/******* check whether variable name contains invalid chars ********/
bool checkChar(char i){
  if(i == '_' || (i <= '9' && i >= '0') || (i <= 'z' && i >= 'a' ) || (i <= 'Z' && i >= 'A')){
    return true;
  }
  return false;
}

/************************ parent process *************************/
void parentProcess(pid_t chldPid){
/**********citation : cited from manpage of waitpid ****************/
  int chldExeStatus;
  do {
    pid_t w = waitpid(chldPid, &chldExeStatus, WUNTRACED | WCONTINUED); // return if a child has stopped or a stopped child has been resumed
    if (w == -1) {                                                      // on error, -1 is returned
      perror("waitpid");
      exit(EXIT_FAILURE);
    }
    if (WIFEXITED(chldExeStatus)) {                                                           // the child terminated normally
      std::cout << "Program exited with status " << WEXITSTATUS(chldExeStatus) << std::endl;  // the exit status of the child
    }
    else if (WIFSIGNALED(chldExeStatus)) {                                                    // the child process was terminated by a signal
      std::cout << "Program was killed by signal " << WTERMSIG(chldExeStatus) << std::endl;   // the signal causing the child process to terminate
    }
    else if (WIFSTOPPED(chldExeStatus)){                                                      // the child process was stopped by delivery of a signal
      std::cout << "Program was stopped by signal " << WSTOPSIG(chldExeStatus) << std::endl;  // the signal which caused the child to stop
    }
    else if (WIFCONTINUED(chldExeStatus)){                                                    // the child process was resumed by delivery of SIGCONT
      std::cout << "Program continued " << std::endl;
    }
  } while (!WIFEXITED(chldExeStatus) && !WIFSIGNALED(chldExeStatus));
}

/****************** check whther the command line is valid ********************/
bool checkCommand(std::string comName){
  if(comName == "" || (comName.find_first_not_of(" \t") == std::string::npos) || comName == "\\"){  // empty, space and '\'
    std::cout << "checkComman:invalid commandline(" << comName << ")" << std::endl;
    return false;
  }
  return true;
}

/********************* open, dup2 and close redirInput file **********************/
void redirRead(std::string redirInput){
  int fdr;
  if(!redirInput.empty()){
    if((fdr = open(redirInput.c_str(),O_CREAT|O_RDONLY, 0666)) < 0){
      perror("Can't open input");
    }
    if(dup2(fdr, STDIN_FILENO) < 0){
      perror("Can't dup(2) input");
    }
    close(fdr);
  }
}

/********************* open, dup2 and close redirOutput file **********************/
void redirWrite(std::string redirOutput){
  int fdw;
  if(!redirOutput.empty()){
    if((fdw = open(redirOutput.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0666)) < 0){
      perror("Can't open output");
    }
    if(dup2(fdw, STDOUT_FILENO) < 0){
      perror("Can't dup(2) output");
    }
    close(fdw);
  }
}

/********************* open, dup2 and close redirOutErr file ************************/
void redirMapErr(std::string redirOutErr){
  int fderr;
  if (!redirOutErr.empty()){
    if((fderr = open(redirOutErr.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0666)) < 0){
      perror("Can't open outERR");
    }
    if(dup2(fderr, STDERR_FILENO) < 0){
      perror("Can't dup(2) outERR");
    }
    close(fderr);
  }
}

#endif
