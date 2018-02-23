#ifndef __VarShell__H__
#define __VarShell__H__
#include "shell.h"

class VarMyShell{
  std::map<std::string, std::string> mapEnvVar;
 public:
  VarMyShell(){
    mapEnvVar.insert(std::pair<std::string, std::string>("$","1808"));  // the builtin value for "$"
  }
  void parseSetArg(std::string commandLine);
  bool checkVar(std::string & var);
  void expEnvVar(std::string var);
  std::map<std::string, std::string> getMapVar();
  VarMyShell (const VarMyShell & rhs){                                  // copy constructor
    *this = rhs;
  }
  VarMyShell & operator =(const VarMyShell & rhs){                      // assignment constructor
    if(this != &rhs){
      mapEnvVar = rhs.mapEnvVar;
    }
    return *this;
  }
  ~VarMyShell(){}
};

/********* parse the "set" command line into vars and values **********/
void VarMyShell::parseSetArg(std::string commandLine){
  std::string var, value;
  bool isVarStart = false, isValueStart = false, isBetweenVarVal = false, isBackslaBefore = false; // several signals
  std::size_t firstT = commandLine.find_first_of("t");
  commandLine.erase(0, firstT + 1);             // erase "set" in commandLine
  for (std::string::iterator it=commandLine.begin(); it!=commandLine.end(); ++it){
    if(isValueStart){                           // when isValueStart is true, take the left contents as value, consider '\'
      if(isBackslaBefore){                      // "\" follows, take whatever this char, set bask isBackslaBefore
	isBackslaBefore = false;
	value.push_back(*it);
      }
      else if(*it == '\\'){                     // encounter "\", set isBackslaBefore
	isBackslaBefore = true;
      }
      else{
	value.push_back(*it);
      }
    }
    else if ((*it != ' ' && *it != '\t')  && !isBetweenVarVal){          // take the non-space char into var when 'it' is before value
      var.push_back(*it);
      isVarStart = true;
    }
    else if((*it == ' ' || *it == '\t') && isVarStart && !isValueStart){ // when 'it' is at the beginning between var and value
      isVarStart = false;
      isBetweenVarVal = true;
    }
    else if((*it != ' ' && *it != '\t')  && !isVarStart && isBetweenVarVal && !isValueStart){ // when 'it' is at the beginning of value
      if(*it != '\\'){
	value.push_back(*it);
      }
      else{
	isBackslaBefore = true;
      }
      isValueStart = true;
    }
  }
  
  if(!checkVar(var)){                     // check whether var is valid
    std::cout << "set:("<<var << ") is not a valid variable name!" << std::endl;
    return;
  }
  /**************** citation : cited from reference of std::map from http://www.cplusplus.com/reference/map/map/find/ **************************/
  std::map<std::string, std::string>::iterator found = mapEnvVar.find(var); // look for the var in mapEnvVar, if found replace it with new value
  if(found != mapEnvVar.end()){
    mapEnvVar.erase(found);
  }
  
  mapEnvVar.insert(std::pair<std::string, std::string>(var,value));
  if(var == "PATH"){                    // if var=="PATH", export it automatically
    expEnvVar(var);
  }
}

/******************** check whether var is valid ************************/
bool VarMyShell::checkVar(std::string & var){
  if (var.empty()){                                                   // var is empty, return false
    return false;
  }
  for(std::string::iterator it = var.begin(); it != var.end(); ++it){ // check whether var is composed of valid chars: number, underscore, letter
    if (*it != '_' && !(*it >= '0' && *it <= '9') && !(*it >= 'A' && *it <= 'Z') && !(*it >= 'a' && *it <= 'z')){
      return false;
    }    
  }
  return true;
}

/************* find the exported variable and export it **********************/
void VarMyShell::expEnvVar(std::string var){
  if(!checkVar(var)){                     // check whether var is valid
    std::cout <<"export: (*"<< var << ") is not a valid variable name!" << std::endl;
    return;
  }
  std::map<std::string, std::string>::iterator found = mapEnvVar.find(var);
  if(found != mapEnvVar.end()){          // found the var and then export the var=>value
    int err = setenv((found->first).c_str(), (found->second).c_str(), 1);  // set env variable
    if(err == -1){
      perror("export:setenv() fails");
    }
  }
  else{
    std::cout << "export variable: (" <<var <<") NOT found!"<< std::endl;
  }
}
/****************** return mapEnvVar **************/
std::map<std::string, std::string> VarMyShell::getMapVar(){
  return mapEnvVar;
}

#endif
