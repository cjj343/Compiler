#include <string>
#include <iostream>
#include "Tokenizer.hpp"
#include "CompilationEngine.hpp"
#include "Symbol_Table.hpp"

int main(int argc, char* argv[])
{
	std::string inputFileName;

	// Check the number of parameters
	if (argc < 2)
	{
		// Tell the user how to run the program
		std::cout << "Usage: " << argv[0] << "<FILE NAME or DIRECTORY>" << std::endl;
		return 1;
	}

	//loop for all the files entered on command line
	for (int fileCount = 1; fileCount < argc; fileCount++)
	{
		
		inputFileName = argv[fileCount]; //get the first user arg

		//For each Jack file entered on command line, we need fresh copies of all tools
		std::unique_ptr<Tokenizer> tokenPtr (new Tokenizer(inputFileName));
		std::unique_ptr<CompilationEngine> compilePtr (new CompilationEngine());
		std::unique_ptr<VMWriter> vmPtr (new VMWriter(inputFileName));
		std::unique_ptr<Symbol_Table> stPtr (new Symbol_Table());

		//test for open errors
		if(tokenPtr->errorCheck())
		{
			//if there is an error, print status and manage memory then exit
			std::cout << "There was an error opening the file." << std::endl;
			return 1;
		}

		tokenPtr->tokenize(); //First tokenize the file
		//tokenPtr->printTokens(); *****Uncomment of you want a text file of tokens**********
		compilePtr->compileClass(tokenPtr, vmPtr, stPtr); //this call completes all remaining processing tasks
	}

	return 0;
}


