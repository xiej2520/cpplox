#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "lox_object.h"
#include "token.h"
#include "expr.h"
#include "stmt.h"

// forward declare...
static std::string parenthesize(const std::string &name, const std::vector<Expr> &exprs);

// need const arguments for template to compile?
struct ExprToString {
	std::string operator()(const std::monostate) {
		return "";
	}
	std::string operator()(const Assign &expr) {
		return expr.name.lexeme + " = " + to_string(expr.name.literal);		
	}
	std::string operator()(const Binary &expr) {
		return parenthesize(expr.op.lexeme, {*expr.left, *expr.right});
	}
	std::string operator()(const Call &expr) {
		std::string res = std::visit(ExprToString(), *expr.callee) + "(";
		for (size_t i=0; i<expr.arguments.size()-1; i++) {
			res += std::visit(ExprToString(), expr.arguments[i]) + ", ";
		}
		if (!expr.arguments.empty()) {
			res += std::visit(ExprToString(), expr.arguments.back());
		}
		return res + ")";
	}
	std::string operator()(const Grouping &expr) {
		return parenthesize("group", {*expr.expression});
	}
	std::string operator()(const Literal &expr) {
		if (std::holds_alternative<std::monostate>(expr.value)) return "nil";
		return to_string(expr.value);
	}
	std::string operator()(const Logical &expr) {
		return parenthesize(expr.op.lexeme, {*expr.left, *expr.right});
	}
	std::string operator()(const Unary &expr) {
		return parenthesize(expr.op.lexeme, {*expr.right});
	}
	std::string operator()(const Variable &expr) {
		return "var " + expr.name.lexeme;
	}
};

struct StmtToString {
	std::string operator()(const std::monostate) {
		return "";
	}
	std::string operator()(const Block &stmt) {
		std::string res = "Block {";
		if (stmt.statements.empty()) return res + "}";
		res += "\n";
		for (const Stmt &s : stmt.statements) {
			res += "\t" + std::visit(StmtToString(), s) + "\n";
		}
		return res + "}";
	}
	std::string operator()(const Expression &stmt) {
		return "Expression: " + std::visit(ExprToString(), stmt.expression);
	}
	std::string operator()(const Function &stmt) {
		return "Function: " + stmt.name.repr();
	}
	std::string operator()(const If &stmt) {
		return "If " + std::visit(ExprToString(), stmt.condition) + " then\n"
			+ std::visit(StmtToString(), *stmt.thenBranch) + "\nelse\n"
			+ std::visit(StmtToString(), *stmt.elseBranch);
	}
	std::string operator()(const Print &stmt) {
		return "Print: " + std::visit(ExprToString(), stmt.expression);
	}
	std::string operator()(const Return &stmt) {
		return "Return: " + stmt.keyword.repr() + std::visit(ExprToString(), stmt.value);
	}
	std::string operator()(const Var &stmt) {
		return "Var " + stmt.name.lexeme + (stmt.initializer.has_value() ?
			" = " + std::visit(ExprToString(), stmt.initializer.value()) : "");
	}
	std::string operator()(const While &stmt) {
		return "While " + std::visit(ExprToString(), stmt.condition) + ":\n" +
			std::visit(StmtToString(), *stmt.body);
	}
};

static std::string parenthesize(const std::string &name, const std::vector<Expr> &exprs) {
	std::string s = "(" + name;
	for (const Expr &expr: exprs) {
		s += " ";
		s += std::visit(ExprToString(), expr);
	}
	s += ")";
	return s;
}

// inline to allow header definition
inline std::string print(const Expr &expr) {
	return std::visit(ExprToString(), expr);
}

inline std::string print(const Stmt &stmt) {
	return std::visit(StmtToString(), stmt);
}
