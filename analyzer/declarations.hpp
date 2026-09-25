#ifndef DECLARATIONS
#define DECLARATIONS

#include <string>
#include <unordered_set>

const std::string WHITESPACE = " \n\r\t\f\v";

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

const std::unordered_set<std::string> keywords{
	"class", "constructor", "function", "method", "field", "static", "var",
	"int",	 "char",		"boolean",	"void",	  "true",  "false",	 "null",
	"this",	 "let",			"do",		"if",	  "else",  "while",	 "return",
};

const std::unordered_set<char> symbols{
	'{', '}', '(', ')', '[', ']', '.', ',', ';', '+',
	'-', '*', '/', '&', '|', '<', '>', '=', '~',
};

struct Token {
	TOKENTYPE type;
	std::string value;
};

inline std::string tokentypeToString(TOKENTYPE t) {
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

#endif /* DECLARATIONS */