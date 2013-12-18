#ifndef __ASTREE_H__
#define __ASTREE_H__

#include <string>
#include <vector>
using namespace std;

#include "auxlib.h"
//extern stringstream astFile;
struct astree {
   int symbol;               // token code
   size_t filenr;            // index into filename stack
   size_t linenr;            // line number from source code
   size_t offset;            // offset of token with current line
   const string* lexinfo;    // pointer to lexical information
   vector<astree*> children; // children of this n-way node
   int blockNumber;
   string typeInfo;
};


astree* new_astree (int symbol, int filenr, int linenr, int offset,
                    const char* lexinfo);
astree* adopt1 (astree* root, astree* child);
astree* adopt2 (astree* root, astree* left, astree* right);
astree* adopt1sym (astree* root, astree* child, int symbol);
astree* adoptgchild(astree* grandparent, astree* parent);
void dump_astree (FILE* outfile, astree* root);
string dumpNode(astree* );
void yyprint (FILE* outfile, unsigned short toknum, astree* yyvaluep);
void free_ast (astree* tree);
void free_ast2 (astree* tree1, astree* tree2);


RCSH("$Id: astree.h,v 1.3 2013-09-20 12:23:31-07 - - $")

#endif
