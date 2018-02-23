#ifndef __Shell__H__
#define __Shell__H__
#include "functions.h"

bool checkChar(char i);
void parentProcess(pid_t chldPid);
bool checkSlash(std::string commName);
void redirRead(std::string redirInput);
void redirWrite(std::string redirOutput);
void redirMapErr(std::string redirOutErr);
// class MyShell
class MyShell{
  std::string commandLine;
  std::vector<std::string> chldPath;
  std::vector<std::string> childArgs;
  std::vector<std::vector<std::string> > subChildArgs;
  std::string redirInput;
  std::string redirOutput;
  std::map<size_t, std::string> mapRedirOutErr;
 public:
 MyShell(std::string n):commandLine(n){}
  void parseCommand();
  void parseVar(std::map<std::string, std::string> & mapEnvVar);
  bool parseRedir();
  bool parsePipe();
  void redirR();
  void redirW();
  void redirErr(size_t n);
  void execute(size_t i, char** envp);
  void getPath(std::string & pPathStr);
  bool findCommName(size_t n);
  void changeDir();
  bool handleSlash(size_t n, char ** envp);
  bool handleSingComm(pid_t * chldPid, char ** envp);
  bool handleMultiComm(pid_t * chldPid, size_t numSubCommand, char ** envp);
  std::string getCommName();
  std::string getCommLine();
  std::string getFirArg();
  size_t getArgNum();
  size_t numSubArgs();
  MyShell (const MyShell & rhs){              // copy constructor
    *this = rhs;
  }
  MyShell & operator =(const MyShell & rhs){  // assignment constructor
    if(this != &rhs){
      commandLine = rhs.commandLine;
      chldPath = rhs.chldPath;
      childArgs = rhs.childArgs;
      subChildArgs = rhs.subChildArgs;
      redirInput = rhs.redirInput;
      redirOutput = rhs.redirOutput;
      mapRedirOutErr = rhs.mapRedirOutErr;
    }
    return *this;
  }
  ~MyShell(){}
};

/**********************  Parse the commandLine into childArgs ***********************/
void MyShell::parseCommand(){
  childArgs.clear();
  bool isBackslaBefore = false;          // check if the character before is backslash
  bool isArgBuff = false;                // check if currArg contains chars
  std::string currArg;
  for (std::string::iterator it=commandLine.begin(); it!=commandLine.end(); ++it){
    if (isBackslaBefore){                // if the character before is backslash, currArg should contain whatever current character
      currArg.push_back(*it);
      isBackslaBefore = false;           // set back
      isArgBuff = true;
    }
    //                                   // handle redirection "2>": case1:"2>***", case2:"** 2>***", case3:"**|2>**"
    else if(*it == '2' && it+1 != commandLine.end() && *(it+1) == '>' && ((it!= commandLine.begin() && (*(it-1) == ' ' || *(it-1) == '|')) || it == commandLine.begin())){
      if(isArgBuff){
	childArgs.push_back(currArg);
	currArg.clear();
      }
      childArgs.push_back("2>");
      ++it;
      isArgBuff = false;
    }                                     //handle "<" ">" "|"
    else if (*it == '<' || *it == '|' || *it == '>'){
      if(isArgBuff){
	childArgs.push_back(currArg);
	currArg.clear();
      }
      currArg.push_back(*it);
      childArgs.push_back(currArg);
      currArg.clear();
      isArgBuff = false;
    }
    else if ((*it == ' ' || *it == '\t') && isArgBuff){ // if current character is space and currArg contains chars, push currArg into childArgs
      childArgs.push_back(currArg);
      currArg.clear();
      isArgBuff = false;                 // set back
    }
    else if (*it == '\\'){               // if current character is backslash
      isBackslaBefore = true;
    }
    else if (*it != ' ' && *it != '\t'){ // if current character is not space and backslash, currArg should contain current character
      currArg.push_back(*it);
      isArgBuff = true;
    }
  }
  if (isArgBuff){
    childArgs.push_back(currArg);
  }
}

/***************** Look for '$varname' in command line, replace the contents if found *******************/
void MyShell::parseVar(std::map<std::string, std::string> & mapEnvVar){
  bool isBackslaBefore = false, foundDollar = false, validVarReplaced = false;
  do{
    validVarReplaced = false;                                // initialize validVarReplaced
    int searchEnd = commandLine.size() - 1;
    do{
      std::string currVar, valueReplace;
      std::size_t dollarFound = commandLine.find_last_of("$", searchEnd);        // look for "$" before search end
      std::size_t dollarEnd = commandLine.size();            // initialize dollarEnd to the end of commandLine 
      if(dollarFound != std::string::npos){                  // if found "$"
	foundDollar = true;
	std::size_t escapeFound = commandLine.find_last_of("\\", dollarFound);
	if(escapeFound != std::string::npos && dollarFound == escapeFound + 1){  // encounter "\$", escape the "$"
	  searchEnd = escapeFound;                           // update the search end to pos of "\\"
	  continue;
	}
	for(size_t i = dollarFound + 1; i < dollarEnd; i++){ // parse the contents after pos dollarFound, to extract "$var"
	  if(isBackslaBefore){
	    isBackslaBefore = false;
	    currVar.push_back(commandLine[i]);
	  }
	  else if(commandLine[i] == '\\'){
	    isBackslaBefore = true;
	  }
	  else if (commandLine[i] == ' ' || commandLine[i] == '\t' || !checkChar(commandLine[i])){
	    dollarEnd = i;                                   // encounter " ", update currVar,  dollarEnd and break the loop
	    break;
	  }
	  else{
	    currVar.push_back(commandLine[i]);
	  }
	}
	if(currVar.empty()){                // if the var is empty, leave the "$" untouched
	  searchEnd = dollarFound - 1;
	  continue;
	}
	std::map<std::string, std::string>::iterator varFound = mapEnvVar.find(currVar); // look for var in mapEnvVar
	if(varFound != mapEnvVar.end()){    // if found, replace "$var" with its value, else erase "$var"
	  validVarReplaced = true;
	  valueReplace = varFound->second;
	}
	searchEnd = dollarFound;            // update the search end to pos of "$" after replecement
	commandLine.replace(dollarFound, dollarEnd - dollarFound, valueReplace);	  
      }
      else{                                // no "$" found, break the loop
	foundDollar = false;
      }
    }while(foundDollar && searchEnd>=0);   // if a "$" is found, after the replacement, we should search "$" in the contents before this var
  }while(validVarReplaced);                // if a valid var is replaced, we should search again the command line 
}                                          // to see if the repalced value has valid var inside, if so replace it again

/****************************  parse redirections  *****************************/
bool MyShell::parseRedir(){
  bool isErase = false, validRedir = true, isWrongPlace = false;
  for(size_t i = 0; i < subChildArgs.size();i++){
    do{
      for(std::vector<std::string>::iterator it = subChildArgs[i].begin(); it != subChildArgs[i].end(); ++it){
	if(*it == "<" || *it == ">" || *it == "2>"){ // encounter redirections, take the following arg as the filename
	  if(it + 1 != subChildArgs[i].end()){       // valid redirections should not be in the end
	    if(*it == "<"){                          // save redirections
	      if(i != 0){                            // redirInput should only be in the first pipe
		isWrongPlace = true;
	      }
	      redirInput = *(it + 1);                // save redirInput
	    }
	    else{
	      if(*it == ">"){                        // save redirOutput and redirOutErr
		if(i != subChildArgs.size() - 1){    // redirOutput should only be in the last pipe
		  isWrongPlace = true;
		}
		redirOutput = *(it + 1);
	      }
	      else if(*it == "2>"){
		std::map<size_t, std::string>::iterator found = mapRedirOutErr.find(i); 
		if(found != mapRedirOutErr.end())    // if exist, erase and keep the last one 
		  mapRedirOutErr.erase(found);
		mapRedirOutErr.insert(std::pair<size_t, std::string>(i,*(it + 1)));
	      }
	    }
	  }
	  else{                                     // invalid redirectioxn: no file name follows, break
	    validRedir = false;
	    break;
	  }
	  
	  if(*(it + 1) == "<" || *(it + 1) == ">" || *(it + 1) == "2>" || isWrongPlace){
	    validRedir = false;                     // invalid redirection: another redirection follows or appear at wrong places, break
	    break;
	  }
	  else{                                     // erase the redirection and filename in subChildArgs[i] when redirections are valid, set isErase
	    subChildArgs[i].erase(it);
	    subChildArgs[i].erase(it);
	    isErase = true;
	  }
	  break;
	}
	isErase = false;                            // if the cur arg is not redirection, set isErase and continue the search
      }
    }while(isErase && validRedir);
    if(!validRedir){                                // validRedir, then jump outof loop, report and return
      break;
    }
  }
  if(!validRedir){
    std::cout<<"The redirection is invalid!"<<std::endl;
    return false;
  }
  return true;
}

/***********************  parse the command line into subcommand divided by "|"  ************************/
bool MyShell::parsePipe(){
  std::vector <std::string> currSubArgs;             // arg temp vector, composing one single command line
  size_t numChlArgs = childArgs.size();
  for(size_t i = 0; i < numChlArgs; i++){    
    if(childArgs[i] != "|"){                         // save as non-"|" args in currSubArgs
      currSubArgs.push_back(childArgs[i]);
    }
    else{                                            // encounter "|", push currSubArgs in subChildArgs, start again
      subChildArgs.push_back(currSubArgs);
      currSubArgs.clear();
    }
  }
  subChildArgs.push_back(currSubArgs);
  size_t numSubArgs = subChildArgs.size();
  for(size_t j = 0; j < numSubArgs; j++){
    if(subChildArgs[j].empty()){                     // invalid subChildArgs: empty
      std::cout<<"Invalid pipes!"<<std::endl;
      return false;
    }
  }
  return true;
}

/**************** handle redirInput file ***************/
void MyShell::redirR(){
  redirRead(redirInput);
}

/**************** handle redirOutput file **************/
void MyShell::redirW(){
  redirWrite(redirOutput);
}

/************* handle mapRedirOutErr files **************/
void MyShell::redirErr(size_t n){
  std::string redirOutErr = mapRedirOutErr[n];
  redirMapErr(redirOutErr);  
}

/******* function to execute the child program *********/
void MyShell::execute(size_t i, char ** envp){
  size_t argSize = subChildArgs[i].size();   // take args from subChildArgs
  char ** argvs = new char *[argSize+1];
  for (size_t j = 0; j < argSize; j++){
    argvs[j] = (char *)subChildArgs[i][j].c_str();
  }
  argvs[argSize] = NULL;
  if (execve(argvs[0], argvs, envp) < 0){    // execve() returns -1 only on error
    std::string messg("myShell: ");
    messg = messg + childArgs[0];
    perror(messg.c_str());                   // when execve() returns, report the error
    exit(EXIT_FAILURE);                      // this may cause mem leak, but that is OK
  }
  delete[] argvs;
}


/* find the built-in cammand and expand it to absolute path if found */
bool MyShell::findCommName(size_t n){
  for (std::vector<std::string>::iterator it = chldPath.begin() ; it != chldPath.end(); ++it){
    std::string pathCommand = *it + '/' + subChildArgs[n][0]; // look for command without "/"
    std::ifstream ifile(pathCommand.c_str());
    if ((bool)ifile == true){
      subChildArgs[n][0] = pathCommand;                       // if found, update the full-path command
      return true;
    }
  }
  return false;
}

/**************** get chldpath from pPathStr ***************/
void MyShell::getPath(std::string & pPathStr){
  chldPath.clear();
  std::size_t found = pPathStr.find_first_of(":");    // look for the first colon
  while (found != std::string::npos){
    std::string currPath = pPathStr.substr(0, found); // get the path
    chldPath.push_back(currPath);
    pPathStr.erase(0, found + 1);                     // erase the useless path
    found = pPathStr.find_first_of(":");
  }
  chldPath.push_back(pPathStr);
}

/**************** change dir while "cd" ***************/
void MyShell::changeDir(){
  int err = 0;
  if (childArgs.size() < 2){                   // just typed in "cd", go to root
    chdir("/home/dl261/");
    err = setenv("PWD", "/home/dl261", 1);
  }
  else{
    std::string newDir = childArgs[1];
    if(newDir == "~"){                        // typed in "cd ~", go to root
      chdir("/home/dl261/");
      err = setenv("PWD", "/home/dl261", 1);
    }
    else{
      int isChange = chdir(newDir.c_str());   // change dir
      if (isChange < 0){
	std::cout << "myShell: cd: " << newDir <<": No such file or directory" << std::endl;
      }
      else{
	std::string currPath = getCurrentPath();
	setenv("PWD", currPath.c_str(), 1);
      }
    }
  }
  if(err == -1){
    perror("******myShell:setenv() fails");
  }
}

/**************** handle slashes in program name ***************/
bool MyShell::handleSlash(size_t n, char ** envp){
  std::string commName = subChildArgs[n][0];
  if (!checkSlash(commName)){              // there is no '/' in command name
    char * pPath = getenv ("PATH");
    std::string pPathStr(pPath);
    getPath(pPathStr);                     // get environment values from PATH
    if(!findCommName(n)){
      std::cout << "Command " << commName << " not found" << std::endl;
      return false;
    }
  }
  else {                                   // there is  '/' in command name
    std::ifstream ifile(commName.c_str());
    if ((bool)ifile == false){
      std::cout << "myShell: " << commName <<": No such file or directory" << std::endl;
      return false;
    }
  }
  return true;
}

/**************** handle single command ***************/
bool MyShell::handleSingComm(pid_t * chldPid, char ** envp){
  if(!handleSlash(0, envp)){                            // handle slashes in program name
    return false;
  }
  chldPid[0] = fork();
  if(chldPid[0] < 0){
    std::cout << "Error, failed to fork!" << std::endl; // failed to fork, return
    return false;
  }
  else if(chldPid[0] == 0){                             // child process
    redirR(), redirW(), redirErr(0);
    execute(0, envp);
  }
  else{
    parentProcess(chldPid[0]);                          // parent process
  }
  return true;
}

/**************** handle multiple commands ***************/
bool MyShell::handleMultiComm(pid_t * chldPid, size_t numSubCommand, char ** envp){
  int fd1[2] = {-1,-1};                                   // initialize file descripters
  int fd2[2] = {-1,-1};
  for(size_t i = 0; i < numSubCommand ; i++){
    if(!handleSlash(i, envp)){                            // handle slashes in program name
      return false;
    }
    pipe(fd1);                                            // pipe
    chldPid[i] = fork();                                  // call Fork to create child process
    if (chldPid[i] < 0){
      std::cout << "Error, failed to fork!" << std::endl; // failed to fork, return
      return false;
    }
    else if(chldPid[i] == 0){                             // succeed in fork
      if(i == 0){                                         // the first child
	if(close(fd1[0]) < 0 || dup2(fd1[1], STDOUT_FILENO) < 0 || close(fd1[1]) < 0){
	  std::cout << "Error, failed in handleMultiComm" << std::endl;
	}                                                 // close pipefd1[0], dup2 pipefd1[1] as stdout mode for the second child
	redirR();                                         // handle the erdirect READ file
      }
      else if(i == numSubCommand - 1){                    // the last child, dup2 pipefd2[0] as STDIN mode
	if(dup2(fd2[0], STDIN_FILENO) < 0 || close(fd2[1]) < 0 || close(fd2[0]) < 0){
	  std::cout << "Error, failed in handleMultiComm" << std::endl;
	}                                                 // close last process end of WRITE, close curr process end of READ, close curr process end of READ
	redirW();                                         // handle redirect WRITE and ERRWRITE
      }
      else{                                               // the middle process, link the last stdout to the curr stdin
	if(dup2(fd1[1], STDOUT_FILENO)<0 || close(fd1[0]) <0 || close(fd1[1])<0 || dup2(fd2[0], STDIN_FILENO)<0 || close(fd2[0]) <0 || close(fd2[1]) < 0){
	  std::cout << "Error, failed in handleMultiComm" << std::endl;
	}
      }
      redirErr(i);                                        // the ith stderr API
      execute(i,envp);                                    // the ith child process, execute
    }
    
    if(i != 0){
      if(close(fd2[0])<0 || close(fd2[1]) < 0){           // after the first child, close fd2
	std::cout << "Error, failed in handleMultiComm" << std::endl;
      }
    }
    fd2[0]=fd1[0], fd2[1]=fd1[1];                         // update fd2
  }
  
  for(size_t j = 0; j < numSubCommand; j++){
    parentProcess(chldPid[j]);
  }
  return true;
}

/**************** return command name***************/
std::string MyShell::getCommName(){
  return childArgs[0];
}
/**************** return whole command line ***************/
std::string MyShell::getCommLine(){
  return commandLine;
}
/**************** return first argument ***************/
std::string MyShell::getFirArg(){
  assert(childArgs.size() > 1);
  return childArgs[1];
}
/*********** get size of chldargs ***********/
size_t MyShell::getArgNum(){
  return childArgs.size();
}
/*********** return size of subchldcrgs  **********/
size_t MyShell::numSubArgs(){
  return subChildArgs.size();
}

#endif
