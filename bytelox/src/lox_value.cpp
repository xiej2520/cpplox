#include "lox_value.hpp"

#define FMT_HEADER_ONLY
#include "fmt/core.h"

namespace bytelox {

void LoxObject::print_object() {
	switch (type) {
		case ObjectType::STRING: fmt::print("{}", as_string()->chars.get()); break;
	}

}

void LoxValue::print_value() {
	switch (type) {
		case ValueType::BOOL:
			fmt::print("{}", as.boolean);
			break;
		case ValueType::NIL: fmt::print("nil"); break;
		case ValueType::NUMBER: fmt::print("{:g}", as.number); break;
		case ValueType::OBJECT: as.obj->print_object(); break;
	}
}

}
