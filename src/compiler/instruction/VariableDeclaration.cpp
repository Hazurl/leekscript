#include "VariableDeclaration.hpp"
#include "../../vm/LSValue.hpp"
#include "../../vm/value/LSNull.hpp"
#include "../semantic/SemanticAnalyser.hpp"
#include "../semantic/SemanticError.hpp"
#include "../value/Reference.hpp"
#include "../value/Function.hpp"

using namespace std;

namespace ls {

VariableDeclaration::VariableDeclaration() {
	global = false;
	constant = false;
}

VariableDeclaration::~VariableDeclaration() {
	for (const auto& ex : expressions) {
		delete ex;
	}
}

void VariableDeclaration::print(ostream& os, int indent, bool debug) const {

	os << (global ? "global " : (constant ? "let " : "var "));

	for (unsigned i = 0; i < variables.size(); ++i) {
		auto name = variables.at(i)->content;
		os << name;
		if (vars.find(name) != vars.end()) {
			os << " " << vars.at(name)->type;
		}
		if (expressions[i] != nullptr) {
			os << " = ";
			expressions.at(i)->print(os, indent, debug);
		}
		if (i < variables.size() - 1) {
			os << ", ";
		}
	}
}

Location VariableDeclaration::location() const {
	auto end = variables.size() > expressions.size() ? variables.back()->location.end : expressions.back()->location().end;
	return {keyword->location.start, end};
}

void VariableDeclaration::analyse_global_functions(SemanticAnalyser* analyser) {
	if (global && function) {
		auto var = variables.at(0);
		auto expr = expressions.at(0);
		auto v = analyser->add_var(var.get(), Type::FUNCTION_P, expr, this);
		vars.insert({var->content, v});
	}
}

void VariableDeclaration::analyse(SemanticAnalyser* analyser, const Type&) {

	type = Type::VOID;

	vars.clear();
	for (unsigned i = 0; i < variables.size(); ++i) {

		auto& var = variables.at(i);
		auto v = analyser->add_var(var.get(), Type::UNKNOWN, expressions.at(i), this);
		if (v == nullptr) {
			continue;
		}

		if (expressions[i] != nullptr) {
			if (Function* f = dynamic_cast<Function*>(expressions[i])) {
				f->name = var->content;
				f->file = VM::current()->file_name;
			}
			expressions[i]->analyse(analyser, Type::UNKNOWN);
			v->type = expressions[i]->type;
			v->type.temporary = false;
			v->type.constant = constant;
			v->value = expressions[i];
		} else {
			v->type = Type::NULLL;
			v->type.constant = constant;
		}
		if (v->type == Type::VOID) {
			analyser->add_error({SemanticError::Type::CANT_ASSIGN_VOID, location(), var->location, {var->content}});
		}
		vars.insert({var->content, v});
	}
}

Compiler::value VariableDeclaration::compile(Compiler& c) const {

	for (unsigned i = 0; i < variables.size(); ++i) {

		std::string name = variables[i]->content;
		auto v = vars.at(name);

		if (expressions[i] != nullptr) {

			Value* ex = expressions[i];

			jit_value_t var = jit_value_create(c.F, VM::get_jit_type(v->type));
			c.add_var(name, var, Type::POINTER, false);

			if (Function* f = dynamic_cast<Function*>(ex)) {
				if (v->has_version && f->versions.find(v->version) != f->versions.end()) {
					jit_insn_store(c.F, var, c.new_pointer((void*) f->versions.at(v->version)->function).v);
				} else {
					jit_insn_store(c.F, var, c.new_pointer((void*) f->default_version->function).v);
				}
			}

			auto val = ex->compile(c);
			ex->compile_end(c);

			val = c.insn_move_inc(val);

			c.set_var_type(name, ex->type);
			c.add_function_var(var, v->type);

			jit_insn_store(c.F, var, val.v);

		} else {

			jit_value_t var = jit_value_create(c.F, LS_POINTER);
			c.add_var(name, var, Type::NULLL, false);

			auto val = c.new_null();
			jit_insn_store(c.F, var, val.v);
		}
	}
	return {nullptr, Type::UNKNOWN};
}

Instruction* VariableDeclaration::clone() const {
	auto vd = new VariableDeclaration();
	vd->keyword = keyword;
	vd->global = global;
	vd->constant = constant;
	vd->function = function;
	for (const auto& v : variables) {
		vd->variables.push_back(v);
	}
	for (const auto& v : expressions) {
		vd->expressions.push_back(v ? v->clone() : nullptr);
	}
	return vd;
}

}
