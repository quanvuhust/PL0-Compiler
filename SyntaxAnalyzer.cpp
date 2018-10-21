#include "SyntaxAnalyzer.h"
#include "Parser.h"

SyntaxAnalyzer::SyntaxAnalyzer()
{
}

void SyntaxAnalyzer::error(int errorCode, string name)
{
    cerr << errorString[errorCode] << name << endl;
}
