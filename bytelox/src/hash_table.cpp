#include "hash_table.hpp"
#include "lox_object.hpp"

namespace bytelox {

constexpr double TABLE_MAX_LOAD = 0.75;
constexpr u32 MIN_CAPACITY = 32;

HashTable::HashTable() {
	adjust_capacity(MIN_CAPACITY);
}

bool HashTable::set(ObjectString *key, LoxValue value) {
	if (size + 1 > capacity * TABLE_MAX_LOAD) {
		adjust_capacity(capacity *= 2);
	}

	Entry *entry = find(key);
	bool is_new_key = entry->key == nullptr;
	// if not nil, we are inserting into tombstone, don't inc size
	if (is_new_key && entry->value.is_nil()) size++;
	
	entry->key = key;
	entry->value = value;
	return is_new_key;
}

bool HashTable::get(ObjectString *key, LoxValue *value) {
	if (size == 0) return false;
	Entry *entry = find(key);
	if (entry->key == nullptr) return false;
	*value = entry->value;
	return true;
}

bool HashTable::del(ObjectString *key) {
	if (size == 0) return false;
	Entry *entry = find(key);
	if (entry->key == nullptr) return false;
	
	// tombstone
	entry->key = nullptr;
	entry->value = LoxValue(true);
	return true;
}

Entry *HashTable::find(ObjectString *key) {
	Entry *tombstone = nullptr;
	for (u32 index = (key->hash & (capacity - 1)); true; index = ((index + 1) & (capacity - 1))) {
		Entry *entry = &entries[index];
		if (entry->key == nullptr) {
			if (entry->value.is_nil()) {
				// empty entry
				return tombstone != nullptr ? tombstone : entry;
			}
			else {
				// tombstone found
				if (tombstone == nullptr) tombstone = entry;
			}
		}
		else if (entry->key == key) {
			return entry; // found key
		}
	}
}

Entry *HashTable::find_in_array(Entry array[], u32 array_cap, ObjectString *key) {
	for (u32 index = (key->hash & (array_cap - 1)); true; index = ((index + 1) & (array_cap - 1))) {
		Entry *entry = &array[index];
		if (entry->key == key || entry->key == nullptr) {
			return entry;
		}
	}
}

ObjectString *HashTable::find_string(std::string_view str) {
	if (size == 0) return nullptr;
	u32 hash = 2166136261u;
	for (u32 i=0; i<str.size(); i++) {
		hash ^= (u8) str[i];
		hash *= 16777619;
	}
	for (u32 index = (hash & (capacity - 1)); ; index = ((index + 1) & (capacity - 1))) {
		Entry *entry = &entries[index];
		if (entry->key == nullptr) {
			// non-tombstone entry
			if (entry->value.is_nil()) return nullptr;
		}
		else if (entry->key->length == str.size() &&
				entry->key->hash == hash &&
				std::memcmp(entry->key->chars.get(), str.data(), str.size()) == 0) {
			return entry->key;
		}
	}
}

void HashTable::adjust_capacity(u32 new_capacity) {
	auto new_entries = std::make_unique_for_overwrite<Entry[]>(new_capacity);
	for (u32 i=0; i<new_capacity; i++) {
		new_entries[i].key = nullptr;
		new_entries[i].value = LoxValue();
	}
	// move over
	size = 0;
	for (u32 i=0; i<capacity; i++) {
		Entry *entry = &entries[i];
		if (entry->key == nullptr) continue;
		
		Entry *dest = find_in_array(new_entries.get(), new_capacity, entry->key);
		dest->key = entry->key;
		dest->value = entry->value;
		size++;
	}

	entries = std::move(new_entries);
	capacity = new_capacity;
}

void HashTable::add_all(HashTable &table) {
	for (u32 i=0; i<table.capacity; i++) {
		Entry *entry = &table.entries[i];
		if (entry->key != nullptr) {
			set(entry->key, entry->value);
		}
	}
}

}
