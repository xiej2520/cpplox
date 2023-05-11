#include "lox_value.hpp"

#define FMT_HEADER_ONLY
#include "fmt/core.h"

namespace bytelox {

void LoxValue::print_value() {
	switch (type) {
		case ValueType::BOOL:
			fmt::print("{}", as.boolean);
			break;
		case ValueType::NIL: fmt::print("nil"); break;
		case ValueType::NUMBER: fmt::print("{:g}", as.number); break;
	}
}

}
