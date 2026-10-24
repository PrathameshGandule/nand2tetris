#include "compengineVM.hpp"
#include <stdexcept>

CompilationEngine::CompilationEngine(JackTokenizer &tokenizer,
									 std::ostream &output)
	: tokenizer(tokenizer), writer(output) {}

std::string CompilationEngine::currentValue() const {
	return tokenizer.currentTokenValue().value;
}

void CompilationEngine::advance() { tokenizer.advance(); }

void CompilationEngine::consume(const std::string &expected) {
	if (currentValue() != expected) {
		throw std::runtime_error("Expected '" + expected + "', got '" +
								 currentValue() + "'");
	}
	advance();
}

void CompilationEngine::consumeIdentifier() {
	Token token = tokenizer.currentTokenValue();
	if (token.type != TOKENTYPE::IDENTIFIER) {
		throw std::runtime_error("Expected identifier, got '" + token.value +
								 "'");
	}
	advance();
}

std::string CompilationEngine::compileType() {
	Token token = tokenizer.currentTokenValue();
	if (token.value == "int" || token.value == "char" ||
		token.value == "boolean") {
		advance();
		return token.value;
	}
	if (token.type == TOKENTYPE::IDENTIFIER) {
		advance();
		return token.value;
	}
	throw std::runtime_error("Expected type, got '" + token.value + "'");
}

std::string CompilationEngine::newLabel(const std::string &prefix) {
	return prefix + std::to_string(labelCounter++);
}

Symbol CompilationEngine::requireSymbol(const std::string &name) const {
	Symbol symbol;
	if (!symbols.lookup(name, symbol)) {
		throw std::runtime_error("Undefined variable: '" + name + "'");
	}
	return symbol;
}

std::string CompilationEngine::vmSegment(const std::string &kind) const {
	if (kind == "field")
		return "this";
	if (kind == "var" || kind == "local")
		return "local";
	if (kind == "static")
		return "static";
	if (kind == "argument")
		return "argument";
	throw std::runtime_error("Cannot map symbol kind to VM segment: '" + kind +
							 "'");
}

void CompilationEngine::pushSymbol(const Symbol &symbol) {
	writer.writePush(vmSegment(symbol.kind), symbol.index);
}

void CompilationEngine::popSymbol(const Symbol &symbol) {
	writer.writePop(vmSegment(symbol.kind), symbol.index);
}

bool CompilationEngine::isOperator(const std::string &value) const {
	return value == "+" || value == "-" || value == "*" || value == "/" ||
		   value == "&" || value == "|" || value == "<" || value == ">" ||
		   value == "=";
}

void CompilationEngine::writeOperator(const std::string &op) {
	if (op == "+")
		writer.writeArithmetic("add");
	else if (op == "-")
		writer.writeArithmetic("sub");
	else if (op == "&")
		writer.writeArithmetic("and");
	else if (op == "|")
		writer.writeArithmetic("or");
	else if (op == "<")
		writer.writeArithmetic("lt");
	else if (op == ">")
		writer.writeArithmetic("gt");
	else if (op == "=")
		writer.writeArithmetic("eq");
	else if (op == "*")
		writer.writeCall("Math.multiply", 2);
	else if (op == "/")
		writer.writeCall("Math.divide", 2);
	else
		throw std::runtime_error("Unsupported operator: '" + op + "'");
}

void CompilationEngine::compileClass() {
	symbols.startClass();
	consume("class");
	Token classToken = tokenizer.currentTokenValue();
	if (classToken.type != TOKENTYPE::IDENTIFIER)
		throw std::runtime_error("Expected class name");
	currentClassName = classToken.value;
	consumeIdentifier();
	consume("{");

	while (currentValue() == "static" || currentValue() == "field")
		compileClassVarDec();
	while (currentValue() == "constructor" || currentValue() == "function" ||
		   currentValue() == "method") {
		compileSubroutineDec();
	}
	consume("}");
}

void CompilationEngine::compileClassVarDec() {
	std::string kind = currentValue();
	if (kind != "static" && kind != "field")
		throw std::runtime_error("Expected static or field");
	advance();
	std::string type = compileType();
	while (true) {
		Token name = tokenizer.currentTokenValue();
		if (name.type != TOKENTYPE::IDENTIFIER)
			throw std::runtime_error("Expected class variable name");
		symbols.define(name.value, type, kind);
		consumeIdentifier();
		if (currentValue() != ",")
			break;
		consume(",");
	}
	consume(";");
}

void CompilationEngine::compileSubroutineDec() {
	currentSubroutineKind = currentValue();
	if (currentSubroutineKind != "constructor" &&
		currentSubroutineKind != "function" &&
		currentSubroutineKind != "method") {
		throw std::runtime_error("Expected constructor, function, or method");
	}
	advance();
	if (currentValue() == "void")
		advance();
	else
		(void)compileType();

	Token name = tokenizer.currentTokenValue();
	if (name.type != TOKENTYPE::IDENTIFIER)
		throw std::runtime_error("Expected subroutine name");
	currentSubroutineName = name.value;
	consumeIdentifier();

	symbols.startSubroutine();
	if (currentSubroutineKind == "method") {
		symbols.define("this", currentClassName, "argument");
	}

	consume("(");
	compileParameterList();
	consume(")");
	compileSubroutineBody();
}

void CompilationEngine::compileParameterList() {
	if (currentValue() == ")")
		return;
	while (true) {
		std::string type = compileType();
		Token name = tokenizer.currentTokenValue();
		if (name.type != TOKENTYPE::IDENTIFIER)
			throw std::runtime_error("Expected parameter name");
		symbols.define(name.value, type, "argument");
		consumeIdentifier();
		if (currentValue() != ",")
			break;
		consume(",");
	}
}

void CompilationEngine::compileSubroutineBody() {
	consume("{");
	while (currentValue() == "var")
		compileVarDec();

	writer.writeFunction(currentClassName + "." + currentSubroutineName,
						 symbols.varCount("var"));
	if (currentSubroutineKind == "method") {
		writer.writePush("argument", 0);
		writer.writePop("pointer", 0);
	} else if (currentSubroutineKind == "constructor") {
		writer.writePush("constant", symbols.varCount("field"));
		writer.writeCall("Memory.alloc", 1);
		writer.writePop("pointer", 0);
	}

	compileStatements();
	consume("}");
}

void CompilationEngine::compileVarDec() {
	consume("var");
	std::string type = compileType();
	while (true) {
		Token name = tokenizer.currentTokenValue();
		if (name.type != TOKENTYPE::IDENTIFIER)
			throw std::runtime_error("Expected local variable name");
		symbols.define(name.value, type, "var");
		consumeIdentifier();
		if (currentValue() != ",")
			break;
		consume(",");
	}
	consume(";");
}

void CompilationEngine::compileStatements() {
	while (currentValue() == "let" || currentValue() == "if" ||
		   currentValue() == "while" || currentValue() == "do" ||
		   currentValue() == "return") {
		compileStatement();
	}
}

void CompilationEngine::compileStatement() {
	if (currentValue() == "let")
		compileLetStatement();
	else if (currentValue() == "if")
		compileIfStatement();
	else if (currentValue() == "while")
		compileWhileStatement();
	else if (currentValue() == "do")
		compileDoStatement();
	else if (currentValue() == "return")
		compileReturnStatement();
	else
		throw std::runtime_error("Unknown statement: '" + currentValue() + "'");
}

void CompilationEngine::compileLetStatement() {
	consume("let");
	Token name = tokenizer.currentTokenValue();
	if (name.type != TOKENTYPE::IDENTIFIER)
		throw std::runtime_error("Expected variable name after let");
	Symbol target = requireSymbol(name.value);
	consumeIdentifier();
	const bool isArray = currentValue() == "[";
	if (isArray) {
		consume("[");
		pushSymbol(target);
		compileExpression();
		consume("]");
		writer.writeArithmetic("add");
	}
	consume("=");
	compileExpression();
	consume(";");
	if (isArray) {
		writer.writePop("temp", 0);
		writer.writePop("pointer", 1);
		writer.writePush("temp", 0);
		writer.writePop("that", 0);
	} else {
		popSymbol(target);
	}
}

void CompilationEngine::compileIfStatement() {
	std::string trueLabel = newLabel("IF_TRUE");
	std::string falseLabel = newLabel("IF_FALSE");
	std::string endLabel = newLabel("IF_END");
	consume("if");
	consume("(");
	compileExpression();
	consume(")");
	writer.writeIf(trueLabel);
	writer.writeGoto(falseLabel);
	writer.writeLabel(trueLabel);
	consume("{");
	compileStatements();
	consume("}");
	if (currentValue() == "else") {
		writer.writeGoto(endLabel);
		writer.writeLabel(falseLabel);
		consume("else");
		consume("{");
		compileStatements();
		consume("}");
		writer.writeLabel(endLabel);
	} else {
		writer.writeLabel(falseLabel);
	}
}

void CompilationEngine::compileWhileStatement() {
	std::string expLabel = newLabel("WHILE_EXP");
	std::string endLabel = newLabel("WHILE_END");
	writer.writeLabel(expLabel);
	consume("while");
	consume("(");
	compileExpression();
	consume(")");
	writer.writeArithmetic("not");
	writer.writeIf(endLabel);
	consume("{");
	compileStatements();
	consume("}");
	writer.writeGoto(expLabel);
	writer.writeLabel(endLabel);
}

void CompilationEngine::compileDoStatement() {
	consume("do");
	compileSubroutineCall();
	consume(";");
	writer.writePop("temp", 0); // discard the return value of a do-statement
}

void CompilationEngine::compileReturnStatement() {
	consume("return");
	if (currentValue() == ";")
		writer.writePush("constant", 0);
	else
		compileExpression();
	consume(";");
	writer.writeReturn();
}

void CompilationEngine::compileExpression() {
	compileTerm();
	while (isOperator(currentValue())) {
		std::string op = currentValue();
		advance();
		compileTerm();
		writeOperator(op);
	}
}

void CompilationEngine::compileTerm() {
	Token token = tokenizer.currentTokenValue();
	if (token.type == TOKENTYPE::INTCONST) {
		writer.writePush("constant", std::stoi(token.value));
		advance();
		return;
	}
	if (token.type == TOKENTYPE::STRINGCONST) {
		std::string value = token.value;
		advance();
		// JackTokenizer normally stores string constants without the
		// surrounding quotes.
		writer.writePush("constant", static_cast<int>(value.size()));
		writer.writeCall("String.new", 1);
		for (unsigned char ch : value) {
			writer.writePush("constant", static_cast<int>(ch));
			writer.writeCall("String.appendChar", 2);
		}
		return;
	}
	if (token.value == "true") {
		writer.writePush("constant", 0);
		writer.writeArithmetic("not");
		advance();
		return;
	}
	if (token.value == "false" || token.value == "null") {
		writer.writePush("constant", 0);
		advance();
		return;
	}
	if (token.value == "this") {
		if (currentSubroutineKind == "function")
			throw std::runtime_error("'this' cannot be used in a function");
		writer.writePush("pointer", 0);
		advance();
		return;
	}
	if (token.value == "-" || token.value == "~") {
		std::string op = token.value;
		advance();
		compileTerm();
		writer.writeArithmetic(op == "-" ? "neg" : "not");
		return;
	}
	if (token.value == "(") {
		consume("(");
		compileExpression();
		consume(")");
		return;
	}
	if (token.type == TOKENTYPE::IDENTIFIER) {
		Token next = tokenizer.peek();
		if (next.value == "[") {
			Symbol array = requireSymbol(token.value);
			consumeIdentifier();
			consume("[");
			pushSymbol(array);
			compileExpression();
			consume("]");
			writer.writeArithmetic("add");
			writer.writePop("pointer", 1);
			writer.writePush("that", 0);
		} else if (next.value == "(" || next.value == ".") {
			compileSubroutineCall();
		} else {
			Symbol symbol = requireSymbol(token.value);
			pushSymbol(symbol);
			consumeIdentifier();
		}
		return;
	}
	throw std::runtime_error("Invalid term: '" + token.value + "'");
}

void CompilationEngine::compileSubroutineCall() {
	Token first = tokenizer.currentTokenValue();
	if (first.type != TOKENTYPE::IDENTIFIER)
		throw std::runtime_error("Expected subroutine or class/variable name");
	std::string firstName = first.value;
	consumeIdentifier();

	std::string callName;
	int implicitArgs = 0;
	if (currentValue() == "(") {
		// foo(...) means this.foo(...), so the current object is argument 0.
		if (currentSubroutineKind == "function") {
			throw std::runtime_error("Unqualified method call from a function "
									 "has no current object");
		}
		writer.writePush("pointer", 0);
		callName = currentClassName + "." + firstName;
		implicitArgs = 1;
		consume("(");
	} else if (currentValue() == ".") {
		consume(".");
		Token subName = tokenizer.currentTokenValue();
		if (subName.type != TOKENTYPE::IDENTIFIER)
			throw std::runtime_error("Expected subroutine name after '.'");
		std::string methodName = subName.value;
		consumeIdentifier();
		Symbol receiver;
		if (symbols.lookup(firstName, receiver)) {
			pushSymbol(receiver);
			callName = receiver.type + "." + methodName;
			implicitArgs = 1;
		} else {
			callName = firstName + "." + methodName;
		}
		consume("(");
	} else {
		throw std::runtime_error("Expected '(' or '.' in subroutine call");
	}
	int explicitArgs = compileExpressionList();
	consume(")");
	writer.writeCall(callName, explicitArgs + implicitArgs);
}

int CompilationEngine::compileExpressionList() {
	int count = 0;
	if (currentValue() == ")")
		return count;
	while (true) {
		compileExpression();
		++count;
		if (currentValue() != ",")
			break;
		consume(",");
	}
	return count;
}
