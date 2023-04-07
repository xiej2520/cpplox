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
		start = end + 1;
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

void defineVisitor(const string &baseName, const vector<string> &types) {
	cout << "\t" << "template<class R>" << "\n";
	cout << "\t" << "class " << "Visitor: VisitorBase {" << "\n";
	cout << "\t\t" << "public:" << "\n";
	string lower;
	for (char c : baseName) {
		lower += tolower(c);
	}
	cout << "\t\t" << "R result;" << "\n";
	for (auto &type : types) {
		string typeName = trim(split(type, ":")[0]);
		cout << "\t\t" << "virtual R visit" << typeName << baseName << "("
			<< typeName << " &" << lower << ") = delete;" << "\n";
	}
	cout << "\t};" << "\n";
}

void defineType(const string &baseName, const string &className, const string &fieldList) {
	cout << "class " << baseName << "::" << className << ": " << baseName << " {\n";
	vector<string> fields = split(fieldList, ",");
	for (string &s : fields) {
		s = trim(s);
	}
	for (string &field : fields) {
		cout << "\t" << field << ";" << "\n";
	}
	cout << "\t" << className << "(" << fieldList << "): ";

	for (int i=0; i<fields.size()-1; i++) {
		auto name = split(fields[i], " ")[1];
		cout << name << "(" << name << "), ";
	}
	auto name = split(fields.back(), " ")[1];
	cout << name << "(" << name << ") {}" << "\n";
	cout << "\t" << "template<class R>" << "\n";
	cout << "\t" << "R accept(Visitor<R> &visitor) {" << "\n";
	cout << "\t\t" << "return visitor->visit" << className << baseName << "(this)" << "\n";
	cout << "\t" << "}" << "\n";
	cout << "};\n\n";
}

void defineAst(const string &baseName, const vector<string> &types) {
	cout << "#pragma once" << "\n\n";
	cout << "#include \"token.h\"" << "\n";
	cout << "\n";
	cout << "class " << baseName << " {" << "\n";
	for (auto &type : types) {
		auto sp = split(type, ":");
		string className = trim(sp[0]);
		cout << "\t" << "class " << className << ";" << "\n";
	}
	cout << "\t" << "class VisitorBase { };" << "\n";
	defineVisitor(baseName, types);
	cout << "\t" << "template<class R>" << "\n";
	cout << "\t" << "R accept(Visitor<R> &visitor) {" << "\n";
	cout << "\t\t" << "do_accept(visitor);" << "\n";
	cout << "\t\t" << "return v.result;" << "\n";
	cout << "\t" << "}" << "\n";
	cout << "\t" << "virtual void do_accept(VisitorBase &v) = delete;" << "\n";
	cout << "};" << "\n";
	cout << "\n";

	for (auto &type : types) {
		auto sp = split(type, ":");
		string className = trim(sp[0]);
		string fields = trim(sp[1]);
		defineType(baseName, className, fields);
	}
	
}


int main(int argc, char **argv) {
	defineAst("Expr", {
		"Binary : Expr left, Token op, Expr right",
		"Grouping: Expr expression",
		"Literal : LoxObject value",
		"Unary : Token op, Expr right"
	});
}
