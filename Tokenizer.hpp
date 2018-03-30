#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <bits/stdc++.h>
#include <cctype>

enum _tokenType {KEYWORD, SYMBOL, IDENTIFIER, INT_CONST, STRING_CONST, DEFAULT};

class Tokenizer
{
	private:
		std::ifstream inputFile;
		bool openError;
		std::string line;		

		struct tokenStruct
		{	
			_tokenType type;
			std::string token;
			
			tokenStruct()
			{
				type = DEFAULT;
				token = "";

			}

			tokenStruct(_tokenType _type, std::string _token)
			{
				type = _type;
				token = _token;
			}
		};

		std::vector<tokenStruct> tokenVector;
		//these all the possible key words
		std::vector<std::string> keywords;
		//all the possible symbols
		std::vector<std::string> symbols;
	


	public:	
		Tokenizer(std::string fileName);
		bool errorCheck();
		bool hasMoreTokens();
		void advance();
		void parse();
		void deliminateKeyword();
		void deliminateSymbol();
		void deliminateStringConstant();
		void deliminateIntegerConstant();
		//void parseIdentifier();
		void tokenizeLine();
		void tokenizeIdentifier(std::string);
		_tokenType tokenType();
		~Tokenizer();

};



#endif