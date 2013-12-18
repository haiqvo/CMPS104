#ifndef __OILCODE_H__
#define __OILCODE_H__

#include <stdio.h>

#include <sstream>

extern stringstream oilFile;

string tempVarName(string type);

string spaces(int x);

string getOilType(string ocType);

string generate(astree* currentAstree);

void oilCodeCreate();


#endif
