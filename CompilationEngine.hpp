#ifndef COMPILATION_ENGINE_HPP
#define COMPILATION_ENGINE_HPP

#include <fstream>
#include <string>
#include <iostream>
#include "Tokenizer.hpp"

class CompilationEngine
{
	private:
		std::ofstream outputFile;
		bool openError;
		int tabCount;

	public:
		CompilationEngine(std::string);
		bool errorCheck();
		void compileClass(std::unique_ptr<Tokenizer> &);
		void compileClassVarDec(std::unique_ptr<Tokenizer> &);
		void compileVarDec(std::unique_ptr<Tokenizer> &);
		void compileSubroutine(std::unique_ptr<Tokenizer> &);
		void compileParameterList(std::unique_ptr<Tokenizer> &);
		void compileStatments(std::unique_ptr<Tokenizer> &);
		void compileLet(std::unique_ptr<Tokenizer> &);
		void compileIf(std::unique_ptr<Tokenizer> &);
		void compileWhile(std::unique_ptr<Tokenizer> &);
		void compileDo(std::unique_ptr<Tokenizer> &);
		void compileReturn(std::unique_ptr<Tokenizer> &);
		void compileExpression(std::unique_ptr<Tokenizer> &);
		void compileTerm(std::unique_ptr<Tokenizer> &);
		void subroutineCall(std::unique_ptr<Tokenizer> &);
		void compileExpressionList(std::unique_ptr<Tokenizer> &);
		void writeKeyword(std::string);
		void writeSymbol(std::string);
		void writeIdentifier(std::string);
		void writeIntConst(std::string);
		void writeStringConst(std::string);
		void safeAdvance(std::unique_ptr<Tokenizer> &, _tokenType);
		void safeAdvanceNoTypeCheck(std::unique_ptr<Tokenizer> &);
		std::string printTabs();
		void incrementTabs();
		void decrementTabs();
		~CompilationEngine();

};



#endif