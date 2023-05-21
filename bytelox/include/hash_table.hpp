#pragma once

#include "common.hpp"
#include "lox_value.hpp"

#include <memory>

namespace bytelox {

struct Entry {
	ObjectString *key;
	LoxValue value;
};

struct HashTable {
	u32 size = 0;
	u32 capacity = 0;
	std::unique_ptr<Entry[]> entries = nullptr;
	HashTable();
	
	bool set(ObjectString *key, LoxValue value);
	bool get(ObjectString *key, LoxValue *value);
	bool del(ObjectString *key);
	// finds the entry with the matching key, which is an ADDRESS
	Entry *find(ObjectString *key);
	Entry *find_in_array(Entry array[], u32 array_cap, ObjectString *key);
	ObjectString *find_string(std::string_view str);
	
	void adjust_capacity(u32 new_capacity);
	void add_all(HashTable &table);
};

}