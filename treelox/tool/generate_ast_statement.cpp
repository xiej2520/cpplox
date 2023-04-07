#include <iostream>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

vector<string> split(const string &s, string delim) {
	vector<string> res;
	size_t start = 0;
	size_t end;
	while ((end = s.find(delim, start)) != string::npos) {
		res.push_back(s.substr(start, end - start));
		start = end + delim.size();
	}
	if (start != s.size()) {
		res.push_back(s.substr(start));
	}
	return res;
}

string trim(const string &s) {
	int start = 0, end = s.size()-1;
	while (start < s.size() && isblank(s[start])) {
		start++;
	}
	while (end > start && isblank(s[end])) {
		end--;
	}
	return s.substr(start, end - start + 1);
}

void defineType(string &className, string &fields) {
	cout << "struct " << className << " {" << "\n";
	auto fieldsList = split(fields, ",");
	for (auto &field : fieldsList) {
		field = trim(field);
	}
	for (auto &field : fieldsList) {
		cout << "\t" << "const " << field << ";" << "\n";
	}

	cout << "\t" << className << "(";
	cout << fields << "): ";

	for (int i=0; i<fieldsList.size()-1; i++) {
		auto name = split(fieldsList[i], " ")[1];
		cout << name << "(" << name << "), ";
	}
	auto name = split(fieldsList.back(), " ")[1];
	cout << name << "(" << name << ") {}" << "\n";
	cout << "};\n\n";
}



void defineAst(const string &baseName, const vector<string> &types) {
	cout << "#pragma once" << "\n\n";
	cout << "#include \"expr.h\"" << "\n";
	cout << "#include <memory>" << "\n";
	cout << "#include <variant>" << "\n";
	cout << "\n";
	for (auto &type : types) {
		auto sp = split(type, ":=");
		cout << "struct " << trim(sp[0]) << ";" << "\n";
	}
	cout << "\n";
	cout << "using Stmt = std::variant" << "\n";
	cout << "<" << "\n";
	cout << "\t" << "std::monostate," << "\n";
	for (int i=0; i<types.size()-1; i++) {
		auto sp = split(types[i], ":=");
		string className = trim(sp[0]);
		cout << "\t" << className << ",\n";
	}
	cout << "\t" << trim(split(types.back(), ":=")[0]) << "\n";
	cout << ">;" << "\n";
	cout << "\n";
	for (auto &type : types) {
		auto sp = split(type, ":=");
		string className = trim(sp[0]);
		string fields = trim(sp[1]);
		defineType(className, fields);
	}
}


int main(int argc, char **argv) {
	defineAst("stmt", {
		"Block := std::vector<Stmt> statements",
		"Expression := Expr expression",
		"Function := Token name, std::vector<Token> params, std::vector<Stmt> body",
		"If := Expr condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch",
		"Print := Expr expression",
		"Var := Token name, Expr initializer",
		"While := Expr condition, std::shared_ptr<Stmt> body"
	});
}
