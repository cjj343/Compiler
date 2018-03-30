#include "Tokenizer.hpp"


Tokenizer::Tokenizer(std::string inputFileName)
{
	//open the file and test for errors
	openError = false;
	inputFile.open(inputFileName);
	keywords = {"class", "constructor", "function", "method", "field", "static", "var", "int", "char", "boolean", "void", "true", "false", "null", "this", "let", "do", "if", "else", "while"};
	symbols = {"{", "}", "(", ")", "[", "]", ".", ",", ";", "+", "-", "*", "/", "&", "|", "<", ">", "=", "~"};


	if (inputFile.fail())
	{
		openError = true;
	}
}

//used by main to get info on status of file opening
bool Tokenizer::errorCheck()
{
	return openError;
}

//check to see if there are more tokens
bool Tokenizer::hasMoreTokens()
{

}

void Tokenizer::advance()
{

}


void Tokenizer::parse()
{	
	std::size_t found;
	int placeMarker = 0;
	bool multiLineComment;
	bool blankLine;
	bool singleLineComment;

	while(!(inputFile.eof()))
	{
		multiLineComment = false;
		blankLine = false;

		getline(inputFile, line);

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
		for(int i = 0; i < line.length(); i++)
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
			//add delimination for keywrods
			deliminateKeyword();

			//add delimination for symbols
			deliminateSymbol();

			//add delimination for strings
			deliminateStringConstant();			

			//and tokens to vector
			tokenizeLine();
		}				
	}
}

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
			for(int i = 0; i < keywords.size(); i++)
			{
				if (temp == keywords[i])
				{
					tokenVector.push_back(tokenStruct(KEYWORD, temp));
					terminalFound = true;
				}
			}

			//check to see if deliminated unit is symbol
			for(int i = 0; i < symbols.size(); i++)
			{
				if (temp == symbols[i])
				{
					tokenVector.push_back(tokenStruct(SYMBOL, temp));
					terminalFound = true;
				}
			}

			//check to see if deliminated unit is string
			if(temp[0] == '\"')
			{
				tokenVector.push_back(tokenStruct(STRING_CONST, temp));
				terminalFound = true;				
			}	

			//we dont need to worry about this if we have already found a terminal unit
			if(!terminalFound)
			{	
				//string streams delims can return empty string
				//test for empty string
				for(int i = 0; i < temp.length(); i++)
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

//adds % symbol between terminal units
void Tokenizer::deliminateKeyword()
{
	std::size_t found;
			
	//check a line for all the keywords
	for(int i = 0; i < keywords.size(); i++)
	{	
		found = line.find(keywords[i]);

		//check to see if mult of same keyword in a line
		while(found != std::string::npos)
		{	
			//add the deliminators
			line.insert(found + keywords[i].length(), "%");
			line.insert(found, "%");

			//search again
			found = line.find(keywords[i], found + 3);
		}
	}
}

//adds % symbol between terminal units
void Tokenizer::deliminateSymbol()
{
	std::size_t found;

	//check a line for all symbols 
	for(int i = 0; i < symbols.size(); i++)
	{	
		found = line.find(symbols[i]);

		//check for multiple of same symbols in line
		while(found != std::string::npos)
		{
			//add deliminators
			line.insert(found + symbols[i].length(), "%");
			line.insert(found, "%");

			//search again
			found = line.find(symbols[i], found + 3);
		}
	}
}

//adds delims to string constants
void Tokenizer::deliminateStringConstant()
{
	std::size_t found;

	//check for first quote
	found = line.find("\"");
	
	//check for multiple strings in line
	while(found != std::string::npos)
	{
		//add deliminator before first quotation mark
		line.insert(found, "%");

		//search for the second quote mark
		found = line.find("\"", found + 2);

		//insert delim after
		line.insert(found + 1, "%");

		//check to see if there is another quote pair
		found = line.find("\"", found + 1);
	}
}

//tokenize identifier
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
			for(int i = 0; i < temp.length(); i++)
			{
				if(!(isalpha(temp[i])))
				{
					intConst = false;
				}
			}

			//if only ints
			if(intConst)
			{
				tokenVector.push_back(tokenStruct(INT_CONST, temp));
			}

			//otherwise token is identifier
			else
			{
				tokenVector.push_back(tokenStruct(IDENTIFIER, temp));
			}
		}		
	}
}

Tokenizer::~Tokenizer()
{
	inputFile.close();
}
