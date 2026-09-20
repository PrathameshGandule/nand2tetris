#include "tokenizer.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>

JackTokenizer::JackTokenizer(fs::path filepath) {
	inputfilename = filepath;
	std::ifstream ifile(filepath);
	if (!ifile) {
		throw std::runtime_error("error opening file: " + filepath.string());
	}
	std::cout << "Input file : " << filepath << "\n";
	logic = std::string{(std::istreambuf_iterator<char>(ifile)),
				   std::istreambuf_iterator<char>()};
	ifile.close();
	pos = 0;
	srclen = logic.length();
}

void JackTokenizer::tokenize() {
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
			std::cout << "invalid char\n";
			pos++;
		}
	}
}

void JackTokenizer::writeXML() {
	outputfilename = inputfilename;
	outputfilename.replace_extension(".xml");
	std::cout << "Output file : " << outputfilename << "\n";
	std::ofstream ofile(outputfilename);
	if (!ofile) {
		throw std::runtime_error("error opening output file: " +
							outputfilename.string());
	}

	// write generated output to the output file
	for (auto const &t : tokens) {
		ofile << tokenToXML(t) << "\n";
	}
	ofile.close();
}

bool JackTokenizer::hasMoreTokens() {
    return currentToken < tokens.size();
}

Token JackTokenizer::advance() {
    if (!hasMoreTokens())
        throw std::runtime_error("No more tokens");

    return tokens[currentToken++];
}

Token JackTokenizer::peek() {
    if (!hasMoreTokens())
        throw std::runtime_error("No more tokens");

    return tokens[currentToken];
}

bool JackTokenizer::isWhitespace() {
	return WHITESPACE.find(logic[pos]) != std::string::npos;
}
void JackTokenizer::handleWhitespce() {
	while (pos < srclen && WHITESPACE.find(logic[pos]) != std::string::npos)
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
	tokens.push_back({TOKENTYPE::SYMBOL, {logic[pos]}});
	pos++;
}

bool JackTokenizer::isStringConstant() { return logic[pos] == '"'; }
void JackTokenizer::handleStringConstant() {
	size_t start = pos;
	pos++;
	while (pos < srclen && logic[pos] != '"') {
		pos++;
	}
	std::string val = logic.substr(start + 1, pos - start - 1);
	tokens.push_back({TOKENTYPE::STRINGCONST, val});
	pos++;
}

bool JackTokenizer::isIntegerConstant() { return isdigit(logic[pos]); }
void JackTokenizer::handleIntegerConstant() {
	size_t start = pos;
	pos++;
	while (pos < srclen && isdigit(static_cast<unsigned char>(logic[pos])))
		pos++;
	std::string val = logic.substr(start, pos - start);
	int intval = stoi(val);
	if(intval < 0 || intval > 32767){
		throw std::runtime_error("integer constant out of range!!! : [ "+val+" ]");
	}
	tokens.push_back({TOKENTYPE::INTCONST, val});
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
	std::string val = logic.substr(start, pos - start);
	if (keywords.count(val)) {
		tokens.push_back({TOKENTYPE::KEYWORD, val});
	} else {
		tokens.push_back({TOKENTYPE::IDENTIFIER, val});
	}
}

void JackTokenizer::printTokens() {
	for (auto const &t : tokens) {
		std::cout << tokentypeToString(t.type) << " :\t( " << t.value << " )\n";
	}
}

std::string JackTokenizer::tokenToXML(Token token) {
	std::string tokenStr = tokentypeToString(token.type);
	std::string tokenVal = token.value;
	if (token.value == "<")
		tokenVal = "&lt;";
	else if (token.value == ">")
		tokenVal = "&gt;";
	else if (token.value == "&")
		tokenVal = "&amp;";
	return "<" + tokenStr + "> " + tokenVal + " </" + tokenStr + ">";
}