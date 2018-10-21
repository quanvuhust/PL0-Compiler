#ifndef SYNTAXANALYZER_H
#define SYNTAXANALYZER_H

#include <string>
#include <unordered_map>

class SyntaxAnalyzer
{
    public:
        SyntaxAnalyzer();
        void error(int errorCode, std::string name="");
    private:
        std::unordered_map<int, const char*> errorString = {{0, "Missing ]"}, {1, "Missing )"},
            {2, "Condition: missing logic operator"}, {3, "Missing operator :="},
            {4, "Missing ("}, {5, "Missing keyword END"}, {6, "Missing keyword THEN"}, {7, "Missing keyword DO in while loop"},
            {8, "Missing IDENT after FOR"}, {9, "Missing keyword TO in for loop"},
            {10, "Missing keyword DO in for loop"}, {11, "Missing keyword PROGRAM"},
            {12, "Missing name program"}, {13, "Missing semicolon \';\'"}, {14, "Missing DOT \'.\'"},
            {15, "Missing block"}, {16, "Missing operator \'=\'"}, {17, "Missing number when assign a const"},
            {18, "Missing variable name."}, {19, "Missing array length."}, {20, "Missing procedure name."},
            {21, "Missing argument name when declare PROCEDURE."}, {22, "Missing const name."},
            {23, "Missing procedure name when call PROCEDURE."}, {24, "Missing factor."}
        };
};

#endif // SYNTAXANALYZER_H
