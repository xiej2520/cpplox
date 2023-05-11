#pragma once

#include "common.hpp"

namespace bytelox {

enum class ValueType {
	BOOL,
	NIL,
	NUMBER
};

struct LoxValue {
	ValueType type;
	union {
		bool boolean;
		double number;
	} as;
	
	// definition needs to be provided
	// implicit conversions allowed
	constexpr LoxValue(): type(ValueType::NIL), as({ .number=0 }) { }
	constexpr LoxValue(bool b): type(ValueType::BOOL), as({ .boolean=b }) { }
	constexpr LoxValue(double d): type(ValueType::NUMBER), as({ .number=d }) { }

	
	[[nodiscard]] constexpr bool is_bool() const {
		return type == ValueType::BOOL;
	}
	[[nodiscard]] constexpr bool is_nil() const {
		return type == ValueType::NIL;
	};
	[[nodiscard]] constexpr bool is_number() const {
		return type == ValueType::NUMBER;
	};
	
	[[nodiscard]] constexpr bool operator==(LoxValue that) const {
		if (type != that.type) {
			return false;
		}
		switch (type) {
			case ValueType::BOOL: return as.boolean == that.as.boolean;
			case ValueType::NIL: return true;
			case ValueType::NUMBER: return as.number == that.as.number;
			default: return false; // unreachable
		}
	}

	void print_value();
};

}
