#include "CompilationEngine.hpp"
#include "Tokenizer.hpp"
#include <assert.h>
#include <iostream>
#include <string>

CompilationEngine::CompilationEngine(std::string fileName)
{
	size_t pos = fileName.find('.');
	//size_t end = fileName.end();
	fileName.erase(pos);
	fileName += ".xml";
	outputFile.open(fileName);
	openError = false;
	tabCount = 0;

	if (outputFile.fail())
	{
		openError = true;
	}
}

void CompilationEngine::compileClass(std::unique_ptr<Tokenizer> & tptr)
{
	std::string curTok;
	
	outputFile << "<class>" << std::endl;
	incrementTabs();

		//class
		assert(tptr->hasMoreTokens());
		assert(tptr->getToken() == "class");
		writeKeyword(tptr->getToken());
		
		//class name
		safeAdvance(tptr, IDENTIFIER);
		writeIdentifier(tptr->getToken());	
		
		//{
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());
		
		//if there are static or global variables for the class
		while(tptr->tokenPeek() == "static" || tptr->tokenPeek() == "field")
		{
			compileClassVarDec(tptr);
		}

		while(tptr->tokenPeek() == "function" || tptr->tokenPeek() == "constructor" || tptr->tokenPeek() == "method")
		{
			compileSubroutine(tptr);
		}

		//}
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

	decrementTabs();
	outputFile << printTabs() << "</class>" << std::endl;
	

}

void CompilationEngine::compileClassVarDec(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<classVarDev>" << std::endl;
	incrementTabs();

		//static or field
		safeAdvance(tptr, KEYWORD);
		writeKeyword(tptr->getToken());

		//check for type or class name
		safeAdvanceNoTypeCheck(tptr);

		if(tptr->tokenType() == KEYWORD)
		{
			writeKeyword(tptr->getToken());
		}

		else if(tptr->tokenType() == IDENTIFIER)
		{
			writeIdentifier(tptr->getToken());
		}

		//first variable name
		safeAdvance(tptr, IDENTIFIER);
		writeIdentifier(tptr->getToken());

		//if multiple variables are declared on the same line
		while(tptr->tokenPeek() == ",")
		{
			//,
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

			//subsequent variable names
			safeAdvance(tptr, IDENTIFIER);
			writeIdentifier(tptr->getToken());
		}

		//;	
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());
	
		decrementTabs();
	outputFile << printTabs() << "</classVarDev>" << std::endl;
}

void CompilationEngine::compileSubroutine(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<subroutineDec>" << std::endl;
	incrementTabs();

		//constructor, function or method
		safeAdvance(tptr, KEYWORD);
		writeKeyword(tptr->getToken());

		//void or type
		safeAdvance(tptr, KEYWORD);
		writeKeyword(tptr->getToken());

		//name of function, constructor or method
		safeAdvance(tptr, IDENTIFIER);
		writeIdentifier(tptr->getToken());

		//(
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		compileParameterList(tptr);
		
		//)	
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());
		
		
		outputFile << printTabs() << "<subroutineBody>" << std::endl;
		incrementTabs();

			//{
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

			//variable decs
			while(tptr->tokenPeek() == "var")
			{
				compileVarDec(tptr);
			}

			compileStatements(tptr);

			//}
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

		decrementTabs();
		outputFile << printTabs() << "</subroutineBody>" << std::endl;

	decrementTabs();
	outputFile << printTabs() << "</subroutineDec>" << std::endl;

}

void CompilationEngine::compileStatements(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<statements>" << std::endl;
			incrementTabs();

	//if there are no statements for some reason
	if(tptr->tokenPeek() == "}")
	{
			decrementTabs();
		outputFile << " </statements>" << std::endl;
	}

	else
	{
		while(tptr->tokenPeek() == "}")
		{	
			if(tptr->tokenPeek() == "let")
			{
				compileLet(tptr);
			}

			else if(tptr->tokenPeek() == "if")
			{
				compileIf(tptr);
			}

			else if(tptr->tokenPeek() == "while")
			{
				compileWhile(tptr);	
			}

			else if(tptr->tokenPeek() == "do")
			{
				compileDo(tptr);	
			}

			else if(tptr->tokenPeek() == "return")
			{
				compileReturn(tptr);
			}									
		}

			decrementTabs();
		outputFile << " </statements>" << std::endl;
	}

}

void CompilationEngine::compileLet(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<letStatement>" << std::endl;
	incrementTabs();

		//should be let
		safeAdvance(tptr, KEYWORD);
		writeKeyword(tptr->getToken());

		//should be identifier
		safeAdvance(tptr, IDENTIFIER);
		writeIdentifier(tptr->getToken());

		//must be a symbol, can be = or [
		if(tptr->tokenPeek() == "[")
		{
			// [
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

			//the stuff in between []
			compileExpression(tptr);

			// ]
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

			// =
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

			//the stuff after =
			compileExpression(tptr);

		}

		else if(tptr->tokenPeek() == "=")
		{
			// =
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

			//the stuff after =
			compileExpression(tptr);
		}

		// ;
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		decrementTabs();
	outputFile << printTabs() << "</letStatement>" << std::endl;
}

void CompilationEngine::compileIf(std::unique_ptr<Tokenizer> & tptr)
{

	outputFile << printTabs() << "<ifStatement>" << std::endl;
	incrementTabs();

		//should be if
		safeAdvance(tptr, KEYWORD);
		writeKeyword(tptr->getToken());

		//(
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		//the stuff in between ()
		compileExpression(tptr);

		//)
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		//{
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		compileStatements(tptr);

		//}
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		if(tptr->tokenPeek() == "else")
		{
			//should be else
			safeAdvance(tptr, KEYWORD);
			writeKeyword(tptr->getToken());

			//(
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

			//the stuff in between ()
			compileExpression(tptr);

			//)
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

			//{
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

			compileStatements(tptr);

			//}
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

		}

			decrementTabs();
	outputFile << printTabs() << "</ifStatement>" << std::endl;
}

void CompilationEngine::compileWhile(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<whileStatement>" << std::endl;
	incrementTabs();

		//should be while
		safeAdvance(tptr, KEYWORD);
		writeKeyword(tptr->getToken());

		//(
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		//the stuff in between ()
		compileExpression(tptr);

		//)
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		//{
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		compileStatements(tptr);

		//}
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		decrementTabs();
	outputFile << printTabs() << "</whileStatement>" << std::endl;
}

void CompilationEngine::compileDo(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<doStatement>" << std::endl;
	incrementTabs();

		subroutineCall(tptr);

		//;
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		decrementTabs();
	outputFile << printTabs() << "</doStatement>" << std::endl;
}

void CompilationEngine::compileReturn(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<returnStatement>" << std::endl;
		incrementTabs();

		//should be return
		safeAdvance(tptr, KEYWORD);
		writeKeyword(tptr->getToken());

		compileExpression(tptr);

		//;
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		decrementTabs();
	outputFile << printTabs() << "</returnStatement>" << std::endl;	

}

void CompilationEngine::compileExpression(std::unique_ptr<Tokenizer> & tptr)
{

	outputFile << printTabs() << "<expression>" << std::endl;
	incrementTabs();

	//loop until we encounter the end of the expression
	while(tptr->tokenPeek() != "]" && tptr->tokenPeek() != ")" && tptr->tokenPeek() != "," && tptr->tokenPeek() != ";")
	{
		//operators
		if(tptr->tokenPeek() == "+" || tptr->tokenPeek() == "-" || tptr->tokenPeek() == "*" || tptr->tokenPeek() == "/" || tptr->tokenPeek() == "&" || tptr->tokenPeek() == "|" || 
			tptr->tokenPeek() == "<" || tptr->tokenPeek() == ">" || tptr->tokenPeek() == "=")
		{
			//operator
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());
		}

		else
		{
			compileTerm(tptr);
		}
	}

	decrementTabs();
	outputFile << printTabs() << "</expression>" << std::endl;

}

void CompilationEngine::compileTerm(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<term>" << std::endl;
	incrementTabs();

		//get the next token without pre determined type
		safeAdvanceNoTypeCheck(tptr);

		//term is int constant
		if(tptr->tokenType() == INT_CONST)
		{
			writeIntConst(tptr->getToken());
		}

		//keyword including constant keywords
		else if(tptr->tokenType() == KEYWORD)
		{
			writeKeyword(tptr->getToken());
		}

		//string
		else if(tptr->tokenType() == STRING_CONST)
		{
			writeStringConst(tptr->getToken());
		}

		//variables and subroutine call
		else if(tptr->tokenType() == IDENTIFIER)
		{
			//unique name
			safeAdvance(tptr, IDENTIFIER);
			writeIdentifier(tptr->getToken());

			//call back to expression if array index in term
			if(tptr->tokenPeek() == "[")
			{
				// [
				safeAdvance(tptr, SYMBOL);
				writeSymbol(tptr->getToken());

				//the stuff in between []
				compileExpression(tptr);

				// ]
				safeAdvance(tptr, SYMBOL);
				writeSymbol(tptr->getToken());
			}

			//for subroutine calls or accessing member of class
			else if(tptr->tokenPeek() == "(" || tptr->tokenPeek() == ".")
			{
				subroutineCall(tptr);
			}
		}

		//checking for further expressions or unary ops
		else if (tptr->tokenType() == SYMBOL)
		{
			//another expression
			if(tptr->tokenPeek() == "(")
			{
				// (
				safeAdvance(tptr, SYMBOL);
				writeSymbol(tptr->getToken());

				//the stuff in between ()
				compileExpression(tptr);

				// )
				safeAdvance(tptr, SYMBOL);
				writeSymbol(tptr->getToken());
			}

			//unary ops
			else if(tptr->tokenPeek() == "-" || tptr->tokenPeek() == "~")
			{
				// unary operator
				safeAdvance(tptr, SYMBOL);
				writeSymbol(tptr->getToken());
			}
		}

	decrementTabs();
	outputFile << printTabs() << "</term>" << std::endl;
}

void CompilationEngine::subroutineCall(std::unique_ptr<Tokenizer> & tptr)
{
	//subroutine name has already been printed in calling function
	//print the parens and call expression list
	if(tptr->tokenPeek() == "(")
	{
		// (
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		compileExpressionList(tptr);

		//)
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());
	}

	//class name or var name has already been printed in calling function
	//print the dot operator and then the sub
	else if(tptr->tokenPeek() == ".")
	{
		//.
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		//subroutineName
		safeAdvance(tptr, IDENTIFIER);
		writeIdentifier(tptr->getToken());

		// (
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		compileExpressionList(tptr);

		//)
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());
	}

}

void CompilationEngine::compileExpressionList(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<expressionList>" << std::endl;
	incrementTabs();

	//loop until end of expression list reached
	while(tptr->tokenPeek() != ")")
	{
		compileExpression(tptr);

		//print the comma if there are multiple expressions
		if(tptr->tokenPeek() == ",")
		{
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());
		}
	}

	decrementTabs();
	outputFile << printTabs() << "</parameterList>" << std::endl;
}

void CompilationEngine::compileParameterList(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<parameterList>";

	if(tptr->tokenPeek() == ")")
	{
		outputFile << " </parameterList>" << std::endl;
	}

	else
	{
		outputFile << std::endl;

		incrementTabs();

		while(tptr->tokenPeek() != ")")
		{
			//type
			safeAdvance(tptr, KEYWORD);
			writeKeyword(tptr->getToken());
			
			//var name
			safeAdvance(tptr, IDENTIFIER);
			writeIdentifier(tptr->getToken());

			//if more than one parameter
			if(tptr->tokenPeek() != ")")
			{	
				//,
				safeAdvance(tptr, SYMBOL);
				writeSymbol(tptr->getToken());
			}
		}

		decrementTabs();
		outputFile << printTabs() << "</parameterList>" << std::endl;
	}
}

void CompilationEngine::compileVarDec(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<varDec>" << std::endl;
	incrementTabs();

		//First token should be the key word var 
		safeAdvance(tptr, KEYWORD);
		writeKeyword(tptr->getToken());

		//next token can either be a keyword like int or an indentifier like a class name
		//therefore we need to advance without specifing a type
		safeAdvanceNoTypeCheck(tptr);

			if(tptr->tokenType() == KEYWORD)
			{
				writeKeyword(tptr->getToken());
			}

			else if(tptr->tokenType() == IDENTIFIER)
			{
				writeIdentifier(tptr->getToken());
			}

		//next token should always be a unique identifier
		safeAdvance(tptr, IDENTIFIER);
		writeIdentifier(tptr->getToken());

		//if multiple variables are declared on the same line
		while(tptr->tokenPeek() == ",")
		{
			//,
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

			safeAdvance(tptr, IDENTIFIER);
			writeIdentifier(tptr->getToken());

		}
	
		//should be ;
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		decrementTabs();
	outputFile << printTabs() << "</varDec>" << std::endl;

}

void CompilationEngine::incrementTabs()
{
	tabCount++;
}

void CompilationEngine::decrementTabs()
{
	tabCount--;
}

std::string CompilationEngine::printTabs()
{
	std::string temp;

	for(int i = 0; i < tabCount; i++)
	{
		temp += "\t";
	}

	return temp;
}

//used by main to get info on status of file opening
bool CompilationEngine::errorCheck()
{
	return openError;
}

void CompilationEngine::writeKeyword(std::string tok)
{
	outputFile << printTabs() << "<keyword> " << tok << " </keyword>" << std::endl;

}

void CompilationEngine::writeSymbol(std::string tok)
{
	outputFile << printTabs() << "<symbol> " << tok << " </symbol>" << std::endl;
}

void CompilationEngine::writeIdentifier(std::string tok)
{
	outputFile << printTabs() << "<indetifier> " << tok << " </indetifier>" << std::endl;
}

void CompilationEngine::writeIntConst(std::string tok)
{
	outputFile << printTabs() << "<integerConstant> " << tok << " </integerConstant>" << std::endl;
}

void CompilationEngine::writeStringConst(std::string tok)
{
	outputFile << printTabs() << "<stringConstant> " << tok << " </stringConstant>" << std::endl;
}

void CompilationEngine::safeAdvance(std::unique_ptr<Tokenizer> & tptr, _tokenType type)
{
	std::string exceptionString;

	try
	{
		if(!tptr->hasMoreTokens())
		{
			//Should read: Error at line X: expected a SYMBOL, KEYWORD or IDENTIFIER
			exceptionString = "Error at line " + std::to_string(tptr->getLine());
			exceptionString += ": expected TOKEN";

			throw std::string(exceptionString);
		}
	}

	catch(std::string exceptionString)
	{
		std::cout << exceptionString << std::endl;
		std::exit(1);
	}

	tptr->advance();
	
	try
	{
		if(tptr->tokenType() != type)
		{
			//Should read: Error at line X: expected a SYMBOL, KEYWORD or IDENTIFIER
			exceptionString = "Error at line " + std::to_string(tptr->getLine());
			exceptionString += ": expected ";

			if(type == KEYWORD)
			{
				exceptionString += "KEYWORD";
			}

			else if(type == IDENTIFIER)
			{
				exceptionString += "IDENTIFIER";
			}

			else if(type == SYMBOL)
			{
				exceptionString += "SYMBOL";
			}


			throw std::string(exceptionString);
		}
	}

	catch(std::string exceptionString)
	{
		std::cout << exceptionString << std::endl;
		std::exit(1);
	}

}

void CompilationEngine::safeAdvanceNoTypeCheck(std::unique_ptr<Tokenizer> & tptr)
{
	std::string exceptionString;

	try
	{
		if(!tptr->hasMoreTokens())
		{
			//Should read: Error at line X: expected a SYMBOL, KEYWORD or IDENTIFIER
			exceptionString = "Error at line " + std::to_string(tptr->getLine());
			exceptionString += ": expected TOKEN";

			throw std::string(exceptionString);
		}
	}

	catch(std::string exceptionString)
	{
		std::cout << exceptionString << std::endl;
		std::exit(1);
	}

	tptr->advance();
}

CompilationEngine::~CompilationEngine()
{

	outputFile.close();
}