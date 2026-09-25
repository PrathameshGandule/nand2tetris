#include "compengine.hpp"
#include "declarations.hpp"
#include "tokenizer.hpp"
#include <fstream>
#include <iostream>
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
	std::cout << "1. inside compile class\n";
	process("class");
	std::cout << "2. done class\n";
	processIdentifier();
	std::cout << "3. done identifier\n";
	process("{");
	std::cout << "4. done {\n";
	while (tokenizer.currentTokenValue().value == "static" ||
		   tokenizer.currentTokenValue().value == "field") {
		std::cout << "5. inside while static | field while loop\n";
		compileClassVarDec();
	}
	while (tokenizer.currentTokenValue().value == "constructor" ||
		   tokenizer.currentTokenValue().value == "function" ||
		   tokenizer.currentTokenValue().value == "method") {
		compileSubroutine();
	}
	std::cout << "7. done with classvardec\n";
	process("}");
	std::cout << "8. done with process }\n";

	closeTag("class");
}

// ('static' | 'field') type varName (',' varName)* ';'
void CompilationEngine::compileClassVarDec() {
	openTag("classVarDec");
	std::cout << "6. inside classVarDec\n";

	Token current = tokenizer.currentTokenValue();

	if (current.value != "static" && current.value != "field") {
		throw std::runtime_error("Expected 'static' or 'field', got : '" +
								 current.value + "'");
	}
	process(current.value);
	compileType();
	processIdentifier();
	while (tokenizer.currentTokenValue().value == ",") {
		process(",");
		processIdentifier();
	}
	process(";");
	closeTag("classVarDec");
}

void CompilationEngine::compileType() {
	openTag("type");
	if (tokenizer.currentTokenValue().value != "int" ||
		tokenizer.currentTokenValue().value != "char" ||
		tokenizer.currentTokenValue().value != "boolean") {
		process(tokenizer.currentTokenValue().value);
	} else if (tokenizer.currentTokenValue().type == TOKENTYPE::IDENTIFIER) {
		processIdentifier();
	} else {
		throw std::runtime_error("Expected : type, got : '" +
								 tokenizer.currentTokenValue().value + "'");
	}
	closeTag("type");
}

void CompilationEngine::compileSubroutine() {}
void CompilationEngine::compileParameterList() {}
void CompilationEngine::compileSubroutineBody() {}
void CompilationEngine::compileVarDec() {}

void CompilationEngine::compileStatements() {}
void CompilationEngine::compileLet() {}
void CompilationEngine::compileIf() {}
void CompilationEngine::compileWhile() {}
void CompilationEngine::compileDo() {}
void CompilationEngine::compileReturn() {}

void CompilationEngine::compileExpression() {}
void CompilationEngine::compileTerm() {}
void CompilationEngine::compileExpressionList() {}