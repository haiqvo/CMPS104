/*
 * Hai Vo 
 * Program 2
 * 
 * Program 2 handles the new flex made scanner and makes the 
 * tok file. It also stil makes the str file too.
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

const string cpp_name = "/usr/bin/cpp";
string yyin_cpp_command;
char *filename = NULL;
const size_t LINESIZE = 1024;
int yyparseFlag = 0;

//Get the tok information from lyutils.cc
stringstream tokFile;

//This ccp and opens the yyin file
void yyin_cpp_popen (char *filename, char* d_suppress_value) {
  if(d_suppress_value != NULL){
    yyin_cpp_command  = cpp_name;
    yyin_cpp_command += " ";
    yyin_cpp_command += "-D";
    yyin_cpp_command += d_suppress_value;
    yyin_cpp_command += " ";
    yyin_cpp_command += filename;
  }else{
    yyin_cpp_command  = cpp_name;
    yyin_cpp_command += " ";
    yyin_cpp_command += filename;
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
void scan_opts (int argc, char** argv) {
  int option;
  char *d_suppress_value = NULL;
  int yylexFlag = 0;
  for(;;) {
    option = getopt (argc, argv, "@:D:lf");
    if (option == EOF) break;
    switch (option) {
      case '@': set_debugflags (optarg);   break;
      case 'l': yylexFlag = 1;             break;
      case 'f': yyparseFlag = 1;           break;
      case 'D': d_suppress_value = optarg; break;
      default:  errprintf ("%:bad option (%c)\n", optopt); break;
    }
  }
  if (optind > argc) {
    errprintf ("Usage: %s [-ly] [filename]\n", get_execname());
    exit (get_exitstatus());
  }
  filename = argv[optind];
  if (yylexFlag == 1){
    yy_flex_debug = 1;
  }else{
    yy_flex_debug = 0;
  }
  yyin_cpp_popen (filename, d_suppress_value);
  DEBUGF ('m', "filename = %s, yyin = %p, fileno (yyin) = %d\n",
          filename, yyin, fileno (yyin));
}

//main function
int main (int argc, char** argv) {
    scan_opts(argc, argv);
    string short_filename = string(basename(filename));
    DEBUGF ('f', "filename = %s\n", basename(filename));
    yytokenize();
    yyin_cpp_pclose();
    string new_filename = changeFileExtension(short_filename, ".str");
    DEBUGF ('f', "filename = %s\n", new_filename.c_str());
    FILE * outputFile = fopen(new_filename.c_str(), "w");
    dump_stringset (outputFile);
    fclose(outputFile);
    ofstream tokFileStream;
    tokFileStream.open(
            changeFileExtension(short_filename, ".tok").c_str());
    tokFileStream << tokFile.str();
    tokFileStream.close();
    return get_exitstatus();
}
