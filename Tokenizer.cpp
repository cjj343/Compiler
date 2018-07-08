#include "Tokenizer.hpp"
#include <assert.h>

/**********************************************************************************
*Params
	- file name
*Excepts raw file (.jack) from main and opens it for processing
*Establishes reserved keywords and tokens 
***********************************************************************************/
Tokenizer::Tokenizer(std::string inputFileName)
{
	//open the file and test for errors
	openError = false;
	inputFile.open(inputFileName);
	keywords = {"class", "constructor", "function", "method", "field", "static", "var", "int", "char", "boolean", "void", "true", "false", "null", "this", "let", "do", "if", "else", "while", "return"};
	symbols = {"{", "}", "(", ")", "[", "]", ".", ",", ";", "+", "-", "*", "/", "&", "|", "<", ">", "=", "~"};
	tokenCount = 0;
	lineCount = 0;

	if (inputFile.fail())
	{
		openError = true;
	}
}

/**********************************************************************************
*Used by main to check for file open errors
*TODO: probbly should be try catch block in constructor
***********************************************************************************/
bool Tokenizer::errorCheck()
{
	return openError;
}

/**********************************************************************************
*Helper function returning true if there are more tokens
***********************************************************************************/
bool Tokenizer::hasMoreTokens()
{	
	if(tokenCount < tokenVector.size())
	{
		return true;
	}

	else
	{
		return false;
	}
}

/**********************************************************************************
*Increments the token index
***********************************************************************************/
void Tokenizer::advance()
{
	tokenCount++;
}

/**********************************************************************************
*Returns the token type
***********************************************************************************/
_tokenType Tokenizer::tokenType()
{
	return tokenVector[tokenCount].type;
}

/**********************************************************************************
*Returns the token
***********************************************************************************/
std::string Tokenizer::getToken()
{
	return tokenVector[tokenCount].token;
}

/**********************************************************************************
*Gets a whole line from the raw jack file
***********************************************************************************/
int Tokenizer::getLine()
{
	return tokenVector[tokenCount - 1].line;
}

/**********************************************************************************
*Check the token immediately after the current token
***********************************************************************************/
std::string Tokenizer::tokenPeek()
{
	assert(tokenCount + 1 < tokenVector.size());
	return tokenVector[tokenCount + 1].token;
}

/**********************************************************************************
*Tokenized a jack file
***********************************************************************************/
void Tokenizer::tokenize()
{	
	bool multiLineComment;
	bool blankLine;

	//loop while the file still has data
	while(!(inputFile.eof()))
	{
		blankLine = false;

		getline(inputFile, line);
		lineCount++;

		//clean line of single line comments
		if (line.find("//") != std::string::npos)
		{
			line.erase(line.find("//"));
		}

		//find and erase multi line comments
		if(line.find("/*") != std::string::npos)
		{
			//if multi line comment starts and ends on same line, following functions will handle
			if(line.find("*/") == std::string::npos)
			{
				line.erase(line.find("/*"));
				multiLineComment = true;
			}			
		}

		//find and erase multi line API comments
		if(line.find("/**") != std::string::npos)
		{
			//if multi line comment starts and ends on same line, following functions will handle
			if(line.find("*/") == std::string::npos)
			{
				line.erase(line.find("/**"));
				multiLineComment = true;
			}			
		}

		//find end of multiline comment and API comment
		if(line.find("*/") != std::string::npos)
		{
			line = line.substr(line.find("*/") + 2);
			multiLineComment = false;			
		}

		//testing for blank lines
		for(size_t i = 0; i < line.length(); i++)
		{
			if(isalnum(line[i]) || ispunct(line[i]))
			{
				blankLine = false;
			}

			//kill loop if we find anything
			if(!blankLine)
			{
				break;
			}
		}

		//tokenize a line
		if(!multiLineComment && !blankLine)
		{
			//add delimination for strings
			deliminateStringConstant();	

			//add delimination for keywrods
			deliminateKeyword();

			//add delimination for symbols
			deliminateSymbol();

			//and tokens to vector
			tokenizeLine();

			stringBeg.clear();
			stringEnd.clear();
		}				
	}
}

/**********************************************************************************
*After a line has been deliminated go through and collect tokens
***********************************************************************************/
void Tokenizer::tokenizeLine()
{
	std::string temp = "";
	std::stringstream check1(line);
	bool terminalFound;
	bool blank;

	while(getline(check1, temp, '%'))
	{
		terminalFound = false;
		blank = true;

		if(!temp.empty())
		{	
			//check to see if deliminated unit is keyword
			for(size_t i = 0; i < keywords.size(); i++)
			{
				if (temp == keywords[i])
				{
					tokenVector.push_back(tokenStruct(KEYWORD, temp, lineCount));
					terminalFound = true;
				}
			}

			//check to see if deliminated unit is symbol
			for(size_t i = 0; i < symbols.size(); i++)
			{
				if (temp == symbols[i])
				{
					tokenVector.push_back(tokenStruct(SYMBOL, temp, lineCount));
					terminalFound = true;
				}
			}

			//check to see if deliminated unit is string
			if(temp[0] == '\"')
			{
				std::string noQuote;

				//delimination leaves the quotes in the token
				//loop through the string and remove the quotes
				for(size_t i = 0; i < temp.size(); i++)
				{
					if(temp[i] != '\"')
						noQuote += temp[i];

				}	

				tokenVector.push_back(tokenStruct(STRING_CONST, noQuote, lineCount));
				terminalFound = true;				
			}	

			//we dont need to worry about this if we have already found a terminal unit
			if(!terminalFound)
			{	
				//string streams delims can return empty string
				//test for empty string
				for(size_t i = 0; i < temp.length(); i++)
				{
					if(isalnum(temp[i]) || ispunct(temp[i]))
					{
						blank = false;
					}

					//kill the loop if not blank
					if(!blank)
					{
						break;
					}
				}

				//if its not blank and not terminal it must be a unique identifier
				if(!blank)
				{
					//now we must tokenize the unique identifier
					tokenizeIdentifier(temp);
				}
			}
		}		
	}
}

/**********************************************************************************
*Adds % around terminal units
***********************************************************************************/
void Tokenizer::deliminateKeyword()
{
	std::size_t found;

	bool withinString = false;
			
	//check a line for all the keywords
	for(size_t i = 0; i < keywords.size(); i++)
	{	
		found = line.find(keywords[i]);

		//check to see if found token is with a string
		for(size_t i = 0; i < stringBeg.size(); i++)
		{
			if(found >= stringBeg[i] && found <= stringEnd[i])
			{
				withinString = true; //if it is ignore it
			}
		}

		//check to see if mult of same keyword in a line
		while(found != std::string::npos)
		{	
			if(checkKeyword(found, keywords[i]))
			{
				if(!withinString)
				{
					//add the deliminators
					line.insert(found + keywords[i].length(), "%");
					line.insert(found, "%");
				}
			}

			withinString = false;

			//search again
			found = line.find(keywords[i], found + 3);

			for(size_t i = 0; i < stringBeg.size(); i++)
			{
				if(found >= stringBeg[i] && found <= stringEnd[i])
				{
					withinString = true;
				}
			}
		}

		withinString = false;
	}
}

/**********************************************************************************
*Checks to make sure that a keyowrd is not part of variable name
*For example: "Print" contains keyowrd int. We need to make sure keyword is alone
***********************************************************************************/
bool Tokenizer::checkKeyword(size_t i, std::string word)
{

	if(isalnum(line[i - 1]))
	{
		return false;
	}

	if(i + word.length() <= line.length())
	{
		if(isalnum(line[i + word.length()]))
		{
			return false;
		}
	}

	return true;
}

/**********************************************************************************
*Adds % around terminal units
***********************************************************************************/
void Tokenizer::deliminateSymbol()
{
	std::size_t found;
	bool withinString = false;

	//check a line for all symbols 
	for(size_t i = 0; i < symbols.size(); i++)
	{	
		found = line.find(symbols[i]);

		//check to see if symbol is within a string
		for(size_t i = 0; i < stringBeg.size(); i++)
		{
			if(found > stringBeg[i] && found < stringEnd[i])
			{
				withinString = true; //if it is then ignore it
			}
		}

		//check for multiple of same symbols in line
		while(found != std::string::npos)
		{
			if(!withinString)
			{
				//add deliminators
				line.insert(found + symbols[i].length(), "%");
				line.insert(found, "%");
			}

			withinString = false;

			//search again
			found = line.find(symbols[i], found + 3);

			for(size_t i = 0; i < stringBeg.size(); i++)
			{
				if(found > stringBeg[i] && found < stringEnd[i])
				{
					withinString = true;
				}
			}
		}

		withinString = false;
	}
}

/**********************************************************************************
*Adds % around terminal units
***********************************************************************************/
void Tokenizer::deliminateStringConstant()
{
	std::size_t found;

	//check for first quote
	found = line.find("\"");

	if(found != std::string::npos)
	{
		stringBeg.push_back(found + 1); //add value of found to a vector containing the beginning location of a string
	}
	
	//check for multiple strings in line
	while(found != std::string::npos)
	{
		line.insert(found, "%"); //add deliminator after first quotation mark
		found = line.find("\"", found + 2); //search for the second quote mark
		stringEnd.push_back(found); //add value of found to a vector containing the ending location of a string
		line.insert(found + 1, "%"); //insert delim after second quote

		//check to see if there is another quote pair
		if((found + 2) != std::string::npos)
		{
			found = line.find("\"", found + 2);
			
			if(found != std::string::npos)
			{
				stringBeg.push_back(found + 1);
			}
		}
	}
}

/**********************************************************************************
*Adds % around terminal units
***********************************************************************************/
void Tokenizer::tokenizeIdentifier(std::string identChunk)
{
	std::string temp = "";
	std::stringstream check1(identChunk);
	bool intConst = true;

	//break apart the unit by space
	while(getline(check1, temp, ' '))
	{
		if(!(temp.empty()))
		{
			//if unit is only ints then it is a integer constant
			for(size_t i = 0; i < temp.length(); i++)
			{
				if(isalpha(temp[i]))
				{
					intConst = false;
				}
			}

			//if only ints
			if(intConst)
			{
				tokenVector.push_back(tokenStruct(INT_CONST, temp, lineCount));
			}

			//otherwise token is identifier
			else
			{
				tokenVector.push_back(tokenStruct(IDENTIFIER, temp, lineCount));
			}
		}		
	}
}

/**********************************************************************************
*Prints out tokens to a text file if desired
*Add call to main for token print
***********************************************************************************/
void Tokenizer::printTokens()
{
	std::ofstream outputfile;
	outputfile.open("tokens.txt");

	for(size_t i = 0; i < tokenVector.size(); i++)
	{
		outputfile << tokenVector[i].type << " " << tokenVector[i].token << "\r\n";
	}

	outputfile.close();
}

Tokenizer::~Tokenizer()
{
	inputFile.close();
}