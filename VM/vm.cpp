#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

const string WHITESPACE = " \n\r\t\f\v";
string staticVarName = "";
const int TEMPADDRSTART = 5;
int symbolcounter = 0;
string currentFunction = "";

enum class ACTION {
	PUSH,	  // 3
	POP,	  // 3
	ADD,	  // 1
	SUB,	  // 1
	NEG,	  // 1
	EQ,		  // 1
	GT,		  // 1
	LT,		  // 1
	AND,	  // 1
	OR,		  // 1
	NOT,	  // 1
	LABEL,	  // 2
	GOTO,	  // 2
	IFGOTO,	  // 2
	CALL,	  // 3
	FUNCTION, // 3
	RETURN	  // 1
};

enum class SEGMENT {
	LOCAL,
	ARGUMENT,
	STATIC,
	CONSTANT,
	THIS,
	THAT,
	TEMP,
	POINTER
};

enum STATUS {
	SUCCESS,
	INPUT_FILE_NOT_PROVIDED,
	COULD_NOT_OPEN_INPUT_FILE,
	COULD_NOT_OPEN_OUTPUT_FILE,
	INVALID_INSTR,
	INVALID_ACTION,
	INVALID_SEGMENT,
	INVALID_ADDRESS,
	INVALID_OPERATION,
	INVALID_TEMP_INDEX,
	INVALID_POINTER_INDEX,
	INVALID_LABEL
};

// unordered_set<string> actions = {"push", "pop"};
unordered_map<string, ACTION> actions({
	{"push", ACTION::PUSH},
	{"pop", ACTION::POP},
	{"add", ACTION::ADD},
	{"sub", ACTION::SUB},
	{"neg", ACTION::NEG},
	{"eq", ACTION::EQ},
	{"gt", ACTION::GT},
	{"lt", ACTION::LT},
	{"and", ACTION::AND},
	{"or", ACTION::OR},
	{"not", ACTION::NOT},
	{"label", ACTION::LABEL},
	{"goto", ACTION::GOTO},
	{"if-goto", ACTION::IFGOTO},
	{"call", ACTION::CALL},
	{"function", ACTION::FUNCTION},
	{"return", ACTION::RETURN},
});
unordered_map<string, SEGMENT> segments({
	{"local", SEGMENT::LOCAL},
	{"argument", SEGMENT::ARGUMENT},
	{"static", SEGMENT::STATIC},
	{"constant", SEGMENT::CONSTANT},
	{"this", SEGMENT::THIS},
	{"that", SEGMENT::THAT},
	{"temp", SEGMENT::TEMP},
	{"pointer", SEGMENT::POINTER},
});

string segmentToString(SEGMENT segment) {
	switch (segment) {
	case SEGMENT::LOCAL:
		return "local";
	case SEGMENT::ARGUMENT:
		return "argument";
	case SEGMENT::STATIC:
		return "static";
	case SEGMENT::CONSTANT:
		return "constant";
	case SEGMENT::THIS:
		return "this";
	case SEGMENT::THAT:
		return "that";
	case SEGMENT::TEMP:
		return "temp";
	case SEGMENT::POINTER:
		return "pointer";
	}
	return "invalid";
}

string actionToString(ACTION action) {
	switch (action) {
	case ACTION::ADD:
		return "add";
	case ACTION::SUB:
		return "sub";
	case ACTION::AND:
		return "and";
	case ACTION::EQ:
		return "eq";
	case ACTION::GT:
		return "gt";
	case ACTION::LT:
		return "lt";
	case ACTION::NEG:
		return "neg";
	case ACTION::NOT:
		return "not";
	case ACTION::OR:
		return "or";
	case ACTION::POP:
		return "pop";
	case ACTION::PUSH:
		return "push";
	case ACTION::LABEL:
		return "label";
	case ACTION::GOTO:
		return "goto";
	case ACTION::IFGOTO:
		return "if-goto";
	case ACTION::CALL:
		return "call";
	case ACTION::FUNCTION:
		return "function";
	case ACTION::RETURN:
		return "return";
	}
	return "invalid";
}

string segmentBase(SEGMENT segment) {
	switch (segment) {
	case SEGMENT::LOCAL:
		return "LCL";
	case SEGMENT::ARGUMENT:
		return "ARG";
	case SEGMENT::THIS:
		return "THIS";
	case SEGMENT::THAT:
		return "THAT";
	default:
		return "";
	}
}

string compActionBase(ACTION action) {
	switch (action) {
	case ACTION::EQ:
		return "EQ";
	case ACTION::GT:
		return "GT";
	case ACTION::LT:
		return "LT";
	default:
		return "";
	}
}

struct Instruction {
	ACTION action;
	SEGMENT segment;
	string label;
	int index;
	int line;
};

struct SymbolPair {
	string truesymbol;
	string endsymbol;
};

// removes whitespaces from left
string ltrim(string str);
// removes whitespaces from right
string rtrim(string str);
// removes comment ;)
string removeComment(string str);
// uses all above functions in one
string trim(string str);
// report error
void reporterror(string filename, int linenum, string msg, string input);
// checks if label provided is valid
bool isValidLabel(const std::string &label);
// handles instruction generation
vector<string> handleInstruction(const Instruction &instruction);
// gets constructed comment for vm command
string getInstruction(const Instruction &ins);
// gets next true and end symbols according to the counter
SymbolPair getNextSymbols(ACTION action);

vector<string> getPushConstantAssembly(const Instruction &ins);
vector<string> getPopGenAssembly(const Instruction &ins);
vector<string> getPushGenAssembly(const Instruction &ins);
vector<string> getPopStaticAssembly(const Instruction &ins);
vector<string> getPushStaticAssembly(const Instruction &ins);
vector<string> getPopTempAssembly(const Instruction &ins);
vector<string> getPushTempAssembly(const Instruction &ins);
vector<string> getPopPointerAssembly(const Instruction &ins);
vector<string> getPushPointerAssembly(const Instruction &ins);
vector<string> getAddAssembly();
vector<string> getSubAssembly();
vector<string> getAndAssembly();
vector<string> getOrAssembly();
vector<string> getNegAssembly();
vector<string> getNotAssembly();
vector<string> getCompGenAssembly(const Instruction &ins);
vector<string> getLabelAssembly(const Instruction &ins);
vector<string> getGotoAssembly(const Instruction &ins);
vector<string> getIfGotoAssembly(const Instruction &ins);
vector<string> getFunctionAssembly(const Instruction &ins);
// get push SEGMENT assembly for LCL, ARG, THIS, THAT
vector<string> getGenPushSegAssembly(SEGMENT segment);
vector<string> getCallAssembly(const Instruction &ins);
vector<string> getReturnAssembly();
int main(int argc, char **argv) {
	// getting filename as an argument
	if (argc != 2) {
		cout << "Usage: ./vm <filename>\n";
		return STATUS::INPUT_FILE_NOT_PROVIDED;
	}
	string inputfilename = string(argv[1]);

	// opening file to read
	ifstream inputfile(inputfilename);
	if (!inputfile.is_open()) {
		cout << "Couldn't open file named : " << inputfilename << "\n";
		return STATUS::COULD_NOT_OPEN_INPUT_FILE;
	}

	size_t slash = inputfilename.find_last_of("/\\");
	size_t dot = inputfilename.find_last_of('.');

	string filename =
		inputfilename.substr(slash == string::npos ? 0 : slash + 1,
							 dot - (slash == string::npos ? 0 : slash + 1));

	staticVarName = filename;

	vector<Instruction> instructions;
	Instruction ins;
	int linecnt = 0;
	string line;
	string cleanedline;

	// input parsing and validation
	while (getline(inputfile, line)) {
		linecnt++;
		ins = {};

		cleanedline = trim(line);

		// Empty/comment-only line
		if (cleanedline.empty()) {
			continue;
		}

		vector<string> instruction;
		int instructionSize = 0;
		stringstream ss(cleanedline);
		string token;

		while (ss >> token) {
			instruction.push_back(token);
		}
		instructionSize = instruction.size();
		if (actions.find(instruction.at(0)) == actions.end()) {
			reporterror(inputfilename, linecnt, "Invalid instruction",
						cleanedline);
			return STATUS::INVALID_INSTR;
		}
		if (instructionSize > 3) {
			reporterror(inputfilename, linecnt,
						"Invalid instruction, too many arguments", cleanedline);
			return STATUS::INVALID_INSTR;
		}

		ACTION action = actions.at(instruction.at(0));
		ins.action = action;
		if (action == ACTION::PUSH || action == ACTION::POP) {
			if (instructionSize != 3) {
				reporterror(inputfilename, linecnt,
							actionToString(action) +
								" requires segment and index!!!\n( " +
								actionToString(action) + " segment i )",
							cleanedline);
				return STATUS::INVALID_INSTR;
			}
			bool is_all_digits =
				all_of(instruction.at(2).begin(), instruction.at(2).end(),
					   [](unsigned char c) { return std::isdigit(c); });
			if (!is_all_digits) {
				reporterror(inputfilename, linecnt,
							"Invalid memory address - (" + instruction.at(2) +
								")!!!",
							cleanedline);
				return STATUS::INVALID_ADDRESS;
			}
			int i = stoi(instruction.at(2));
			if (segments.find(instruction.at(1)) == segments.end()) {
				reporterror(inputfilename, linecnt,
							"Invalid segment!!!\nvalid segments : local, "
							"argument, static, "
							"constant, this, that, temp, pointer",
							cleanedline);
				return STATUS::INVALID_SEGMENT;
			}
			SEGMENT segment = segments.at(instruction[1]);
			if (action == ACTION::POP && segment == SEGMENT::CONSTANT) {
				reporterror(inputfilename, linecnt,
							"Instruction (pop constant) not allowed!!!",
							cleanedline);
				return STATUS::INVALID_INSTR;
			}
			if (segment == SEGMENT::TEMP && (i < 0 || i > 7)) {
				reporterror(inputfilename, linecnt,
							"Invalid index for TEMP segment!!!\nvalid index - "
							"0, 1, 2, 3, 4, 5, 6, 7",
							cleanedline);
				return STATUS::INVALID_TEMP_INDEX;
			}
			if (segment == SEGMENT::POINTER && i != 0 && i != 1) {
				reporterror(
					inputfilename, linecnt,
					"Invalid index for POINTER segment!!!\nvalid index - 0, 1",
					cleanedline);
				return STATUS::INVALID_POINTER_INDEX;
			}
			ins.segment = segment;
			ins.index = i;
		} else if (action == ACTION::FUNCTION || action == ACTION::CALL) {
			if (instructionSize != 3) {
				string errmsg = "";
				if(action == ACTION::FUNCTION){
					errmsg = actionToString(action)+" requies label and no. of variables(nVars)";
				} else if (action == ACTION::CALL){
					errmsg = actionToString(action)+" requies label and no. of arguments(nArgs)";
				}
				reporterror(inputfilename, linecnt, errmsg,
							cleanedline);
				return STATUS::INVALID_INSTR;
			}
			bool is_all_digits =
				all_of(instruction.at(2).begin(), instruction.at(2).end(),
					   [](unsigned char c) { return std::isdigit(c); });
			string funcname = instruction.at(1);
			if (!isValidLabel(funcname)) {
				reporterror(inputfilename, linecnt, "Invalid function name!!!",
							cleanedline);
				return STATUS::INVALID_LABEL;
			}
			if (action == ACTION::FUNCTION && !is_all_digits) {
				reporterror(inputfilename, linecnt,
							"invalid no. of variables(nVars) value",
							cleanedline);
				return STATUS::INVALID_ADDRESS;
			}
			if (action == ACTION::CALL && !is_all_digits) {
				reporterror(inputfilename, linecnt,
							"invalid no. of arguments(nArgs) value",
							cleanedline);
				return STATUS::INVALID_ADDRESS;
			}
			int i = stoi(instruction.at(2));
			ins.label = funcname;
			ins.index = i;
		}

		else if (action == ACTION::LABEL || action == ACTION::GOTO ||
				 action == ACTION::IFGOTO) {
			if (instructionSize != 2) {
				reporterror(inputfilename, linecnt,
							"branching commands require a label!!!",
							cleanedline);
				return STATUS::INVALID_INSTR;
			}
			string label = instruction.at(1);
			if (!isValidLabel(label)) {
				reporterror(inputfilename, linecnt, "Invalid function name!!!",
							cleanedline);
				return STATUS::INVALID_LABEL;
			}
			ins.label = label;
		}

		ins.line = linecnt;
		instructions.push_back(ins);
	}

	// for (auto &ins : instructions) {
	// 	cout << static_cast<int>(ins.action) << " ";
	// 	cout << static_cast<int>(ins.segment) << " ";
	// 	cout << ins.label << " ";
	// 	cout << static_cast<int>(ins.index) << " ";
	// 	cout << ins.line << "\n";
	// 	cout << "\n";
	// }
	vector<string> output;
	vector<string> tempout;
	string error;
	for (const auto &ins : instructions) {
		tempout = handleInstruction(ins);
		output.insert(output.end(), tempout.begin(), tempout.end());
	}

	string outputfilename = filename + ".asm";
	ofstream ofile(outputfilename);
	if (!ofile) {
		cerr << "Error opening file\n";
		return STATUS::COULD_NOT_OPEN_OUTPUT_FILE;
	}
	for (auto &out : output) {
		ofile << out << "\n";
	}
	ofile.close();

	return STATUS::SUCCESS;
}

string ltrim(string str) {
	size_t start = str.find_first_not_of(WHITESPACE);
	return (start == string::npos) ? "" : str.substr(start);
}

string rtrim(string str) {
	size_t end = str.find_last_not_of(WHITESPACE);
	return (end == string::npos) ? "" : str.substr(0, end + 1);
}

string trim(string str) { return rtrim(ltrim(removeComment(str))); }

string removeComment(string str) {
	size_t pos = str.find("//");
	return (pos == string::npos) ? str : str.substr(0, pos);
}

void reporterror(string filename, int linenum, string msg, string input) {
	cout << "ERROR: AT LINE " << linenum << " : " << filename << ":" << linenum
		 << "\n";
	cout << "[ " << input << " ]\n";
	cout << msg << "\n";
}

/**
 * Validates if a string is a valid VM label/Hack symbol.
 * Rules:
 *  - Must not be empty.
 *  - Valid chars: A-Z, a-z, 0-9, '.', '_', ':', '$'
 *  - Cannot start with a digit (0-9).
 */
bool isValidLabel(const std::string &label) {
	if (label.empty()) {
		return false;
	}

	// First character check: cannot be a digit
	char first = label[0];
	if (std::isdigit(static_cast<unsigned char>(first))) {
		return false;
	}

	// Character set check
	for (char ch : label) {
		bool isValidChar = std::isalnum(static_cast<unsigned char>(ch)) ||
						   ch == '.' || ch == '_' || ch == ':' || ch == '$';
		if (!isValidChar) {
			return false;
		}
	}

	return true;
}

vector<string> handleInstruction(const Instruction &ins) {
	string instrstr = getInstruction(ins);
	vector<string> res({"// " + instrstr});
	vector<string> temp;
	switch (ins.action) {
	case ACTION::PUSH:
		switch (ins.segment) {
		case SEGMENT::CONSTANT:
			temp = getPushConstantAssembly(ins);
			break;
		case SEGMENT::LOCAL:
		case SEGMENT::ARGUMENT:
		case SEGMENT::THIS:
		case SEGMENT::THAT:
			temp = getPushGenAssembly(ins);
			break;
		case SEGMENT::STATIC:
			temp = getPushStaticAssembly(ins);
			break;
		case SEGMENT::TEMP:
			temp = getPushTempAssembly(ins);
			break;
		case SEGMENT::POINTER:
			temp = getPushPointerAssembly(ins);
			break;
		}
		break;
	case ACTION::POP:
		switch (ins.segment) {
		case SEGMENT::CONSTANT:
			temp = {};
			break;
		case SEGMENT::LOCAL:
		case SEGMENT::ARGUMENT:
		case SEGMENT::THIS:
		case SEGMENT::THAT:
			temp = getPopGenAssembly(ins);
			break;
		case SEGMENT::STATIC:
			temp = getPopStaticAssembly(ins);
			break;
		case SEGMENT::TEMP:
			temp = getPopTempAssembly(ins);
			break;
		case SEGMENT::POINTER:
			temp = getPopPointerAssembly(ins);
			break;
		}
		break;
	case ACTION::ADD:
		temp = getAddAssembly();
		break;
	case ACTION::SUB:
		temp = getSubAssembly();
		break;
	case ACTION::AND:
		temp = getAndAssembly();
		break;
	case ACTION::OR:
		temp = getOrAssembly();
		break;
	case ACTION::NEG:
		temp = getNegAssembly();
		break;
	case ACTION::NOT:
		temp = getNotAssembly();
		break;
	case ACTION::EQ:
	case ACTION::GT:
	case ACTION::LT:
		temp = getCompGenAssembly(ins);
		break;
	case ACTION::LABEL:
		temp = getLabelAssembly(ins);
		break;
	case ACTION::GOTO:
		temp = getGotoAssembly(ins);
		break;
	case ACTION::IFGOTO:
		temp = getIfGotoAssembly(ins);
		break;
	case ACTION::FUNCTION:
		temp = getFunctionAssembly(ins);
		break;
	case ACTION::CALL:
		temp = getCallAssembly(ins);
		break;
	case ACTION::RETURN:
		temp = getReturnAssembly();
		break;
	}
	res.insert(res.end(), temp.begin(), temp.end());
	return res;
}

string getInstruction(const Instruction &ins) {
	if (ins.action == ACTION::PUSH || ins.action == ACTION::POP) {
		return actionToString(ins.action) + " " + segmentToString(ins.segment) +
			   " " + to_string(ins.index);
	} else if (ins.action == ACTION::LABEL || ins.action == ACTION::GOTO ||
			   ins.action == ACTION::IFGOTO) {
		return actionToString(ins.action) + " " + ins.label;
	} else if (ins.action == ACTION::FUNCTION || ins.action == ACTION::CALL) {
		return actionToString(ins.action) + " " + ins.label + " " +
			   to_string(ins.index);
	} else {
		return actionToString(ins.action);
	}
}

SymbolPair getNextSymbols(ACTION action) {
	string truesymbol = "__VM_" + staticVarName + "_" + compActionBase(action) +
						"_TRUE_" + to_string(symbolcounter);
	string endsymbol = "__VM_" + staticVarName + "_" + compActionBase(action) +
					   "_END_" + to_string(symbolcounter);
	symbolcounter++;
	return {truesymbol, endsymbol};
}

vector<string> getPushConstantAssembly(const Instruction &ins) {
	return {
		"@" + to_string(ins.index), "D=A", "@SP", "A=M", "M=D", "@SP", "M=M+1",
	};
}

vector<string> getPopGenAssembly(const Instruction &ins) {
	return {"@" + to_string(ins.index),
			"D=A",
			"@" + segmentBase(ins.segment),
			"D=D+M",
			"@R13",
			"M=D",

			"@SP",
			"AM=M-1",
			"D=M",

			"@R13",
			"A=M",
			"M=D"};
}

vector<string> getPushGenAssembly(const Instruction &ins) {
	return {"@" + to_string(ins.index),
			"D=A",
			"@" + segmentBase(ins.segment),
			"A=D+A",
			"D=M",

			"@SP",
			"A=M",
			"M=D",

			"@SP",
			"M=M+1"};
}

vector<string> getPopStaticAssembly(const Instruction &ins) {
	string staticvar = "__VM_" + staticVarName + "." + to_string(ins.index);
	return {"@SP", "AM=M-1", "D=M", "@" + staticvar, "M=D"};
}
vector<string> getPushStaticAssembly(const Instruction &ins) {
	string staticvar = "__VM_" + staticVarName + "." + to_string(ins.index);
	return {"@" + staticvar, "D=M", "@SP", "A=M", "M=D", "@SP", "M=M+1"};
}

vector<string> getPopTempAssembly(const Instruction &ins) {
	return {"@SP", "AM=M-1", "D=M", "@" + to_string(TEMPADDRSTART + ins.index),
			"M=D"};
}
vector<string> getPushTempAssembly(const Instruction &ins) {
	return {"@" + to_string(TEMPADDRSTART + ins.index),
			"D=M",
			"@SP",
			"A=M",
			"M=D",
			"@SP",
			"M=M+1"};
}

vector<string> getPopPointerAssembly(const Instruction &ins) {
	string choice = ins.index == 0	 ? segmentBase(SEGMENT::THIS)
					: ins.index == 1 ? segmentBase(SEGMENT::THAT)
									 : "";
	return {"@SP", "AM=M-1", "D=M", "@" + choice, "M=D"};
}
vector<string> getPushPointerAssembly(const Instruction &ins) {
	string choice = ins.index == 0	 ? segmentBase(SEGMENT::THIS)
					: ins.index == 1 ? segmentBase(SEGMENT::THAT)
									 : "";
	return {"@" + choice, "D=M", "@SP", "A=M", "M=D", "@SP", "M=M+1"};
}

vector<string> getAddAssembly() {
	return {"@SP", "AM=M-1", "D=M", "A=A-1", "M=D+M"};
}
vector<string> getSubAssembly() {
	return {"@SP", "AM=M-1", "D=M", "A=A-1", "M=M-D"};
}
vector<string> getAndAssembly() {
	return {"@SP", "AM=M-1", "D=M", "A=A-1", "M=D&M"};
}
vector<string> getOrAssembly() {
	return {"@SP", "AM=M-1", "D=M", "A=A-1", "M=D|M"};
}
vector<string> getNegAssembly() { return {"@SP", "A=M-1", "M=-M"}; }

vector<string> getNotAssembly() { return {"@SP", "A=M-1", "M=!M"}; }

vector<string> getCompGenAssembly(const Instruction &ins) {
	SymbolPair symbols = getNextSymbols(ins.action);
	return {
		"@SP",
		"AM=M-1",
		"D=M",
		"A=A-1",
		"D=M-D",
		"@" + symbols.truesymbol,
		"D;J" + compActionBase(ins.action),
		"@SP",
		"A=M-1",
		"M=0",
		"@" + symbols.endsymbol,
		"0;JMP",
		"(" + symbols.truesymbol + ")",
		"@SP",
		"A=M-1",
		"M=-1",
		"(" + symbols.endsymbol + ")",
	};
}

vector<string> getLabelAssembly(const Instruction &ins) {
	return {"(" + currentFunction + "$" + ins.label + ")"};
}

vector<string> getGotoAssembly(const Instruction &ins) {
	return {"@" + currentFunction + "$" + ins.label, "0;JMP"};
}
vector<string> getIfGotoAssembly(const Instruction &ins) {
	return {"@SP", "AM=M-1", "D=M", "@" + currentFunction + "$" + ins.label,
			"D;JNE"};
}

vector<string> getFunctionAssembly(const Instruction &ins) {
	vector<string> res;
	currentFunction = ins.label;
	res.push_back("(" + ins.label + ")");
	for (int i = 0; i < ins.index; i++) {
		res.push_back("@SP");
		res.push_back("A=M");
		res.push_back("M=0");
		res.push_back("@SP");
		res.push_back("M=M+1");
	}
	return res;
}

vector<string> getGenPushSegAssembly(SEGMENT segment) {
	return {
		"@" + segmentBase(segment), "D=M", "@SP", "A=M", "M=D", "@SP", "M=M+1"};
}

vector<string> getCallAssembly(const Instruction &ins) {
	vector<string> res;
	vector<string> temp;
	vector<SEGMENT> segments(
		{SEGMENT::LOCAL, SEGMENT::ARGUMENT, SEGMENT::THIS, SEGMENT::THAT});
	string returnLabel = "__VM_RETURN_" + to_string(symbolcounter);
	symbolcounter++;
	// push return address
	res.push_back("@" + returnLabel);
	res.push_back("D=A");
	res.push_back("@SP");
	res.push_back("A=M");
	res.push_back("M=D");
	res.push_back("@SP");
	res.push_back("M=M+1");
	for (auto &seg : segments) {
		temp = getGenPushSegAssembly(seg);
		res.insert(res.end(), temp.begin(), temp.end());
	}
	// ARG = SP - 5 - nArgs
	res.push_back("@SP");
	res.push_back("D=M");
	res.push_back("@5");
	res.push_back("D=D-A");
	res.push_back("@" + to_string(ins.index));
	res.push_back("D=D-A");
	res.push_back("@ARG");
	res.push_back("M=D");

	// LCL = SP
	res.push_back("@SP");
	res.push_back("D=M");
	res.push_back("@LCL");
	res.push_back("M=D");

	// goto function
	res.push_back("@" + ins.label);
	res.push_back("0;JMP");

	// return address label
	res.push_back("(" + returnLabel + ")");

	return res;
}

vector<string> getReturnAssembly() {
	return {// FRAME = LCL
			"@LCL", "D=M", "@R13", "M=D",

			// RET = *(FRAME - 5)
			"@5", "A=D-A", "D=M", "@R14", "M=D",

			// *ARG = pop()
			"@SP", "AM=M-1", "D=M", "@ARG", "A=M", "M=D",

			// SP = ARG + 1
			"@ARG", "D=M+1", "@SP", "M=D",

			// THAT = *(FRAME - 1)
			"@R13", "AM=M-1", "D=M", "@THAT", "M=D",

			// THIS = *(FRAME - 2)
			"@R13", "AM=M-1", "D=M", "@THIS", "M=D",

			// ARG = *(FRAME - 3)
			"@R13", "AM=M-1", "D=M", "@ARG", "M=D",

			// LCL = *(FRAME - 4)
			"@R13", "AM=M-1", "D=M", "@LCL", "M=D",

			// goto RET
			"@R14", "A=M", "0;JMP"};
}