#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_set>
#include <vector>

using namespace std;
namespace fs = filesystem;
const string WHITESPACE = " \n\r\t\f\v";

// return status enum
enum STATUS {
	INPUT_NOT_PROVIDED,
	INVALID_INPUT,
	INVALID_FILE_TYPE,
	EMPTY_INPUT_DIRECTORY,
	COULD_NOT_OPEN_INPUT_FILE,
	COULD_NOT_OPEN_OUTPUT_FILE
};

enum class TOKENTYPE { KEYWORD, SYMBOL, INTCONST, STRINGCONST, IDENTIFIER };

string tokentypeToString(TOKENTYPE t) {
	switch (t) {
	case TOKENTYPE::KEYWORD:
		return "keyword";
	case TOKENTYPE::SYMBOL:
		return "symbol";
	case TOKENTYPE::INTCONST:
		return "intConst";
	case TOKENTYPE::STRINGCONST:
		return "stringConst";
	case TOKENTYPE::IDENTIFIER:
		return "identifier";
	default:
		return "invalidtoken";
	}
}

const unordered_set<string> keywords{
	"class", "constructor", "function", "method", "field", "static", "var",
	"int",	 "char",		"boolean",	"void",	  "true",  "false",	 "null",
	"this",	 "let",			"do",		"if",	  "else",  "while",	 "return",
};

const unordered_set<char> symbols{
	'{', '}', '(', ')', '[', ']', '.', ',', ';', '+',
	'-', '*', '/', '&', '|', '<', '>', '=', '~',
};

struct Token {
	TOKENTYPE type;
	string value;
};

class JackTokenizer {
  private:
	vector<Token> tokens;
	fs::path inputfilename;
	string logic;
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
	string tokenToXML(Token token);
};

JackTokenizer::JackTokenizer(fs::path filepath) {
	inputfilename = filepath;
	ifstream ifile(filepath);
	if (!ifile) {
		throw runtime_error("error opening file: " + filepath.string());
	}
	cout << "Input file : " << filepath << "\n";
	logic = string{(std::istreambuf_iterator<char>(ifile)),
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
			cout << "invalid char\n";
			pos++;
		}
	}
}

void JackTokenizer::writeXML() {
	outputfilename = inputfilename;
	outputfilename.replace_extension(".xml");
	cout << "Output file : " << outputfilename << "\n";
	ofstream ofile(outputfilename);
	if (!ofile) {
		throw runtime_error("error opening output file: " +
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
        throw runtime_error("No more tokens");

    return tokens[currentToken++];
}

Token JackTokenizer::peek() {
    if (!hasMoreTokens())
        throw runtime_error("No more tokens");

    return tokens[currentToken];
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
	string val = logic.substr(start + 1, pos - start - 1);
	tokens.push_back({TOKENTYPE::STRINGCONST, val});
	pos++;
}

bool JackTokenizer::isIntegerConstant() { return isdigit(logic[pos]); }
void JackTokenizer::handleIntegerConstant() {
	size_t start = pos;
	pos++;
	while (pos < srclen && isdigit(static_cast<unsigned char>(logic[pos])))
		pos++;
	string val = logic.substr(start, pos - start);
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
	string val = logic.substr(start, pos - start);
	if (keywords.count(val)) {
		tokens.push_back({TOKENTYPE::KEYWORD, val});
	} else {
		tokens.push_back({TOKENTYPE::IDENTIFIER, val});
	}
}

void JackTokenizer::printTokens() {
	for (auto const &t : tokens) {
		cout << tokentypeToString(t.type) << " :\t( " << t.value << " )\n";
	}
}

string JackTokenizer::tokenToXML(Token token) {
	string tokenStr = tokentypeToString(token.type);
	string tokenVal = token.value;
	if (token.value == "<")
		tokenVal = "&lt;";
	else if (token.value == ">")
		tokenVal = "&gt;";
	else if (token.value == "&")
		tokenVal = "&amp;";
	return "<" + tokenStr + "> " + tokenVal + " </" + tokenStr + ">";
}

int main(int argc, char **argv) {

	// checking for arguments provided
	if (argc != 2) {
		cout << "Usage : analyzer <file.jack | directory>\n";
		return STATUS::INPUT_NOT_PROVIDED;
	}

	// variable declarations
	fs::path cmdinput = argv[1];

	fs::path canoninp = fs::canonical(cmdinput);
	vector<fs::path> inputfileslist;
	fs::path outputfilename = "";
	vector<string> output({"<name>prathamesh</name>", "<name>diksha</name>"});

	if (!fs::exists(cmdinput)) {
		cerr << "Error: file/directory not found: " << cmdinput << '\n';
		return STATUS::INVALID_INPUT;
	}
	/* Here we're trying to build the inputfileslist
	 * even if we have only 1 file as an input or directory
	 * we store them in a vector to generalize the output file creation
	 * - output .xml file is created in same place as input .jack file
	 * - if directory is passed as an input it scans for .jack files and creates
	 * .xml file per .jack file
	 */
	if (fs::is_regular_file(canoninp)) {
		if (canoninp.extension() != ".jack") {
			cerr << "Error: input file must have .jack extension\n";
			return STATUS::INVALID_FILE_TYPE;
		}
		inputfileslist.push_back(canoninp);
	} else if (fs::is_directory(canoninp)) {
		for (const auto &entry : fs::directory_iterator(canoninp)) {
			if (entry.is_regular_file() &&
				entry.path().extension() == ".jack") {
				inputfileslist.push_back(fs::canonical(entry.path()));
			}
		}

		// No jack files found
		if (inputfileslist.empty()) {
			cerr << "Error: directory contains no .jack files\n";
			return STATUS::EMPTY_INPUT_DIRECTORY;
		}
	} else {
		cerr << "Error: input is neither a regular file "
				"nor a directory\n";
		return STATUS::INVALID_FILE_TYPE;
	}

	// main code start iterate per input file
	for (auto const &filepath : inputfileslist) {
		JackTokenizer tokenizer(filepath);
		tokenizer.tokenize();
		tokenizer.writeXML();
	}

	return 0;
}
