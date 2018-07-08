#ifndef VMWRITER_HPP
#define VMWRITER_HPP

enum Segment {_CONST, _ARG, _LOCAL, _STATIC, _THIS, _THAT, _POINTER, _TEMP};
enum Command {ADD, SUB, NEG, EQ, GT, LT, AND, OR, NOT};

#include <string>
#include <fstream>

class VMWriter
{
	private:
		std::ofstream outputFile;
		std::string className;

	public:
		VMWriter(std::string);
		void writePush(Segment, int);
		void writePop(Segment, int);
		void writeArithmetic(Command);
		void writeLabel(std::string);
		void writeGoto(std::string);
		void writeIf(std::string);
		void writeCall(std::string, int);
		void writeFunction(std::string, std::string, int);
		void writeReturn();
		void setClassName(std::string);
		std::string getClassName();
		~VMWriter();
};
#endif