#pragma once

#include <algorithm>
#include <string>

using i64 = std::int64_t;
using u64 = std::uint64_t;
using i32 = std::int32_t;
using u32 = std::uint32_t;

struct BigInt {
	u32 size;  // number of u64 value bytes in vals
	u64 *vals; // u64 value array, little endian, twos complement representation

	BigInt();
	BigInt(const BigInt &);
	BigInt(const long long &);
	BigInt(const std::string &);
	
	~BigInt();

	bool sign() const;

	BigInt &operator=(const BigInt &);
	BigInt &operator=(const long long &);
	BigInt &operator=(const std::string &);

	BigInt &operator+=(const BigInt &);
	/*
	BigInt &operator-=(const BigInt &);
	BigInt &operator*=(const BigInt &);
	BigInt &operator/=(const BigInt &);
	BigInt &operator%=(const BigInt &);
	BigInt &operator+=(const long long &);
	BigInt &operator-=(const long long &);
	BigInt &operator*=(const long long &);
	BigInt &operator/=(const long long &);
	BigInt &operator%=(const long long &);
	BigInt &operator+=(const std::string &);
	BigInt &operator-=(const std::string &);
	BigInt &operator*=(const std::string &);
	BigInt &operator/=(const std::string &);
	BigInt &operator%=(const std::string &);

	[[nodiscard]] BigInt &operator+(const BigInt &) const;
	[[nodiscard]] BigInt &operator-(const BigInt &) const;
	[[nodiscard]] BigInt &operator*(const BigInt &) const;
	[[nodiscard]] BigInt &operator/(const BigInt &) const;
	[[nodiscard]] BigInt &operator%(const BigInt &) const;
	[[nodiscard]] BigInt &operator+(const long long &) const;
	[[nodiscard]] BigInt &operator-(const long long &) const;
	[[nodiscard]] BigInt &operator*(const long long &) const;
	[[nodiscard]] BigInt &operator/(const long long &) const;
	[[nodiscard]] BigInt &operator%(const long long &) const;
	[[nodiscard]] BigInt &operator+(const std::string &) const;
	[[nodiscard]] BigInt &operator-(const std::string &) const;
	[[nodiscard]] BigInt &operator*(const std::string &) const;
	[[nodiscard]] BigInt &operator/(const std::string &) const;
	[[nodiscard]] BigInt &operator%(const std::string &) const;

	BigInt &operator++(); // pre-increment
	BigInt &operator--();
	BigInt operator++(int); // post-increment
	BigInt operator--(int);

	[[nodiscard]] bool operator<(const BigInt &) const;
	[[nodiscard]] bool operator>(const BigInt &) const;
	[[nodiscard]] bool operator<=(const BigInt &) const;
	[[nodiscard]] bool operator>=(const BigInt &) const;
	[[nodiscard]] bool operator==(const BigInt &) const;
	[[nodiscard]] bool operator!=(const BigInt &) const;
	[[nodiscard]] bool operator<(const long long &) const;
	[[nodiscard]] bool operator>(const long long &) const;
	[[nodiscard]] bool operator<=(const long long &) const;
	[[nodiscard]] bool operator>=(const long long &) const;
	[[nodiscard]] bool operator==(const long long &) const;
	[[nodiscard]] bool operator!=(const long long &) const;
	[[nodiscard]] bool operator<(const std::string &) const;
	[[nodiscard]] bool operator>(const std::string &) const;
	[[nodiscard]] bool operator<=(const std::string &) const;
	[[nodiscard]] bool operator>=(const std::string &) const;
	[[nodiscard]] bool operator==(const std::string &) const;
	[[nodiscard]] bool operator!=(const std::string &) const;

	friend std::istream &operator>>(std::istream &, BigInt &);
	friend std::ostream &operator<<(std::ostream &, BigInt &);

	int to_int() const;
	long long to_ll() const;
	std::string to_string() const;
	*/
};

static void mult64to128(uint64_t op1, uint64_t op2, uint64_t *hi, uint64_t *lo)
{
    uint64_t u1 = (op1 & 0xffffffff);
    uint64_t v1 = (op2 & 0xffffffff);
    uint64_t t = (u1 * v1);
    uint64_t w3 = (t & 0xffffffff);
    uint64_t k = (t >> 32);

    op1 >>= 32;
    t = (op1 * v1) + k;
    k = (t & 0xffffffff);
    uint64_t w1 = (t >> 32);

    op2 >>= 32;
    t = (u1 * op2) + k;
    k = (t >> 32);

    *hi = (op1 * op2) + w1 + k;
    *lo = (t << 32) + w3;
}