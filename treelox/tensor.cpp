#include <assert.h>
#include <iostream>
#include <vector>
#include <memory>
#include <numeric>
using std::shared_ptr;
using std::make_shared;
using std::vector;
using std::size_t;

template<typename T>
class Tensor {
public:
	vector<size_t> dims;
	shared_ptr<T[]> shared_elems;
	T *elems;
	Tensor(vector<size_t> dims): dims(dims) {
		size_t size = 1;
		for (size_t i : dims) {
			size *= i;
		}
		assert(size != 0);
		shared_elems = make_shared<T[]>(size);
		elems = shared_elems.get();
	}

	Tensor(vector<size_t> dims, vector<T> data): dims(dims) {
		size_t size = 1;
		for (size_t i : dims) {
			size *= i;
		}
		assert(size != 0);
		assert(size == data.size());

		shared_elems = make_shared<T[]>(size);
		for (size_t i=0; i<data.size(); i++) {
			shared_elems[i] = data[i];
		}
		elems = shared_elems.get();
	}

	Tensor(Tensor &t, size_t slice_index) {
		dims = vector<size_t>(t.dims.begin()+1, t.dims.end());
		shared_elems = t.shared_elems;
		elems = shared_elems.get() + slice_index * std::reduce(dims.begin(), dims.end());
	}
	~Tensor() { }

	Tensor operator[](size_t index) {
		return Tensor(*this, index);
	}
	T &val() {
		return *elems;
	}
};

template<class T>
void tensor_dfs_out(std::ostream &out, vector<size_t> &dims, T *elems, size_t dim_index) {
	out << "[";
	if (dim_index == dims.size()-1) {
		for (int i=0; i+1<dims[dim_index]; i++) {
			out << elems[i] << ", ";
		}
		if (dims[dim_index] > 0) {
			out << elems[dims[dim_index]-1];
		}
	}
	else {
		size_t prod = 1;
		for (int i=dim_index+1; i<dims.size(); i++) {
			prod *= dims[i];
		}
		for (int i=0; i+1<dims[dim_index]; i++) {
			tensor_dfs_out(out, dims, elems + i * prod, dim_index+1);
			out << ", ";
		}
		if (dims[dim_index] > 0) {
			tensor_dfs_out(out, dims, elems + (dims[dim_index] - 1) * prod, dim_index+1);
		}
	}
	out << "]";
}

template <class T>
std::ostream &operator<<(std::ostream &out, Tensor<T> t) {
	out << "[";
	tensor_dfs_out(out, t.dims, t.elems, 0);
	out << "]";
	return out;
}

int main() {
	Tensor<int> t({2, 2, 2}, {0, 1, 2, 3, 4, 5, 6, 7});	

	auto s = t[1];
	std::cout << t.val() << "\n";
	std::cout << s.val() << "\n";
	std::cout << t[0].val() << " " << t[1].val() << " " << t[0][0].val() << " " << t[0][1].val() << "\n";
	std::cout << t << "\n";
}
