#include "vmwriter.hpp"
#include <stdexcept>

void VMWriter::writePush(const std::string& segment, int index) {
    output << "push " << segment << ' ' << index << '\n';
}

void VMWriter::writePop(const std::string& segment, int index) {
    if (segment == "constant") {
        throw std::runtime_error("VM does not support pop constant");
    }
    output << "pop " << segment << ' ' << index << '\n';
}

void VMWriter::writeArithmetic(const std::string& command) {
    output << command << '\n';
}

void VMWriter::writeLabel(const std::string& label) {
    output << "label " << label << '\n';
}

void VMWriter::writeGoto(const std::string& label) {
    output << "goto " << label << '\n';
}

void VMWriter::writeIf(const std::string& label) {
    output << "if-goto " << label << '\n';
}

void VMWriter::writeCall(const std::string& name, int nArgs) {
    output << "call " << name << ' ' << nArgs << '\n';
}

void VMWriter::writeFunction(const std::string& name, int nLocals) {
    output << "function " << name << ' ' << nLocals << '\n';
}

void VMWriter::writeReturn() {
    output << "return\n";
}
