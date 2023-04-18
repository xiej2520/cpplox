#include "bigint.h"

static constexpr u64 b_63 = 1ULL << 63;

BigInt::BigInt() {
	size = 0;
	//vals = nullptr;	
}	

BigInt::BigInt(const BigInt &bi): size(bi.size) {
	vals = new u64[size];
	std::copy(bi.vals, bi.vals + size, vals);
}

BigInt::BigInt(const long long &ll) {
	size = 1;
	vals = new u64[1];
	vals[0] = ll;
}

BigInt::BigInt(const std::string &s) {
	if (s.empty()) size = 0;
	for (size_t i=0; i<s.size(); ++i) {
		if (s[i] != ' ' && s[i] != ',' && s[i] < '0' && s[i] > '9') throw std::exception();
	}

		/*
	size_t val_idx = 0;
	u64 sum = 0;
	for (size_t i=s.size()-1; i>(s[0] == '-'); --i) {
		 * Easier to do this with +=
		u64 next = sum * 10 + s[i];
		if (next < sum) {
			vals[val_idx++] = next;
		}
		else {
			sum = next;
		}
	}
		*/
}

BigInt::~BigInt() {
	delete[] vals;
}

bool BigInt::sign() const {
	if (size == 0) return false;
	return vals[size-1] & b_63;
}

BigInt &BigInt::operator=(const BigInt &bi) {
	size = bi.size;
	delete[] vals;
	vals = new u64[size];
	std::copy(bi.vals, bi.vals + size, vals);
	return *this;
}

BigInt &BigInt::operator=(const long long &ll) {
	size = 1;
	delete[] vals;
	vals = new u64[1];
	vals[0] = ll;
	return *this;
}

BigInt &BigInt::operator=(const std::string &s) {
	return *this;
}

BigInt &BigInt::operator+=(const BigInt &bi) {
	// do not preemptively allocate more memory if same size
	if (bi.size > size) {
		u64 *new_vals = new u64[bi.size];
		std::copy(vals, vals + size, new_vals);
		std::fill(new_vals + size, new_vals + bi.size, 0);
		size = bi.size;
		delete[] vals;
		vals = new_vals;
	}
	u64 carry = 0;
	for (size_t i=0; i<size; i++) {
		u64 next = vals[i] + bi.vals[i] + carry;
		carry = next < vals[i];
		vals[i] = next;
	}

	if (carry == 1) {
		u64 *new_vals = new u64[size + 1];
		std::copy(vals, vals + size, new_vals);
		new_vals[size] = 1;
		delete[] vals;
		vals = new_vals;
		size++;
	}
	return *this;
}


// if((a + b) < a)
