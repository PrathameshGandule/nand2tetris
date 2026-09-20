#include "declarations.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class JackTokenizer {
  private:
	std::vector<Token> tokens;
	fs::path inputfilename;
	std::string logic;
	size_t pos = 0;
	size_t srclen = 0;
	size_t currentToken = 0;
	fs::path outputfilename;

  public:
	JackTokenizer(fs::path filepath);
	void tokenize();
	void writeXML();

	bool hasMoreTokens();
	Token advance();
	Token peek();

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
	std::string tokenToXML(Token token);
};