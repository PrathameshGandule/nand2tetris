#include <algorithm>
#include <bitset>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

const string WHITESPACE = " \n\r\t\f\v";
int varMem = 16;

unordered_map<string, int> symbols({
	{"SP", 0},	 {"LCL", 1},		{"ARG", 2},		{"THIS", 3}, {"THAT", 4},
	{"R0", 0},	 {"R1", 1},			{"R2", 2},		{"R3", 3},	 {"R4", 4},
	{"R5", 5},	 {"R6", 6},			{"R7", 7},		{"R8", 8},	 {"R9", 9},
	{"R10", 10}, {"R11", 11},		{"R12", 12},	{"R13", 13}, {"R14", 14},
	{"R15", 15}, {"SCREEN", 16384}, {"KBD", 24576},
});

unordered_map<string, string> dest({
	{"", "000"},
	{"A", "100"},
	{"D", "010"},
	{"M", "001"},
	{"AD", "110"},
	{"DA", "110"},
	{"DM", "011"},
	{"MD", "011"},
	{"AM", "101"},
	{"MA", "101"},
	{"ADM", "111"},
	{"AMD", "111"},
	{"DAM", "111"},
	{"DMA", "111"},
	{"MAD", "111"},
	{"MDA", "111"},
});

unordered_map<string, string> comp({
	{"0", "0101010"},	{"1", "0111111"},	{"-1", "0111010"},
	{"D", "0001100"},	{"A", "0110000"},	{"M", "1110000"},
	{"!D", "0001111"},	{"-A", "0110011"},	{"-M", "1110011"},
	{"D+1", "0011111"}, {"A+1", "0110111"}, {"M+1", "1110111"},
	{"D-1", "0001110"}, {"A-1", "0110010"}, {"M-1", "1110010"},
	{"D+A", "0000010"}, {"D+M", "1000010"}, {"D-A", "0010011"},
	{"D-M", "1010011"}, {"A-D", "0000111"}, {"M-D", "1000111"},
	{"D&A", "0000000"}, {"D&M", "1000000"}, {"D|A", "0010101"},
	{"D|M", "1010101"},
});

unordered_map<string, string> jump({
	{"", "000"},
	{"JGT", "001"},
	{"JEQ", "010"},
	{"JGE", "011"},
	{"JLT", "100"},
	{"JNE", "101"},
	{"JLE", "110"},
	{"JMP", "111"},
});

enum STATUS {
	SUCCESS,
	INPUT_FILE_NOT_PROVIDED,
	COULD_NOT_OPEN_INPUT_FILE,
	COULD_NOT_OPEN_OUTPUT_FILE,
	INVALID_A_INSTR,
	INVALID_C_INSTR,
	INVALID_LABEL,
	INVALID_VARIABLE
};

enum INSTR { A, C };

// removes whitespaces from left
string ltrim(string str);
// removes whitespaces from right
string rtrim(string str);
// removes comment ;)
string removeComment(string str);
// uses all above functions in one
string trim(string str);
// return type of instruction using INSTR enum
int typeOfInstruction(string input);
// convert A instruction to binary specification
string processAInstrcution(string input, string &error);
// validates labels and vars
bool isValidSymbol(const std::string &symbol);
// convert C instruction to binary specification
string processCInstrcution(string input, string &error);

int main(int argc, char **argv) {

	// getting filename as an argument
	if (argc != 2) {
		cout << "Usage: assembler <filename>\n";
		return STATUS::INPUT_FILE_NOT_PROVIDED;
	}
	string inputfilename = string(argv[1]);

	// opening file to read
	ifstream inputfile(inputfilename);
	if (!inputfile.is_open()) {
		cout << "Couldn't open file named : " << inputfilename << "\n";
		return STATUS::COULD_NOT_OPEN_INPUT_FILE;
	}

	vector<string> instructions;
	vector<int> lineNumber;
	// cleaning input lines of code
	//  removing comments
	//  removing whitespaces from left and right
	string line;
	string cleanedline;
	string label;
	int cnt = 0;
	int romAddress = 0;
	while (getline(inputfile, line)) {
		cnt++;
		cleanedline = trim(line);
		// ignoring empty lines after cleaning
		if (cleanedline != "") {
			// cout<<"|"<<cleanedline<<"|"<<cnt<<"\n";
			// if encountered potetial label, process it right now
			if (cleanedline[0] == '(') {
				// if invalid label, not enclosed in () return error
				if (cleanedline[cleanedline.length() - 1] != ')') {
					cout << "ERROR: AT LINE " << cnt << " : " << inputfilename
						 << ":" << cnt << "\n";
					cout << "Label syntax error [ " << cleanedline << " ]\n";
				} else {
					label = cleanedline.substr(1, cleanedline.length() - 2);
					// validate the label
					if (!isValidSymbol(label)) {
						cout << "ERROR: AT LINE " << cnt << " : "
							 << inputfilename << ":" << cnt << "\n";
						cout << "Invalid label [ " << label << " ]\n";
						return STATUS::INVALID_LABEL;
					}
					// if label already exists return error
					if (symbols.find(label) != symbols.end()) {
						cout << "ERROR: AT LINE " << cnt << " : "
							 << inputfilename << ":" << cnt << "\n";
						cout << "Label [ " << label
							 << " ] already declared!!!\n";
						return STATUS::INVALID_LABEL;
						// allocate current rom address to label in symbol table
					} else {
						symbols[label] = romAddress;
					}
				}
			} else {
				// push instruction, line number and increment romAddress;
				instructions.push_back(cleanedline);
				lineNumber.push_back(cnt);
				romAddress++;
			}
		}
	}

	// for (auto &ins : instructions) {
	// 	cout << ins << "\n";
	// }
	// cout << "\n\n";

	int instrlen = instructions.size();
	vector<string> output(instrlen);
	string tempOutput = "";
	string error = "";
	int instrType;
	for (int i = 0; i < instrlen; i++) {
		error = "";
		instrType = typeOfInstruction(instructions[i]);
		if (instrType == INSTR::A) {
			tempOutput = processAInstrcution(instructions[i], error);
			if (tempOutput == "" || error != "") {
				cout << "ERROR: AT LINE " << lineNumber[i] << " : "
					 << inputfilename << ":" << lineNumber[i] << "\n";
				cout << error << "\n";
				return STATUS::INVALID_A_INSTR;
			} else {
				output[i] = tempOutput;
			}
		} else if (instrType == INSTR::C) {
			tempOutput = processCInstrcution(instructions[i], error);
			if (tempOutput == "" || error != "") {
				cout << "ERROR: AT LINE " << lineNumber[i] << " : "
					 << inputfilename << ":" << lineNumber[i] << "\n";
				cout << error << "\n";
				return STATUS::INVALID_C_INSTR;
			} else {
				output[i] = tempOutput;
			}
		}
	}

	int endPointPos = inputfilename.find_last_of('.');
	string outputfilename = inputfilename.substr(0, endPointPos)+".hack";
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

int typeOfInstruction(string input) {
	if (input[0] == '@')
		return INSTR::A;
	else
		return INSTR::C;
}

string processAInstrcution(string input, string &error) {
	string address = input.substr(1);
	int addressInt;
	bool is_all_digits =
		std::all_of(address.begin(), address.end(),
					[](unsigned char c) { return std::isdigit(c); });
	if (is_all_digits) {
		addressInt = stoi(address);
		if (addressInt < 0 || addressInt > 32767) {
			error = "Address out of range (0 to 32767) [ " + input + " ]\n";
			return "";
		}
	} else {
		if (symbols.find(address) != symbols.end()) {
			addressInt = symbols[address];
		} else {
			if (varMem >= 16384) {
				error =
					"RAM ran out of memory (16 to 16384) [ " + input + " ]\n";
				return "";
			}
			if (!isValidSymbol(address)) {
				error = "Invalid variable [ " + input + " ]\n";
				return "";
			}
			symbols[address] = varMem;
			addressInt = varMem;
			varMem++;
		}
	}
	bitset<16> addressBin(addressInt);
	error = "";
	return addressBin.to_string();
}

/**
 * Validates if a string is a valid Hack symbol.
 * Rules:
 *  - Must not be empty.
 *  - Valid chars: A-Z, a-z, 0-9, '.', '_', ':', '$'
 *  - Cannot start with a digit (0-9).
 */
bool isValidSymbol(const std::string &symbol) {
	if (symbol.empty()) {
		return false;
	}

	// First character check: cannot be a digit
	char first = symbol[0];
	if (std::isdigit(static_cast<unsigned char>(first))) {
		return false;
	}

	// Character set check
	for (char ch : symbol) {
		bool isValidChar = std::isalnum(static_cast<unsigned char>(ch)) ||
						   ch == '.' || ch == '_' || ch == ':' || ch == '$';
		if (!isValidChar) {
			return false;
		}
	}

	return true;
}

string processCInstrcution(string input, string &error) {
	int equalCount = count(input.begin(), input.end(), '=');
	int semicolonCount = count(input.begin(), input.end(), ';');

	// A C-instruction can have at most one '=' and one ';'
	if (equalCount > 1) {
		error = "Invalid C instruction [ " + input +
				" ]\n"
				"Multiple '=' found!!!\n";
		return "";
	}

	if (semicolonCount > 1) {
		error = "Invalid C instruction [ " + input +
				" ]\n"
				"Multiple ';' found!!!\n";
		return "";
	}

	size_t equalPos = input.find('=');
	size_t semicolonPos = input.find(';');

	string destStr = "";
	string compStr = "";
	string jumpStr = "";

	// --------------------------------------------------
	// Extract destination
	// --------------------------------------------------
	if (equalPos != string::npos) {
		// '=' cannot be first character
		if (equalPos == 0) {
			error = "Invalid C instruction [ " + input +
					" ]\n"
					"Empty destination!!!\n";
			return "";
		}

		destStr = input.substr(0, equalPos);
	}

	// --------------------------------------------------
	// Extract jump
	// --------------------------------------------------
	if (semicolonPos != string::npos) {
		// ';' cannot be last character
		if (semicolonPos == input.length() - 1) {
			error = "Invalid C instruction [ " + input +
					" ]\n"
					"Empty jump!!!\n";
			return "";
		}

		jumpStr = input.substr(semicolonPos + 1);
	}

	// --------------------------------------------------
	// Extract computation
	// --------------------------------------------------
	size_t compStart;
	size_t compEnd;

	if (equalPos != string::npos)
		compStart = equalPos + 1;
	else
		compStart = 0;

	if (semicolonPos != string::npos)
		compEnd = semicolonPos;
	else
		compEnd = input.length();

	// comp cannot be empty
	if (compStart >= compEnd) {
		error = "Invalid C instruction [ " + input +
				" ]\n"
				"Empty computation!!!\n";
		return "";
	}

	compStr = input.substr(compStart, compEnd - compStart);

	// --------------------------------------------------
	// Validate extracted components
	// --------------------------------------------------
	if (dest.find(destStr) == dest.end()) {
		error = "Invalid destination [ " + destStr + " ]\nInput [ " + input +
				" ]\n";
		return "";
	}

	if (comp.find(compStr) == comp.end()) {
		error = "Invalid computation [ " + compStr + " ]\nInput [ " + input +
				" ]\n";
		return "";
	}

	if (jump.find(jumpStr) == jump.end()) {
		error = "Invalid jump [ " + jumpStr + " ]\nInput [ " + input + " ]\n";
		return "";
	}

	// --------------------------------------------------
	// Generate machine instruction
	// --------------------------------------------------
	return "111" + comp[compStr] + dest[destStr] + jump[jumpStr];
}