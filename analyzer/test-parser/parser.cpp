#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

enum class TokenType {
	Print,
	LParen,
	RParen,
	Number,
	Plus,
	Semicolon,
	EOFToken
};

struct Token {
	TokenType type;
	string value;
};

vector<Token> tokens({
	{TokenType::Print, "print"},
	{TokenType::LParen, "("},
	{TokenType::Number, "10"},
	{TokenType::Plus, "+"},
	{TokenType::Number, "20"},
	{TokenType::RParen, ")"},
	{TokenType::Plus, "+"},
	{TokenType::Number, "30"},
	{TokenType::Semicolon, ";"},
	{TokenType::EOFToken, ""},
});

int currentToken = 0;
int indent = 0;

// -------------------------------------------------- //
//                Utility functions                   //
// -------------------------------------------------- //

Token peek() { return tokens[currentToken]; }

Token advance() { return tokens[currentToken++]; }

bool match(TokenType type) { return peek().type == type; }

void expect(TokenType type) {
	if (!match(type)) {
		throw runtime_error("Unexpected token: " + peek().value);
	}

	advance();
}

void printIndent() {
	for (int i = 0; i < indent; i++) {
		cout << "  ";
	}
}

void openTag(const string &tag) {
	printIndent();
	cout << "<" << tag << ">\n";
	indent++;
}

void closeTag(const string &tag) {
	indent--;
	printIndent();
	cout << "</" << tag << ">\n";
}

void printToken(const string &tag, const string &value) {
	printIndent();
	cout << "<" << tag << "> " << value << " </" << tag << ">\n";
}

// --------------------------------------------------
// Parser
// --------------------------------------------------

void parseProgram();
void parseStatement();
void parseExpression();
void parsePrimary();

// program → statement*
void parseProgram() {

	openTag("program");

	while (!match(TokenType::EOFToken)) {
		parseStatement();
	}

	closeTag("program");
}

// statement → "print" expression ";"
void parseStatement() {

	openTag("statement");

	expect(TokenType::Print);
	printToken("keyword", "print");

	parseExpression();

	expect(TokenType::Semicolon);
	printToken("symbol", ";");

	closeTag("statement");
}

// expression → primary ("+" primary)*
void parseExpression() {

	openTag("expression");

	parsePrimary();

	while (match(TokenType::Plus)) {

		advance();
		printToken("symbol", "+");

		parsePrimary();
	}

	closeTag("expression");
}

// primary → NUMBER | "(" expression ")"
void parsePrimary() {

	openTag("term");

	if (match(TokenType::Number)) {

		Token token = advance();

		printToken("integerConstant", token.value);

		closeTag("term");
		return;
	}

	if (match(TokenType::LParen)) {

		advance();
		printToken("symbol", "(");

		parseExpression();

		expect(TokenType::RParen);
		printToken("symbol", ")");

		closeTag("term");
		return;
	}

	throw runtime_error("Expected number or '(' but got: " + peek().value);
}

// --------------------------------------------------
// Main
// --------------------------------------------------

int main() {

	try {
		parseProgram();

		if (!match(TokenType::EOFToken)) {
			throw runtime_error("Expected EOF");
		}
	} catch (const exception &e) {
		cerr << "Parser error: " << e.what() << '\n';
		return 1;
	}

	return 0;
}