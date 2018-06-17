#include <string>
#include <iostream>
#include "Tokenizer.hpp"
#include "CompilationEngine.hpp"



int main(int argc, char* argv[])
{
	std::string inputFileName;
	std::unique_ptr<Tokenizer> tokenPtr;
	std::unique_ptr<CompilationEngine> compilePtr;
	
	//Tokenizer *tokenPtr = nullptr;
	//CompilationEngine *compilePtr = nullptr;

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
		//get the first user arg
		inputFileName = argv[fileCount];

		//creat a new object passing file name to constructor
		tokenPtr.reset(new Tokenizer(inputFileName));
		compilePtr.reset(new CompilationEngine(inputFileName));

		//tokenPtr = new Tokenizer(inputFileName);
		//compilePtr = new CompilationEngine(inputFileName);

		//test for open errors
		if(tokenPtr->errorCheck() || compilePtr->errorCheck())
		{
			//if there is an error, print status and manage memory then exit
			std::cout << "There was an error opening the file." << std::endl;
			//delete tokenPtr;
			//tokenPtr = nullptr;
			//delete compilePtr;
			//ompilePtr = nullptr;
			return 1;
		}

		tokenPtr->tokenize();
		//tokenPtr->printTokens();
		compilePtr->compileClass(tokenPtr);




		

		
		

		//delete tokenPtr;
		//tokenPtr = nullptr;

		//delete compilePtr;
		//compilePtr = nullptr;

	}

	return 0;
}


