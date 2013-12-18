/*
 * Hai Vo 
 * Program 5
 * 
 * Program 5 uses the abstract syntax tree to create a low level 
 * language used by the the compiler to perform optimizations
 * and is store in the oil file. 
 * makes the sym, tok, str, and ast file too.
*/
#include <string>
using namespace std;

#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "auxlib.h"
#include "stringset.h"
#include "astree.h"
#include "lyutils.h"
#include "symtable.h"
#include "typecheck.h"
#include "oilcode.h"

const string cpp_name = "/usr/bin/cpp";
string yyin_cpp_command;
char *filename = NULL;
const size_t LINESIZE = 1024;


//Get the tok information from lyutils.cc
stringstream tokFile;
stringstream oilFile;

//This ccp and opens the yyin file
void yyin_cpp_popen (char *filename, char* d_suppress_value) {
  ifstream testFile(filename);
  //checks for working files
  if(!testFile){
    if(filename == NULL){
      cerr << "no file entered" << endl;
    }else{
      cerr << "bad filename:  " << filename << endl;
    }
    set_exitstatus(EXIT_FAILURE);
    exit (get_exitstatus());
  }
  testFile.close();
  //the -D supression code
  if(d_suppress_value != NULL){
    yyin_cpp_command  = cpp_name;
    yyin_cpp_command += " ";
    yyin_cpp_command += "-D";
    yyin_cpp_command += d_suppress_value;
    yyin_cpp_command += " ";
    yyin_cpp_command += filename;
    yyin_cpp_command += " 2>/dev/null";
  }else{
    yyin_cpp_command  = cpp_name;
    yyin_cpp_command += " ";
    yyin_cpp_command += filename;
    yyin_cpp_command += " 2>/dev/null";
  }
  yyin = popen (yyin_cpp_command.c_str(), "r");
  if (yyin == NULL) {
    syserrprintf (yyin_cpp_command.c_str());
    exit (get_exitstatus());
  }
}

//close the yyin file
void yyin_cpp_pclose (void) {
  int pclose_rc = pclose(yyin);
  eprint_status (yyin_cpp_command.c_str(), pclose_rc);
  if (pclose_rc != 0) set_exitstatus (EXIT_FAILURE);
}

//used to get rid  of the \n in the function.
void chomp (char *string, char delim) {
  size_t len = strlen (string);
  if (len == 0) return;
  char *nlpos = string + len - 1;
  if (*nlpos == delim) *nlpos = '\0';
}


//the new scanner by calling yylex()
void yytokenize () {
  for(;;){
    int yyint = yylex();
    if (yyint == YYEOF) break;
  }
}

//change the extension of the file
const string changeFileExtension(string filename, string extension) {
  string lastSlash;
  filename.replace(filename.end()-3,filename.end(),extension);
  return filename;
}

//handles the options for the oc call and opens the yyin
bool scan_opts (int argc, char** argv) {
  int option;
  char *d_suppress_value = NULL;
  yy_flex_debug = 0;
  yydebug = 0;
  for(;;) {
    option = getopt (argc, argv, "@:D:ly");
    if(option == 63){
      return false;
    }
    if (option == EOF) break;
    switch (option) {
      case '@': set_debugflags (optarg);   break;
      case 'l': yy_flex_debug = 1;         break;
      case 'y': yydebug = 1;               break;
      case 'D': d_suppress_value = optarg; break;
      default:  
        errprintf ("%:bad option (%c)\n", optopt); 
        return false;
    }
  }
  if (optind > argc) {
    cerr << "No file found" << endl;
    return false;
  }
  filename = argv[optind];
  yyin_cpp_popen (filename, d_suppress_value);
  DEBUGF ('m', "filename = %s, yyin = %p, fileno (yyin) = %d\n",
          filename, yyin, fileno (yyin));
  return true;
}



//main function
int main (int argc, char** argv) {
  if (scan_opts(argc, argv) == false){
    set_exitstatus(EXIT_FAILURE);
    return get_exitstatus();
  }
  string short_filename = string(basename(filename));
  DEBUGF ('f', "filename = %s\n", basename(filename));
  //yytokenize();
  yyparse();
  if(get_exitstatus() == 0) createSymbolTable();
  if(get_exitstatus() == 0) oilCodeCreate();
  yyin_cpp_pclose();
  if(get_exitstatus() == 0){
    //Stringset file
    string new_filename = changeFileExtension(short_filename, ".str");
    DEBUGF ('f', "filename = %s\n", new_filename.c_str());
    FILE * outputFile = fopen(new_filename.c_str(), "w");
    dump_stringset (outputFile);
    fclose(outputFile);
    //tok file
    ofstream tokFileStream;
    tokFileStream.open(
          changeFileExtension(short_filename, ".tok").c_str());
    tokFileStream << tokFile.str();
    tokFileStream.close();
    //ast file
    FILE* astFile = 
      fopen(changeFileExtension(short_filename, ".ast").c_str(), "w");
    dump_astree(astFile, yyparse_astree);
    fclose(astFile);
    //sym file
    FILE* symFile = 
      fopen(changeFileExtension(short_filename, ".sym").c_str(), "w");
    identTable->dump(symFile, 0);
    fclose(symFile);
    ofstream oilFileStream;
    oilFileStream.open(
          changeFileExtension(short_filename, ".oil").c_str());
    oilFileStream << oilFile.str();
    oilFileStream.close();

    int exit = system(("gcc -g -o " + 
      changeFileExtension(short_filename, "") + " -x c " + 
      changeFileExtension(short_filename, "") + ".oil " + 
      " oclib.c").c_str());
    if(exit > 0)
      set_exitstatus(EXIT_FAILURE);
  }
  return get_exitstatus();
}
