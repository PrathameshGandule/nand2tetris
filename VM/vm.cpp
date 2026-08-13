#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>
using namespace std;

const string WHITESPACE = " \n\r\t\f\v";
const string SP = "R0";

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
string seg_to_string(SEGMENT segment) {
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
}

unordered_set<string> actions = {"push", "pop"};
unordered_set<string> segments = {"local", "argument", "static", "constant",
								  "this",  "that",	   "temp",	 "pointer"};
unordered_set<string> operations = {"add", "sub", "neg", "eq", "gt",
									"lt",  "and", "or",	 "not"};

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
vector<string> handleInstruction(vector<string> &instruction);
// generates CONSTANT segment stack
vector<string> getConstantAssembly(string i);

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

	vector<vector<string>> instructions;
	vector<int> lineNumber;
	int cnt = 0;
	string line;
	string cleanedline;
	char delimeter = ' ';
	bool is_all_digits;

	while (getline(inputfile, line)) {
		cnt++;

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

			if (operations.find(instruction[0]) == operations.end()) {
				reporterror(inputfilename, cnt,
							"Invalid arithmetic or logical command",
							cleanedline);
				return STATUS::INVALID_OPERATION;
			}
		}

		// push / pop command
		else if (instruction.size() == 3) {

			if (actions.find(instruction[0]) == actions.end()) {
				reporterror(inputfilename, cnt, "Invalid action (push or pop)",
							cleanedline);
				return STATUS::INVALID_ACTION;
			}

			if (segments.find(instruction[1]) == segments.end()) {
				reporterror(inputfilename, cnt, "Invalid segment", cleanedline);
				return STATUS::INVALID_SEGMENT;
			}

			bool is_all_digits =
				all_of(instruction[2].begin(), instruction[2].end(),
					   [](unsigned char c) { return std::isdigit(c); });

			if (!is_all_digits) {
				reporterror(inputfilename, cnt, "Not a positive integer",
							cleanedline);
				return STATUS::INVALID_ADDRESS;
			}
		}

		// Anything other than 1 or 3 tokens
		else {
			reporterror(inputfilename, cnt, "Invalid instruction format",
						cleanedline);
			return STATUS::INVALID_INSTR;
		}

		instructions.push_back(instruction);
		lineNumber.push_back(cnt);
	}

	// for (auto &ins : instructions) {
	// 	for (auto &i : ins) {
	// 		cout << i << " ";
	// 	}
	// 	cout << "\n";
	// }
	vector<string> output;
	for (auto &ins : instructions) {
		vector<string> temp = handleInstruction(ins);
		for(auto &i : temp){
			output.push_back(i);
		}
		cout << "\n";
	}

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

vector<string> handleInstruction(vector<string> &instruction) {
	if (instruction.size() == 1) {

	} else if (instruction.size() == 3) {
		string instrcomment = "// "+instruction[0]+" "+instruction[1]+" "+instruction[2];
		vector<string> res = {instrcomment};
		vector<string> temp;
		if (instruction[1] == seg_to_string(SEGMENT::CONSTANT)) {
			temp = getConstantAssembly(instruction[2]);
			res.insert(res.end(), temp.begin(), temp.end());
			return res;
		}
	}
}

vector<string> getConstantAssembly(string i) {
	return {
		"@" + i, "D=A", "@" + SP, "A=M", "M=D", "@" + SP, "M=M+1",
	};
}