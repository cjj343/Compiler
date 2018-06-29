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
	
	outputFile << "<class>\r\n";
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
	outputFile << printTabs() << "<classVarDec>\r\n";
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
	outputFile << printTabs() << "</classVarDec>\r\n";
}

void CompilationEngine::compileSubroutine(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<subroutineDec>\r\n";
	incrementTabs();

		//constructor, function or method
		safeAdvance(tptr, KEYWORD);
		writeKeyword(tptr->getToken());

		//if the subroutine is a constructor, next token should be the name of the class
		if(tptr->getToken() == "constructor")
		{
			//name of class
			safeAdvance(tptr, IDENTIFIER);
			writeIdentifier(tptr->getToken());
		}

		//for method or function next token should be void or TYPE
		else
		{
			//void or type
			safeAdvance(tptr, KEYWORD);
			writeKeyword(tptr->getToken());
		}

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
		
		
		outputFile << printTabs() << "<subroutineBody>\r\n";
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
		outputFile << printTabs() << "</subroutineBody>\r\n";

	decrementTabs();
	outputFile << printTabs() << "</subroutineDec>\r\n";
	

}

void CompilationEngine::compileStatements(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<statements>";
	

	//if there are no statements for some reason
	if(tptr->tokenPeek() == "}")
	{
		outputFile << "\r\n</statements>\r\n";
		return;
	}

	else
	{
		incrementTabs();

		outputFile << "\r\n";

		while(tptr->tokenPeek() != "}")
		{	

			safeAdvanceNoTypeCheck(tptr);

			if(tptr->getToken() == "let")
			{
				compileLet(tptr);
			}

			else if(tptr->getToken() == "if")
			{
				compileIf(tptr);
			}

			else if(tptr->getToken() == "while")
			{
				compileWhile(tptr);	
			}

			else if(tptr->getToken() == "do")
			{
				compileDo(tptr);	
			}

			else if(tptr->getToken() == "return")
			{
				compileReturn(tptr);
			}
									
		}

			decrementTabs();
		outputFile << printTabs() << " </statements>\r\n";
	}

}

void CompilationEngine::compileLet(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<letStatement>\r\n";
	incrementTabs();

		//should be let
		//safeAdvance(tptr, KEYWORD);
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
			//bool is to distinguish between unary term and otherwise
			compileExpression(tptr, false);

			// ]
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

			// =
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

			//the stuff after =
			//bool is to distinguish between unary term and otherwise
			compileExpression(tptr, false);

		}

		else if(tptr->tokenPeek() == "=")
		{
			// =
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

			//the stuff after =
			compileExpression(tptr, false);
		}

		// ;
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		decrementTabs();
	outputFile << printTabs() << "</letStatement>\r\n";
}

void CompilationEngine::compileIf(std::unique_ptr<Tokenizer> & tptr)
{

	outputFile << printTabs() << "<ifStatement>\r\n";
	incrementTabs();

		//should be if
		//safeAdvance(tptr, KEYWORD);
		writeKeyword(tptr->getToken());

		//(
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		//the stuff in between ()
		//bool is to distinguish between unary term and otherwise
		compileExpression(tptr, false);

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

			//{
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

			compileStatements(tptr);

			//}
			safeAdvance(tptr, SYMBOL);
			writeSymbol(tptr->getToken());

		}

			decrementTabs();
	outputFile << printTabs() << "</ifStatement>\r\n";
}

void CompilationEngine::compileWhile(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<whileStatement>\r\n";
	incrementTabs();

		//should be while
		//safeAdvance(tptr, KEYWORD);
		writeKeyword(tptr->getToken());

		//(
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		//the stuff in between ()
		//bool is to distinguish between unary term and otherwise
		compileExpression(tptr, false);

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
	outputFile << printTabs() << "</whileStatement>\r\n";
}

void CompilationEngine::compileDo(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<doStatement>\r\n";
	incrementTabs();

		//should be do
		//safeAdvance(tptr, KEYWORD);
		writeKeyword(tptr->getToken());

		//should be identifier
		safeAdvance(tptr, IDENTIFIER);
		writeIdentifier(tptr->getToken());

		subroutineCall(tptr);

		//;
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		decrementTabs();
	outputFile << printTabs() << "</doStatement>\r\n";
}

void CompilationEngine::compileReturn(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<returnStatement>\r\n";
		incrementTabs();

		//should be return
		//safeAdvance(tptr, KEYWORD);
		writeKeyword(tptr->getToken());

		if(tptr->tokenPeek() != ";")
		{
			//bool is to distinguish between unary term and otherwise
			compileExpression(tptr, false);
		}

		//;
		safeAdvance(tptr, SYMBOL);
		writeSymbol(tptr->getToken());

		decrementTabs();
	outputFile << printTabs() << "</returnStatement>\r\n";	

}

void CompilationEngine::compileExpression(std::unique_ptr<Tokenizer> & tptr, bool unaryTerm)
{

	outputFile << printTabs() << "<expression>\r\n";
	incrementTabs();

	//loop until we encounter the end of the expression
	while(tptr->tokenPeek() != "]" && tptr->tokenPeek() != ")" && tptr->tokenPeek() != "," && tptr->tokenPeek() != ";")
	{

		//operators
		if((tptr->tokenPeek() == "+" || tptr->tokenPeek() == "-" || tptr->tokenPeek() == "*" || tptr->tokenPeek() == "/" || tptr->tokenPeek() == "&" || tptr->tokenPeek() == "|" || 
			tptr->tokenPeek() == "<" || tptr->tokenPeek() == ">" || tptr->tokenPeek() == "=") && !unaryTerm)
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
	outputFile << printTabs() << "</expression>\r\n";

}

void CompilationEngine::compileTerm(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<term>\r\n";
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
			writeIdentifier(tptr->getToken());

			//call back to expression if array index in term
			if(tptr->tokenPeek() == "[")
			{
				// [
				safeAdvance(tptr, SYMBOL);
				writeSymbol(tptr->getToken());

				//the stuff in between []
				//bool is to distinguish between unary term and otherwise
				compileExpression(tptr, false);

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
			if(tptr->getToken() == "(")
			{
				// (, already advanced at beginning of function
				writeSymbol(tptr->getToken());

				if(tptr->tokenPeek() == "-" || tptr->tokenPeek() == "~")
				{
					//the stuff in between ()
					//bool is to distinguish between unary term and otherwise
					compileExpression(tptr, true);
				}

				else
				{
					compileExpression(tptr, false);
				}

				// )
				safeAdvance(tptr, SYMBOL);
				writeSymbol(tptr->getToken());
			}

			//unary ops
			else if(tptr->getToken() == "-" || tptr->getToken() == "~")
			{
				// unary operator, already advanced at beginning of function
				writeSymbol(tptr->getToken());


					compileTerm(tptr);
			}
		}

	decrementTabs();
	outputFile << printTabs() << "</term>\r\n";
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

		//bool is to distinguish between unary term and otherwise
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
	outputFile << printTabs() << "<expressionList>";
	
	if(tptr->tokenPeek() == ")")
	{
		outputFile << "\r\n" << printTabs() << "</expressionList>\r\n";
	}

	else
	{
		outputFile << "\r\n";
		incrementTabs();

		//loop until end of expression list reached
		while(tptr->tokenPeek() != ")" && tptr->tokenPeek() != ";")
		{

			//bool is to distinguish between unary term and otherwise
			compileExpression(tptr, false);

			//print the comma if there are multiple expressions
			if(tptr->tokenPeek() == ",")
			{
				safeAdvance(tptr, SYMBOL);
				writeSymbol(tptr->getToken());
			}
		}


		decrementTabs();
		outputFile << printTabs() << " </expressionList>\r\n";
	}


}

void CompilationEngine::compileParameterList(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<parameterList>";

	if(tptr->tokenPeek() == ")")
	{
		outputFile << "\r\n" << printTabs() << "</parameterList>\r\n";
		return;
	}

	else
	{
		outputFile << "\r\n";

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
		outputFile << printTabs() << "</parameterList>\r\n";
	}
}

void CompilationEngine::compileVarDec(std::unique_ptr<Tokenizer> & tptr)
{
	outputFile << printTabs() << "<varDec>\r\n";
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
	outputFile << printTabs() << "</varDec>\r\n";

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
	outputFile << printTabs() << "<keyword> " << tok << " </keyword>\r\n";
}

void CompilationEngine::writeSymbol(std::string tok)
{
	//In order to display correctly in HTML <,>,& need a unique representation
	if(tok == "<")
	{
		outputFile << printTabs() << "<symbol> " << "&lt;" << " </symbol>\r\n";
	}

	else if(tok == ">")
	{
		outputFile << printTabs() << "<symbol> " << "&gt;" << " </symbol>\r\n";
	}
	
	else if(tok == "&")
	{
		outputFile << printTabs() << "<symbol> " << "&amp;" << " </symbol>\r\n";
	}

	else
	{
		outputFile << printTabs() << "<symbol> " << tok << " </symbol>\r\n";
	}
}

void CompilationEngine::writeIdentifier(std::string tok)
{
	outputFile << printTabs() << "<identifier> " << tok << " </identifier>\r\n";
}

void CompilationEngine::writeIntConst(std::string tok)
{
	outputFile << printTabs() << "<integerConstant> " << tok << " </integerConstant>\r\n";
}

void CompilationEngine::writeStringConst(std::string tok)
{
	outputFile << printTabs() << "<stringConstant> " << tok << " </stringConstant>\r\n";
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