#include "compengine.hpp"
#include "declarations.hpp"
#include "tokenizer.hpp"
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

int indent = 0;

int main(int argc, char **argv) {

	try {

		// checking for arguments provided
		if (argc != 2) {
			std::cout << "Usage : analyzer <file.jack | directory>\n";
			return STATUS::INPUT_NOT_PROVIDED;
		}

		// variable declarations
		fs::path cmdinput = argv[1];

		fs::path canoninp = fs::canonical(cmdinput);
		std::vector<fs::path> inputfileslist;

		if (!fs::exists(cmdinput)) {
			std::cerr << "Error: file/directory not found: " << cmdinput
					  << '\n';
			return STATUS::INVALID_INPUT;
		}
		/* Here we're trying to build the inputfileslist
		 * even if we have only 1 file as an input or directory
		 * we store them in a std::vector to generalize the output file creation
		 * - output .xml file is created in same place as input .jack file
		 * - if directory is passed as an input it scans for .jack files and
		 * creates .xml file per .jack file
		 */
		if (fs::is_regular_file(canoninp)) {
			if (canoninp.extension() != ".jack") {
				std::cerr << "Error: input file must have .jack extension\n";
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
				std::cerr << "Error: directory contains no .jack files\n";
				return STATUS::EMPTY_INPUT_DIRECTORY;
			}
		} else {
			std::cerr << "Error: input is neither a regular file "
						 "nor a directory\n";
			return STATUS::INVALID_FILE_TYPE;
		}

		// main code start iterate per input file
		for (auto const &filepath : inputfileslist) {
			JackTokenizer tokenizer(filepath);
			tokenizer.tokenize();
			// tokenizer.writeXML();
			fs::path outputfilename = filepath;
			outputfilename.replace_extension(".xml");
			std::ofstream ofile(outputfilename);
			if (!ofile) {
				throw std::runtime_error("error opening output file: " +
										 outputfilename.string());
			}
			CompilationEngine parser(tokenizer, ofile);
			tokenizer.advance();
			parser.compileClass();
			std::cout<<"Outputfile : "<<outputfilename<<"\n";
		}

		return 0;
	} catch (std::exception &e) {
		std::cout << "Some exception occured!!!\n" << e.what() << std::endl;
	}
}
