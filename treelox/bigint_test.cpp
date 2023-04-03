#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "bigint.h"

#include <climits>
#include <iostream>
using namespace std;

TEST_CASE("long long constructor") {
	i64 ll = 82347593201;
	BigInt a(ll);
	CHECK(a.size == 1);
	CHECK(a.vals[0] == ll);
}

TEST_CASE("addition no overflow") {
	u64 diff = 9157348;
	BigInt a = ULLONG_MAX - diff;
	BigInt b = diff;
	a += b;
	CHECK(a.size == 1);
	CHECK(a.vals[0] == ULLONG_MAX);
}

TEST_CASE("addition overflow") {
	BigInt a = ULLONG_MAX;	
	BigInt b = ULLONG_MAX;

	a += b;
	CHECK(a.size == 2);
	CHECK(a.vals[0] == ULLONG_MAX + ULLONG_MAX);
	CHECK(a.vals[1] == 1);

	a += 2;
	CHECK(a.size == 2);
	CHECK(a.vals[0] == 0);
	CHECK(a.vals[1] == 2);
}
