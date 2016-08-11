#include "../../compiler/value/If.hpp"

#include "../../compiler/value/Number.hpp"
#include "../../vm/LSValue.hpp"
#include "../../vm/value/LSNull.hpp"

using namespace std;

namespace ls {

If::If() {
	elze = nullptr;
	condition = nullptr;
	then = nullptr;
	inversed = false;
	type = Type::UNKNOWN;
}

If::~If() {
	delete condition;
	delete then;
	if (elze != nullptr) {
		delete elze;
	}
}

void If::print(ostream& os, int indent, bool debug) const {
	os << "if ";
	condition->print(os, indent, debug);
	os << " ";
	then->print(os, indent, debug);
	if (elze != nullptr) {
		os << " else ";
		elze->print(os, indent, debug);
	}
	if (debug) {
		os << " " << type;
	}
}

unsigned If::line() const {
	return 0;
}

void If::analyse(SemanticAnalyser* analyser, const Type& req_type) {

	condition->analyse(analyser, Type::BOOLEAN);
	then->analyse(analyser, req_type);

	if (elze != nullptr) {

		elze->analyse(analyser, req_type);

		type = Type::get_compatible_type(then->type, elze->type);
		if (then->type != type) {
			then->analyse(analyser, type);
		}
		if (elze->type != type) {
			elze->analyse(analyser, type);
		}
	} else {
		type = Type::POINTER; // Pointer because the else will give null

		then->analyse(analyser, Type::POINTER);
	}

//	if (Expression* cond_ex = dynamic_cast<Expression*>(condition)) {
//		if (cond_ex->op->type == TokenType::DOUBLE_EQUAL) {
//			if (Number* v2n = dynamic_cast<Number*>(cond_ex->v2)) {
//				if (v2n->value == 0) {
//					inversed = true;
//					condition = cond_ex->v1;
//				}
//			}
//		}
//	}

	if (req_type.nature == Nature::POINTER) {
		type.nature = req_type.nature;
	}
}

int is_true(LSValue* v) {
	return v->isTrue();
}

jit_value_t If::compile(Compiler& c) const {

	jit_value_t res = jit_value_create(c.F, JIT_POINTER);
	jit_label_t label_else = jit_label_undefined;
	jit_label_t label_end = jit_label_undefined;

	jit_value_t cond = condition->compile(c);

	if (condition->type.nature == Nature::POINTER) {

		jit_value_t const_true = jit_value_create_nint_constant(c.F, jit_type_int, 1);

		jit_type_t args_types[1] = {JIT_POINTER};
		jit_type_t sig = jit_type_create_signature(jit_abi_cdecl, JIT_INTEGER, args_types, 1, 0);
		jit_value_t cond_bool = jit_insn_call_native(c.F, "is_true", (void*) is_true, sig, &cond, 1, JIT_CALL_NOTHROW);
		if (condition->type.must_manage_memory()) {
			VM::delete_obj(c.F, cond);
		}

		jit_value_t cmp = inversed ? jit_insn_eq(c.F, cond_bool, const_true) : jit_insn_ne(c.F, cond_bool, const_true);
		jit_insn_branch_if(c.F, cmp, &label_else);

	} else {

		inversed ? jit_insn_branch_if(c.F, cond, &label_else) : jit_insn_branch_if_not(c.F, cond, &label_else);
	}

	jit_value_t then_v = then->compile(c);
	jit_insn_store(c.F, res, then_v);
	jit_insn_branch(c.F, &label_end);

	jit_insn_label(c.F, &label_else);

//	if (then->type.must_manage_memory()) {
//		VM::delete_temporary(c.F, then_v);
//	}

	if (elze != nullptr) {
		jit_value_t else_v = elze->compile(c);
		if (type != Type::VOID) {
			jit_insn_store(c.F, res, else_v);
		}
	} else {
		if (type != Type::VOID) {
			jit_insn_store(c.F, res, VM::create_null(c.F));
		}
	}

	jit_insn_label(c.F, &label_end);

	return res;
}

}
