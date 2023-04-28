#include "lox_value.hpp"

#define FMT_HEADER_ONLY
#include "fmt/core.h"

namespace bytelox {

void print_value(LoxValue value) {
	fmt::print("{:g}", value);
}

}
