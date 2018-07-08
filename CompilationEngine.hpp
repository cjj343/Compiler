#ifndef COMPILATION_ENGINE_HPP
#define COMPILATION_ENGINE_HPP

#include <string>
#include "Tokenizer.hpp"
#include "Symbol_Table.hpp"
#include "VMWriter.hpp"
#include <memory>

class CompilationEngine
{
	private:
		int labelNum;

	public:
		CompilationEngine();
		void compileClass(std::unique_ptr<Tokenizer> &, std::unique_ptr<VMWriter> &, std::unique_ptr<Symbol_Table> &);
		void compileClassVarDec(std::unique_ptr<Tokenizer> &, std::unique_ptr<Symbol_Table> &);
		void compileVarDec(std::unique_ptr<Tokenizer> &, std::unique_ptr<Symbol_Table> &);
		void compileSubroutine(std::unique_ptr<Tokenizer> &, std::unique_ptr<VMWriter> &, std::unique_ptr<Symbol_Table> &);
		void compileParameterList(std::unique_ptr<Tokenizer> &, std::unique_ptr<Symbol_Table> &, std::string);
		void compileStatements(std::unique_ptr<Tokenizer> &, std::unique_ptr<VMWriter> &, std::unique_ptr<Symbol_Table> &);
		void compileLet(std::unique_ptr<Tokenizer> &, std::unique_ptr<VMWriter> &, std::unique_ptr<Symbol_Table> &);
		void compileIf(std::unique_ptr<Tokenizer> &, std::unique_ptr<VMWriter> &, std::unique_ptr<Symbol_Table> &);
		void compileWhile(std::unique_ptr<Tokenizer> &, std::unique_ptr<VMWriter> &, std::unique_ptr<Symbol_Table> &);
		void compileDo(std::unique_ptr<Tokenizer> &, std::unique_ptr<VMWriter> &, std::unique_ptr<Symbol_Table> &);
		void compileReturn(std::unique_ptr<Tokenizer> &, std::unique_ptr<VMWriter> &, std::unique_ptr<Symbol_Table> &);
		void compileExpression(std::unique_ptr<Tokenizer> &, std::unique_ptr<VMWriter> &, std::unique_ptr<Symbol_Table> &);
		void compileTerm(std::unique_ptr<Tokenizer> &, std::unique_ptr<VMWriter> &, std::unique_ptr<Symbol_Table> &);
		void subroutineCall(std::unique_ptr<Tokenizer> &, std::unique_ptr<VMWriter> &, std::unique_ptr<Symbol_Table> &);
		int compileExpressionList(std::unique_ptr<Tokenizer> &, std::unique_ptr<VMWriter> &, std::unique_ptr<Symbol_Table> &);
		void safeAdvance(std::unique_ptr<Tokenizer> &, _tokenType);
		void safeAdvanceNoTypeCheck(std::unique_ptr<Tokenizer> &);
		_kind tokenToKind(std::string);
		Command tokenToCommand(std::string, bool);
		Segment kindToSeg(_kind);
		~CompilationEngine();
};
#endif