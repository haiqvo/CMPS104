/*
 * Hai Vo 
 * Program 1
 * 
 *Cpp the .oc file and than token it to a .str file 
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

#include "auxlib.h"
#include "stringset.h"

const string CPP = "/usr/bin/cpp";
const size_t LINESIZE = 1024;
int yylexFlag = 0;
int yyparseFlag = 0;

//used to get rid  of the \n in the function.
void chomp (char *string, char delim) {
   size_t len = strlen (string);
   if (len == 0) return;
   char *nlpos = string + len - 1;
   if (*nlpos == delim) *nlpos = '\0';
}

//Function to token the pfile that is received when ccp the oc file
void tokenize (FILE *pfile, char *filename){
  int linenr = 1;
  DEBUGF ('f', "the file being token is = %s\n", filename);
  for(;;){
    char filestring[LINESIZE];
    char *fget_string = fgets (filestring, LINESIZE, pfile);
    if (fget_string == NULL) break;
    char *savepos = NULL;
        char *tokenstring = filestring;
        for(int tokenct = 1;; ++tokenct){
          char *token = strtok_r (tokenstring, " \t\n", &savepos);
          tokenstring = NULL;
          if (token == NULL) break;
          intern_stringset (token);
        }
        ++linenr;
  }
}

//change the extension of the file
const string changeFileExtension(string filename) {
  string lastSlash;
  string newExtension = ".str";
    filename.replace(filename.end()-3,filename.end(),newExtension);
    return filename;
}

//main function
int main (int argc, char** argv) {
  char *d_suppress_value = NULL;
  char *full_path_file;
  int c;
  //get the option the use can enter
  while ((c = getopt (argc, argv, "@:D:lf")) != -1)
    switch (c){
      case 'l':
        yylexFlag = 1;
        break;
      case 'f':
        yyparseFlag = 1;
        break;
        case '@':
            set_debugflags(optarg);
            break;
        case 'D':
            d_suppress_value = optarg;
            break;
        default:
            errprintf("bad option %c\n", optopt);
            break;
    }
    //get the file you plan to token
    full_path_file = argv[optind];

    string filename = string(basename(full_path_file));
    DEBUGF ('f', "filename = %s\n", basename(full_path_file));
    string command;
    if(d_suppress_value != NULL){
      command = CPP + " "+ "-D" + d_suppress_value +
                               " " + full_path_file;
    }else{
      command = CPP + " " + full_path_file;
    }
    
    FILE * pfile = popen (command.c_str(), "r");
    if (pfile == NULL) syserrprintf (command.c_str());
    else {
      tokenize(pfile, full_path_file);
      int pclose_rc = pclose (pfile);
        eprint_status (command.c_str(), pclose_rc);
    }
    string new_filename = changeFileExtension(filename);
    DEBUGF ('f', "filename = %s\n", new_filename.c_str());
    FILE * outputFile = fopen(new_filename.c_str(), "w");
    dump_stringset (outputFile);
    fclose(outputFile);
    return 0;
}
