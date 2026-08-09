#include <bitset>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace std;

enum STATUS{
	SUCCESS,
	FILE_NOT_PROVIDED,
	COULD_NOT_INPUT_OPEN_FILE,
	COULD_NOT_OPEN_OUTPUT_FILE,
	INVALID_A_INSTR
};

string handleAInstruction(string input){
	size_t pos = input.find('@');
	string addressStr = "";
	int addressInt = 0;
	if (pos != string::npos) {
		addressStr = input.substr(pos+1);
		addressInt = stoi(addressStr);
		bitset<16> addressBin(addressInt);
		return addressBin.to_string();
	} else {
		addressStr = "";
		cout<<"Invalid A instruction at [ "<<input<<" ]\n";
		return "";
	}
}

int main(int argc, char **argv) {
	if(argc != 2){
		cout<<"Usage: assembler <filename>\n";
		return STATUS::FILE_NOT_PROVIDED;
	}
	ifstream file(argv[1]);
	if(!file.is_open()){
		cout<<"Error opening file\n";
		return STATUS::COULD_NOT_INPUT_OPEN_FILE;
	}
	string line;
	vector<string> instructions;
	vector<string> output;
	while(getline(file, line)){
		instructions.push_back(line);
	}
	for(auto &instr : instructions){
		string tempOutput = "";
		if(instr[0] == '@'){
			tempOutput = handleAInstruction(instr);
			output.push_back(tempOutput);
		}
	}
	ofstream ofile(string(argv[1])+".bin");
	if(!ofile){
		cerr<<"Error opening file\n";
		return STATUS::COULD_NOT_OPEN_OUTPUT_FILE;
	}
	for(auto &out : output){
		ofile<<out<<"\n";
	}
	ofile.close();
	return STATUS::SUCCESS;
}