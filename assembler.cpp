#include <bitset>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

const string WHITESPACE = " \n\r\t\f\v";

enum STATUS {
	SUCCESS,
	INPUT_FILE_NOT_PROVIDED,
	COULD_NOT_OPEN_INPUT_FILE,
	COULD_NOT_OPEN_OUTPUT_FILE,
	INVALID_A_INSTR
};

enum INSTR { A, C, LABEL };

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
string processAInstrcution(string input);

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
	int cnt = 0;
	while (getline(inputfile, line)) {
		cnt++;
		cleanedline = trim(line);
		// ignoring empty lines after cleaning
		if (cleanedline != "") {
			instructions.push_back(cleanedline);
			// cout<<"|"<<cleanedline<<"|"<<cnt<<"\n";
			lineNumber.push_back(cnt);
		}
	}

	int instrlen = instructions.size();
	vector<string> output(instrlen);
	string tempOutput = "";
	for (int i = 0; i < instrlen; i++) {
		if (typeOfInstruction(instructions[i]) == INSTR::A) {
			tempOutput = processAInstrcution(instructions[i]);
			if (tempOutput == "") {
				cout << "ERROR: AT LINE " << lineNumber[i] << " : "
					 << inputfilename << ":" << lineNumber[i] << "\n";
				return STATUS::INVALID_A_INSTR;
			}
			output[i] = tempOutput;
		} else if (typeOfInstruction(instructions[i]) == INSTR::C) {
			cout << "yet to process C instr...\n";
		} else if (typeOfInstruction(instructions[i]) == INSTR::LABEL) {
			cout << "yet to process labels...\n";
		}
	}

	for (auto &out : output) {
		cout << out << "\n";
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

int typeOfInstruction(string input) {
	if (input[0] == '@')
		return INSTR::A;
	else if (input[0] == '(')
		return INSTR::LABEL;
	else
		return INSTR::C;
}

string processAInstrcution(string input) {
	string address = input.substr(1);
	size_t processed_chars = 0;
	int addressInt = stoi(address, &processed_chars);
	if (processed_chars != address.length()) {
		cout << "Invalid A instruction [ " << input << " ]\n";
		return "";
	}
	if (addressInt < 0 || addressInt > 32767) {
		cout << "Address out of range (0 to 32767) [ " << input << " ]\n";
		return "";
	}
	bitset<16> addressBin(addressInt);
	return addressBin.to_string();
}