#include <string>
#include <iostream>
#include "Tokenizer.hpp"




int main(int argc, char* argv[])
{
	std::string inputFileName;
	Tokenizer *tokenPtr = nullptr;

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
		tokenPtr = new Tokenizer(inputFileName);

		//test for open errors
		if(tokenPtr->errorCheck())
		{
			//if there is an error, print status and manage memory then exit
			std::cout << "There was an error opening the file." << std::endl;
			delete tokenPtr;
			tokenPtr = nullptr;
			return 1;
		}

		tokenPtr->parse();


		//loop while there are still more tokens to be read
		//while(tokenPtr->hasMoreTokens())
		//{


		//}

		delete tokenPtr;
		tokenPtr = nullptr;

	}

	return 0;
}


