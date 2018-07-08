#include "CompilationEngine.hpp"
#include <assert.h>
#include <iostream>

/*******************************************************
*Constructor
*Sets label number (used for distinguising loops) to 0
*******************************************************/
CompilationEngine::CompilationEngine()
{
	labelNum = 0;
}

/*******************************************************
*Params: 
	- pointer to tokenizer
	- pointer to vm writer
	- pointer to symbol table
*Starts recursive descent compilation
*******************************************************/
void CompilationEngine::compileClass(std::unique_ptr<Tokenizer> & tptr, std::unique_ptr<VMWriter> & vmPtr, std::unique_ptr<Symbol_Table> & stPtr)
{					
		assert(tptr->hasMoreTokens()); //prime the compiler by asserting first token exists and is class. Jack must start this way
		assert(tptr->getToken() == "class"); //class
		safeAdvance(tptr, IDENTIFIER); //class name
		vmPtr->setClassName(tptr->getToken()); //set class name to private variable in vm writer so it can be used later
		safeAdvance(tptr, SYMBOL); //{
		
		//if there are static or field variables for the class
		while(tptr->tokenPeek() == "static" || tptr->tokenPeek() == "field")
		{
			compileClassVarDec(tptr, stPtr);
		}

		//looping through all the subroutines
		while(tptr->tokenPeek() == "function" || tptr->tokenPeek() == "constructor" || tptr->tokenPeek() == "method")
		{
			compileSubroutine(tptr, vmPtr, stPtr);
		}

		safeAdvance(tptr, SYMBOL); //}
}

/*******************************************************
*Params: 
	- pointer to tokenizer
	- pointer to symbol table
*Adds all field and static variables to symbol table
*******************************************************/
void CompilationEngine::compileClassVarDec(std::unique_ptr<Tokenizer> & tptr, std::unique_ptr<Symbol_Table> & stPtr)
{
	_kind kind;
	std::string name;
	std::string type;

	safeAdvance(tptr, KEYWORD); //static or field, save in temp variable
	kind = tokenToKind(tptr->getToken()); //save for later
	safeAdvanceNoTypeCheck(tptr); //check for type or class name, save in temp variable
	type = tptr->getToken();
	safeAdvance(tptr, IDENTIFIER); //first variable name
	name = tptr->getToken(); //get the variable name 
	stPtr->define(name, type, kind); //send it to the symbol table to be defined
	
	//if multiple variables are declared on the same line
	while(tptr->tokenPeek() == ",")
	{
		safeAdvance(tptr, SYMBOL); //,
		safeAdvance(tptr, IDENTIFIER); //subsequent variable names
		name = tptr->getToken(); //get the variable name
		stPtr->define(name, type, kind); //send it to the symbol table to be defined
	}

	safeAdvance(tptr, SYMBOL); //;	
}

/*******************************************************
*Params: 
	- pointer to tokenizer
	- pointer to vm writer
	- pointer to symbol table
*Starts recursive descent compilation
*******************************************************/
void CompilationEngine::compileSubroutine(std::unique_ptr<Tokenizer> & tptr, std::unique_ptr<VMWriter> & vmPtr, std::unique_ptr<Symbol_Table> & stPtr)
{
	std::string subType;
	std::string name;

	stPtr->startSubroutine(); //clears the symbol table for a new subroutine. This is what creats local scope
	safeAdvance(tptr, KEYWORD); //constructor, function or method
	subType = tptr->getToken(); //save the subroutine type
	safeAdvanceNoTypeCheck(tptr); //next advance will be class name if constructor or return type
	safeAdvance(tptr, IDENTIFIER); //name of function, constructor or method
	name = tptr->getToken(); //save the function name
	safeAdvance(tptr, SYMBOL); //(
	compileParameterList(tptr, stPtr, subType); //compile the parameters
	safeAdvance(tptr, SYMBOL); //)
	safeAdvance(tptr, SYMBOL); //{

	//variable decs, loop until all variables are declared
	//jack must declare variables at start of function
	while(tptr->tokenPeek() == "var")
	{
		compileVarDec(tptr, stPtr);
	}

	//send the subroutine type, name and arg count to writer. Ex: "function Main.foo 2"
	if(subType == "function")
	{
		vmPtr->writeFunction(subType, name, stPtr->variableCount(VAR));
	}

	//if constructor we must allocate space for the new object and set "this" to base
	else if(subType == "constructor")
	{
		vmPtr->writeFunction(subType, name, stPtr->variableCount(VAR));
		vmPtr->writePush(_CONST, stPtr->variableCount(FIELD)); //allocate space
		vmPtr->writeCall("Memory.alloc", 1); //hard coded OS function, accepts 1 arg
		vmPtr->writePop(_POINTER, 0); //set this to base		
	}

	//if method we must set the base of "this" segment
	else if(subType == "method")
	{
		vmPtr->writeFunction(subType, name, stPtr->variableCount(VAR));
		vmPtr->writePush(_ARG, 0);
		vmPtr->writePop(_POINTER, 0);
	}

	compileStatements(tptr, vmPtr, stPtr); //after compiling variables move on to body of function
	safeAdvance(tptr, SYMBOL); //}
}

/*******************************************************
*Params: 
	- pointer to tokenizer
	- pointer to vm writer
	- pointer to symbol table
*Simple loop to process each statment within function
*******************************************************/
void CompilationEngine::compileStatements(std::unique_ptr<Tokenizer> & tptr, std::unique_ptr<VMWriter> & vmPtr, std::unique_ptr<Symbol_Table> & stPtr)
{
	//loop until we reach the end of a function block
	while(tptr->tokenPeek() != "}")
	{	
		safeAdvanceNoTypeCheck(tptr);
		
		//switch for the different types of statements in jack
		//all statements must begin with one of these keywords
		if(tptr->getToken() == "let")
		{
			compileLet(tptr, vmPtr, stPtr);
		}

		else if(tptr->getToken() == "if")
		{
			compileIf(tptr, vmPtr, stPtr);
		}

		else if(tptr->getToken() == "while")
		{
			compileWhile(tptr, vmPtr, stPtr);
		}

		else if(tptr->getToken() == "do")
		{
			compileDo(tptr, vmPtr, stPtr);
		}

		else if(tptr->getToken() == "return")
		{
			compileReturn(tptr, vmPtr, stPtr);
		}					
	}
}

/*******************************************************
*Params: 
	- pointer to tokenizer
	- pointer to vm writer
	- pointer to symbol table
*Compiles let statement
*******************************************************/
void CompilationEngine::compileLet(std::unique_ptr<Tokenizer> & tptr, std::unique_ptr<VMWriter> & vmPtr, std::unique_ptr<Symbol_Table> & stPtr)
{
	std::string varName;

	safeAdvance(tptr, IDENTIFIER); //should be identifier
	varName = tptr->getToken();

	//must be a symbol, can be = or [
	if(tptr->tokenPeek() == "[")
	{	
		safeAdvance(tptr, SYMBOL); // [
		compileExpression(tptr, vmPtr, stPtr); //the stuff in between []
		vmPtr->writePush(kindToSeg(stPtr->kindOf(varName)), stPtr->indexOf(varName)); //push the base of the array
		vmPtr->writeArithmetic(ADD); //add the base of array and the evaluated expression to get address

		//safety net in case compile expression didnt process enought tokens
		if(tptr->tokenPeek() == "]")
		{
			safeAdvance(tptr, SYMBOL);
		}

		safeAdvance(tptr, SYMBOL); // =
		compileExpression(tptr, vmPtr, stPtr); //the stuff after =
		vmPtr->writePop(_TEMP, 0); //put the evaluated expression in correct address
		vmPtr->writePop(_POINTER, 1); //''
		vmPtr->writePush(_TEMP, 0); //''
		vmPtr->writePop(_THAT, 0); //''
	}

	else if(tptr->tokenPeek() == "=")
	{
		safeAdvance(tptr, SYMBOL); // =
		compileExpression(tptr, vmPtr, stPtr); //the stuff after =
		vmPtr->writePop(kindToSeg(stPtr->kindOf(varName)), stPtr->indexOf(varName)); //put the evaluate expression in correct memory location
	}

	//safety net in case compile expression didnt process enought tokens
	if(tptr->tokenPeek() == ";")
	{
		safeAdvance(tptr, SYMBOL);
	}
}

/*******************************************************
*Params: 
	- pointer to tokenizer
	- pointer to vm writer
	- pointer to symbol table
*Compiles if statement
*******************************************************/
void CompilationEngine::compileIf(std::unique_ptr<Tokenizer> & tptr, std::unique_ptr<VMWriter> & vmPtr, std::unique_ptr<Symbol_Table> & stPtr)
{
		std::string ifTrue = "IF_TRUE_";
		std::string ifFalse = "IF_FALSE_";
		ifTrue += std::to_string(labelNum);
		ifFalse += std::to_string(labelNum);
		labelNum++; //label names are the same except for an incremental counter that distinguishes them

		safeAdvance(tptr, SYMBOL); //(
		compileExpression(tptr, vmPtr, stPtr); //the stuff in between ()
		vmPtr->writeArithmetic(tokenToCommand("~", true)); //for conditionals we negate the evaluated expression to make vm code easier to write
		vmPtr->writeIf(ifTrue); //if true go to else or bypass
		safeAdvance(tptr, SYMBOL); //{
		compileStatements(tptr, vmPtr, stPtr); //compile the statements with the conditional
		vmPtr->writeGoto(ifFalse); //by pass else block		
		safeAdvance(tptr, SYMBOL); //}

		//conditional can if-else, or just if in jack
		if(tptr->tokenPeek() == "else")
		{
			safeAdvance(tptr, KEYWORD); //should be else
			vmPtr->writeLabel(ifTrue); //else label
			safeAdvance(tptr, SYMBOL); //{
			compileStatements(tptr, vmPtr, stPtr); //compile the statements with the conditional
			safeAdvance(tptr, SYMBOL); //}
		}

		else if(tptr->tokenPeek() != "else")
		{
			vmPtr->writeLabel(ifTrue); //by pass if statements			
		}

		vmPtr->writeLabel(ifFalse);
}

/*******************************************************
*Params: 
	- pointer to tokenizer
	- pointer to vm writer
	- pointer to symbol table
*Compiles while statement
*******************************************************/
void CompilationEngine::compileWhile(std::unique_ptr<Tokenizer> & tptr, std::unique_ptr<VMWriter> & vmPtr, std::unique_ptr<Symbol_Table> & stPtr)
{
		std::string whileTrue = "WHILE_TRUE_";
		std::string whileFalse = "WHILE_FALSE_";
		whileTrue += std::to_string(labelNum);
		whileFalse += std::to_string(labelNum);
		labelNum++;
		
		vmPtr->writeLabel(whileTrue); //go here why condition is true
		safeAdvance(tptr, SYMBOL); //(
		compileExpression(tptr, vmPtr, stPtr); //the stuff in between ()
		vmPtr->writeArithmetic(tokenToCommand("~", true)); //for conditionals we negate the evaluated expression to make vm code easier to write
		vmPtr->writeIf(whileFalse); //exit loop
		safeAdvance(tptr, SYMBOL); //{
		compileStatements(tptr, vmPtr, stPtr); //compile the statements within the conditional
		vmPtr->writeGoto(whileTrue); //go back through loop
		vmPtr->writeLabel(whileFalse); //go here once loop broken
		safeAdvance(tptr, SYMBOL); //}
}

/*******************************************************
*Params: 
	- pointer to tokenizer
	- pointer to vm writer
	- pointer to symbol table
*Compiles do statement
*******************************************************/
void CompilationEngine::compileDo(std::unique_ptr<Tokenizer> & tptr, std::unique_ptr<VMWriter> & vmPtr, std::unique_ptr<Symbol_Table> & stPtr)
{
	safeAdvance(tptr, IDENTIFIER); //should be identifier
	subroutineCall(tptr, vmPtr, stPtr); //a "do" statement should be subroutine call

	//For do statements, the subroutine being called should be void
	//we need to discard the 0 that void functions return in jack
	vmPtr->writePop(_TEMP, 0);

	//safety net for expression handler
	if(tptr->tokenPeek() == ";")
	{
		safeAdvance(tptr, SYMBOL);
	}
}

/*******************************************************
*Params: 
	- pointer to tokenizer
	- pointer to vm writer
	- pointer to symbol table
*Compiles return statement
*******************************************************/
void CompilationEngine::compileReturn(std::unique_ptr<Tokenizer> & tptr, std::unique_ptr<VMWriter> & vmPtr, std::unique_ptr<Symbol_Table> & stPtr)
{
	//if there is a return value
	if(tptr->tokenPeek() != ";")
	{
		compileExpression(tptr, vmPtr, stPtr); //the return value is always some sort of expression
		vmPtr->writeReturn(); //write return 

		//safetey net for expression evaluater
		if(tptr->tokenPeek() == ";")
		{
			safeAdvance(tptr, SYMBOL); //;
		}
	}

	//no return value
	else
	{
		//if there is no expression after return then the return type is void
		//void function return constant 0 in jack
		vmPtr->writePush(_CONST, 0);
		vmPtr->writeReturn(); //write return
		safeAdvance(tptr, SYMBOL); //;
	}
}

/*******************************************************
*Params: 
	- pointer to tokenizer
	- pointer to vm writer
	- pointer to symbol table
*Compiles an expression recursively
*Possible bugs
*******************************************************/
void CompilationEngine::compileExpression(std::unique_ptr<Tokenizer> & tptr, std::unique_ptr<VMWriter> & vmPtr, std::unique_ptr<Symbol_Table> & stPtr)
{
	std::string test;
	
	safeAdvanceNoTypeCheck(tptr); //advance to first token of expression
	compileTerm(tptr, vmPtr, stPtr); //compile that term
	safeAdvanceNoTypeCheck(tptr); //advance to next token
	test = tptr->getToken(); //put the token a variable for later use

	while(test == "+" || test == "-" || test == "*" || test == "/" || test == "&" || test == "|" || test == "<" || test == ">" || test == "=")
	{	
		std::string op = test; //save the operator so that we can use stack based expression evaluation 

		safeAdvanceNoTypeCheck(tptr); //next token
		test = tptr->getToken();
		compileTerm(tptr, vmPtr, stPtr); //compile it

		if(op == "*")
		{
			//janky, hard coded way of calling these functions
			//math and divide accept two args
			vmPtr->writeCall("Math.multiply", 2);
		}

		else if(op == "/")
		{
			vmPtr->writeCall("Math.divide", 2);
		}

		//write the operator in postFix
		else
		{
			vmPtr->writeArithmetic(tokenToCommand(op, false)); //bool is to distinguish between unary and other
		}

		//test for more operators, no operator precedence in jack
		if(tptr->tokenPeek() != ")")
		{
			safeAdvanceNoTypeCheck(tptr); 
			test = tptr->getToken();
		}
	}
}

/*******************************************************
*Params: 
	- pointer to tokenizer
	- pointer to vm writer
	- pointer to symbol table
*In tandem with compileExpression
*Compiles an expression recursively
*Possible bugs
*******************************************************/
void CompilationEngine::compileTerm(std::unique_ptr<Tokenizer> & tptr, std::unique_ptr<VMWriter> & vmPtr, std::unique_ptr<Symbol_Table> & stPtr)
{
		//term is int constant
		if(tptr->tokenType() == INT_CONST)
		{
			vmPtr->writePush(_CONST, std::stoi(tptr->getToken()));
		}

		//keyword including constant keywords
		else if(tptr->tokenType() == KEYWORD)
		{
			//false and null are represented by 0 in Jack
			if(tptr->getToken() == "false" || tptr->getToken() == "null" )
			{
				vmPtr->writePush(_CONST, 0);
			}

			//true is represented by -1
			else if(tptr->getToken() == "true")
			{
				vmPtr->writePush(_CONST, 1);
				vmPtr->writeArithmetic(tokenToCommand("-", true));
			}

			//pointer to segment base
			else if(tptr->getToken() == "this")
			{
				vmPtr->writePush(_POINTER, 0);
			}
		}

		//strings are handled using several OS functions
		else if(tptr->tokenType() == STRING_CONST)
		{
			std::string str = tptr->getToken();
			vmPtr->writePush(_CONST, str.length());
			vmPtr->writeCall("String.new", 1);

			for(size_t i = 0; i < str.length(); i++)
			{
				vmPtr->writePush(_CONST, (int)str[i]);
				vmPtr->writeCall("String.appendChar", 2);
			}
		}
	
		//variables and subroutine call
		else if(tptr->tokenType() == IDENTIFIER)
		{
			//call back to expression if array index in term
			if(tptr->tokenPeek() == "[")
			{
				std::string tok = tptr->getToken(); //save the token for later
				safeAdvance(tptr, SYMBOL); //[
				compileExpression(tptr, vmPtr, stPtr); //recursive call back to expression
				vmPtr->writePush(kindToSeg(stPtr->kindOf(tok)), stPtr->indexOf(tok)); //write the vm code to push evaluated expression at correct memory location
				vmPtr->writeArithmetic(ADD); //calculate the correct memory adress
				vmPtr->writePop(_POINTER, 1); //pop value into that address
				vmPtr->writePush(_THAT, 0);
			}

			//for subroutine calls or accessing member of class
			else if(tptr->tokenPeek() == "(" || tptr->tokenPeek() == ".")
			{
				subroutineCall(tptr, vmPtr, stPtr);
			}

			//otherwise just push the variable name onto the stack
			else
			{
				vmPtr->writePush(kindToSeg(stPtr->kindOf(tptr->getToken())), stPtr->indexOf(tptr->getToken()));
			}
		}
		
		//checking for further expressions or unary ops
		//recursibly call back for additional expressions
		else if (tptr->tokenType() == SYMBOL)
		{
			//another expression
			if(tptr->getToken() == "(")
			{
				compileExpression(tptr, vmPtr, stPtr); //the stuff in between ()
				safeAdvance(tptr, SYMBOL); //)

			}

			//unary ops
			else if(tptr->getToken() == "-" || tptr->getToken() == "~")
			{

				std::string op = tptr->getToken();
				safeAdvanceNoTypeCheck(tptr);

				if(tptr->getToken() == "(")
				{
					compileExpression(tptr, vmPtr, stPtr);
					safeAdvance(tptr, SYMBOL); //)
				}

				else
				{
					compileTerm(tptr, vmPtr, stPtr);
				}

				vmPtr->writeArithmetic(tokenToCommand(op, true)); //bool is used to distinguish between neg and sub operator
			}
		}
}

/*******************************************************
*Params: 
	- pointer to tokenizer
	- pointer to vm writer
	- pointer to symbol table
*Compiles a subroutine call
*******************************************************/
void CompilationEngine::subroutineCall(std::unique_ptr<Tokenizer> & tptr, std::unique_ptr<VMWriter> & vmPtr, std::unique_ptr<Symbol_Table> & stPtr)
{
	std::string name;
	std::string subName;
	std::string namePlusSub;
	int numArgs;


	//if the next token is a ( then the call must be to a method of the current class
	if(tptr->tokenPeek() == "(")
	{
		subName = tptr->getToken();
		name = vmPtr->getClassName(); //current class name
		namePlusSub = name + "." + subName; //collect tokens and add them together to form correct call name
		safeAdvance(tptr, SYMBOL); // (
		vmPtr->writePush(_POINTER, 0); //method calls push base adress as first argument
		numArgs = compileExpressionList(tptr, vmPtr, stPtr);
		vmPtr->writeCall(namePlusSub, numArgs + 1); //plus one accounts for pointer to base

		if(tptr->tokenPeek() == ")")
		{
			safeAdvance(tptr, SYMBOL); //)
		}
	}

	//Handles calls other than in class methods
	else if(tptr->tokenPeek() == ".")
	{
		name = tptr->getToken();

		//if the name of the function call exists in the symbol table then it is an object call
		if(stPtr->kindOf(name) != NONE)
		{
			std::string typeName = stPtr->typeOf(name);
			safeAdvance(tptr, SYMBOL); //.
			safeAdvance(tptr, IDENTIFIER);
			subName = tptr->getToken(); //subroutineName
			namePlusSub = typeName + '.' + subName; //write the name of the object's class, not the object name
			safeAdvance(tptr, SYMBOL); // (
			vmPtr->writePush(kindToSeg(stPtr->kindOf(name)), stPtr->indexOf(name));
			numArgs = compileExpressionList(tptr, vmPtr, stPtr); 
			vmPtr->writeCall(namePlusSub, numArgs + 1); //+1 arg for method call
		}

		//call to a function
		else
		{	
			safeAdvance(tptr, SYMBOL); //.
			safeAdvance(tptr, IDENTIFIER);
			subName = tptr->getToken(); //subroutineName
			namePlusSub = name + '.' + subName; //put the name and subname together. EX: Foo.bar
			safeAdvance(tptr, SYMBOL); // (
			numArgs = compileExpressionList(tptr, vmPtr, stPtr); //only way to know the args for calling a function is to count and return them in expression list
			vmPtr->writeCall(namePlusSub, numArgs); //write function call
		}	
	}
}

/*******************************************************
*Params: 
	- pointer to tokenizer
	- pointer to vm writer
	- pointer to symbol table
*Compiles an expression list
*******************************************************/
int CompilationEngine::compileExpressionList(std::unique_ptr<Tokenizer> & tptr, std::unique_ptr<VMWriter> & vmPtr, std::unique_ptr<Symbol_Table> & stPtr)
{
	int argCount = 0;
	
	//if no args return 0
	if(tptr->tokenPeek() == ")")
	{
		return argCount;
	}

	else
	{
		argCount++;

		//loop until end of expression list reached
		while(tptr->getToken() != ")" && tptr->getToken() != ";")
		{
			compileExpression(tptr, vmPtr, stPtr); //for each parameter we need to calculate in case its an expression

			//print the comma if there are multiple expressions
			if(tptr->getToken() == ",")
			{
				argCount++;
			}
		}

		return argCount; //subroutine call needs to know how many args the call has
	}
}


/*******************************************************
*Params: 
	- pointer to tokenizer
	- pointer to vm writer
	- pointer to symbol table
*Compiles a parameter list
*******************************************************/
void CompilationEngine::compileParameterList(std::unique_ptr<Tokenizer> & tptr, std::unique_ptr<Symbol_Table> & stPtr, std::string subType)
{
	std::string type;
	std::string name;

	//if the subroutine type is a method we need to add a dummy variable into the symbol table to make sure the index is correct
	//TODO: probably a better way to handle this
	if(subType == "method")
	{
		stPtr->define("null", "null", ARG);
	}

	//no paramenters
	if(tptr->tokenPeek() == ")")
	{
		return;
	}

	else
	{
		while(tptr->tokenPeek() != ")")
		{
			safeAdvanceNoTypeCheck(tptr); //type
			type = tptr->getToken();
			safeAdvance(tptr, IDENTIFIER); //var name
			name = tptr->getToken();
			stPtr->define(name, type, ARG); // send the variable to the symbol table

			//if more than one parameter
			if(tptr->tokenPeek() != ")")
			{	
				safeAdvance(tptr, SYMBOL); //,
			}
		}
	}
}


/*******************************************************
*Params: 
	- pointer to tokenizer
	- pointer to symbol table
*Compiles variable declarations
*******************************************************/
void CompilationEngine::compileVarDec(std::unique_ptr<Tokenizer> & tptr, std::unique_ptr<Symbol_Table> & stPtr)
{
	std::string name;
	std::string type;
	_kind kind;	
	
	safeAdvance(tptr, KEYWORD); //First token should be the key word var, save in temp string
	kind = tokenToKind(tptr->getToken());
	safeAdvanceNoTypeCheck(tptr); //next token can either be a keyword like int or an indentifier like a class name
	type = tptr->getToken(); //therefore we need to advance without specifing a type
	safeAdvance(tptr, IDENTIFIER); //next token should always be a unique identifier
	name = tptr->getToken();
	stPtr->define(name, type, kind); //send the info to symbol table to be defined

	//if multiple variables are declared on the same line
	while(tptr->tokenPeek() == ",")
	{
		safeAdvance(tptr, SYMBOL); //,
		safeAdvance(tptr, IDENTIFIER); //all we need to get is the next name since type and kind should be the same for same line decs
		name = tptr->getToken();
		stPtr->define(name, type, kind);
	}
	
	safeAdvance(tptr, SYMBOL); //;
}

/*******************************************************
*Params: 
	- pointer to tokenizer
	- type of token expected
*Very basic compiler error checking
*Returns the line of code where error occured
*Returns type of token expected on error line
*Exits when error encountered
*******************************************************/
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

/*******************************************************
*Params: 
	- pointer to tokenizer
*Very basic compiler error checking
*Doesnt compare type of token
*Only prints error if out of tokens
*******************************************************/
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

/********************************************************************************
*Helper functions for converting identifiers for vm writer
*********************************************************************************/
_kind CompilationEngine::tokenToKind(std::string tok)
{
	if(tok == "static")
		return STATIC;
	else if(tok == "field")
		return FIELD;
	else if(tok == "var")
		return VAR;
	else
		return NONE;
}

/********************************************************************************
*Helper functions for converting identifiers for vm writer
*********************************************************************************/
Command CompilationEngine::tokenToCommand(std::string tok, bool urnary)
{

	if(tok == "+")
		return ADD;
	else if(tok == "-")
	{
		if(urnary)
			return NEG;
		else
			return SUB;
	}
	else if(tok == "~")
		return NOT;
	else if(tok == "=")
		return EQ;
	else if(tok == ">")
		return GT;
	else if(tok == "<")
		return LT;
	else if(tok == "&")
		return AND;
	else
		return OR;

}

/********************************************************************************
*Helper functions for converting identifiers for vm writer
*********************************************************************************/
Segment CompilationEngine::kindToSeg(_kind kind)
{
	switch(kind)
	{
		case STATIC:
			return _STATIC;
		case FIELD:
			return _THIS;
		case ARG:
			return _ARG;
		case VAR:
			return _LOCAL;
		case NONE:
			return _CONST;
	}

	return _CONST;
}

CompilationEngine::~CompilationEngine()
{
	
}