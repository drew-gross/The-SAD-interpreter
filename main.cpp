#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <list>

using namespace std;

void DebugBreakIf (const bool &breakCondition)
{
	if ( breakCondition )
		__debugbreak();
}

//why are we calling our pairs 'Variables'? I find this slightly confusing.
//in fact, i dont see the point of these typedefs to begin with?

typedef pair<const string, string> Variable;
typedef map<const string, string> VariableMap;
typedef list<VariableMap> VariableStacks;
typedef vector<string> LinesOfCode;

VariableStacks variableStacks;

LinesOfCode ReadFile(char const * fileName) 
{
	ifstream inFile(fileName, ifstream::in);
	LinesOfCode code;
	string currentLine;
	char currentChar = 0;

	while (!inFile.eof()) 
	{
		currentChar = inFile.get();
		if(currentChar == '\n')
		{
			code.push_back(currentLine);
			continue;
		}
		currentLine += currentChar;
	}

	return code;
}

bool contains(string const & searchIn, char const searchFor)
{
	return (searchIn.find(searchFor) != -1);
}

pair<string, string> split(string const & str, char const splitter) {
	int splitterIndex = str.find(splitter);
	string varName(str.substr(0, splitterIndex));
	string varVal(str.substr(splitterIndex + 1, str.length() - splitterIndex));
	return make_pair(varName, varVal);
}

void CreateNewScope ()
{
	variableStacks.push_back(VariableMap());
}

void DeleteCurrentScope ()
{
	variableStacks.pop_back();
}

void IgnoreRestOfScope(ifstream & file)
{
	while(file.get()!='}');
}

// Cycle through all scopes from local to global and find if it exists, if not then it is global
Variable &FindOrCreateVariable(const string &variableName)
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
		iVar = variableStacks.back().insert ( make_pair(variableName, string ("") ) ).first;
	}  

	return *iVar;
}

Variable* Find(const string &variableName )
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
		string currentLine = *it;
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
			cout << Find(*it)->second << endl;
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
				if (split(currentLine, '+').second == "")
				{
					firstVar->second += '\n';
				}
				else 
				{
					Variable *secondVar = Find(split(currentLine, '+').second);
					firstVar->second += secondVar->second;
				}
			}
		}
	}

	cout << "-----------------------------------------------\n" << endl;

	for ( VariableStacks::iterator sIt = variableStacks.begin(); sIt != variableStacks.end(); sIt++)
	{
		cout << "Next scope" << endl;
		for ( VariableMap::iterator it = sIt->begin(); it != sIt->end(); it++ )
		{
			cout << it->first << "=" << it->second << endl;
		}
		cout << endl;
	}
	cin.get();
}