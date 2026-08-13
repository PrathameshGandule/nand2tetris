#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

const string WHITESPACE = " \n\r\t\f\v";

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

struct Instruction {
	ACTION action;
	SEGMENT segment;
	int index;
	int line;
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
vector<string> handleInstruction(const Instruction &instruction);
// gets constructed comment for vm command
string getComment(const Instruction &ins);

vector<string> getPushConstantAssembly(const int i);
vector<string> getPopLocalAssembly(int i);

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
			ins.action = actions.at(instruction[0]);
			ins.segment = segments.at(instruction[1]);
			ins.index = stoi(instruction[2]);
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
	for (const auto &ins : instructions) {
		tempout = handleInstruction(ins);
		output.insert(output.end(), tempout.begin(), tempout.end());
	}

	int endPointPos = inputfilename.find_last_of('.');
	string outputfilename = inputfilename.substr(0, endPointPos) + ".asm";
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

vector<string> handleInstruction(const Instruction &ins) {
	string instrcomment = getComment(ins);
	vector<string> res({instrcomment});
	vector<string> temp;
	switch (ins.action) {
	case ACTION::PUSH:
		switch (ins.segment) {
		case SEGMENT::CONSTANT:
			temp = getPushConstantAssembly(ins.index);
			res.insert(res.end(), temp.begin(), temp.end());
			return res;
		}
	case ACTION::POP:
		switch (ins.segment) {
		case SEGMENT::LOCAL:
			temp = getPopLocalAssembly(ins.index);
			res.insert(res.end(), temp.begin(), temp.end());
			return res;
		}
	}
	return {};
}

vector<string> getPushConstantAssembly(const int i) {
	return {
		"@" + to_string(i), "D=A", "@SP", "A=M", "M=D", "@SP", "M=M+1",
	};
}

vector<string> getPopLocalAssembly(int i) {
	return {
		"@" + to_string(i),
		"D=A",
		"@LCL",
		"D=D+M",
		"@R13",
		"M=D",

		"@SP",
		"AM=M-1",
		"D=M",

		"@R13",
		"A=M",
		"M=D",
	};
}

string getComment(const Instruction &ins) {
	if (ins.action == ACTION::PUSH || ins.action == ACTION::POP) {
		return "// " + actionToString(ins.action) + " " +
			   segmentToString(ins.segment) + " " + to_string(ins.index);
	} else {
		return "// " + actionToString(ins.action);
	}
}