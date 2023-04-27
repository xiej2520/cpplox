#pragma once

#include "common.hpp"
#include "lox_value.hpp"

#include <vector>

// convert enum class to int with prepend +
template <typename T>
constexpr auto operator+(T e) noexcept
	-> std::enable_if_t<std::is_enum<T>::value, std::underlying_type_t<T>> {
	return static_cast<std::underlying_type_t<T>>(e);
}

enum class OP {
	CONSTANT,
	RETURN
};

struct Chunk {
	std::vector<u8> code;
	std::vector<LoxValue> constants;
	std::vector<int> lines;
	void write(u8 byte, int line);

	size_t count();
	size_t add_constant(LoxValue value);
};
