#ifndef VM_WRITER_HPP
#define VM_WRITER_HPP

#include <ostream>
#include <string>

class VMWriter {
  private:
	std::ostream &output;

  public:
	explicit VMWriter(std::ostream &output) : output(output) {}

	void writePush(const std::string &segment, int index);
	void writePop(const std::string &segment, int index);
	void writeArithmetic(const std::string &command);
	void writeLabel(const std::string &label);
	void writeGoto(const std::string &label);
	void writeIf(const std::string &label);
	void writeCall(const std::string &name, int nArgs);
	void writeFunction(const std::string &name, int nLocals);
	void writeReturn();
};

#endif /* VM_WRITER_HPP */