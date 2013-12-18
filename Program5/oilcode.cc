#include <string>
using namespace std;

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "astree.h"
#include "lyutils.h"
#include "symtable.h"
#include "typecheck.h"
#include "oilcode.h"

int varCount = 0;


string tempVariableName(string type){
  if(type == "ubyte "){
    stringstream ss;
    ss << ++varCount;
    return "b" + ss.str();
  }
  else if(type == "int "){
    stringstream ss;
    ss << ++varCount;
    return "i" + ss.str();
  }else{
    stringstream ss;
    ss << ++varCount;
    return "p" + ss.str();
  }
}

string spaces(int x){
  return string(x, ' ');
}

string getOilType(string ocType){
  if(ocType == "bool" || ocType == "char")
    return "ubyte ";
  else if(ocType == "int")
    return "int ";
  else if(ocType == "int[]")
    return "int *";
  else if(ocType == "string" || ocType == "bool[]" || 
    ocType == "char[]")
    return "ubyte *";
  else if(ocType == "string[]")
    return "ubyte **";
  else if(ocType == "void")
    return "void";
  else{
    if(*(ocType.end()-1) == ']')
      return "struct "  + string(ocType.begin(), ocType.end() - 2)
        + " **";
    else
      return "struct " + ocType + " *";
  }
}

string createCode(astree* currentAstree){      
  if(currentAstree->symbol == NONTERM_CONSTANT){
    if(currentAstree->children[0]->symbol == TOK_NULL){
      return "0";
    }
    else if(currentAstree->children[0]->symbol == TOK_TRUE){
      return "1";
    }
    else if(currentAstree->children[0]->symbol == TOK_FALSE){
      return "0";
    }
    return *(currentAstree->children[0]->lexinfo);
  }
  else if(currentAstree->symbol == NONTERM_BLOCK){
    vector<astree*>::iterator i = currentAstree->children.begin();
    vector<astree*>::iterator j = currentAstree->children.end();
    for(; i != j; i++){
      createCode(*i);
    }
  }
  else if(currentAstree->symbol == NONTERM_VARDECL){
    string type = getOilType(currentAstree->children[2]->typeInfo);
    string rAddr = createCode(currentAstree->children[2]);
    oilFile << spaces(8) << type << '_' << currentAstree->children[0]
      ->blockNumber <<'_' << *(currentAstree->children[1]->lexinfo);
    oilFile << " = " << rAddr << ';' << endl;
  } 
  else if(currentAstree->symbol == NONTERM_IFELSE){
    string cond = createCode(currentAstree->children[0]);
    stringstream ss1;
    ss1 << ++varCount;
    string elseTag = "else_" + ss1.str();
    oilFile << spaces(8) << "if (!" << cond << ") goto " << elseTag 
      << ';' << endl;
    createCode(currentAstree->children[1]);
    stringstream ss2;
    ss2 << ++varCount;
    string fiTag = "fi_" + ss2.str();
    oilFile << spaces(8) << "goto " << fiTag << ';' << endl;
    oilFile << elseTag << ":;" << endl;
    if(currentAstree->children.size() > 2)
      createCode(currentAstree->children[2]);
    oilFile << fiTag << ":;" << endl;
  }
  else if(currentAstree->symbol == NONTERM_WHILE){
    stringstream ss1;
    ss1 << ++varCount;
    string whileLabel = "while_" + ss1.str();
    stringstream ss2;
    ss2 << ++varCount;
    string breakLabel = "break_" + ss2.str();
    oilFile << whileLabel << ":;" << endl;
    string cond = createCode(currentAstree->children[0]);
    oilFile << spaces(8) << "if (!" << cond << ") goto " << breakLabel
      << ';' <<endl;
    createCode(currentAstree->children[1]);
    oilFile << spaces(8) << "goto " << whileLabel << ";" << endl;
    oilFile << breakLabel << ":;" << endl;
  }
  else if(currentAstree->symbol == NONTERM_RET){
    string rAddr = createCode(currentAstree->children[0]);
    oilFile << spaces(8) << "return " << rAddr << ';' << endl;
  }
  else if(currentAstree->symbol == NONTERM_VAR){
    stringstream ss("");
    ss << "_";
    if(currentAstree->blockNumber != 1){
      ss << currentAstree->blockNumber;
    }
    ss << "_" << *(currentAstree->children[0]->lexinfo);
    return ss.str();
  }
  else if(currentAstree->symbol == NONTERM_UNOP){
    if(*(currentAstree->children[0]->lexinfo) == "ord"){
      string right = createCode(currentAstree->children[1]);
      string type = getOilType(currentAstree->typeInfo);
      string tempVar = tempVariableName(type);
      oilFile << spaces(8) << type << tempVar << " = " <<
        "(int)" << right << ';' << endl;
      return tempVar;
    }
    else if(*(currentAstree->children[0]->lexinfo) == "chr"){
      string right = createCode(currentAstree->children[1]);
      string type = getOilType(currentAstree->typeInfo);
      string tempVar = tempVariableName(type);
      oilFile << spaces(8) << type << tempVar << " = " <<
        "(ubyte)" << right << ';' << endl;
      return tempVar;
      }
    else{
      string right = createCode(currentAstree->children[1]);
      string type = getOilType(currentAstree->typeInfo);
      string tempVar = tempVariableName(type);
      oilFile << spaces(8) << type << tempVar << " = " <<
        *(currentAstree->children[0]->lexinfo) << right << ';' << endl;
      return tempVar;
      }
  }
  else if(currentAstree->symbol == NONTERM_BINOP){
    if(currentAstree->children[1]->symbol == '='){
      string left = createCode(currentAstree->children[0]);
      string right = createCode(currentAstree->children[2]);
      oilFile << spaces(8) << left << " = " << right << ';' << endl;
      return right;
    }else{
      string left = createCode(currentAstree->children[0]);
      string right = createCode(currentAstree->children[2]);
      string type = getOilType(currentAstree->typeInfo);
      string tempVar = tempVariableName(type);
      oilFile << spaces(8) << type << tempVar << " = " << left <<
        " " << *(currentAstree->children[1]->lexinfo) <<
        " " << right << ';' << endl;
      return tempVar;
      }
  }
  else if(currentAstree->symbol == NONTERM_ALLOCATOR){
    if(currentAstree->children.size() == 2){
      if(*(currentAstree->lexinfo) == string("paren_alloc")){
        string type = getOilType(currentAstree->typeInfo);
        string tempVar = tempVariableName(type);
        oilFile << spaces(8) << type << tempVar << " = " <<
          "xcalloc(" << createCode(currentAstree->children[1]) <<
          ", sizeof(ubyte));" << endl;
        return tempVar;
      }else{
        string type = getOilType(currentAstree->typeInfo);
        string tempVar = tempVariableName(type);
        oilFile << spaces(8) << type << tempVar << " = " <<
          "xcalloc(" << createCode(currentAstree->children[1]) 
          << ", sizeof(" << string(type.begin(), type.end()-2) << "));" 
          << endl;
        return tempVar;
      }
    }else{
      string type = getOilType(currentAstree->typeInfo);
      string tempVar = tempVariableName(type);
      oilFile << spaces(8) << type << tempVar << " = " <<
        "xcalloc(1, sizeof(" << string(type.begin(), type.end()-2) 
          << "));" << endl;
      return tempVar;
    }
  }
  else if(currentAstree->symbol == NONTERM_CALL){
    if(currentAstree->typeInfo == "void"){
      vector<astree*>::iterator j = currentAstree->children.begin();
      vector<astree*>::iterator i = currentAstree->children.end()-1;
      stringstream params;
      for(; i != j; i--){
        if(i == currentAstree->children.begin() + 1){
          params << createCode(*i);
        }else
          params << createCode(*i) << ", ";
      }
      oilFile << spaces(8) << "__" << 
        *(currentAstree->children[0]->lexinfo) << "(";
      oilFile << params.str() << ")";
      oilFile << ";" << endl;
    }else{
      vector<astree*>::iterator j = currentAstree->children.begin();
      vector<astree*>::iterator i = currentAstree->children.end()-1;
      stringstream params;
      for(; i != j; i--){
        if(i == currentAstree->children.begin() + 1){
          params << createCode(*i);
        }
        else
          params << createCode(*i) << ", ";
      }
      params << ")";
      string type = getOilType(currentAstree->typeInfo);
      string tempVar = tempVariableName(type);
      oilFile << spaces(8) << type << tempVar << " = __" <<
        *(currentAstree->children[0]->lexinfo) << "(";
      oilFile << params.str() <<  ";" << endl;
      return tempVar;
    }
  }
  else if(currentAstree->symbol == TOK_FIELD){
    string left = createCode(currentAstree->children[0]);
    string type = getOilType(currentAstree->typeInfo);
    string tempVar = tempVariableName(type);
    oilFile << spaces(8) << type << tempVar << " = " <<
      left << "->" << *(currentAstree->children[1]->lexinfo) 
      << ";" <<endl;
    return tempVar;
  }
  else if(currentAstree->symbol == TOK_ARRAY){
    string param = createCode(currentAstree->children[1]);
    string left = createCode(currentAstree->children[0]);
    return left + "[" + param + "]";
  }
  return "";
}

void oilCodeCreate(){
  oilFile << "#define __OCLIB_C__\n#include \"oclib.oh\"" 
    << endl << endl;
  for(vector<astree*>::iterator i = yyparse_astree->children.begin();
    i != yyparse_astree->children.end(); i++){
    if((*i)->symbol == NONTERM_STRUCTDEF){
      oilFile << "struct " << *((*i)->children[0]->lexinfo) <<
        " {\n";
      vector<astree*>::iterator decl = (*i)->children.begin() + 1;
      vector<astree*>::iterator end = (*i)->children.end();
      for(; decl != end; decl++){
        oilFile << spaces(8) << 
        getOilType(SymbolTable::getType((*decl)->children[0])) <<
        *((*decl)->children[1]->lexinfo) << ';' << endl;
      }
      oilFile << "};" << endl << endl;
    }
  }
  
  for(vector<astree*>::iterator i = yyparse_astree->children.begin();
    i != yyparse_astree->children.end(); i++){
    if((*i)->symbol == NONTERM_VARDECL){
    oilFile << getOilType(SymbolTable::getType((*i)->children[0])) <<
      "__" << *((*i)->children[1])->lexinfo << ';' << endl;
    }
  }
  oilFile << endl;
  
  for(vector<astree*>::iterator i = yyparse_astree->children.begin();
    i != yyparse_astree->children.end(); i++){
    if((*i)->symbol == NONTERM_FUNCTION && 
      *((*((*i)->children.end()-1))->children[0]->lexinfo) != ";"){
      oilFile << getOilType((SymbolTable::parseSignature(
        SymbolTable::functionSign(*i)))[0]) << endl;
      oilFile << "__" << *((*i)->children[1]->lexinfo) << '(' << endl;
      vector<astree*>::iterator end = (*i)->children.begin() + 1;
      vector<astree*>::iterator decl = (*i)->children.end() - 2;
      for(; decl != end; decl--){
        oilFile << spaces(8) << 
          getOilType(SymbolTable::getType((*decl)->children[0])) <<
          "_" << (*decl)->children[0]->blockNumber << "_" <<
          *((*decl)->children[1]->lexinfo) <<
          (decl == (*i)->children.begin() + 2 ? "" : ",\n");
      }
      oilFile << ")" << endl;      
      oilFile << '{' << endl;
      vector<astree*>::iterator element = (*((*i)->
        children.end() - 1))->children.begin();
      vector<astree*>::iterator endElement = 
        (*((*i)->children.end() - 1))->children.end();
      for(; element != endElement; element++){
        createCode(*element);
      }
      oilFile << '}' << endl << endl;
    }
  }

  oilFile << "void" << endl;
  oilFile << "__ocmain()" << endl;
  oilFile << '{' << endl;
    
  for(vector<astree*>::iterator i = yyparse_astree->children.begin();
    i != yyparse_astree->children.end(); i++){
    if((*i)->symbol == NONTERM_VARDECL){
      string rAddr = createCode((*i)->children[2]);
      oilFile << spaces(8) << "__" << *((*i)->children[1]->lexinfo) <<
        " = " << rAddr << ';' << endl;
    }
    else if((*i)->symbol != NONTERM_FUNCTION && 
      (*i)->symbol != NONTERM_STRUCTDEF){
      createCode((*i));
    }
  }
  oilFile << '}' << endl;
}
