#include <bits/stdc++.h>
using namespace std;

/*
// Single-character
...
STAR,
// one or two character
...
LESS_EQUAL
...
// Literals
...
NUMBER
// Keywords
...

END_OF_FILE
*/

vector<string> tokens = {
	// Single character
	"LEFT_PAREN",
	"RIGHT_PAREN",
	"LEFT_BRACE",
	"RIGHT_BRACE",
	"COMMA",
	"DOT",
	"MINUS",
	"PLUS",
	"SEMICOLON",
	"SLASH",
	"STAR",
	
	// one or two character
	"BANG",
	"BANG_EQUAL",
	"EQUAL",
	"EQUAL_EQUAL",
	"GREATER",
	"GREATER_EQUAL",
	"LESS",
	"LESS_EQUAL",
	
	// Literals
	"IDENTIFIER",
	"STRING",
	"NUMBER",
	
	// Keywords
	"AND",
	"CLASS",
	"ELSE",
	"FALSE",
	"FUN",
	"FOR",
	"IF",
	"NIL",
	"OR",
	"PRINT",
	"RETURN",
	"SUPER",
	"THIS",
	"TRUE",
	"VAR",
	"WHILE",
	
	"END_OF_FILE"
};

int main() {
	cout << "enum class TokenType {\n";
	for (string &t : tokens) {
		cout << "\t" << t << ",\n";
	}
	cout << "};\n";
	cout << "constexpr std::string to_string(TokenType t) {\n";
	cout << "\tswitch (t) {\n";
	for (string &t : tokens) {
		cout << "\tcase " << "TokenType::" << t << ": return \"" << t << "\";\n";
	}
	cout << "default: return \"ERROR_TOKEN_NOT_FOUND\";\n";
	cout << "\t}\n";
	cout << "}\n";
}