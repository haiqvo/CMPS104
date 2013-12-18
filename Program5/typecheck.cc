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

SymbolTable* typeTable = new SymbolTable(NULL);
SymbolTable* identTable = new SymbolTable(NULL);

bool isString(string type){
  if(typeTable->lookup(type) == NULL) return false;
  if(typeTable->lookup(type)->symbol == NONTERM_STRUCTDEF ||
     type == "string"){
    return true;
  }
  size_t brackIndex = type.find("[]");
  if(brackIndex != string::npos) return true;
  return false;
}

string typeCheck(astree* currentAstree, SymbolTable* currentSymTable){
  if(currentAstree->symbol == NONTERM_CONSTANT){
    switch(currentAstree->children[0]->symbol){
      case TOK_TRUE: 
      case TOK_FALSE: {
        currentAstree->typeInfo = "bool";
        return "bool";
      }
      case TOK_INTCON: {
        currentAstree->typeInfo = "int";
        return "int";
      }
      case TOK_CHARCON: {
        currentAstree->typeInfo = "char";
        return "char";
      }
      case TOK_STRINGCON: {
        currentAstree->typeInfo = "string";
        return "string";
      }
      case TOK_NULL: {
        currentAstree->typeInfo = "null";
        return "null";
      }
    }
  }
  else if(currentAstree->symbol == NONTERM_BINOP){
    switch(currentAstree->children[1]->symbol){
      case '=':{
        string expr1 = typeCheck(currentAstree->children[0],
                                 currentSymTable);
        string expr2 = typeCheck(currentAstree->children[2],
                                 currentSymTable);
        if(expr1 != expr2 && !(isString(expr1) && expr2 == "null")){
          set_exitstatus(EXIT_FAILURE);
          cerr <<"-" << dumpNode(currentAstree->children[0]) <<
            " Types does not match (binop): "<< expr1 << " != " 
            << expr2 << endl;
        }
        currentAstree->typeInfo = expr1;
        return expr1;
      }
      
      case '<':
      case TOK_LE:
      case '>':
      case TOK_GE:{
        string expr1 = typeCheck(currentAstree->children[0],
                                 currentSymTable);
        string expr2 = typeCheck(currentAstree->children[2],
                                 currentSymTable);
        if(expr1 != expr2 && 
          (expr1 == "bool" ||expr1 == "char" || expr1 == "int") &&
          (expr2 == "bool" || expr2 == "char" ||expr2 == "int")){
            set_exitstatus(EXIT_FAILURE);
            cerr <<"-" << dumpNode(currentAstree->children[0]) <<
              " Types does not match (binop): " <<
              expr1 << " != " << expr2 << endl;
        }
        else if(expr1 != expr2){
          set_exitstatus(EXIT_FAILURE);
          cerr <<"-" << dumpNode(currentAstree->children[0]) <<
            " Cannot compare "<< *currentAstree->children[1]->lexinfo<<
            ": " << expr1 << " != " << expr2 << endl;
        }
        currentAstree->typeInfo = "bool";
        return "bool";
      }
      case TOK_EQ:
      case TOK_NE:{
        string expr1 = typeCheck(currentAstree->children[0],
                                 currentSymTable);
        string expr2 = typeCheck(currentAstree->children[2],
                                 currentSymTable);
        if(expr1 != expr2 && !(isString(expr1) && expr2 == "null") 
           && !(isString(expr2) && expr1 == "null")){
          set_exitstatus(EXIT_FAILURE);
          cerr <<"-" << dumpNode(currentAstree->children[0]) <<
            " Types does not match (binop): " <<
            expr1 << " != " << expr2 << endl;
        }
        currentAstree->typeInfo = "bool";
        return "bool";
      }
      case '+':
      case '-':
      case '*':
      case '/':
      case '%':{
        string expr1 = typeCheck(currentAstree->children[0],
                                 currentSymTable);
        string expr2 = typeCheck(currentAstree->children[2],
                                 currentSymTable);
        if(expr1 != "int"){
          set_exitstatus(EXIT_FAILURE);
          cerr << "-" << dumpNode(currentAstree->children[0]) <<
            " Math require int" << endl;
        }
        if(expr2 != "int"){
          set_exitstatus(EXIT_FAILURE);
          cerr << "-" << dumpNode(currentAstree->children[2]) <<
            " Math require int" << endl;
        }
        currentAstree->typeInfo = "int";
        return "int";
      }
    }
  }
  else if(currentAstree->symbol == NONTERM_UNOP){
    switch(currentAstree->children[0]->symbol){
      case '!':{
        string exprType = typeCheck(currentAstree->children[1],
                                    currentSymTable);
        if(exprType != "bool"){
          set_exitstatus(EXIT_FAILURE);
          cerr << "-" << dumpNode(currentAstree->children[1]) <<
          " Expression does not evaluate to bool" << endl;
        }
        currentAstree->typeInfo = "bool";
        return "bool";
      }
      case '+':
      case '-':{
        string exprType = typeCheck(currentAstree->children[1],
                                    currentSymTable);
        if(exprType != "int"){
          set_exitstatus(EXIT_FAILURE);
          cerr << "-" << dumpNode(currentAstree->children[1]) <<
          " Expression does not evaluate to int." << endl;
        }
        currentAstree->typeInfo = "int";
        return "int";
      }
      case TOK_ORD:{
        string exprType = typeCheck(currentAstree->children[1],
                                    currentSymTable);
        if(exprType != "char"){
          set_exitstatus(EXIT_FAILURE);
          cerr << "-" << dumpNode(currentAstree->children[1]) <<
            " Expression does not evaluate to char." << endl;
        }
        currentAstree->typeInfo = "int";
        return "int";
      }
      case TOK_CHR:{
        string exprType = typeCheck(currentAstree->children[1],
                                    currentSymTable);
        if(exprType != "int"){
          set_exitstatus(EXIT_FAILURE);
          cerr << "-" << dumpNode(currentAstree->children[1]) <<
            " Expression does not evaluate to int." << endl;
        }
        currentAstree->typeInfo = "char";
        return "char";
      }
    }
  }
  else if(currentAstree->symbol == NONTERM_VAR){
    string variableName = *((currentAstree->children[0])->lexinfo);
    astree* type = currentSymTable->lookup(variableName);
    if(type == NULL){
      set_exitstatus(EXIT_FAILURE);
      cerr <<"-" << dumpNode(currentAstree) << 
      " Undefined Identifier: " << variableName << endl;
    }
    else{
      string typeAsString = SymbolTable::getType(type);
      currentAstree->typeInfo = typeAsString;
      currentAstree->blockNumber = type->blockNumber;
      return typeAsString;
    }
  }  
  else if(currentAstree->symbol == TOK_FIELD){
    astree* returnType = NULL;
    string currentType = typeCheck(currentAstree->children[0],
                                   currentSymTable);
    astree* structFromTable = typeTable->lookup(currentType);
    if(structFromTable->symbol != NONTERM_STRUCTDEF){
      set_exitstatus(EXIT_FAILURE);
      cerr << "-" << dumpNode(currentAstree) <<" ("<< currentType<< 
      ") is not a struct" << endl;
    }
    else{
      string fieldName = *(currentAstree->children[1]->lexinfo);
      bool fieldFound = false;
      vector<astree*>::iterator decl = structFromTable
                                       ->children.begin() + 1;
      vector<astree*>::iterator end = structFromTable->children.end();
      for(; decl != end; decl++){
        if(*((*decl)->children[1]->lexinfo) == fieldName){
          fieldFound = true;
          returnType = (*decl)->children[0];
          break;
        }
      }
      if(!fieldFound){
        set_exitstatus(EXIT_FAILURE);
        cerr<< "-" << dumpNode(currentAstree) <<
        ": No such field " <<endl;
      }
    }
    if(returnType == NULL){
      currentAstree->typeInfo = "void";
      return "void";
    }
    currentAstree->typeInfo = SymbolTable::getType(returnType);
    return SymbolTable::getType(returnType);
  }
  else if(currentAstree->symbol == TOK_ARRAY){
    string firstExprType = typeCheck(currentAstree->children[0],
                                 currentSymTable);
    if((*(firstExprType.end()-2) =='[' && 
      *(firstExprType.end()-1) == ']') ||
        firstExprType == "string"){
      string secondExprType = typeCheck(currentAstree->children[1],
                                   currentSymTable);
      if(secondExprType != "int"){
        set_exitstatus(EXIT_FAILURE);
        cerr << "-" << dumpNode(currentAstree) << 
        "index must be of type int" << endl;
      }
    }
    else{
      set_exitstatus(EXIT_FAILURE);
      cerr << "-" << dumpNode(currentAstree) << 
      " Array access invalid for non-array types" << endl;
    }
    if(firstExprType == "string"){
      currentAstree->typeInfo = "char";
      return "char";
    }else if(firstExprType == ""){
      currentAstree->typeInfo = "";
      return "";
    }else{
      currentAstree->typeInfo = string(firstExprType.begin(), 
        firstExprType.end()-2);
      return string(firstExprType.begin(), firstExprType.end()-2);
    }
  }
  else if(currentAstree->symbol == NONTERM_CALL){
    vector<string> funcSignVec;
    vector<string> callSignVec; 
    string funcName = *(currentAstree->children[0]->lexinfo);
    callSignVec.push_back("");
    vector<astree*>::iterator expr = currentAstree
                                     ->children.begin() + 1;
    vector<astree*>::iterator end = currentAstree->children.end();
    for(; expr != end; expr++){
      callSignVec.push_back(typeCheck(*expr, currentSymTable));
    }
    astree* functionAst = currentSymTable->lookup(funcName);
    if(functionAst == NULL){
      set_exitstatus(EXIT_FAILURE);
      cerr << "-" << dumpNode(currentAstree) <<
        " No such function accessible within the scope: " << 
        funcName << endl;
    }
    else{
      funcSignVec = SymbolTable::parseSignature
                    (SymbolTable::functionSign(functionAst));
      callSignVec[0] = funcSignVec[0];
      if(funcSignVec != callSignVec){
        set_exitstatus(EXIT_FAILURE);
        cerr << "-" << dumpNode(currentAstree) << 
        " No match" << endl;
      }
    }
    currentAstree->typeInfo = callSignVec[0];
    return callSignVec[0];
  }
  else if(currentAstree->symbol == NONTERM_ALLOCATOR){
    if(currentAstree->children.size() == 2){
      if(*(currentAstree->lexinfo) == string("paren_alloc")){
        string basetype = *(currentAstree->children[0]->children[0]
                            ->lexinfo);
        string exprType = typeCheck(currentAstree->children[1],
                                    currentSymTable);
        if(exprType != "int"){
          set_exitstatus(EXIT_FAILURE);
          cerr << " Must instantiate int" << endl;
        }
        if(basetype != "string"){
          set_exitstatus(EXIT_FAILURE);
          cerr << "Allocation can only be in string" << endl;
        }
        currentAstree->typeInfo = basetype;
        return "string";
      } 
      else{
        string basetype = *(currentAstree->children[0]->children[0]
                            ->lexinfo);
        string exprType = typeCheck(currentAstree->children[1],
                                    currentSymTable);
        if(exprType != "int"){
          set_exitstatus(EXIT_FAILURE);
          cerr <<"-" << dumpNode(currentAstree->children[1]) <<
          " Array must be [int]"
          << endl;
        }
        currentAstree->typeInfo = basetype + "[]";
        return (basetype + "[]");
      }
    }
    else {
      string basetype = *(currentAstree->children[0]->
                          children[0]->lexinfo);
      currentAstree->typeInfo = basetype;
      return basetype;
    }
  }
  else if(currentAstree->symbol == NONTERM_STRUCTDEF){
        currentAstree->typeInfo = *(currentAstree->children[0]
          ->lexinfo);
        return *(currentAstree->children[0]->lexinfo);
  }
  return "";
}

void recursiveSymTable(astree* currentAstree, 
                       SymbolTable* currentSymTable){
  if(currentAstree->symbol == NONTERM_BLOCK){
    SymbolTable* newBlock = currentSymTable->enterBlock();
    for(vector<astree*>::iterator i = currentAstree->children.begin();
      i != currentAstree->children.end(); i++){
      recursiveSymTable(*i, newBlock);
    }
  }
  else if(currentAstree->symbol == NONTERM_IFELSE){
    vector<astree*>::iterator i = (currentAstree->
                                  children.begin());
    string conType = typeCheck(*i, currentSymTable);
    if(conType != "bool"){
      set_exitstatus(EXIT_FAILURE);
      cerr << "Must be type bool" << endl;
    }
    i++;
    vector<astree*>::iterator j = (currentAstree->children.end());
    for(; i != j; i++){
      recursiveSymTable(*i, currentSymTable);
    }
  }
  else if(currentAstree->symbol == NONTERM_VARDECL){
    string name = *(currentAstree->children[1]->lexinfo); 
    astree* type = currentAstree->children[0]; 
    if(typeTable->lookup(SymbolTable::getType(type)) != NULL){
      currentSymTable->addSymbol(name, type);
      type->blockNumber = currentSymTable->getNumber();
    }
    else{
      set_exitstatus(EXIT_FAILURE);
      cerr << dumpNode(currentAstree) << " Type " <<
        SymbolTable::getType(type) << " not declared" << endl;
    }
    string convertToString = SymbolTable::getType(type);
    string expressionType = typeCheck(currentAstree->children[2],
                            currentSymTable);
    if(convertToString != expressionType && 
      !((isString(convertToString) && expressionType == "null"))){
      set_exitstatus(EXIT_FAILURE);
      cerr << "Expression does not match the declared type " << endl;
    }
  }
  else if(currentAstree->symbol == NONTERM_RET){
    astree* functionAST = currentSymTable->parentFunction(NULL);
    string funcType;
    if(functionAST == NULL){
      funcType = "void";
    }else{
      funcType = (SymbolTable::parseSignature
        (SymbolTable::functionSign(functionAST)))[0];
    }
    if(currentAstree->children[0]->symbol == ';'){
      string toRetType = "void";
      if(funcType != toRetType){
        set_exitstatus(EXIT_FAILURE);
        cerr << "Function has to return " << funcType << 
        " NOT" << toRetType << " Error at " << 
        dumpNode(currentAstree->children[0]) << endl;
      }
    }else{
      string toRetType = typeCheck(currentAstree->children[0],
                          currentSymTable);
      if(funcType != toRetType){
        set_exitstatus(EXIT_FAILURE);
        if(functionAST == NULL){
          cerr<<"global return";
        }
        else{
          cerr << "did not return";
        }
        cerr << funcType << " not " << toRetType << " Error at " <<
         dumpNode(currentAstree->children[0]) << endl;
      }
    }
  }
  else if(currentAstree->symbol == NONTERM_WHILE){
    string conType = typeCheck(currentAstree->children[0], 
      currentSymTable);
    if(conType != "bool"){
      set_exitstatus(EXIT_FAILURE);
      cerr << "Must be bool type" << endl;
    }
    recursiveSymTable(currentAstree->children[1], currentSymTable);
  }
  else{
    typeCheck(currentAstree, currentSymTable);
  }
}

void initalizeTable(){
  typeTable->addSymbol("void",new_astree(TOK_VOID, 0, 0, 0, "void"));
  typeTable->addSymbol("bool",new_astree(TOK_BOOL, 0, 0, 0, "bool"));
  typeTable->addSymbol("char",new_astree(TOK_CHAR, 0, 0, 0, "char"));
  typeTable->addSymbol("int", new_astree(TOK_INT, 0, 0, 0, "int"));
  typeTable->addSymbol("string",
    new_astree(TOK_STRING, 0, 0, 0, "string"));
  typeTable->addSymbol("bool[]",
    new_astree(TOK_ARRAY, 0, 0, 0, "bool[]"));
  typeTable->addSymbol("char[]",
    new_astree(TOK_ARRAY, 0, 0, 0, "char[]"));
  typeTable->addSymbol("int[]", 
    new_astree(TOK_ARRAY, 0, 0, 0, "int[]"));
  typeTable->addSymbol("string[]",
    new_astree(TOK_ARRAY, 0, 0, 0, "string[]"));
}

void createSymbolTable(){
  initalizeTable();
  for(vector<astree*>::iterator i = yyparse_astree->children.begin();
         i != yyparse_astree->children.end(); i++){
    if((*i)->symbol == NONTERM_FUNCTION){
      astree* currentAstree = *i;
      SymbolTable* currentSymTable = identTable;
      string name = *(currentAstree->children[1]->lexinfo);
      astree* sig = currentAstree; 
      SymbolTable* functionBlock = currentSymTable->
                               enterFunction(name,sig);
      vector<astree*>::iterator decl = 
                currentAstree->children.begin() + 2;
      vector<astree*>::iterator end = 
                currentAstree->children.end() - 1;
      for(; decl != end; decl++){
        functionBlock->addSymbol(*((*decl)->children[1]->lexinfo),
        (*decl)->children[0]);
        (*decl)->children[0]->blockNumber = functionBlock->getNumber();
      }
      vector<astree*>::iterator i = 
          (*(currentAstree->children.end() - 1))->children.begin();
      vector<astree*>::iterator j = 
          (*(currentAstree->children.end() - 1))->children.end();
      for(; i != j; i++){
        recursiveSymTable(*i, functionBlock);
      }
    } 
    else if((*i)->symbol == NONTERM_STRUCTDEF){
      astree* structAstree = *i;
      string name = *(structAstree->children[0]->lexinfo);
      SymbolTable* structBlock = typeTable->
                      enterFunction(name, structAstree);
      typeTable->enterFunction(name + "[]", structAstree);
      vector<astree*>::iterator decl = structAstree->
                       children.begin() + 1;
      vector<astree*>::iterator end = structAstree->children.end();
      for(; decl != end; decl++){
        structBlock->addSymbol(*((*decl)->children[1]->lexinfo),
                              (*decl)->children[0]);
      }
    }else{
      recursiveSymTable(*i, identTable);
    }
  }
}
