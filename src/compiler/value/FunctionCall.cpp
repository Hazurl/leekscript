#include "../../compiler/value/FunctionCall.hpp"

#include <math.h>

#include "../../compiler/value/Function.hpp"
#include "../../compiler/value/ObjectAccess.hpp"
#include "../../compiler/value/VariableValue.hpp"
#include "../../vm/VM.hpp"
#include "../semantic/SemanticAnalyser.hpp"
#include "../semantic/SemanticException.hpp"
#include "../../vm/standard/ArraySTD.hpp"
#include "../../vm/standard/NumberSTD.hpp"
#include "../../vm/value/LSClass.hpp"
#include "../../vm/value/LSNumber.hpp"
#include "../../vm/value/LSArray.hpp"

using namespace std;

namespace ls {

FunctionCall::FunctionCall() {
	function = nullptr;
	type = Type::VALUE;
	std_func = nullptr;
	this_ptr = nullptr;
}

FunctionCall::~FunctionCall() {
	delete function;
	for (auto arg : arguments) {
		delete arg;
	}
}

void FunctionCall::print(std::ostream& os) const {
	function->print(os);
	os << "(";
	for (unsigned i = 0; i < arguments.size(); ++i) {
		arguments.at(i)->print(os);
		if (i < arguments.size() - 1) {
			os << ", ";
		}
	}
	os << ")";
}

void FunctionCall::analyse(SemanticAnalyser* analyser, const Type) {

	constant = false;

	function->analyse(analyser);

	int a = 0;
	for (Value* arg : arguments) {
//		cout << "ANALYSE arg " << a << " : " << function->type.getArgumentType(a) << endl;
		arg->analyse(analyser, function->type.getArgumentType(a++));
	}

	//cout << "Function call function type : " << function->type << endl;

	// Detect standard library functions
	ObjectAccess* oa = dynamic_cast<ObjectAccess*>(function);
	if (oa != nullptr) {

		VariableValue* vv = dynamic_cast<VariableValue*>(oa->object);
		if (vv != nullptr and vv->name->content == "Number") {
			string field_name = oa->field->content;
			if (field_name == "abs") {
				function->type.setArgumentType(0, Type::INTEGER);
				function->type.setReturnType(Type::INTEGER);
				is_native = true;
			} else if (field_name == "floor") {
				function->type.setArgumentType(0, Type::FLOAT);
				function->type.setReturnType(Type::INTEGER);
				is_native = true;
			} else if (field_name == "round") {
				function->type.setArgumentType(0, Type::FLOAT);
				function->type.setReturnType(Type::FLOAT);
				is_native = true;
			} else if (field_name == "ceil") {
				function->type.setArgumentType(0, Type::FLOAT);
				function->type.setReturnType(Type::INTEGER);
				is_native = true;
			} else if (field_name == "cos") {
				function->type.setArgumentType(0, Type::FLOAT);
				function->type.setReturnType(Type::FLOAT);
				is_native = true;
			} else if (field_name == "sin") {
				function->type.setArgumentType(0, Type::FLOAT);
				function->type.setReturnType(Type::FLOAT);
				is_native = true;
			} else if (field_name == "max") {
				function->type.setArgumentType(0, Type::FLOAT);
				function->type.setArgumentType(1, Type::FLOAT);
				function->type.setReturnType(Type::FLOAT);
				is_native = true;
			} else if (field_name == "min") {
				function->type.setArgumentType(0, Type::FLOAT);
				function->type.setArgumentType(1, Type::FLOAT);
				function->type.setReturnType(Type::FLOAT);
				is_native = true;
			} else if (field_name == "sqrt") {
				function->type.setArgumentType(0, Type::FLOAT);
				function->type.setReturnType(Type::FLOAT);
				is_native = true;
			} else if (field_name == "pow") {
				function->type.setArgumentType(0, Type::FLOAT);
				function->type.setArgumentType(1, Type::FLOAT);
				function->type.setReturnType(Type::FLOAT);
				is_native = true;
			}
			native_func = field_name;
		}

		if (!is_native) {

//			cout << "oa : " << oa->field << endl;

			Type object_type = oa->object->type;

			vector<Type> arg_types;
			for (auto arg : arguments) arg_types.push_back(arg->type);

			if (arg_types.size() > 0) {
			cout << "ARG : " << arg_types[0] << endl;
//			cout << "ARG : " << arg_types[1] << endl;
			}
			if (object_type.raw_type == RawType::CLASS) {
				// String.size("salut")

//				cout << "object_type : " << object_type << endl;
//				cout << "object field : " << oa->field << endl;

				string clazz = ((VariableValue*) oa->object)->name->content;

				LSClass* object_class = (LSClass*) analyser->program->system_vars[clazz];

				StaticMethod* m = object_class->getStaticMethod(oa->field->content, arg_types);

				if (m != nullptr) {
//					cout << "method : " << m->addr << endl;
					std_func = m->addr;
					function->type = m->type;
				} else {
					throw SemanticException(SemanticException::Type::STATIC_METHOD_NOT_FOUND, oa->field);
				}

			} else { // "salut".size()

//				cout << "obj type : " << object_type << endl;

				LSClass* object_class = (LSClass*) analyser->program->system_vars[object_type.clazz];

				Method* m = object_class->getMethod(oa->field->content, object_type, arg_types);

				if (m != nullptr) {
//					cout << "method : " << m->addr << " : " << m->type << endl;
					this_ptr = oa->object;
					std_func = m->addr;
					function->type = m->type;
				} else {
					throw SemanticException(SemanticException::Type::METHOD_NOT_FOUND, oa->field);
				}
			}
		}
	}

	VariableValue* vv = dynamic_cast<VariableValue*>(function);
	if (vv != nullptr) {
		string name = vv->name->content;
		if (name == "+" or name == "-" or name == "*" or name == "/" or name == "^" or name == "%") {
			bool isByValue = true;
			Type effectiveType;
			for (Value* arg : arguments) {
				arg->analyse(analyser, Type::NEUTRAL);
				effectiveType = arg->type;
				if (arg->type.nature != Nature::VALUE) {
					isByValue = false;
				}
			}
			if (isByValue) {
				function->type.setArgumentType(0, effectiveType);
				function->type.setArgumentType(1, effectiveType);
				function->type.setReturnType(effectiveType);
			}
			type = function->type.getReturnType();

			return;
		}
	}

	a = 0;
	for (Value* arg : arguments) {
		arg->analyse(analyser, function->type.getArgumentType(a));
		a++;
	}

	a = 0;
	for (Value* arg : arguments) {
		function->will_take(analyser, a++, arg->type);
	}

	// The function is a variable
	if (vv and vv->var->value != nullptr) {
//		cout << "function call, fun is vv: " << vv->var << endl;
		a = 0;
		for (Value* arg : arguments) {
//			cout << "arg " << a << " " << arg->type << endl;
			vv->var->will_take(analyser, a++, arg->type);
		}
		type = vv->var->value->type.getReturnType();
	} else {
		type = function->type.getReturnType();
	}

//	cout << "Function call function type : " << function->type << endl;
}

void func_print(LSValue* v) {
	cout << " p>>> ";
	v->print(cout);
	cout << endl;
}
void func_print_int(int v) {
	cout << " i>>> " << v << endl;
}
void func_print_bool(bool v) {
	cout << " b>>> " << std::boolalpha << v << endl;
}
void func_print_float(double v) {
	cout << " f>>> " << v << endl;
}

LSValue* create_float_object_3(double n) {
	return LSNumber::get(n);
}
LSValue* create_int_object_3(int n) {
	return LSNumber::get(n);
}

jit_value_t FunctionCall::compile_jit(Compiler& c, jit_function_t& F, Type req_type) const {

//	cout << "compile function call" << endl;
//	cout << type << endl;

	/*
	 * Standard library constructors
	 */
	VariableValue* vv = dynamic_cast<VariableValue*>(function);
	if (vv != nullptr) {
		if (vv->name->content == "Number") {
			jit_value_t n = jit_value_create_nint_constant(F, JIT_INTEGER, 0);
			if (req_type.nature == Nature::POINTER) {
				return VM::value_to_pointer(F, n, Type::INTEGER);
			}
			return n;
		}
		if (vv->name->content == "String") {
			if (arguments.size() > 0) {
				return arguments[0]->compile_jit(c, F, Type::POINTER);
			}
			return JIT_CREATE_CONST_POINTER(F, new LSString(""));
		}
		if (vv->name->content == "Array") {
			return JIT_CREATE_CONST_POINTER(F, new LSArray<LSValue*>());
		}
	}

	/*
	 * Compile native standard functions
	 */
	if (is_native) {

//		cout << "function call number" << endl;

		jit_value_t res = nullptr;

		if (native_func == "abs") {
			jit_value_t v = arguments[0]->compile_jit(c, F, Type::VALUE);
			res = jit_insn_abs(F, v);
		} else if (native_func == "floor") {
			jit_value_t v = arguments[0]->compile_jit(c, F, Type::VALUE);
			res = jit_insn_floor(F, v);
		} else if (native_func == "round") {
			jit_value_t v = arguments[0]->compile_jit(c, F, Type::VALUE);
			res = jit_insn_round(F, v);
		} else if (native_func == "ceil") {
			jit_value_t v = arguments[0]->compile_jit(c, F, Type::VALUE);
			res = jit_insn_ceil(F, v);
		} else if (native_func == "cos") {
			jit_value_t v = arguments[0]->compile_jit(c, F, Type::VALUE);
			res = jit_insn_cos(F, v);
		} else if (native_func == "sin") {
			jit_value_t v = arguments[0]->compile_jit(c, F, Type::VALUE);
			res = jit_insn_sin(F, v);
		} else if (native_func == "max") {
			jit_value_t v1 = arguments[0]->compile_jit(c, F, Type::VALUE);
			jit_value_t v2 = arguments[1]->compile_jit(c, F, Type::VALUE);
			res = jit_insn_max(F, v1, v2);
		} else if (native_func == "min") {
			jit_value_t v1 = arguments[0]->compile_jit(c, F, Type::VALUE);
			jit_value_t v2 = arguments[1]->compile_jit(c, F, Type::VALUE);
			res = jit_insn_min(F, v1, v2);
		} else if (native_func == "sqrt") {
			jit_value_t v1 = arguments[0]->compile_jit(c, F, Type::VALUE);
			res = jit_insn_sqrt(F, v1);
		} else if (native_func == "pow") {
			jit_value_t v1 = arguments[0]->compile_jit(c, F, Type::VALUE);
			jit_value_t v2 = arguments[1]->compile_jit(c, F, Type::VALUE);
			res = jit_insn_pow(F, v1, v2);
		}

		if (res != nullptr) {
//			cout << "function call native number" << endl;
			if (req_type.nature == Nature::POINTER && type.nature == Nature::VALUE) {
				return VM::value_to_pointer(F, res, type);
			}
			return res;
		}
	}

	/*
	 * Standard function call on object
	 */
	if (this_ptr != nullptr) {

//		cout << "compile std function " << function->type << endl;

		int arg_count = arguments.size() + 1;
		vector<jit_value_t> args = { this_ptr->compile_jit(c, F, Type::POINTER) };
		vector<jit_type_t> args_types = { JIT_POINTER };

		for (int i = 0; i < arg_count - 1; ++i) {
//			cout << "arg " << i << " : " << function->type.getArgumentType(i) << endl;
			args.push_back(arguments[i]->compile_jit(c, F, function->type.getArgumentType(i)));
			args_types.push_back(function->type.getArgumentType(i).nature != Nature::VALUE ? JIT_POINTER :
				(function->type.getArgumentType(i).raw_type == RawType::FUNCTION)	? JIT_POINTER :
				(function->type.getArgumentType(i).raw_type == RawType::FLOAT)	? JIT_FLOAT :
				(function->type.getArgumentType(i).raw_type == RawType::LONG) ? JIT_INTEGER_LONG :
				JIT_INTEGER);
		}

		jit_type_t ret_type = type.raw_type == RawType::FLOAT ? JIT_FLOAT : JIT_POINTER;

		jit_type_t sig = jit_type_create_signature(jit_abi_cdecl, ret_type, args_types.data(), arg_count, 0);

		jit_value_t res = jit_insn_call_native(F, "std_func", (void*) std_func, sig, args.data(), arg_count, JIT_CALL_NOTHROW);

		// Destroy temporary arguments
		for (int i = 0; i < arg_count - 1; ++i) {
			if (function->type.getArgumentType(i).nature == Nature::POINTER) {
				VM::delete_temporary(F, args[i + 1]);
			}
		}

		VM::delete_temporary(F, args[0]);

		if (req_type.nature == Nature::POINTER && type.nature == Nature::VALUE) {
			return VM::value_to_pointer(F, res, type);
		}
		return res;
	}

	/*
	 * Static standard function call
	 */
	if (std_func != nullptr) {

//		cout << "compile static std function" << endl;

		int arg_count = arguments.size();
		vector<jit_value_t> args;
		vector<jit_type_t> args_types;

		for (int i = 0; i < arg_count; ++i) {
//			cout << "arg " << i << " : " << function->type.getArgumentType(i) << endl;
			args.push_back(arguments[i]->compile_jit(c, F, function->type.getArgumentType(i)));
			args_types.push_back(function->type.getArgumentType(i).nature != Nature::VALUE ? JIT_POINTER :
				(function->type.getArgumentType(i).raw_type == RawType::FUNCTION) ? JIT_POINTER :
				(function->type.getArgumentType(i).raw_type == RawType::FLOAT)	? JIT_FLOAT :
				(function->type.getArgumentType(i).raw_type == RawType::LONG) ? JIT_INTEGER_LONG :
				JIT_INTEGER);
		}

		jit_type_t ret_type = type.raw_type == RawType::FLOAT ? JIT_FLOAT : JIT_POINTER;

		jit_type_t sig = jit_type_create_signature(jit_abi_cdecl, ret_type, args_types.data(), arg_count, 0);

		jit_value_t res = jit_insn_call_native(F, "std_func", (void*) std_func, sig, args.data(), arg_count, JIT_CALL_NOTHROW);

		// Destroy temporary arguments
		for (int i = 0; i < arg_count; ++i) {
			if (function->type.getArgumentType(i).nature == Nature::POINTER) {
				VM::delete_temporary(F, args[i]);
			}
		}

		if (req_type.nature == Nature::POINTER && type.nature == Nature::VALUE) {
			return VM::value_to_pointer(F, res, type);
		}
		return res;
	}


	VariableValue* f = dynamic_cast<VariableValue*>(function);

	if (f != nullptr) {
		if (f->name->content == "print") {
			jit_value_t v = arguments[0]->compile_jit(c, F, Type::NEUTRAL);
			if (arguments[0]->type == Type::INTEGER) {
				jit_type_t args[1] = {JIT_INTEGER};
				jit_type_t sig = jit_type_create_signature(jit_abi_cdecl, jit_type_void, args, 1, 0);
				jit_insn_call_native(F, "print", (void*) func_print_int, sig, &v, 1, JIT_CALL_NOTHROW);
			} else if (arguments[0]->type == Type::FLOAT) {
				jit_type_t args[1] = {JIT_FLOAT};
				jit_type_t sig = jit_type_create_signature(jit_abi_cdecl, jit_type_void, args, 1, 0);
				jit_insn_call_native(F, "print", (void*) func_print_float, sig, &v, 1, JIT_CALL_NOTHROW);
			} else if (arguments[0]->type == Type::BOOLEAN) {
				jit_type_t args[1] = {JIT_INTEGER};
				jit_type_t sig = jit_type_create_signature(jit_abi_cdecl, jit_type_void, args, 1, 0);
				jit_insn_call_native(F, "print", (void*) func_print_bool, sig, &v, 1, JIT_CALL_NOTHROW);
			} else {
				jit_type_t args[1] = {JIT_POINTER};
				jit_type_t sig = jit_type_create_signature(jit_abi_cdecl, jit_type_void, args, 1, 0);
				jit_insn_call_native(F, "print", (void*) func_print, sig, &v, 1, JIT_CALL_NOTHROW);
			}

			return JIT_CREATE_CONST_POINTER(F, LSNull::null_var);
		}
		if (function->type.getArgumentType(0).nature == Nature::VALUE and function->type.getArgumentType(1).nature == Nature::VALUE) {
			jit_value_t (*jit_func)(jit_function_t, jit_value_t, jit_value_t) = nullptr;
			if (f->name->content == "+") {
				jit_func = &jit_insn_add;
			} else if (f->name->content == "-") {
				jit_func = &jit_insn_sub;
			} else if (f->name->content == "*") {
				jit_func = &jit_insn_mul;
			} else if (f->name->content == "/") {
				jit_func = &jit_insn_div;
			} else if (f->name->content == "^") {
				jit_func = &jit_insn_pow;
			} else if (f->name->content == "%") {
				jit_func = &jit_insn_rem;
			}
			if (jit_func != nullptr) {
				jit_value_t v0 = arguments[0]->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t v1 = arguments[1]->compile_jit(c, F, Type::NEUTRAL);
				jit_value_t ret = jit_func(F,v0,v1);
				if (req_type.nature == Nature::POINTER) {
					return VM::value_to_pointer(F, ret, type);
				}
				return ret;
			}
		}
	}

	vector<jit_value_t> fun;

	if (function->type.nature == Nature::POINTER) {
		jit_value_t fun_addr = function->compile_jit(c, F, Type::NEUTRAL);
		fun.push_back(jit_insn_load_relative(F, fun_addr, 16, JIT_POINTER));
	} else {
		fun.push_back(function->compile_jit(c, F, Type::NEUTRAL));
	}

	int arg_count = arguments.size();
	vector<jit_value_t> args;
	vector<jit_type_t> args_types;

	for (int i = 0; i < arg_count; ++i) {
		args.push_back(arguments[i]->compile_jit(c, F, function->type.getArgumentType(i)));
		args_types.push_back(function->type.getArgumentType(i).nature != Nature::VALUE ? JIT_POINTER :
				(function->type.getArgumentType(i).raw_type == RawType::FUNCTION) ? JIT_POINTER :
				(function->type.getArgumentType(i).raw_type == RawType::FLOAT) ? JIT_FLOAT :
				(function->type.getArgumentType(i).raw_type == RawType::LONG) ? JIT_INTEGER_LONG :
				JIT_INTEGER);
	}

	//cout << "function call return type : " << info << endl;

	jit_type_t return_type = type.nature != Nature::VALUE ? JIT_POINTER :
			(type.raw_type == RawType::FUNCTION) ? JIT_POINTER :
			(type.raw_type == RawType::LONG) ? JIT_INTEGER_LONG :
			(type.raw_type == RawType::FLOAT) ? JIT_FLOAT :
			JIT_INTEGER;

	jit_type_t sig = jit_type_create_signature(jit_abi_cdecl, return_type, args_types.data(), arg_count, 0);

	jit_value_t ret = jit_insn_call_indirect(F, fun[0], sig, args.data(), arg_count, 0);

	//cout << "function call type " << type << endl;

	// Destroy temporary arguments
	for (int i = 0; i < arg_count; ++i) {
		if (function->type.getArgumentType(i).nature == Nature::POINTER) {
			VM::delete_temporary(F, args[i]);
		}
	}

	// Custom function call : 1 op
	VM::inc_ops(F, 1);

	if (req_type.nature == Nature::POINTER && type.nature == Nature::VALUE) {
		return VM::value_to_pointer(F, ret, type);
	}

	return ret;
}

}