#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

const string WHITESPACE = " \n\r\t\f\v";
string staticVarName = "";
const int TEMPADDRSTART = 5;
int labelcounter = 0;

enum class ACTION { PUSH, POP, ADD, SUB, NEG, EQ, GT, LT, AND, OR, NOT };

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
	int index;
	int line;
};

struct LabelPair {
	string truelabel;
	string endlabel;
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
// handles instruction generation
vector<string> handleInstruction(const Instruction &instruction, string &error);
// gets constructed comment for vm command
string getInstruction(const Instruction &ins);
// gets next true and end labels according to the counter
LabelPair getNextLabels(ACTION action);

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
vector<string> getEqAssembly(const Instruction &ins);
vector<string> getGtAssembly(const Instruction &ins);
vector<string> getLtAssembly(const Instruction &ins);

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

	while (getline(inputfile, line)) {
		linecnt++;
		ins = {};

		cleanedline = trim(line);

		// Empty/comment-only line
		if (cleanedline.empty()) {
			continue;
		}

		vector<string> instruction;
		stringstream ss(cleanedline);
		string token;

		while (ss >> token) {
			instruction.push_back(token);
		}

		// Arithmetic / logical command
		if (instruction.size() == 1) {

			if (actions.find(instruction[0]) == actions.end()) {
				reporterror(inputfilename, linecnt,
							"Invalid arithmetic or logical command",
							cleanedline);
				return STATUS::INVALID_OPERATION;
			}
			ACTION action = actions.at(instruction[0]);

			if (action == ACTION::PUSH || action == ACTION::POP) {
				reporterror(inputfilename, linecnt,
							"Push/pop require a segment and index",
							cleanedline);
				return STATUS::INVALID_INSTR;
			}
			ins.action = actions[instruction[0]];
			ins.line = linecnt;
		}

		// push / pop command
		else if (instruction.size() == 3) {

			if (actions.find(instruction[0]) == actions.end()) {
				reporterror(inputfilename, linecnt,
							"Invalid action (push or pop)", cleanedline);
				return STATUS::INVALID_ACTION;
			}

			if (segments.find(instruction[1]) == segments.end()) {
				reporterror(inputfilename, linecnt, "Invalid segment",
							cleanedline);
				return STATUS::INVALID_SEGMENT;
			}

			bool is_all_digits =
				all_of(instruction[2].begin(), instruction[2].end(),
					   [](unsigned char c) { return std::isdigit(c); });

			if (!is_all_digits) {
				reporterror(inputfilename, linecnt, "Not a positive integer",
							cleanedline);
				return STATUS::INVALID_ADDRESS;
			}

			// semantic validations
			ACTION action = actions.at(instruction[0]);
			SEGMENT segment = segments.at(instruction[1]);
			int i = stoi(instruction[2]);
			if (action != ACTION::PUSH && action != ACTION::POP) {
				reporterror(inputfilename, linecnt,
							"Invalid Action with segments!!!", cleanedline);
				return STATUS::INVALID_ACTION;
			}

			if (action == ACTION::POP && segment == SEGMENT::CONSTANT) {
				reporterror(inputfilename, linecnt,
							"Invalid action and segment combination!!!",
							cleanedline);
				return STATUS::INVALID_INSTR;
			}

			if (segment == SEGMENT::TEMP) {
				if (i < 0 || i > 7) {
					reporterror(inputfilename, linecnt,
								"Invalid index for TEMP SEG type (0..7)!!!",
								cleanedline);
					return STATUS::INVALID_TEMP_INDEX;
				}
			}
			if (segment == SEGMENT::POINTER) {
				if (i != 0 && i != 1) {
					reporterror(
						inputfilename, linecnt,
						"Invalid index for POINTER SEG type (0 or 1)!!!",
						cleanedline);
					return STATUS::INVALID_POINTER_INDEX;
				}
			}
			ins.action = action;
			ins.segment = segment;
			ins.index = i;
			ins.line = linecnt;

		}

		// Anything other than 1 or 3 tokens
		else {
			reporterror(inputfilename, linecnt, "Invalid instruction format",
						cleanedline);
			return STATUS::INVALID_INSTR;
		}

		instructions.push_back(ins);
	}

	// for (auto &ins : instructions) {
	// 	cout << static_cast<int>(ins.action) << " ";
	// 	cout << static_cast<int>(ins.segment) << " ";
	// 	cout << static_cast<int>(ins.index) << " ";
	// 	cout << ins.line << "\n";
	// 	cout << "\n";
	// }
	vector<string> output;
	vector<string> tempout;
	string error;
	for (const auto &ins : instructions) {
		tempout = handleInstruction(ins, error);
		if (error != "") {
			reporterror(inputfilename, ins.line, error, getInstruction(ins));
			return STATUS::INVALID_INSTR;
		}
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
	cout << msg << " [ " << input << " ]\n";
}

vector<string> handleInstruction(const Instruction &ins, string &error) {
	string instrstr = getInstruction(ins);
	error = "";
	vector<string> res({"// " + instrstr});
	vector<string> temp;
	switch (ins.action) {
	case ACTION::PUSH:
		switch (ins.segment) {
		case SEGMENT::CONSTANT:
			temp = getPushConstantAssembly(ins);
			res.insert(res.end(), temp.begin(), temp.end());
			return res;
		case SEGMENT::LOCAL:
		case SEGMENT::ARGUMENT:
		case SEGMENT::THIS:
		case SEGMENT::THAT:
			temp = getPushGenAssembly(ins);
			res.insert(res.end(), temp.begin(), temp.end());
			return res;
		case SEGMENT::STATIC:
			temp = getPushStaticAssembly(ins);
			res.insert(res.end(), temp.begin(), temp.end());
			return res;
		case SEGMENT::TEMP:
			temp = getPushTempAssembly(ins);
			res.insert(res.end(), temp.begin(), temp.end());
			return res;
		case SEGMENT::POINTER:
			temp = getPushPointerAssembly(ins);
			res.insert(res.end(), temp.begin(), temp.end());
			return res;
		}
	case ACTION::POP:
		switch (ins.segment) {
		case SEGMENT::CONSTANT:
			return {};
		case SEGMENT::LOCAL:
		case SEGMENT::ARGUMENT:
		case SEGMENT::THIS:
		case SEGMENT::THAT:
			temp = getPopGenAssembly(ins);
			res.insert(res.end(), temp.begin(), temp.end());
			return res;
		case SEGMENT::STATIC:
			temp = getPopStaticAssembly(ins);
			res.insert(res.end(), temp.begin(), temp.end());
			return res;
		case SEGMENT::TEMP:
			temp = getPopTempAssembly(ins);
			res.insert(res.end(), temp.begin(), temp.end());
			return res;
		case SEGMENT::POINTER:
			if (ins.index != 0 && ins.index != 1) {
				error = "This operation is not permitted (except 0 or 1)!!!";
				return {};
			}
			temp = getPopPointerAssembly(ins);
			res.insert(res.end(), temp.begin(), temp.end());
			return res;
		}
	case ACTION::ADD:
		temp = getAddAssembly();
		res.insert(res.end(), temp.begin(), temp.end());
		return res;
	case ACTION::SUB:
		temp = getSubAssembly();
		res.insert(res.end(), temp.begin(), temp.end());
		return res;
	case ACTION::AND:
		temp = getAndAssembly();
		res.insert(res.end(), temp.begin(), temp.end());
		return res;
	case ACTION::OR:
		temp = getOrAssembly();
		res.insert(res.end(), temp.begin(), temp.end());
		return res;
	case ACTION::NEG:
		temp = getNegAssembly();
		res.insert(res.end(), temp.begin(), temp.end());
		return res;
	case ACTION::NOT:
		temp = getNotAssembly();
		res.insert(res.end(), temp.begin(), temp.end());
		return res;
	case ACTION::EQ:
		temp = getEqAssembly(ins);
		res.insert(res.end(), temp.begin(), temp.end());
		return res;
	case ACTION::GT:
		temp = getGtAssembly(ins);
		res.insert(res.end(), temp.begin(), temp.end());
		return res;
	case ACTION::LT:
		temp = getLtAssembly(ins);
		res.insert(res.end(), temp.begin(), temp.end());
		return res;
	}
	return {};
}

string getInstruction(const Instruction &ins) {
	if (ins.action == ACTION::PUSH || ins.action == ACTION::POP) {
		return actionToString(ins.action) + " " + segmentToString(ins.segment) +
			   " " + to_string(ins.index);
	} else {
		return actionToString(ins.action);
	}
}

LabelPair getNextLabels(ACTION action) {
	string truelabel =
		compActionBase(action) + "_TRUE_" + to_string(labelcounter);
	string endlabel =
		compActionBase(action) + "_END_" + to_string(labelcounter);
	labelcounter++;
	return {truelabel, endlabel};
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
	string staticvar = staticVarName + "." + to_string(ins.index);
	return {"@SP", "AM=M-1", "D=M", "@" + staticvar, "M=D"};
}
vector<string> getPushStaticAssembly(const Instruction &ins) {
	string staticvar = staticVarName + "." + to_string(ins.index);
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
vector<string> getEqAssembly(const Instruction &ins) {
	LabelPair labels = getNextLabels(ins.action);
	return {
		"@SP",
		"AM=M-1",
		"D=M",
		"A=A-1",
		"D=M-D",
		"@" + labels.truelabel,
		"D;JEQ",
		"@SP",
		"A=M-1",
		"M=0",
		"@" + labels.endlabel,
		"0;JMP",
		"(" + labels.truelabel + ")",
		"@SP",
		"A=M-1",
		"M=-1",
		"(" + labels.endlabel + ")",
	};
}
vector<string> getLtAssembly(const Instruction &ins) {
	LabelPair labels = getNextLabels(ins.action);
	return {
		"@SP",
		"AM=M-1",
		"D=M",
		"A=A-1",
		"D=M-D",
		"@" + labels.truelabel,
		"D;JLT",
		"@SP",
		"A=M-1",
		"M=0",
		"@" + labels.endlabel,
		"0;JMP",
		"(" + labels.truelabel + ")",
		"@SP",
		"A=M-1",
		"M=-1",
		"(" + labels.endlabel + ")",
	};
}
vector<string> getGtAssembly(const Instruction &ins) {
	LabelPair labels = getNextLabels(ins.action);
	return {
		"@SP",
		"AM=M-1",
		"D=M",
		"A=A-1",
		"D=M-D",
		"@" + labels.truelabel,
		"D;JGT",
		"@SP",
		"A=M-1",
		"M=0",
		"@" + labels.endlabel,
		"0;JMP",
		"(" + labels.truelabel + ")",
		"@SP",
		"A=M-1",
		"M=-1",
		"(" + labels.endlabel + ")",
	};
}