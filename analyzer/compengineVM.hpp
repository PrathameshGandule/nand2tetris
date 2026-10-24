#ifndef JACK_COMPILATION_ENGINE_HPP
#define JACK_COMPILATION_ENGINE_HPP

#include "tokenizer.hpp"
#include "symboltable.hpp"
#include "vmwriter.hpp"
#include <string>

class CompilationEngine {
  private:
	JackTokenizer &tokenizer;
	SymbolTable symbols;
	VMWriter writer;
	std::string currentClassName;
	std::string currentSubroutineName;
	std::string currentSubroutineKind;
	int labelCounter = 0;

	std::string currentValue() const;
	void advance();
	void consume(const std::string &expected);
	void consumeIdentifier();
	std::string compileType();
	std::string newLabel(const std::string &prefix);
	Symbol requireSymbol(const std::string &name) const;
	std::string vmSegment(const std::string &kind) const;
	void pushSymbol(const Symbol &symbol);
	void popSymbol(const Symbol &symbol);
	bool isOperator(const std::string &value) const;
	void writeOperator(const std::string &op);

	void compileClassVarDec();
	void compileSubroutineDec();
	void compileParameterList();
	void compileSubroutineBody();
	void compileVarDec();
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
	int compileExpressionList();

  public:
	CompilationEngine(JackTokenizer &tokenizer, std::ostream &output);
	void compileClass();
};

#endif
