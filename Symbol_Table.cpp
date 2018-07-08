#include "Symbol_Table.hpp"

/*************************************************************************
*Default constructor
	*sets all counter variables to 0
	*makes sure class and subroutine hash tables are clear
*************************************************************************/
Symbol_Table::Symbol_Table()
{
	classScope.clear();
	subroutineScope.clear();
	staticCount = 0;
	fieldCount = 0;
	argCount = 0;
	varCount = 0;

}

/*************************************************************************
	*This function should be called after each method, function or
	subroutine is processed
	*clears subroutine hash table
	*resets local variable counters
*************************************************************************/
void Symbol_Table::startSubroutine()
{
	subroutineScope.clear();
	argCount = 0;
	varCount = 0;
}

/*************************************************************************
	*Populates the symbol stable struct data using variable name as key
	*increments counter with each addition
*************************************************************************/
void Symbol_Table::define(std::string name, std::string type, _kind kind)
{
	switch(kind)
	{
		case STATIC:
			classScope[name] = STData(type, kind, staticCount);
			staticCount++;
				break;
		case FIELD:
			classScope[name] = STData(type, kind, fieldCount);
			fieldCount++;
				break;
		case ARG:
			subroutineScope[name] = STData(type, kind, argCount);
			argCount++;
				break;
		case VAR:
			subroutineScope[name] = STData(type, kind, varCount);
			varCount++;
				break;
		case NONE:
			break;
	}
}

/*************************************************************************
	*returns the counter for each variable kind (Static, Field, etc)
*************************************************************************/
int Symbol_Table::variableCount(_kind kind)
{
	switch(kind)
	{
		case STATIC:
			return staticCount;
				break;
		case FIELD:
			return fieldCount;
				break;
		case ARG:
			return argCount;
				break;
		case VAR:
			return varCount;
				break;
		case NONE:
			return 0;
				break;
	}

	return 0;
}

/*************************************************************************
	*Variable name is passed in as a parameter
	*Function searches hash map and returns kind if found
*************************************************************************/
_kind Symbol_Table::kindOf(std::string name)
{
	std::map<std::string, STData>::iterator itr;

	//first look for name in subroutine scope
	itr = subroutineScope.find(name);

	//if found return the requested value
	if(itr != subroutineScope.end())
	{
		return itr->second.kind;
	}

	//if not found in subroutine scope, check class scope
	else
	{
		itr = classScope.find(name);

		//if found return the requested value
		if(itr != classScope.end())
		{
			return itr->second.kind;
		}

		//otherwise return none
		else
		{
			return NONE;
		}
	}
}

/*************************************************************************
	*Variable name is passed in as a parameter
	*Function searches hash map and returns type if found
*************************************************************************/
std::string Symbol_Table::typeOf(std::string name)
{
	std::map<std::string, STData>::iterator itr;

	itr = subroutineScope.find(name);

	if(itr != subroutineScope.end())
	{
		return itr->second.type;
	}

	else
	{
		itr = classScope.find(name);

		if(itr != classScope.end())
		{
			return itr->second.type;
		}

		else
		{
			return "NONE";
		}
	}
}

/*************************************************************************
	*Variable name is passed in as a parameter
	*Function searches hash map and returns index if found
*************************************************************************/
int Symbol_Table::indexOf(std::string name)
{	
	std::map<std::string, STData>::iterator itr;

	itr = subroutineScope.find(name);

	if(itr != subroutineScope.end())
	{
		return itr->second.index;
	}

	else
	{
		itr = classScope.find(name);

		if(itr != classScope.end())
		{
			return itr->second.index;
		}

		else
		{
			return -1;
		}
	}

}

Symbol_Table::~Symbol_Table()
{

}