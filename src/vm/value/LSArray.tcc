#ifndef LS_ARRAY_TCC
#define LS_ARRAY_TCC

#include "../LSValue.hpp"
#include "../VM.hpp"
#include "LSNull.hpp"
#include "LSNumber.hpp"
#include "LSBoolean.hpp"
#include "LSFunction.hpp"
#include "LSObject.hpp"
#include "LSSet.hpp"

#include <algorithm>

namespace ls {

template <class T>
LSValue* LSArray<T>::array_class(new LSClass("Array"));

template <>
inline LSArray<LSValue*>::~LSArray() {
	for (auto v : *this) {
		LSValue::delete_ref(v);
	}
}
template <typename T>
LSArray<T>::~LSArray() {}

template <>
inline void LSArray<LSValue*>::push_clone(LSValue* value) {
	this->push_back(value->clone_inc());
}
template <typename T>
void LSArray<T>::push_clone(T value) {
	this->push_back(value);
}

template <>
inline void LSArray<LSValue*>::push_move(LSValue* value) {
	this->push_back(value->move_inc());
}
template <typename T>
void LSArray<T>::push_move(T value) {
	this->push_back(value);
}

template <>
inline void LSArray<LSValue*>::push_inc(LSValue* value) {
	if (!value->native) value->refs++;
	this->push_back(value);
}
template <class T>
inline void LSArray<T>::push_inc(T value) {
	this->push_back(value);
}

template <class T>
LSArray<T>::LSArray() {}

template <class T>
LSArray<T>::LSArray(std::initializer_list<T> values_list) {
	for (auto i : values_list) {
		this->push_back(i);
	}
}

template <class T>
LSArray<T>::LSArray(const std::vector<T>& vec) : LSValue(), std::vector<T>(vec) {}

template <>
inline LSArray<LSValue*>::LSArray(const LSArray<LSValue*>& other) : LSValue(other), std::vector<LSValue*>() {
	reserve(other.size());
	for (LSValue* v : other) {
		push_back(v->clone_inc());
	}
}
template <typename T>
inline LSArray<T>::LSArray(const LSArray<T>& other) : LSValue(other), std::vector<T>(other) {
}

template <typename T>
int LSArray<T>::int_size() {
	return this->size();
}

template <>
inline LSArray<LSValue*>* LSArray<LSValue*>::ls_clear() {
	for (auto v : *this) {
		LSValue::delete_ref(v);
	}
	this->clear();
	return this;
}
template <class T>
inline LSArray<T>* LSArray<T>::ls_clear() {
	this->clear();
	return this;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_remove(int index) {
	LSValue* previous = this->operator [] (index);
	this->erase(this->begin() + index);
	previous->refs--;
	if (refs == 0) delete this;
	return previous;
}
template <typename T>
inline T LSArray<T>::ls_remove(int index) {
	T previous = this->operator [] (index);
	this->erase(this->begin() + index);
	if (refs == 0) delete this;
	return previous;
}

template <>
inline bool LSArray<LSValue*>::ls_remove_element(LSValue* element) {
	for (size_t i = 0; i < this->size(); ++i) {
		if (*(*this)[i] == *element) {
			LSValue::delete_temporary(element);
			LSValue::delete_ref(this->operator[] (i));
			(*this)[i] = this->back();
			this->pop_back();
			if (refs == 0) delete this;
			return true;
		}
	}
	LSValue::delete_temporary(element);
	if (refs == 0) delete this;
	return false;
}
template <typename T>
inline bool LSArray<T>::ls_remove_element(T element) {
	for (size_t i = 0; i < this->size(); ++i) {
		if ((*this)[i] == element) {
			(*this)[i] = this->back();
			this->pop_back();
			if (refs == 0) delete this;
			return true;
		}
	}
	if (refs == 0) delete this;
	return false;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_sum() {
	if (this->size() == 0) return LSNumber::get(0);
	LSValue* sum = this->operator [] (0)->clone();
	for (size_t i = 1; i < this->size(); ++i) {
		sum = sum->ls_add((*this)[i]);
	}
	if (refs == 0) delete this;
	return sum;
}

template <class T>
T LSArray<T>::ls_sum() {
	T sum = 0;
	for (auto v : *this) {
		sum += v;
	}
	if (refs == 0) delete this;
	return sum;
}

template <typename T>
inline T LSArray<T>::ls_product() {
	T product = 1;
	for (auto v : *this) {
		product *= v;
	}
	if (refs == 0) delete this;
	return product;
}

template <>
inline double LSArray<LSValue*>::ls_average() {
	if (refs == 0) delete this;
	return 0; // No average for a no integer array
}

template <class T>
double LSArray<T>::ls_average() {
	if (this->size() == 0) return 0;
	return (double) this->ls_sum() / this->size();
}

template <>
inline LSValue* LSArray<LSValue*>::ls_first() {
	if (this->size() == 0) {
		if (refs == 0) {
			delete this;
		}
		jit_exception_throw(new VM::ExceptionObj(VM::Exception::ARRAY_OUT_OF_BOUNDS));
	}
	LSValue* first = front();
	if (refs == 0) {
		if (first->refs == 1) {
			(*this)[0] = nullptr;
			first->refs = 0;
		}
		delete this;
		// In that case `first` will survive
	}
	return first->move(); /* return temporary */
}

template <class T>
inline T LSArray<T>::ls_first() {
	if (this->size() == 0) {
		if (refs == 0) {
			delete this;
		}
		jit_exception_throw(new VM::ExceptionObj(VM::Exception::ARRAY_OUT_OF_BOUNDS));
	}
	T first = this->front();
	if (refs == 0) {
		delete this;
	}
	return first;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_last() {
	if (this->size() == 0) {
		if (refs == 0) {
			delete this;
		}
		jit_exception_throw(new VM::ExceptionObj(VM::Exception::ARRAY_OUT_OF_BOUNDS));
	}
	LSValue* last = back();
	if (refs == 0) {
		if (last->refs == 1) {
			pop_back();
			last->refs = 0;
		}
		delete this;
	}
	return last->move();
}

template <class T>
inline T LSArray<T>::ls_last() {
	if (this->size() == 0) {
		if (refs == 0) {
			delete this;
		}
		jit_exception_throw(new VM::ExceptionObj(VM::Exception::ARRAY_OUT_OF_BOUNDS));
	}
	T last = this->back();
	if (refs == 0) {
		delete this;
	}
	return last;
}

template <typename T>
inline bool LSArray<T>::ls_empty() {
	bool e = this->empty();
	if (refs == 0) delete this;
	return e;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_pop() {
	if (this->empty()) {
		if (refs == 0) {
			delete this;
		}
		jit_exception_throw(new VM::ExceptionObj(VM::Exception::ARRAY_OUT_OF_BOUNDS));
	}
	LSValue* last = this->back();
	last->refs--;
	this->pop_back();
	if (refs == 0) {
		delete this;
	}
	return last->move();
}

template <class T>
inline T LSArray<T>::ls_pop() {
	if (this->empty()) {
		if (refs == 0) {
			delete this;
		}
		jit_exception_throw(new VM::ExceptionObj(VM::Exception::ARRAY_OUT_OF_BOUNDS));
	}
	T last = this->back();
	this->pop_back();
	if (refs == 0) {
		delete this;
	}
	return last;
}

template <typename T>
inline int LSArray<T>::ls_size() {
	int s = this->size();
	if (refs == 0) {
		delete this;
	}
	return s;
}

template <class T>
template <class R>
LSArray<R>* LSArray<T>::ls_map(LSFunction<R>* function) {
	auto fun = (R (*)(void*, T)) function->function;
	LSArray<R>* result = new LSArray<R>();
	result->reserve(this->size());
	for (auto v : *this) {
		R r = fun(function, ls::clone(v));
		result->push_move(r);
	}
	LSValue::delete_temporary(this);
	return result;
}

template <typename T>
inline LSArray<LSValue*>* LSArray<T>::ls_chunk(int size) {
	if (size <= 0) size = 1;

	LSArray<LSValue*>* new_array = new LSArray<LSValue*>();

	new_array->reserve(this->size() / size + 1);

	size_t i = 0;
	while (i < this->size()) {
		LSArray<T>* sub_array = new LSArray<T>();
		sub_array->reserve(size);

		size_t j = std::min(i + size, this->size());
		if (refs == 0) {
			for (; i < j; ++i) {
				sub_array->push_inc((*this)[i]);
			}
		} else {
			for (; i < j; ++i) {
				sub_array->push_clone((*this)[i]);
			}
		}

		new_array->push_inc(sub_array);
	}
	if (refs == 0) {
		delete this;
	}
	return new_array;
}

template <>
inline LSArray<LSValue*>* LSArray<LSValue*>::ls_unique() {
	if (this->empty()) return this;

	auto it = this->begin();
	auto next = it;

	while (true) {
		++next;
		while (next != this->end() && (**next) == (**it)) {
			LSValue::delete_ref(*next);
			next++;
		}
		++it;
		if (next == this->end()) {
			break;
		}
		*it = *next;
	}
	this->resize(std::distance(this->begin(), it));
	return this;
}
template <class T>
LSArray<T>* LSArray<T>::ls_unique() {
	auto it = std::unique(this->begin(), this->end());
	this->resize(std::distance(this->begin(), it));
	return this;
}

template <>
inline LSArray<LSValue*>* LSArray<LSValue*>::ls_sort() {
	std::sort(this->begin(), this->end(), [](LSValue* a, LSValue* b) -> bool {
		return *a < *b;
	});
	return this;
}
template <class T>
LSArray<T>* LSArray<T>::ls_sort() {
	std::sort(this->begin(), this->end());
	return this;
}

template <class T>
void LSArray<T>::ls_iter(LSFunction<LSValue*>* function) {
	auto fun = (void* (*)(void*, T)) function->function;
	for (auto v : *this) {
		fun(function, v);
	}
	if (refs == 0) {
		delete this;
	}
}

template <>
inline bool LSArray<LSValue*>::ls_contains(LSValue* val) {
	for (auto v : *this) {
		if (*v == *val) {
			if (refs == 0) delete this;
			if (val->refs == 0) delete val;
			return true;
		}
	}
	if (refs == 0) delete this;
	if (val->refs == 0) delete val;
	return false;
}
template <class T>
bool LSArray<T>::ls_contains(T val) {
	for (auto v : *this) {
		if (v == val) {
			if (refs == 0) delete this;
			return true;
		}
	}
	if (refs == 0) delete this;
	return false;
}

template <>
inline LSArray<LSValue*>* LSArray<LSValue*>::ls_push(LSValue* val) {
	this->push_back(val->move_inc());
	return this;
}
template <class T>
LSArray<T>* LSArray<T>::ls_push(T val) {
	this->push_back(val);
	return this;
}

template <>
inline LSArray<LSValue*>* LSArray<LSValue*>::ls_push_all_ptr(LSArray<LSValue*>* array) {
	this->reserve(this->size() + array->size());
	if (array->refs == 0) {
		for (LSValue* v : *array) {
			this->push_back(v);
		}
		array->clear();
		delete array;
	} else {
		for (LSValue* v : *array) {
			this->push_clone(v);
		}
	}
	return this;
}
template <typename T>
inline LSArray<T>* LSArray<T>::ls_push_all_ptr(LSArray<LSValue*>* array) {
	this->reserve(this->size() + array->size());
	for (LSValue* v : *array) {
		if (LSNumber* n = dynamic_cast<LSNumber*>(v)) {
			this->push_back(n->value);
		}
	}
	if (array->refs == 0) delete array;
	return this;
}

template <>
inline LSArray<LSValue*>* LSArray<LSValue*>::ls_push_all_int(LSArray<int>* array) {
	this->reserve(this->size() + array->size());
	for (int v : *array) {
		this->push_inc(LSNumber::get(v));
	}
	if (array->refs == 0) delete array;
	return this;
}

template <typename T>
inline LSArray<T>* LSArray<T>::ls_push_all_int(LSArray<int>* array) {
	this->reserve(this->size() + array->size());
	this->insert(this->end(), array->begin(), array->end());
	if (array->refs == 0) delete array;
	return this;
}

template <>
inline LSArray<LSValue*>* LSArray<LSValue*>::ls_push_all_flo(LSArray<double>* array) {
	this->reserve(this->size() + array->size());
	for (double v : *array) {
		this->push_inc(LSNumber::get(v));
	}
	if (array->refs == 0) delete array;
	return this;
}

template <typename T>
inline LSArray<T>* LSArray<T>::ls_push_all_flo(LSArray<double>* array) {
	this->reserve(this->size() + array->size());
	this->insert(this->end(), array->begin(), array->end());
	if (array->refs == 0) delete array;
	return this;
}

template <typename T>
inline LSArray<T>* LSArray<T>::ls_shuffle() {
	if (refs == 0) {
		for (size_t i = 0; i < this->size(); ++i) {
			size_t j = rand() % this->size();
			T tmp = (*this)[i];
			(*this)[i] = (*this)[j];
			(*this)[j] = tmp;
		}
		return this;
	} else {
		LSArray<T>* new_array = new LSArray<T>();
		new_array->reserve(this->size());
		for (auto v : *this) {
			new_array->push_clone(v);
		}
		for (size_t i = 0; i < new_array->size(); ++i) {
			size_t j = rand() % new_array->size();
			T tmp = (*new_array)[i];
			(*new_array)[i] = (*new_array)[j];
			(*new_array)[j] = tmp;
		}
		return new_array;
	}
}

template <typename T>
inline LSArray<T>* LSArray<T>::ls_reverse() {
	if (refs == 0) {
		for (size_t i = 0, j = this->size(); i < j; ++i, --j) {
			T tmp = (*this)[i];
			(*this)[i] = (*this)[j-1];
			(*this)[j-1] = tmp;
		}
		return this;
	} else {
		LSArray<T>* new_array = new LSArray<T>();
		new_array->reserve(this->size());
		for (auto it = this->rbegin(); it != this->rend(); it++) {
			new_array->push_clone(*it);
		}
		return new_array;
	}
}

template <>
inline LSArray<LSValue*>* LSArray<LSValue*>::ls_filter(LSFunction<bool>* function) {
	auto fun = (bool (*)(void*, void*)) function->function;

	if (refs == 0) {
		for (size_t i = 0; i < this->size(); ) {
			LSValue* v = (*this)[i];
			if (!fun(function, v)) {
				LSValue::delete_ref(v);
				(*this)[i] = this->back();
				this->pop_back();
			} else {
				++i;
			}
		}
		return this;
	} else {
		LSArray<LSValue*>* new_array = new LSArray<LSValue*>();
		new_array->reserve(this->size());
		for (auto v : *this) {
			if (fun(function, v)) new_array->push_clone(v);
		}
		return new_array;
	}
}

template <class T>
LSArray<T>* LSArray<T>::ls_filter(LSFunction<bool>* function) {
	auto fun = (bool (*)(void*, T)) function->function;
	if (refs == 0) {
		for (size_t i = 0; i < this->size(); ) {
			T v = (*this)[i];
			if (!fun(function, v)) {
				this->erase(this->begin() + i);
			} else {
				++i;
			}
		}
		return this;
	} else {
		LSArray<T>* new_array = new LSArray<T>();
		new_array->reserve(this->size());
		for (auto v : *this) {
			if (fun(function, v)) new_array->push_clone(v);
		}
		return new_array;
	}
}

template <class T>
template <class R>
R LSArray<T>::ls_foldLeft(LSFunction<R>* function, R v0) {
	auto fun = (R (*)(void*, R, T)) function->function;
	R result = ls::move(v0);
	for (const auto& v : *this) {
		result = fun(function, result, v);
	}
	if (refs == 0) delete this;
	return result;
}

template <class T>
template <class R>
R LSArray<T>::ls_foldRight(LSFunction<R>* function, R v0) {
	auto fun = (R (*)(void*, T, R)) function->function;
	R result = ls::move(v0);
	for (auto it = this->rbegin(); it != this->rend(); it++) {
		result = fun(function, *it, result);
	}
	if (refs == 0) delete this;
	return result;
}

template <>
inline LSArray<LSValue*>* LSArray<LSValue*>::ls_insert(LSValue* value, int pos) {
	if (pos >= (int) this->size()) {
		this->resize(pos, LSNull::get());
	}
	this->insert(this->begin() + pos, value->move_inc());
	return this;
}

template <class T>
LSArray<T>* LSArray<T>::ls_insert(T value, int pos) {
	if (pos >= (int) this->size()) {
		this->resize(pos, (T) 0);
	}
	this->insert(this->begin() + pos, value);
	return this;
}

template <class T>
LSArray<LSValue*>* LSArray<T>::ls_partition(LSFunction<bool>* function) {
	LSArray<T>* array_true = new LSArray<T>();
	LSArray<T>* array_false = new LSArray<T>();
	auto fun = (bool (*)(void*, T)) function->function;
	for (const auto& v : *this) {
		if (fun(function, v)) {
			array_true->push_clone(v);
		} else {
			array_false->push_clone(v);
		}
	}
	if (refs == 0) delete this;
	array_true->refs = 1;
	array_false->refs = 1;
	return new LSArray<LSValue*> {array_true, array_false};
}

template <class T>
template <class R, class T2>
LSArray<R>* LSArray<T>::ls_map2(LSArray<T2>* array, LSFunction<R>* function) {
	LSArray<R>* result = new LSArray<R>();
	result->reserve(this->size());
	auto fun = (R (*)(void*, T, T2)) function->function;
	for (size_t i = 0; i < this->size(); ++i) {
		T v1 = this->operator [] (i);
		T2 v2 = array->operator [] (i);
		R res = fun(function, v1, v2);
		result->push_move(res);
	}
	LSValue::delete_temporary(this);
	LSValue::delete_temporary(array);
	return result;
}

template <>
inline int LSArray<LSValue*>::ls_search(LSValue* needle, int start) {
	for (size_t i = start; i < this->size(); i++) {
		if (*needle == *(*this)[i]) {
			if (refs == 0) delete this;
			LSValue::delete_temporary(needle);
			return i;
		}
	}
	if (refs == 0) delete this;
	LSValue::delete_temporary(needle);
	return -1;
}

template <class T>
int LSArray<T>::ls_search(T needle, int start) {
	for (size_t i = start; i < this->size(); i++) {
		if (needle == (*this)[i]) {
			if (refs == 0) delete this;
			return i;
		}
	}
	if (refs == 0) delete this;
	return -1;
}

template <>
inline LSString* LSArray<LSValue*>::ls_join(LSString* glue) {
	if (this->empty()) {
		if (refs == 0) delete this;
		if (glue->refs == 0) delete glue;
		return new LSString();
	}
	glue->refs++; // because we will use it several times
	auto it = this->begin();
	LSValue* result = new LSString();
	result = result->ls_add(*it);
	for (++it; it != this->end(); ++it) {
		result = result->ls_add(glue);
		result = result->ls_add(*it);
	}
	glue->refs--;
	if (refs == 0) delete this;
	if (glue->refs == 0) delete glue;
	return (LSString*) result;
}

template <class T>
LSString* LSArray<T>::ls_join(LSString* glue) {
	if (this->empty()) {
		if (refs == 0) delete this;
		if (glue->refs == 0) delete glue;
		return new LSString();
	}
	size_t i = 0;
	std::string result = LSNumber::print(this->operator[] (i));
	for (i++; i < this->size(); i++) {
		result = result + *glue + LSNumber::print(this->operator[] (i));
	}
	if (refs == 0) delete this;
	if (glue->refs == 0) delete glue;
	return new LSString(result);
}

template <>
inline LSArray<LSValue*>* LSArray<LSValue*>::ls_fill(LSValue* element, int size) {
	this->clear();
	this->reserve(size);
	for (int i = 0; i < size; i++) {
		this->push_move(element);
	}
	LSValue::delete_temporary(element); // only useful if size = 0
	return this;
}

template <typename T>
inline LSArray<T>* LSArray<T>::ls_fill(T element, int size) {
	this->clear();
	this->resize(size, element);
	return this;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_max() {
	if (this->empty()) {
		if (refs == 0) delete this;
		jit_exception_throw(new VM::ExceptionObj(VM::Exception::ARRAY_OUT_OF_BOUNDS));
	}
	LSValue* max = (*this)[0];
	for (size_t i = 1; i < this->size(); ++i) {
		if (*(*this)[i] > *max) {
			max = (*this)[i];
		}
	}
	if (refs == 0) {
		max = max->clone();
		delete this;
	}
	return max;
}
template <class T>
T LSArray<T>::ls_max() {
	if (this->empty()) {
		if (refs == 0) delete this;
		jit_exception_throw(new VM::ExceptionObj(VM::Exception::ARRAY_OUT_OF_BOUNDS));
	}
	T max = (*this)[0];
	for (size_t i = 1; i < this->size(); ++i) {
		if ((*this)[i] > max) {
			max = (*this)[i];
		}
	}
	if (refs == 0) delete this;
	return max;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_min() {
	if (this->empty()) {
		if (refs == 0) delete this;
		jit_exception_throw(new VM::ExceptionObj(VM::Exception::ARRAY_OUT_OF_BOUNDS));
	}
	LSValue* min = (*this)[0];
	for (size_t i = 1; i < this->size(); ++i) {
		if (*(*this)[i] < *min) {
			min = (*this)[i];
		}
	}
	if (refs == 0) {
		min = min->clone();
		delete this;
	}
	return min;
}
template <class T>
T LSArray<T>::ls_min() {
	if (this->empty()) {
		if (refs == 0) delete this;
		jit_exception_throw(new VM::ExceptionObj(VM::Exception::ARRAY_OUT_OF_BOUNDS));
	}
	T min = (*this)[0];
	for (size_t i = 1; i < this->size(); ++i) {
		if ((*this)[i] < min) {
			min = (*this)[i];
		}
	}
	if (refs == 0) delete this;
	return min;
}

template <>
inline bool LSArray<int>::is_permutation(LSArray<int>* other) {
	bool result = this->size() == other->size() and std::is_permutation(this->begin(), this->end(), other->begin());
	LSValue::delete_temporary(this);
	LSValue::delete_temporary(other);
	return result;
}

/*
 * LSValue methods
 */
template <class T>
bool LSArray<T>::isTrue() const {
	return this->size() > 0;
}

template <class T>
LSValue* LSArray<T>::ls_not() {
	bool r = this->size() == 0;
	if (refs == 0) delete this;
	return LSBoolean::get(r);
}

template <class T>
LSValue* LSArray<T>::abso() const {
	return LSNumber::get(this->size());
}

template <class T>
LSValue* LSArray<T>::ls_tilde() {
	LSArray<T>* array = new LSArray<T>();
	array->reserve(this->size());
	if (refs == 0) {
		for (auto i = this->rbegin(); i != this->rend(); ++i) {
			array->push_back(*i);
		}
		this->clear();
		delete this;
	} else {
		for (auto i = this->rbegin(); i != this->rend(); ++i) {
			array->push_clone(*i);
		}
	}
	return array;
}

template <typename T>
LSValue* LSArray<T>::ls_add(LSNull* v) {
	LSArray<LSValue*>* r = new LSArray<LSValue*>();
	r->reserve(this->size() + 1);
	for (auto v : *this) {
		r->push_inc(LSNumber::get(v));
	}
	r->push_back(v);
	if (refs == 0) delete this;
	return r;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add(LSNull* v) {
	if (refs == 0) {
		push_back(v);
		return this;
	}
	LSArray<LSValue*>* new_array = (LSArray<LSValue*>*) this->clone();
	new_array->push_back(v);
	return new_array;
}

template <>
inline LSValue* LSArray<double>::ls_add(LSNull* v) {
	LSArray<LSValue*>* r = new LSArray<LSValue*>();
	r->reserve(this->size() + 1);
	for (auto v : *this) {
		r->push_inc(LSNumber::get(v));
	}
	r->push_back(v);
	if (refs == 0) delete this;
	return r;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add(LSBoolean* v) {
	if (refs == 0) {
		push_back(v);
		return this;
	}
	LSArray<LSValue*>* new_array = (LSArray<LSValue*>*) this->clone();
	new_array->push_back(v);
	return new_array;
}

template <typename T>
LSValue* LSArray<T>::ls_add(LSBoolean* v) {
	LSArray<LSValue*>* r = new LSArray<LSValue*>();
	r->reserve(this->size() + 1);
	for (auto v : *this) {
		r->push_inc(LSNumber::get(v));
	}
	r->push_back(v);
	if (refs == 0) delete this;
	return r;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add(LSNumber* v) {
	if (refs == 0) {
		push_move(v);
		return this;
	}
	LSArray<LSValue*>* new_array = (LSArray<LSValue*>*) this->clone();
	new_array->push_move(v);
	return new_array;
}

template <>
inline LSValue* LSArray<double>::ls_add(LSNumber* v) {
	if (refs == 0) {
		this->push_back(v->value);
		if (v->refs == 0) delete v;
		return this;
	}
	LSArray<double>* r = (LSArray<double>*) this->clone();
	r->push_back(v->value);
	if (v->refs == 0) delete v;
	return r;
}

template <>
inline LSValue* LSArray<int>::ls_add(LSNumber* v) {
	if (v->value == (int) v->value) {
		if (refs == 0) {
			this->push_back(v->value);
			if (v->refs == 0) delete v;
			return this;
		}
		LSArray<int>* r = (LSArray<int>*) this->clone();
		r->push_back(v->value);
		if (v->refs == 0) delete v;
		return r;
	}
	LSArray<double>* ret = new LSArray<double>();
	ret->insert(ret->end(), this->begin(), this->end());
	ret->push_back(v->value);
	if (refs == 0) delete this;
	if (v->refs == 0) delete v;
	return ret;
}

template <typename T>
LSValue* LSArray<T>::ls_add(LSString* v) {
	LSArray<LSValue*>* r = new LSArray<LSValue*>();
	for (auto v : *this) {
		r->push_inc(LSNumber::get(v));
	}
	r->push_move(v);
	if (refs == 0) delete this;
	return r;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add(LSString* v) {
	if (refs == 0) {
		this->push_move((LSValue*) v);
		return this;
	}
	LSArray<LSValue*>* r = (LSArray<LSValue*>*) this->clone();
	r->push_move(v);
	return r;
}

template <typename T>
LSValue* LSArray<T>::ls_add(LSArray<LSValue*>* array) {
	LSArray<LSValue*>* ret = new LSArray<LSValue*>();
	ret->reserve(this->size() + array->size());

	for (auto v : *this) {
		ret->push_inc(LSNumber::get(v));
	}
	if (array->refs == 0) {
		for (LSValue* v : *array) {
			ret->push_back(v); // steal the ownership
		}
		array->clear();
		delete array;
	} else {
		for (LSValue* v : *array) {
			ret->push_clone(v);
		}
	}
	if (refs == 0) delete this;
	return ret;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add(LSArray<LSValue*>* array) {
	if (refs == 0) {
		return ls_push_all_ptr(array);
	}
	return ((LSArray<LSValue*>*) this->clone())->ls_push_all_ptr(array);
}

template <typename T>
inline LSValue* LSArray<T>::ls_add(LSArray<int>* array) {
	if (refs == 0) {
		return ls_push_all_int(array);
	}
	return ((LSArray<T>*) this->clone())->ls_push_all_int(array);
}
template <typename T>
inline LSValue* LSArray<T>::ls_add(LSArray<double>* array) {
	if (refs == 0) {
		return ls_push_all_flo(array);
	}
	return ((LSArray<T>*) this->clone())->ls_push_all_flo(array);
}
template <>
inline LSValue* LSArray<int>::ls_add(LSArray<double>* array) {
	LSArray<double>* ret = new LSArray<double>();
	ret->reserve(this->size() + array->size());

	ret->insert(ret->end(), this->begin(), this->end());
	ret->insert(ret->end(), array->begin(), array->end());

	if (refs == 0) delete this;
	if (array->refs == 0) delete array;
	return ret;
}

template <typename T>
LSValue* LSArray<T>::ls_add(LSObject* v) {
	LSArray<LSValue*>* r = new LSArray<LSValue*>();
	for (auto v : *this) {
		r->push_inc(LSNumber::get(v));
	}
	r->push_move(v);
	if (refs == 0) delete this;
	return r;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add(LSObject* v) {
	if (refs == 0) {
		this->push_move(v);
		return this;
	}
	LSArray<LSValue*>* new_array = (LSArray<LSValue*>*) this->clone();
	new_array->push_move(v);
	return new_array;
}

template <typename T>
LSValue* LSArray<T>::ls_add(LSFunction<LSValue*>*) {
	return this;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add(LSFunction<LSValue*>* v) {
	if (refs == 0) {
		push_move((LSValue*) v);
		return this;
	}
	LSArray<LSValue*>* new_array = (LSArray<LSValue*>*) this->clone();
	new_array->push_move(v);
	return new_array;
}

template <typename T>
LSValue* LSArray<T>::ls_add(LSClass*) {
	return this;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add(LSClass* v) {
	if (refs == 0) {
		push_move(v);
		return this;
	}
	LSArray<LSValue*>* new_array = (LSArray<LSValue*>*) this->clone();
	new_array->push_move(v);
	return new_array;
}

template <typename T>
inline LSValue* LSArray<T>::ls_add_eq(LSNull*) {
	return this;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add_eq(LSNull* v) {
	push_back(v);
	return this;
}

template <typename T>
inline LSValue* LSArray<T>::ls_add_eq(LSBoolean*) {
	return this;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add_eq(LSBoolean* v) {
	push_back(v);
	return this;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add_eq(LSNumber* v) {
	push_move(v);
	return this;
}

template <>
inline LSValue* LSArray<double>::ls_add_eq(LSNumber* v) {
	this->push_back(v->value);
	if (v->refs == 0) delete v;
	return this;
}

template <>
inline LSValue* LSArray<int>::ls_add_eq(LSNumber* v) {
	this->push_back(v->value);
	LSValue::delete_temporary(v);
	return this;
}

template <typename T>
LSValue* LSArray<T>::ls_add_eq(LSString*) {
	return this;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add_eq(LSString* v) {
	this->push_move((LSValue*) v);
	return this;
}

template <typename T>
LSValue* LSArray<T>::ls_add_eq(LSArray<LSValue*>* array) {
	LSArray<LSValue*>* r = new LSArray<LSValue*>();
	r->reserve(this->size() + array->size());

	for (auto v : *this) {
		r->push_inc(LSNumber::get(v));
	}
	if (array->refs == 0) {
		for (LSValue* v : *array) {
			r->push_back(v); // steal the ownership
		}
		array->clear();
		delete array;
	} else {
		for (LSValue* v : *array) {
			r->push_clone(v);
		}
	}
	r->refs = 1;
	LSValue::delete_ref(this);
	return r;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add_eq(LSArray<LSValue*>* array) {
	ls_push_all_ptr(array);
	return this;
}

template <typename T>
inline LSValue* LSArray<T>::ls_add_eq(LSArray<int>* array) {
	return ls_push_all_int(array);
}

template <typename T>
inline LSValue* LSArray<T>::ls_add_eq(LSArray<double>* array) {
	return ls_push_all_flo(array);
}

template <>
inline LSValue* LSArray<int>::ls_add_eq(LSArray<double>*) {
	return this;
}

template <class T>
template <class T2>
LSValue* LSArray<T>::add_set(LSSet<T2>* set) {
	this->reserve(this->size() + set->size());
	if (set->refs == 0) {
		for (T2 v : *set) {
			this->push_back(ls::convert<T>(v));
		}
		set->clear();
		delete set;
	} else {
		for (T2 v : *set) {
			this->push_clone(ls::convert<T>(v));
		}
	}
	return this;
}

template <class T>
LSValue* LSArray<T>::ls_add_eq(LSSet<LSValue*>*) {
	return this;
}
template <>
inline LSValue* LSArray<LSValue*>::ls_add_eq(LSSet<LSValue*>* set) {
	return add_set(set);
}

template <class T>
LSValue* LSArray<T>::ls_add_eq(LSSet<int>* set) {
	return add_set(set);
}

template <class T>
LSValue* LSArray<T>::ls_add_eq(LSSet<double>* set) {
	return add_set(set);
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add_eq(LSObject* v) {
	this->push_move(v);
	return this;
}
template <typename T>
inline LSValue* LSArray<T>::ls_add_eq(LSObject*) {
	return this;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add_eq(LSFunction<LSValue*>* v) {
	push_move(v);
	return this;
}
template <typename T>
inline LSValue* LSArray<T>::ls_add_eq(LSFunction<LSValue*>*) {
	return this;
}

template <>
inline LSValue* LSArray<LSValue*>::ls_add_eq(LSClass* v) {
	push_move(v);
	return this;
}
template <typename T>
inline LSValue* LSArray<T>::ls_add_eq(LSClass*) {
	return this;
}

template <class T, class T2>
bool array_equals(const LSArray<T>* self, const LSArray<T2>* array) {
	if (self->size() != array->size()) {
		return false;
	}
	auto j = array->begin();
	for (auto i = self->begin(); i != self->end(); i++, j++) {
		if (!ls::equals(*j, *i)) return false;
	}
	return true;
}

template <class T>
bool LSArray<T>::eq(const LSArray<LSValue*>* array) const {
	return array_equals(this, array);
}

template <class T>
bool LSArray<T>::eq(const LSArray<int>* array) const {
	return array_equals(this, array);
}

template <class T>
bool LSArray<T>::eq(const LSArray<double>* array) const {
	return array_equals(this, array);
}

template <class T, class T2>
bool array_lt(const LSArray<T>* self, const LSArray<T2>* array) {
	auto i = self->begin();
	auto j = array->begin();
	while (i != self->end()) {
		if (j == array->end()) return false;
		if (3 < (*i)->typeID()) return false;
		if ((*i)->typeID() < 3) return true;
		if (((LSNumber*) *i)->value < *j) return true;
		if (*j < ((LSNumber*) *i)->value) return false;
		++i; ++j;
	}
	return j != array->end();
}

template <typename T>
inline bool LSArray<T>::lt(const LSArray<LSValue*>* v) const {
	auto i = this->begin();
	auto j = v->begin();
	while (i != this->end()) {
		if (j == v->end()) return false;
		if ((*j)->typeID() < 3) return false;
		if (3 < (*j)->typeID()) return true;
		if (*i < ((LSNumber*) *j)->value) return true;
		if (((LSNumber*) *j)->value < *i) return false;
		++i; ++j;
	}
	return (j != v->end());
}

template <>
inline bool LSArray<LSValue*>::lt(const LSArray<LSValue*>* v) const {
	return std::lexicographical_compare(begin(), end(), v->begin(), v->end(), [](const LSValue* a, const LSValue* b) -> bool {
		return *a < *b;
	});
}

template <>
inline bool LSArray<LSValue*>::lt(const LSArray<int>* v) const {
	return array_lt(this, v);
}

template <typename T>
inline bool LSArray<T>::lt(const LSArray<int>* v) const {
	return std::lexicographical_compare(this->begin(), this->end(), v->begin(), v->end());
}

template <>
inline bool LSArray<LSValue*>::lt(const LSArray<double>* v) const {
	return array_lt(this, v);
}

template <typename T>
inline bool LSArray<T>::lt(const LSArray<double>* v) const {
	return std::lexicographical_compare(this->begin(), this->end(), v->begin(), v->end());
}

template <typename T>
inline bool LSArray<T>::in(const T value) const {
	for (auto i = this->begin(); i != this->end(); i++) {
		if (*i == value) {
			LSValue::delete_temporary(this);
			return true;
		}
	}
	LSValue::delete_temporary(this);
	return false;
}

template <>
inline bool LSArray<LSValue*>::in(LSValue* const value) const {
	for (auto i = this->begin(); i != this->end(); i++) {
		if (**i == *value) {
			LSValue::delete_temporary(this);
			LSValue::delete_temporary(value);
			return true;
		}
	}
	LSValue::delete_temporary(this);
	LSValue::delete_temporary(value);
	return false;
}

template <typename T>
inline T LSArray<T>::atv(const int i) {
	try {
		return ((std::vector<T>*) this)->at(i);
	} catch (...) {
		LSValue::delete_temporary(this);
		jit_exception_throw(new VM::ExceptionObj(VM::Exception::ARRAY_OUT_OF_BOUNDS));
		return 0;
	}
}

template <>
inline int* LSArray<int>::atLv(int i) {
	return &this->operator[] (i);
}

template <typename T>
inline LSValue* LSArray<T>::range(int start, int end) const {

	LSArray<T>* range = new LSArray<T>();

	size_t start_i = std::max<size_t>(0, start);
	size_t end_i = std::min<size_t>(this->size() - 1, end);

	for (size_t i = start_i; i <= end_i; ++i) {
		range->push_clone(this->operator [] (i));
	}
	return range;
}

template <class T>
LSValue* LSArray<T>::rangeL(int, int) {
	return this;
}

template <class T>
LSValue* LSArray<T>::clone() const {
	return new LSArray<T>(*this);
}

template <typename T>
std::ostream& LSArray<T>::dump(std::ostream& os) const {
	os << "[";
	for (auto i = this->begin(); i != this->end(); i++) {
		if (i != this->begin()) os << ", ";
		os << (*i);
	}
	os << "]";
	return os;
}

template <>
inline std::ostream& LSArray<LSValue*>::dump(std::ostream& os) const {
	os << "[";
	for (auto i = this->begin(); i != this->end(); i++) {
		if (i != this->begin()) os << ", ";
		(*i)->dump(os);
	}
	os << "]";
	return os;
}

template <typename T>
std::string LSArray<T>::json() const {
	std::string res = "[";
	bool last_valid = true;
	for (auto i = this->begin(); i != this->end(); i++) {
		if (i != this->begin() and last_valid) res += ", ";
		auto j = ls::to_json(*i);
		if (j.size()) res += j;
		last_valid = j.size() > 0;
	}
	return res + "]";
}

template <class T>
LSValue* LSArray<T>::at(const LSValue* key) const {
	int index = 0;
	if (const LSNumber* n = dynamic_cast<const LSNumber*>(key)) {
		index = (int) n->value;
	} else if (const LSBoolean* b = dynamic_cast<const LSBoolean*>(key)) {
		index = (int) b->value;
	} else {
		LSValue::delete_temporary(this);
		LSValue::delete_temporary(key);
		jit_exception_throw(new VM::ExceptionObj(VM::Exception::ARRAY_KEY_IS_NOT_NUMBER));
	}
	LSValue* res = nullptr;
	try {
		res = ls::convert<LSValue*>(ls::clone(((std::vector<T>*) this)->at(index)));
	} catch (std::exception& e) {
		LSValue::delete_temporary(this);
		LSValue::delete_temporary(key);
		jit_exception_throw(new VM::ExceptionObj(VM::Exception::ARRAY_OUT_OF_BOUNDS));
	}
	return res;
}

template <class T>
LSValue** LSArray<T>::atL(const LSValue* key) {
	if (const LSNumber* n = dynamic_cast<const LSNumber*>(key)) {
		try {
			LSValue** v = (LSValue**) &(((std::vector<T>*)this)->at((int) n->value));
			return v;
		} catch (std::exception& e) {
			return nullptr;
		}
	}
	return nullptr;
}

template <class T>
LSValue* LSArray<T>::getClass() const {
	return LSArray<T>::array_class;
}

} // end of namespace ls

#endif
