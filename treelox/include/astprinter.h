#pragma once

#include <iostream>
#include <vector>
#include <string>
#include "lox_object.h"
#include "token.h"
#include "expr.h"
#include "stmt.h"

// forward declare...
static std::string parenthesize(std::string_view name, std::vector<Expr *> exprs);
inline std::string to_string(const Expr &expr);
inline std::string to_string(const Stmt &stmt);

// need const arguments for template to compile?
struct ExprToString {
	std::string operator()(const std::monostate) {
		return "";
	}
	std::string operator()(const Assign &expr) {
		return expr.name.lexeme + to_string(*expr.value);
	}
	std::string operator()(const Binary &expr) {
		return parenthesize(expr.op.lexeme, {expr.left.get(), expr.right.get()});
	}
	std::string operator()(const Call &expr) {
		std::string res = std::visit(ExprToString(), *expr.callee) + "(";
		if (!expr.arguments.empty()) {
			for (size_t i=0; i<expr.arguments.size()-1; i++) {
				res += std::visit(ExprToString(), expr.arguments[i]) + ", ";
			}
			res += std::visit(ExprToString(), expr.arguments.back());
		}
		return res + ")";
	}
	std::string operator()(const Grouping &expr) {
		return parenthesize("group", {expr.expression.get()});
	}
	std::string operator()(const Literal &expr) {
		if (std::holds_alternative<std::monostate>(expr.value)) return "nil";
		return to_string(expr.value);
	}
	std::string operator()(const Logical &expr) {
		return parenthesize(expr.op.lexeme, {expr.left.get(), expr.right.get()});
	}
	std::string operator()(const Unary &expr) {
		return parenthesize(expr.op.lexeme, {expr.right.get()});
	}
	std::string operator()(const Variable &expr) {
		return "var " + to_string(expr.name);
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
		return "Function: " + to_string(stmt.name);
	}
	std::string operator()(const If &stmt) {
		return "If " + std::visit(ExprToString(), stmt.condition) + " then\n"
			+ std::visit(StmtToString(), *stmt.then_branch) + "\nelse\n"
			+ std::visit(StmtToString(), *stmt.else_branch);
	}
	std::string operator()(const Print &stmt) {
		return "Print: " + std::visit(ExprToString(), stmt.expression);
	}
	std::string operator()(const Return &stmt) {
		return "Return: " + to_string(stmt.keyword) + std::visit(ExprToString(), stmt.value);
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

static std::string parenthesize(std::string_view name, std::vector<Expr *> exprs) {
	std::string s = "(" + std::string(name);
	for (Expr *expr: exprs) {
		s += " ";
		s += std::visit(ExprToString(), *expr);
	}
	s += ")";
	return s;
}

// inline to allow header definition
inline std::string to_string(const Expr &expr) {
	return std::visit(ExprToString(), expr);
}

inline std::string to_string(const Stmt &stmt) {
	return std::visit(StmtToString(), stmt);
}
