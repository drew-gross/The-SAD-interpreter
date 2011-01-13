#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <list>

void DebugBreakIf (const bool & breakCondition)
{
	if ( breakCondition )
		__debugbreak();
}

typedef std::pair<const std::string, std::string> Variable;
typedef std::map<const std::string, std::string> VariableMap;
typedef std::list<VariableMap> VariableStacks;
typedef std::vector<std::string> LinesOfCode;

VariableStacks variableStacks;

LinesOfCode ReadFile(char const * fileName) 
{
	std::ifstream inFile(fileName, std::ifstream::in);
	LinesOfCode code;
	while (!inFile.eof()) 
	{
		std::string curLine;
		char curChar = 0;
		while (curChar != '\n' && !inFile.eof())
		{
			curChar = inFile.get();
			if (curChar != '\n')
			{
				curLine+=(curChar);
			}
		}
		code.push_back(curLine);
	}
	return code;
}

bool contains(std::string const & str, char searchFor)
{
	return (str.find(searchFor) != -1);
}

std::pair<std::string, std::string> split(std::string const & str, char splitter) {
	int splitterIndex = str.find(splitter);
	std::string varName(str.substr(0, splitterIndex));
	std::string varVal(str.substr(splitterIndex + 1, str.length() - splitterIndex));
	return std::make_pair(varName, varVal);
}

void CreateNewScope ()
{
	variableStacks.push_back(VariableMap());
}
void DeleteCurrentScope ()
{
	variableStacks.pop_back();
}

void ExitScope(std::ifstream & file)
{
	while (true) 
	{
		char currentChar = file.get();
		if (currentChar == '}') 
		{
			return;
		}
	}
}

// Cycle through all scopes from local to global and find if it exists, if not then it is global
Variable &FindOrCreateVariable(const std::string &variableName )
{
	VariableMap::iterator iVar;
	bool wasVarFound = false;
	for ( VariableStacks::reverse_iterator rIt = variableStacks.rbegin(); ! wasVarFound && rIt != variableStacks.rend(); rIt++ )
	{
		iVar = rIt->find(variableName);
		if ( iVar != rIt->end() )
		{
			wasVarFound = true;
		}
	}

	if ( ! wasVarFound )
	{
		iVar = variableStacks.back().insert ( std::make_pair(variableName, std::string ("") ) ).first;
	}  

	return *iVar;
}

Variable* Find(const std::string &variableName )
{
	VariableMap::iterator iVar;
	bool wasVarFound = false;
	for ( VariableStacks::reverse_iterator rIt = variableStacks.rbegin(); ! wasVarFound && rIt != variableStacks.rend(); rIt++ )
	{
		iVar = rIt->find(variableName);
		if ( iVar != rIt->end() )
		{
			return &*iVar;
		}
	}
	return nullptr;
}

int main() {
	LinesOfCode code = ReadFile("foo.sd");

	CreateNewScope();

	for (LinesOfCode::iterator it = code.begin(); it != code.end(); it++) {
		std::string currentLine = *it;
		if (currentLine == "{" ) // Create a new scope
		{
			CreateNewScope();
		}
		else if ( currentLine == "}" ) // Delete the current scope
		{
			DeleteCurrentScope();
		} 
		else if (currentLine == "/") // goto next closing curly brace
		{
			while (*it != "}")
			{
				it++;
			}
		} 
		else if (currentLine == ">") // output next line
		{
			it++;
			std::cout << Find(*it)->second << std::endl;
		} 
		else if (currentLine == "?") // if next line is false, skip line after that
		{
			it++;
			if (Find(*it)->second == "")
			{
				it++;
			}
		} 
		else if (currentLine == "\\") // goto next closing curly brace
		{
			while (*it != "{")
			{
				it--;
			}
		}
		else
		{
			if (contains(currentLine, '='))
			{
				Variable var = split(currentLine, '=');
				Variable &addedVariable = FindOrCreateVariable( var.first );

				Variable *assigned = Find(var.second);

				if (assigned == nullptr) {
					addedVariable.second = var.second;
				} else {
					addedVariable.second = assigned->second;
				}
			} 
			else if (contains(currentLine, '+'))
			{
				Variable *firstVar = Find(split(currentLine, '+').first);
				Variable *secondVar = Find(split(currentLine, '+').second);

				firstVar->second += secondVar->second;
			}
		}
	}

	std::cout << "-----------------------------------------------\n" << std::endl;

	for ( VariableStacks::iterator sIt = variableStacks.begin(); sIt != variableStacks.end(); sIt++)
	{
		std::cout << "Next scope" << std::endl;
		for ( VariableMap::iterator it = sIt->begin(); it != sIt->end(); it++ )
		{
			std::cout << it->first << "=" << it->second << std::endl;
		}
		std::cout << std::endl;
	}
	std::cin.get();
}