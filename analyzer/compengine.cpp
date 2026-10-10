#include "compengine.hpp"
#include "declarations.hpp"
#include "tokenizer.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>

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

void CompilationEngine::processIntegerConst() {
	Token current = tokenizer.currentTokenValue();
	if (current.type != TOKENTYPE::INTCONST) {
		throw std::runtime_error(
			"Expected : " + tokentypeToString(TOKENTYPE::INTCONST) +
			" got : " + tokentypeToString(current.type) + "");
	}
	writeToken(current);
	tokenizer.advance();
}
void CompilationEngine::processStringConst() {
	Token current = tokenizer.currentTokenValue();
	if (current.type != TOKENTYPE::STRINGCONST) {
		throw std::runtime_error(
			"Expected : " + tokentypeToString(TOKENTYPE::STRINGCONST) +
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

	printsyms();
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
	sym.kind = current.value;
	process(current.value);
	sym.type = tokenizer.currentTokenValue().value;
	compileType();
	sym.name = tokenizer.currentTokenValue().value;
	if (sym.kind == "field") {
		sym.index = fieldcnt;
		fieldcnt++;
	} else {
		sym.index = staticcnt;
		staticcnt++;
	}
	if (classSymbols.find(sym.name) != classSymbols.end()) {
		throw std::runtime_error("variable '" + sym.name +
								 "' already declared!!!");
	}
	classSymbols[sym.name] = {sym.name, sym.type, sym.kind, sym.index};
	compileVarName();
	while (tokenizer.currentTokenValue().value == ",") {
		process(",");
		sym.name = tokenizer.currentTokenValue().value;
		if (sym.kind == "field") {
			sym.index = fieldcnt;
			fieldcnt++;
		} else {
			sym.index = staticcnt;
			staticcnt++;
		}
		if (classSymbols.find(sym.name) != classSymbols.end()) {
			throw std::runtime_error("variable '" + sym.name +
									 "' already declared!!!");
		}
		classSymbols[sym.name] = {sym.name, sym.type, sym.kind, sym.index};
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

	closeTag("subroutineDec");
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
	int a = 1;
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

// term (op term)*
void CompilationEngine::compileExpression() {
	openTag("expression");

	compileTerm();
	while (tokenizer.currentTokenValue().value == "+" ||
		   tokenizer.currentTokenValue().value == "-" ||
		   tokenizer.currentTokenValue().value == "*" ||
		   tokenizer.currentTokenValue().value == "/" ||
		   tokenizer.currentTokenValue().value == "&" ||
		   tokenizer.currentTokenValue().value == "|" ||
		   tokenizer.currentTokenValue().value == "<" ||
		   tokenizer.currentTokenValue().value == ">" ||
		   tokenizer.currentTokenValue().value == "=") {
		compileOp();
		compileTerm();
	}

	closeTag("expression");
}

// integerConstant | stringConstant | keywordConstant | varName | varName '['
// expression ']' | subroutineCall | '(' expression ')' | unaryOp term
void CompilationEngine::compileTerm() {
	openTag("term");

	Token current = tokenizer.currentTokenValue();

	if (current.type == TOKENTYPE::INTCONST) {
		processIntegerConst();
	} else if (current.type == TOKENTYPE::STRINGCONST) {
		processStringConst();
	} else if (current.value == "true" || current.value == "false" ||
			   current.value == "null" || current.value == "this") {
		compileKeywordConstant();
	} else if (current.value == "-" || current.value == "~") {
		compileUnaryOp();
		compileTerm();
	} else if (current.value == "(") {
		process("(");
		compileExpression();
		process(")");
	} else if (current.type == TOKENTYPE::IDENTIFIER) {
		Token next = tokenizer.peek();
		if (next.value == "[") {
			compileVarName();
			process("[");
			compileExpression();
			process("]");
		} else if (next.value == "(" || next.value == ".") {
			compileSubroutineCall();
		} else {
			compileVarName();
		}
	} else {
		throw std::runtime_error("Invalid term, got : '" + current.value + "'");
	}

	closeTag("term");
}

// subroutineName '(' expressionList ')' |
// (className | varName) '.' subroutineName '(' expressionList ')'
void CompilationEngine::compileSubroutineCall() {
	openTag("subroutineCall");

	processIdentifier();
	if (tokenizer.currentTokenValue().value == "(") {
		process("(");
		compileExpressionList();
		process(")");
	} else if (tokenizer.currentTokenValue().value == ".") {
		process(".");
		processIdentifier();
		process("(");
		compileExpressionList();
		process(")");
	}

	closeTag("subroutineCall");
}

// (expression (',' expression)* )?
void CompilationEngine::compileExpressionList() {
	openTag("expressionList");

	if (tokenizer.currentTokenValue().value == ")") {
		closeTag("expressionList");
		return;
	}

	compileExpression();
	while (tokenizer.currentTokenValue().value == ",") {
		process(",");
		compileExpression();
	}

	closeTag("expressionList");
}

// '+' | '-' | '*' | '/' | '&' | '|' | '<' | '>' | '='
void CompilationEngine::compileOp() {
	openTag("op");

	Token current = tokenizer.currentTokenValue();
	if (current.value == "+" || current.value == "-" || current.value == "*" ||
		current.value == "/" || current.value == "&" || current.value == "|" ||
		current.value == "<" || current.value == ">" || current.value == "=") {
		process(current.value);
	} else {
		throw std::runtime_error("Expected operator , got '" + current.value +
								 "'");
	}

	closeTag("op");
}

// '-' | '~'
void CompilationEngine::compileUnaryOp() {
	openTag("unaryOp");

	Token current = tokenizer.currentTokenValue();
	if (current.value == "-" || current.value == "~") {
		process(current.value);
	} else {
		throw std::runtime_error("Expected unary operator , got '" +
								 current.value + "'");
	}

	closeTag("unaryOp");
}

// 'true' | 'false' | 'null' | 'this'
void CompilationEngine::compileKeywordConstant() {
	openTag("keywordConstant");

	Token current = tokenizer.currentTokenValue();
	if (current.value == "true" || current.value == "false" ||
		current.value == "null" || current.value == "this") {
		process(current.value);
	} else {
		throw std::runtime_error("Expected keyword constant , got '" +
								 current.value + "'");
	}

	closeTag("keywordConstant");
}

void CompilationEngine::printsyms() {
	std::string res = "";
	res = "SYMBOL TABLE\n\nname\ttype\tkind\tindex\n\n";
	for (auto &sym : classSymbols) {
		res += sym.second.name + "\t" + sym.second.type + "\t" + sym.second.kind + "\t" +
			   std::to_string(sym.second.index) + "\n";
	}
	std::cout << res << std::endl;
}