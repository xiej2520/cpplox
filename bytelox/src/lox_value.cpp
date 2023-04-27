#include "lox_value.hpp"

#define FMT_HEADER_ONLY
#include "fmt/printf.h"

void print_value(LoxValue value) {
	fmt::printf("%g", value);
}
