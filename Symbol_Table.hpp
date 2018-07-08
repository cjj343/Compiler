#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

//These are the different kinds of variables available in JACK
enum _kind {STATIC, FIELD, ARG, VAR, NONE};

#include <string>
#include <map>

class Symbol_Table
{
	private:
		int staticCount;
		int fieldCount;
		int argCount;
		int varCount;

		//This struct will hold the data for each variable
		//A hash table holds these structs based on the scope of the variable
		struct STData
		{
			//int, char, etc
			std::string type;
			//STATIC, FIELD, ETC
			_kind kind;
			//this variable is used by the VM writer to keep track of how many of each variable
			int index;

			//default constructor
			STData()
			{
				type = "";
				kind = NONE;
				index = 0;
			}

			//fill up the struct constructor
			//variable name is the key for the has table
			STData(std::string t, _kind k, int i)
			{
				type = t;
				kind = k;
				index = i;
			}
		};

		//the hash table holds all the static and field variable for a class
		//this hash table is not destroyed until one class file has been processed 
		std::map<std::string, STData> classScope;
		//this hash table holds all the args and vars
		//it is overwritten after one subroutine, method or function has been processed
		std::map<std::string, STData> subroutineScope;

	public:
		Symbol_Table();
		void startSubroutine();
		void define(std::string, std::string, _kind);
		int variableCount(_kind);
		_kind kindOf(std::string);
		std::string typeOf(std::string);
		int indexOf(std::string);
		~Symbol_Table();
};

#endif