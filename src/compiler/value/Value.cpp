#include "Value.hpp"
#include "../../vm/Type.hpp"

namespace ls {

Value::Value() {
	type = Type::UNKNOWN;
	constant = false;
}

Value::~Value() {}

bool Value::will_take(SemanticAnalyser*, const std::vector<Type>& args_type, int) {
	return type.will_take(args_type);
}

bool Value::will_store(SemanticAnalyser*, const Type&) {
	return false;
}

bool Value::must_be_pointer(SemanticAnalyser*) {
	if (type.nature == Nature::POINTER) {
		return false;
	}
	type.nature = Nature::POINTER;
	types = type;
	return true;
}

void Value::must_return(SemanticAnalyser*, const Type& ret_type) {
	type.setReturnType(ret_type);
}

void Value::will_be_in_array(SemanticAnalyser*) {}

bool Value::isLeftValue() const {
	return false;
}

std::string Value::tabs(int indent) {
	return std::string(indent * 4, ' ');
}

std::string Value::to_string() const {
	std::ostringstream oss;
	oss << this;
	return oss.str();
}

std::ostream& operator << (std::ostream& os, const Value* v) {
	v->print(os);
	return os;
}

}
