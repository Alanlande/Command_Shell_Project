#include "varShell.h"
#include "functions.h"
extern char ** environ;

std::string getCurrentPath();            // return current path
void parentProcess(pid_t chldPid);       // handle parent process 
bool checkCommand(std::string comName);  // check whther the command line is valid

/********************** main helper function to execue the commands ***********************/
bool executeComm(MyShell * commShell, VarMyShell * varCommShell, char ** envp){
  std::string commName;
  size_t numSubCommand;
  std::map<std::string, std::string> mapVarValue = varCommShell->getMapVar();  //get the "var=>value" from varCommShell
  commShell->parseVar(mapVarValue);                  // look for '$varname' in command line, replace the contents if found
  if(!checkCommand(commShell->getCommLine())){       // if after replecing the vars, the command line is invalid
    return false;
  }  
  commShell->parseCommand();             // Parse the commandLine into childArgs
  if(!commShell->parsePipe()){           // parse childArgs into subCommandLine childArgs, didvided by "|", if the pipe is invalid, return 
    return false;
  }
  if(!commShell->parseRedir()){          // If the command line include invalid use of redirection, report and return
    return false;
  }
  if((numSubCommand = commShell->numSubArgs()) == 1){ // handle the single command
    commName = commShell->getCommName();
    /******************* handle the special builtin commands *******************/
    if(commName == "exit"){              // the command name is "exit"
      return true;
    }
    else if (commName == "cd"){          // the command name is "cd", change directory
      commShell->changeDir();
      return false;
    }
    else if(commName == "set"){          // set variable=>value
      varCommShell->parseSetArg(commShell->getCommLine());
      return false;
    }
    else if(commName == "export"){       // export variable=>value
      if (commShell->getArgNum() > 1){
	varCommShell->expEnvVar(commShell->getFirArg());
      }
      else{
	std::cout << "Export: Please type in valid variable name." << std::endl;
      }
      return false;
    }
  }
  /******************* handle the commands passed into execve *******************/
  pid_t * chldPid = new pid_t[numSubCommand];
  if(numSubCommand == 1){                 // the command line contains only one cammamd
    if(!commShell->handleSingComm(chldPid, envp)){
      delete [] chldPid;
      return false;
    }
  }
  else{                                  // the command line contains multiple cammamds
    if(!commShell->handleMultiComm(chldPid, numSubCommand, envp)){
      delete [] chldPid;
      return false;
    }
  }
  delete [] chldPid;
  return false;
}

/**************************************** main function ******************************************/
int main (int argc, char ** argv){
  std::string comName;
  VarMyShell * varCommShell = new VarMyShell();
  std::cout << "myShell:" << getCurrentPath() <<"$ ";
  while (std::getline (std::cin, comName)){                   // read from stdin the a command line
    bool isExit = false;
    if (checkCommand(comName)){
      MyShell * commShell = new MyShell(comName);
      std::cout<<"The commandline you typed in: ("<<comName<<")"<<std::endl;
      isExit = executeComm(commShell, varCommShell, environ); // execute the command line, return whether the parsed new command line is "exit"
      delete commShell;
    }
    if(isExit){
      break;
    }
    std::cout << "myShell:" << getCurrentPath() <<"$ ";
  } 

  if (!std::cin){                                             // std::cin fails, exit
    if (!std::cin.eof()) {
      std::cout << "\nOther failures!" << std::endl;
    }
    std::cout << '\n';
  }
  delete varCommShell;
  return EXIT_SUCCESS;
}
