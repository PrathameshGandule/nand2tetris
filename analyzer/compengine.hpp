#ifndef COMPILATION_ENGINE
#define COMPILATION_ENGINE

#include "declarations.hpp"
#include "tokenizer.hpp"
#include <fstream>
class CompilationEngine {
  private:
	JackTokenizer &tokenizer;
	std::ofstream &output;

	int indent = 0;

	void process(const std::string &expected);
	void processIdentifier();
	void writeToken(const Token &token);
	void printIndent();
	void openTag(const std::string &tag);
	void closeTag(const std::string &tag);

  public:
	CompilationEngine(JackTokenizer &tokenizer, std::ofstream &output);

	void compileClass();
	void compileClassVarDec();
	void compileType();
	void compileSubroutineDec();
	void compileParameterList();
	void compileSubroutineBody();
	void compileVarDec();
	void compileClassName();
	void compileSubroutineName();
	void compileVarName();

	void compileStatements();
	void compileStatement();
	void compileLetStatement();
	void compileIfStatement();
	void compileWhileStatement();
	void compileDoStatement();
	void compileReturnStatement();

	void compileExpression();
	void compileTerm();
	void compileSubroutineCall();
	void compileExpressionList();
	void compileOp();
	void compileUnaryOp();
	void compileKeywordConstant();
};

#endif /* COMPILATION_ENGINE */