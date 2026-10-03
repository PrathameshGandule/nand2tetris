#include "compengine.hpp"
#include "declarations.hpp"
#include "tokenizer.hpp"
#include <fstream>
#include <stdexcept>
#include <string>

CompilationEngine::CompilationEngine(JackTokenizer &tokenizer,
									 std::ofstream &output)
	: tokenizer(tokenizer), output(output) {}

void CompilationEngine::printIndent() {
	for (int i = 0; i < indent; i++) {
		output << "    ";
	}
}

void CompilationEngine::openTag(const std::string &tag) {
	printIndent();
	output << "<" << tag << ">\n";
	indent++;
}

void CompilationEngine::closeTag(const std::string &tag) {
	indent--;
	printIndent();
	output << "</" << tag << ">\n";
}

void CompilationEngine::writeToken(const Token &token) {
	printIndent();
	output << tokenizer.tokenToXML(token) << "\n";
}

void CompilationEngine::process(const std::string &expected) {
	Token current = tokenizer.currentTokenValue();
	if (current.value != expected) {
		throw std::runtime_error("Expected : '" + expected + "' got : '" +
								 current.value + "'");
	}
	writeToken(current);
	tokenizer.advance();
}

void CompilationEngine::processIdentifier() {
	Token current = tokenizer.currentTokenValue();
	if (current.type != TOKENTYPE::IDENTIFIER) {
		throw std::runtime_error(
			"Expected : " + tokentypeToString(TOKENTYPE::IDENTIFIER) +
			" got : " + tokentypeToString(current.type) + "");
	}
	writeToken(current);
	tokenizer.advance();
}

// 'class' className '{' classVarDec* subRoutineDec* '}'
void CompilationEngine::compileClass() {
	openTag("class");
	// std::cout << "1. inside compile class\n";
	process("class");
	// std::cout << "2. done class\n";
	compileClassName();
	// std::cout << "3. done identifier\n";
	process("{");
	// std::cout << "4. done {\n";
	while (tokenizer.currentTokenValue().value == "static" ||
		   tokenizer.currentTokenValue().value == "field") {
		// std::cout << "5. inside while static | field while loop\n";
		compileClassVarDec();
	}
	while (tokenizer.currentTokenValue().value == "constructor" ||
		   tokenizer.currentTokenValue().value == "function" ||
		   tokenizer.currentTokenValue().value == "method") {
		compileSubroutineDec();
	}
	// std::cout << "7. done with classvardec\n";
	process("}");
	// std::cout << "8. done with process }\n";

	closeTag("class");
}

// ('static' | 'field') type varName (',' varName)* ';'
void CompilationEngine::compileClassVarDec() {
	openTag("classVarDec");
	// std::cout << "6. inside classVarDec\n";

	Token current = tokenizer.currentTokenValue();

	if (current.value != "static" && current.value != "field") {
		throw std::runtime_error("Expected 'static' or 'field', got : '" +
								 current.value + "'");
	}
	process(current.value);
	compileType();
	compileVarName();
	while (tokenizer.currentTokenValue().value == ",") {
		process(",");
		compileVarName();
	}
	process(";");
	closeTag("classVarDec");
}

// 'int'|'char'|'boolean'|className
void CompilationEngine::compileType() {
	openTag("type");
	if (tokenizer.currentTokenValue().value == "int" ||
		tokenizer.currentTokenValue().value == "char" ||
		tokenizer.currentTokenValue().value == "boolean") {
		process(tokenizer.currentTokenValue().value);
	} else if (tokenizer.currentTokenValue().type == TOKENTYPE::IDENTIFIER) {
		compileClassName();
	} else {
		throw std::runtime_error(
			"Expected : type (int/char/boolean) , got : '" +
			tokenizer.currentTokenValue().value + "'");
	}
	closeTag("type");
}

// ('constructor'|'function'|'method') ('void'|type) subroutineName
// '('parameterList')' subroutineBody
void CompilationEngine::compileSubroutineDec() {
	openTag("subroutineDec");

	Token current = tokenizer.currentTokenValue();
	if (current.value == "constructor" || current.value == "function" ||
		current.value == "method") {
		process(current.value);
	} else {
		throw std::runtime_error("Expected : subroutine declaration start "
								 "(constructor/function/method), got : '" +
								 tokenizer.currentTokenValue().value + "'");
	}

	current = tokenizer.currentTokenValue();
	if (current.value == "void") {
		process(current.value);
	} else {
		compileType();
	}

	compileSubroutineName();
	process("(");
	compileParameterList();
	process(")");
	compileSubroutineBody();

	closeTag("subRoutineDec");
}

// ((type varName) (',' type varName)*)?
void CompilationEngine::compileParameterList() {
	openTag("parameterList");

	if (tokenizer.currentTokenValue().value == ")") {
		closeTag("parameterList");
		return;
	}

	compileType();
	compileVarName();

	while (tokenizer.currentTokenValue().value == ",") {
		process(",");
		compileType();
		compileVarName();
	}

	closeTag("parameterList");
}

// '{' varDec* statements '}'
void CompilationEngine::compileSubroutineBody() {
	openTag("subroutineBody");

	process("{");

	while (tokenizer.currentTokenValue().value == "var") {
		compileVarDec();
	}

	compileStatements();

	process("}");

	closeTag("subroutineBody");
}

// 'var' type varName (',' varName)* ';'
void CompilationEngine::compileVarDec() {
	openTag("varDec");

	process("var");
	compileType();
	compileVarName();
	while (tokenizer.currentTokenValue().value == ",") {
		process(",");
		compileVarName();
	}
	process(";");

	closeTag("varDec");
}

// identifier
void CompilationEngine::compileClassName() {
	openTag("className");
	processIdentifier();
	closeTag("className");
}

// identifier
void CompilationEngine::compileSubroutineName() {
	openTag("subroutineName");
	processIdentifier();
	closeTag("subroutineName");
}

// identifier
void CompilationEngine::compileVarName() {
	openTag("varName");
	processIdentifier();
	closeTag("varName");
}

// statement*
void CompilationEngine::compileStatements() {
	openTag("statements");

	while (tokenizer.currentTokenValue().value == "let" ||
		   tokenizer.currentTokenValue().value == "if" ||
		   tokenizer.currentTokenValue().value == "while" ||
		   tokenizer.currentTokenValue().value == "do" ||
		   tokenizer.currentTokenValue().value == "return") {
		compileStatement();
	}

	closeTag("statements");
}

// letStatement | ifStatement | whileStatement | doStatement | returnStatement
void CompilationEngine::compileStatement() {
	openTag("statement");

	if (tokenizer.currentTokenValue().value == "let") {
		compileLetStatement();
	} else if (tokenizer.currentTokenValue().value == "if") {
		compileIfStatement();
	} else if (tokenizer.currentTokenValue().value == "while") {
		compileWhileStatement();
	} else if (tokenizer.currentTokenValue().value == "do") {
		compileDoStatement();
	} else if (tokenizer.currentTokenValue().value == "return") {
		compileReturnStatement();
	} else {
		throw std::runtime_error(
			"Expected : type of statement (let/if/while/do/return), got : '" +
			tokenizer.currentTokenValue().value + "'");
	}

	closeTag("statement");
}

// 'let' varName ('[' expression ']')? '=' expression ';'
void CompilationEngine::compileLetStatement() {
	openTag("letStatement");

	process("let");
	compileVarName();
	if (tokenizer.currentTokenValue().value == "[") {
		process("[");
		compileExpression();
		process("]");
	}
	process("=");
	compileExpression();
	process(";");

	closeTag("letStatement");
}

// 'if' '(' expression ')' '{' statements '}' ('else' '{' statements '}')?
void CompilationEngine::compileIfStatement() {
	openTag("ifStatement");

	process("if");
	process("(");
	compileExpression();
	process(")");
	process("{");
	compileStatements();
	process("}");
	if (tokenizer.currentTokenValue().value == "else") {
		process("else");
		process("{");
		compileStatements();
		process("}");
	}

	closeTag("ifStatement");
}

// 'while' '(' expression ')' '{' statements '}'
void CompilationEngine::compileWhileStatement() {
	openTag("whileStatement");

	process("while");
	process("(");
	compileExpression();
	process(")");
	process("{");
	compileStatements();
	process("}");

	closeTag("whileStatement");
}

// 'do' subroutineCall ';'
void CompilationEngine::compileDoStatement() {
	openTag("doStatement");

	process("do");
	compileSubroutineCall();
	process(";");

	closeTag("doStatement");
}

// 'return' expression? ';'
void CompilationEngine::compileReturnStatement() {
	openTag("returnStatement");

	process("return");
	if (tokenizer.currentTokenValue().value != ";") {
		compileExpression();
	}
	process(";");

	closeTag("returnStatement");
}

void CompilationEngine::compileExpression() {
	
}
void CompilationEngine::compileTerm() {}
void CompilationEngine::compileSubroutineCall() {}
void CompilationEngine::compileExpressionList() {}
void CompilationEngine::compileOp() {}
void CompilationEngine::compileUnaryOp() {}
void CompilationEngine::compileKeywordConstant() {}