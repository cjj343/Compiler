#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <sstream>

enum _tokenType {KEYWORD, SYMBOL, IDENTIFIER, INT_CONST, STRING_CONST, DEFAULT};

class Tokenizer
{
	private:
		std::ifstream inputFile;
		bool openError;
		std::string line;
		size_t tokenCount;
		int lineCount;
		std::vector<size_t> stringBeg;
		std::vector<size_t> stringEnd;

		struct tokenStruct
		{	
			_tokenType type;
			std::string token;
			int line;
			
			tokenStruct()
			{
				type = DEFAULT;
				token = "";
				line = 0;

			}

			tokenStruct(_tokenType _type, std::string _token, int _line)
			{
				type = _type;
				token = _token;
				line = _line;
			}
		};

		//holds tokens
		std::vector<tokenStruct> tokenVector;
		//holds key words
		std::vector<std::string> keywords;
		//holds symbols
		std::vector<std::string> symbols;		



	public:	
		Tokenizer(std::string fileName);
		bool errorCheck();
		bool hasMoreTokens();
		void advance();
		void tokenize();
		void deliminateKeyword();
		void deliminateSymbol();
		void deliminateStringConstant();
		void deliminateIntegerConstant();
		void tokenizeLine();
		void tokenizeIdentifier(std::string);
		void printTokens();
		_tokenType tokenType();
		std::string getToken();
		std::string tokenPeek();
		int getLine();
		bool checkKeyword(size_t, std::string);
		~Tokenizer();
};
#endif