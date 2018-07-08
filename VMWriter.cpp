#include "VMWriter.hpp"
#include <iostream>

/*****************************************************************************************
*Params
	- file name from main
*Takes the jack file name and opens a file of the same name with vm extension for writing
******************************************************************************************/
VMWriter::VMWriter(std::string fileName)
{
	size_t pos = fileName.find('.');
	fileName.erase(pos);
	fileName += ".vm";
	outputFile.open(fileName);

	//throw an exception and exit if there was an error opening the file
	try
	{
		std::string exceptionString = "There was an error opening the file. Exiting...";

		if(outputFile.fail())
			throw std::string(exceptionString);
	}

	catch(std::string exceptionString)
	{
		std::cout << exceptionString << std::endl;
		exit(1);
	}
}

/*****************************************************************************************
*Params
	- Segment enum
	- index of segment
*Writes VM push command
******************************************************************************************/
void VMWriter::writePush(Segment seg, int i)
{
	switch(seg)
	{
		case _CONST:
			outputFile << "push constant " << i << std::endl;
			break;

		case _ARG:
			outputFile << "push argument " << i << std::endl;
			break;

		case _LOCAL:
			outputFile << "push local " << i << std::endl;
			break;

		case _STATIC:
			outputFile << "push static " << i << std::endl;
			break;

		case _THIS:
			outputFile << "push this " << i << std::endl;
			break;

		case _THAT:
			outputFile << "push that " << i << std::endl;
			break;

		case _POINTER:
			outputFile << "push pointer " << i << std::endl;
			break;

		case _TEMP:
			outputFile << "push temp " << i << std::endl;
			break;
	}
}

/*****************************************************************************************
*Params
	- Segment enum
	- index of segment
*Writes VM pop command
******************************************************************************************/
void VMWriter::writePop(Segment seg, int i)
{
	switch(seg)
	{
		case _CONST:
			outputFile << "pop constant " << i << std::endl;
			break;

		case _ARG:
			outputFile << "pop argument " << i << std::endl;
			break;

		case _LOCAL:
			outputFile << "pop local " << i << std::endl;
			break;

		case _STATIC:
			outputFile << "pop static " << i << std::endl;
			break;

		case _THIS:
			outputFile << "pop this " << i << std::endl;
			break;

		case _THAT:
			outputFile << "pop that " << i << std::endl;
			break;

		case _POINTER:
			outputFile << "pop pointer " << i << std::endl;
			break;

		case _TEMP:
			outputFile << "pop temp " << i << std::endl;
			break;
	}

}

/*****************************************************************************************
*Params
	- Command enum
*Writes VM arithmetic commands
******************************************************************************************/
void VMWriter::writeArithmetic(Command com)
{
	switch(com)
	{
		case ADD:
			outputFile << "add" << std::endl;
			break;

		case SUB:
			outputFile << "sub" << std::endl;
			break;

		case NEG:
			outputFile << "neg" << std::endl;
			break;

		case EQ:
			outputFile << "eq" << std::endl;
			break;

		case GT:
			outputFile << "gt" << std::endl;
			break;

		case LT:
			outputFile << "lt" << std::endl;
			break;

		case AND:
			outputFile << "and" << std::endl;
			break;

		case OR:
			outputFile << "or" << std::endl;
			break;

		case NOT:
			outputFile << "not" << std::endl;
			break;
	}

}

/*****************************************************************************************
*Params
	- Label name
*Writes VM label
******************************************************************************************/
void VMWriter::writeLabel(std::string label)
{
	outputFile << "label " << label << std::endl;
}

/*****************************************************************************************
*Params
	- Label name
*Writes VM goto statement
******************************************************************************************/
void VMWriter::writeGoto(std::string label)
{
	outputFile << "goto " << label << std::endl;
}

/*****************************************************************************************
*Params
	- Label name
*Writes VM if-goto statement
******************************************************************************************/
void VMWriter::writeIf(std::string label)
{
	outputFile << "if-goto " << label << std::endl;
}

/*****************************************************************************************
*Params
	- function name
	- number of args for the function
*Writes function call
******************************************************************************************/
void VMWriter::writeCall(std::string function, int args)
{
	outputFile << "call " << function << " " << args << std::endl;	
}

/*****************************************************************************************
*Params
	- TODO: type parameter is useless
	- name of function
	- number of local variables for function
*Writes function declaration
******************************************************************************************/
void VMWriter::writeFunction(std::string type, std::string name, int locals)
{
	outputFile << "function " << className << "." << name << " " << locals << std::endl;
}

/*****************************************************************************************
*Writes return
******************************************************************************************/
void VMWriter:: writeReturn()
{
	outputFile << "return" << std::endl;
}

/*****************************************************************************************
*Sets the class name
******************************************************************************************/
void VMWriter::setClassName(std::string name)
{
	className = name;
}

/*****************************************************************************************
*Get the class name
******************************************************************************************/
std::string VMWriter::getClassName()
{
	return className;
}

VMWriter::~VMWriter()
{

}