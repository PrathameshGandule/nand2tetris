#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <vector>
using namespace std;
namespace fs = filesystem;

const string WHITESPACE = " \n\r\t\f\v";

enum class TOKENTYPE { KEYWORD, SYMBOL, IDENTIFIER, INTCONST, STRCONST };

struct Token {
	string type;
	string value;
};

const unordered_set<string> keywords{
	"class", "constructor", "function", "method", "field", "static", "var",
	"int",	 "char",		"boolean",	"void",	  "true",  "false",	 "null",
	"this",	 "let",			"do",		"if",	  "else",  "while",	 "return",
};

const unordered_set<char> symbols{
	'{', '}', '(', ')', '[', ']', '.', ',', ';', '+',
	'-', '*', '/', '&', '|', '<', '>', '=', '~',
};

class JackTokenizer {
  private:
	vector<Token> tokens;
	string logic;
	size_t pos = 0;
	size_t srclen = 0;

  public:
	JackTokenizer(fs::path filepath);

	bool isWhitespace();
	bool isSingleLineComment();
	bool isMultiLineComment();
	bool isSymbol();
	bool isStringConstant();
	bool isIntegerConstant();
	bool isIdentifierStart();
	bool isIdentifierChar();

	void handleWhitespce();
	void handleSingleLineComment();
	void handleMultiLineComment();
	void handleSymbol();
	void handleStringConstant();
	void handleIntegerConstant();
	void handleKeywordsAndIdentifiers();

	void printTokens();
};

JackTokenizer::JackTokenizer(fs::path filepath) {
	ifstream ifile(filepath);
	logic = string{(std::istreambuf_iterator<char>(ifile)),
				   std::istreambuf_iterator<char>()};
	pos = 0;
	srclen = logic.length();
	while (pos < srclen) {
		// whitespcaes
		if (isWhitespace()) {
			handleWhitespce();
			// single line comments
		} else if (isSingleLineComment()) {
			handleSingleLineComment();
			// multiline comments
		} else if (isMultiLineComment()) {
			handleMultiLineComment();
			// symbols
		} else if (isSymbol()) {
			handleSymbol();
			// string constants
		} else if (isStringConstant()) {
			handleStringConstant();
			// digits
		} else if (isIntegerConstant()) {
			handleIntegerConstant();
			// keywords and identifiers
		} else if (isIdentifierStart()) {
			handleKeywordsAndIdentifiers();
		} else {
			cout << "invalid char\n";
			pos++;
		}
	}
}

bool JackTokenizer::isWhitespace() {
	return WHITESPACE.find(logic[pos]) != string::npos;
}
void JackTokenizer::handleWhitespce() {
	while (pos < srclen && WHITESPACE.find(logic[pos]) != string::npos)
		pos++;
}

bool JackTokenizer::isSingleLineComment() {
	return pos + 1 < srclen && logic[pos] == '/' && logic[pos + 1] == '/';
}
void JackTokenizer::handleSingleLineComment() {
	while (pos < srclen && logic[pos] != '\n')
		pos++;
}

bool JackTokenizer::isMultiLineComment() {
	return pos + 1 < srclen && logic[pos] == '/' && logic[pos + 1] == '*';
}
void JackTokenizer::handleMultiLineComment() {
	pos += 2; // skip /*
	while (pos + 1 < srclen && !(logic[pos] == '*' && logic[pos + 1] == '/')) {
		pos++;
	}
	if (pos + 1 < srclen)
		pos += 2; // skip */
}

bool JackTokenizer::isSymbol() { return symbols.count(logic[pos]); }
void JackTokenizer::handleSymbol() {
	tokens.push_back({"symbol", {logic[pos]}});
	pos++;
}

bool JackTokenizer::isStringConstant() { return logic[pos] == '"'; }
void JackTokenizer::handleStringConstant() {
	size_t start = pos;
	pos++;
	while (pos < srclen && logic[pos] != '"' && logic[pos] != '\n')
		pos++;
	string val = logic.substr(start + 1, pos - start - 1);
	tokens.push_back({"strconst", val});
	pos++;
}

bool JackTokenizer::isIntegerConstant() { return isdigit(logic[pos]); }
void JackTokenizer::handleIntegerConstant() {
	size_t start = pos;
	pos++;
	while (pos < srclen && isdigit(static_cast<unsigned char>(logic[pos])))
		pos++;
	string val = logic.substr(start, pos - start);
	tokens.push_back({"intconst", val});
}

bool JackTokenizer::isIdentifierStart() {
	return isalpha(logic[pos]) || logic[pos] == '_';
}
bool JackTokenizer::isIdentifierChar() {
	return isalnum(static_cast<unsigned char>(logic[pos])) || logic[pos] == '_';
}
void JackTokenizer::handleKeywordsAndIdentifiers() {
	size_t start = pos;
	while (pos < srclen && isIdentifierChar())
		pos++;
	string val = logic.substr(start, pos - start);
	if (keywords.count(val)) {
		tokens.push_back({"keyword", val});
	} else {
		tokens.push_back({"identifier", val});
	}
}

void JackTokenizer::printTokens() {
	for (auto const &t : tokens) {
		cout << t.type << " :\t( " << t.value << " )\n";
	}
}

int main() {
	// cout << "testing parsing logic\n";
	// string source = "     let x = 10 ;\nlet a = \"prathamesh\"";
	// ifstream ifile("3test.txt");
	// std::string logic((std::istreambuf_iterator<char>(ifile)),
	// 				  std::istreambuf_iterator<char>());
	// cout << "|" << logic << "|\n";
	// size_t pos = 0;
	// size_t srclen = logic.length();
	// cout << srclen << endl;

	JackTokenizer jacktokenizer("3test.txt");
	jacktokenizer.printTokens();

	// cout<<"|\n";

	return 0;
}
