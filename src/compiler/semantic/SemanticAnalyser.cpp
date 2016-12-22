#include "../../compiler/semantic/SemanticAnalyser.hpp"

#include "../../compiler/instruction/ExpressionInstruction.hpp"
#include "../../vm/Program.hpp"
#include "../../vm/Context.hpp"
#include "../../vm/standard/ValueSTD.hpp"
#include "../../vm/standard/NullSTD.hpp"
#include "../../vm/standard/NumberSTD.hpp"
#include "../../vm/standard/BooleanSTD.hpp"
#include "../../vm/standard/StringSTD.hpp"
#include "../../vm/standard/ArraySTD.hpp"
#include "../../vm/standard/MapSTD.hpp"
#include "../../vm/standard/SetSTD.hpp"
#include "../../vm/standard/ObjectSTD.hpp"
#include "../../vm/standard/SystemSTD.hpp"
#include "../../vm/standard/FunctionSTD.hpp"
#include "../../vm/standard/ClassSTD.hpp"
#include "../../vm/standard/IntervalSTD.hpp"
#include "../../vm/standard/JsonSTD.hpp"
#include "SemanticError.hpp"
#include "../instruction/VariableDeclaration.hpp"
#include "../../vm/value/LSNumber.hpp"
#include "../../vm/value/LSArray.hpp"
#include "../../vm/value/LSNull.hpp"

#include <functional>

using namespace std;

namespace ls {

SemanticAnalyser::SemanticAnalyser() {
	program = nullptr;
	in_program = false;
//	loops.push(0);
//	variables.push_back(vector<map<std::string, SemanticVar*>> {});
//	functions_stack.push(nullptr); // The first function is the main function of the program
//	parameters.push_back(map<std::string, SemanticVar*> {});
}

SemanticAnalyser::~SemanticAnalyser() {}

void SemanticVar::will_take(SemanticAnalyser* analyser, const std::vector<Type>& arg_types) {
	if (value != nullptr) {
		value->will_take(analyser, arg_types);
		this->type.will_take(arg_types);
	}
}

void SemanticVar::will_take_element(SemanticAnalyser* analyser, const Type& type) {
	if (value != nullptr) {
		value->will_take_element(analyser, type);
		this->type.will_take_element(type);
	}
}

void SemanticVar::must_be_pointer(SemanticAnalyser* analyser) {
	if (value != nullptr) {
		value->must_be_pointer(analyser);
		this->type.nature = Nature::POINTER;
	}
}

LSValue* op_add(void*, LSValue* x, LSValue* y) {
	return x->ls_add(y);
}
LSValue* op_sub(void*, LSValue* x, LSValue* y) {
	return x->ls_sub(y);
}
LSValue* op_mul(void*, LSValue* x, LSValue* y) {
	return x->ls_mul(y);
}
LSValue* op_div(void*, LSValue* x, LSValue* y) {
	return x->ls_div(y);
}
int op_int_div(void*, LSValue* x, LSValue* y) {
	LSValue* res = x->ls_int_div(y);
	int v = ((LSNumber*) res)->value;
	LSValue::delete_temporary(res);
	return v;
}
LSValue* op_pow(void*, LSValue* x, LSValue* y) {
	return x->ls_pow(y);
}
LSValue* op_mod(void*, LSValue* x, LSValue* y) {
	return x->ls_mod(y);
}

void SemanticAnalyser::analyse(Program* program, Context* context, std::vector<Module*>& modules, bool v1_mode) {

	this->program = program;

	enter_function(program->main);

	// Add context variables
	/*
	for (auto var : context->vars) {
		add_var(new Token(var.first), Type(var.second->getRawType(), Nature::POINTER), nullptr, nullptr);
	}
	*/

	// Include STD modules
	ValueSTD().include(this, program);
	NullSTD().include(this, program);
	BooleanSTD().include(this, program);
	NumberSTD().include(this, program);
	StringSTD().include(this, program);
	ArraySTD().include(this, program);
	MapSTD().include(this, program);
	SetSTD().include(this, program);
	ObjectSTD().include(this, program);
	FunctionSTD().include(this, program);
	ClassSTD().include(this, program);
	SystemSTD().include(this, program);
	IntervalSTD().include(this, program);
	JsonSTD().include(this, program);

	// Include custom modules
	for (Module* module : modules) {
		module->include(this, program);
	}

	// Add function operators
	std::vector<std::string> ops = {"+", "-", "*", "×", "/", "÷", "**", "%"};
	std::vector<void*> ops_funs = {(void*) &op_add, (void*) &op_sub, (void*) &op_mul, (void*) &op_mul, (void*) &op_div, (void*) &op_div, (void*) &op_pow, (void*) &op_mod};

	Type op_type = Type(RawType::FUNCTION, Nature::POINTER);
	op_type.setArgumentType(0, Type::POINTER);
	op_type.setArgumentType(1, Type::POINTER);
	op_type.setReturnType(Type::POINTER);
	auto value_class = program->system_vars["Value"];

	for (unsigned o = 0; o < ops.size(); ++o) {
		auto fun = new LSFunction<LSValue*>(ops_funs[o]);
		fun->args = {value_class, value_class};
		fun->return_type = value_class;
		program->system_vars.insert({ops[o], fun});
		add_var(new Token(ops[o]), op_type, nullptr, nullptr);
	}

	if (v1_mode) {
		auto debug = new LSFunction<LSValue*>((void*) +[](LSFunction<LSValue*>*, LSValue* v) {
			v->print(*VM::output);
			LSValue::delete_temporary(v);
			*VM::output << std::endl;
		});
		auto debug_type = Type::FUNCTION_P;
		debug_type.setArgumentType(0, Type::POINTER);
		debug_type.setReturnType(Type::VOID);
		program->system_vars.insert({"debug", debug});
		add_var(new Token("debug"), debug_type, nullptr, nullptr);

		auto charAt = new LSFunction<LSValue*>((void*) +[](LSFunction<LSValue*>*, LSString* v, int p) {
			auto s = v->charAt(p);
			LSValue::delete_temporary(v);
			return s;
		});
		auto charAt_type = Type::FUNCTION_P;
		charAt_type.setArgumentType(0, Type::STRING);
		charAt_type.setArgumentType(1, Type::INTEGER);
		charAt_type.setReturnType(Type::STRING);
		program->system_vars.insert({"charAt", charAt});
		add_var(new Token("charAt"), charAt_type, nullptr, nullptr);

		auto replace = new LSFunction<LSValue*>((void*) +[](LSFunction<LSValue*>*, LSString* string, LSString* from, LSString* to) {
			std::string str(*string);
			size_t start_pos = 0;

			// Replace \\ by \ (like Java does)
			std::string f = *from;
			while((start_pos = f.find("\\\\", start_pos)) != std::string::npos) {
				f.replace(start_pos, 2, "\\");
				start_pos += 1;
			}
			start_pos = 0;
			std::string t = *to;
			while((start_pos = t.find("\\\\", start_pos)) != std::string::npos) {
				t.replace(start_pos, 2, "\\");
				start_pos += 1;
			}

			start_pos = 0;
			while((start_pos = str.find(f, start_pos)) != std::string::npos) {
				str.replace(start_pos, from->length(), t);
				start_pos += t.size();
			}
			if (string->refs == 0) { delete string; }
			if (from->refs == 0) { delete from; }
			if (to->refs == 0) { delete to; }
			return new LSString(str);
		});
		auto replace_type = Type::FUNCTION_P;
		replace_type.setArgumentType(0, Type::STRING);
		replace_type.setArgumentType(1, Type::STRING);
		replace_type.setArgumentType(2, Type::STRING);
		replace_type.setReturnType(Type::STRING);
		program->system_vars.insert({"replace", replace});
		add_var(new Token("replace"), replace_type, nullptr, nullptr);

		auto count = new LSFunction<LSValue*>((void*) +[](LSFunction<LSValue*>*, LSArray<LSValue*>* a) {
			int s = a->size();
			LSValue::delete_temporary(a);
			return s;
		});
		auto count_type = Type::FUNCTION_P;
		count_type.setArgumentType(0, Type::ARRAY);
		count_type.setReturnType(Type::INTEGER);
		program->system_vars.insert({"count", count});
		add_var(new Token("count"), count_type, nullptr, nullptr);

		auto pushAll = new LSFunction<LSValue*>((void*) +[](LSFunction<LSValue*>*, LSArray<LSValue*>* a, LSArray<LSValue*>* b) {
			return a->ls_push_all_ptr(b);
		});
		auto pushAll_type = Type::FUNCTION_P;
		pushAll_type.setArgumentType(0, Type::PTR_ARRAY);
		pushAll_type.setArgumentType(1, Type::PTR_ARRAY);
		pushAll_type.setReturnType(Type::VOID);
		program->system_vars.insert({"pushAll", pushAll});
		add_var(new Token("pushAll"), pushAll_type, nullptr, nullptr);
	}

	in_program = true;

	program->main->type.setReturnType(Type::UNKNOWN);
	program->main->body->analyse(this, Type::UNKNOWN);
	if (program->main->type.return_types.size() > 1) { // the body contains return instruction
		Type return_type = program->main->body->type;
		for (size_t i = 1; i < program->main->type.return_types.size(); ++i) {
			return_type = Type::get_compatible_type(return_type, program->main->type.return_types[i]);
		}
		program->main->type.return_types.clear();
		program->main->type.setReturnType(return_type);
		program->main->body->analyse(this, return_type); // second pass
	} else {
		program->main->type.setReturnType(program->main->body->type);
	}

	program->functions = functions;
}

void SemanticAnalyser::enter_function(Function* f) {

	// Create function scope
	variables.push_back(vector<map<std::string, SemanticVar*>> {});
	// First function block
	variables.back().push_back(map<std::string, SemanticVar*> {});
	// Parameters
	parameters.push_back(map<std::string, SemanticVar*> {});

	loops.push(0);
	functions_stack.push(f);
}

void SemanticAnalyser::leave_function() {

	variables.pop_back();
	parameters.pop_back();
	functions_stack.pop();
	loops.pop();
}

void SemanticAnalyser::enter_block() {
	variables.back().push_back(map<std::string, SemanticVar*> {});
}

void SemanticAnalyser::leave_block() {
	variables.back().pop_back();
}

Function* SemanticAnalyser::current_function() const {
	if (functions_stack.empty()) {
		return nullptr;
	}
	return functions_stack.top();
}

void SemanticAnalyser::set_potential_return_type(Type type) {
	current_function()->type.setReturnType(type);
}

void SemanticAnalyser::enter_loop() {
	loops.top()++;
}

void SemanticAnalyser::leave_loop() {
	loops.top()--;
}

bool SemanticAnalyser::in_loop(int deepness) const {
	return loops.top() >= deepness;
}

SemanticVar* SemanticAnalyser::add_parameter(Token* v, Type type) {

	SemanticVar* arg = new SemanticVar(v->content, VarScope::PARAMETER, type, parameters.back().size(), nullptr, nullptr, current_function());
	parameters.back().insert(pair<string, SemanticVar*>(v->content, arg));
	return arg;
}

SemanticVar* SemanticAnalyser::get_var(Token* v) {

	// Search in interval variables : global for the program
	try {
		return internal_vars.at(v->content);
	} catch (exception& e) {}

	// Search recursively in the functions
	int f = functions_stack.size() - 1;
	while (f >= 0) {
		// Search in the function parameters
		try {
			return parameters.at(f).at(v->content);
		} catch (exception& e) {}

		// Search in the local variables of the function
		int b = variables.at(f).size() - 1;
		while (b >= 0) {
			try {
				return variables.at(f).at(b).at(v->content);
			} catch (exception& e) {}
			b--;
		}
		f--;
	}
	add_error({SemanticError::Type::UNDEFINED_VARIABLE, v->line, {v->content}});
	return nullptr;
}

SemanticVar* SemanticAnalyser::add_var(Token* v, Type type, Value* value, VariableDeclaration* vd) {

	// Internal variable, before execution
	if (!in_program) {
		internal_vars.insert(pair<string, SemanticVar*>(
			v->content,
			new SemanticVar(v->content, VarScope::INTERNAL, type, 0, value, vd, current_function())
		));
		return internal_vars.at(v->content);
	}

	if (variables.back().back().find(v->content) != variables.back().back().end()) {
		add_error({SemanticError::Type::VARIABLE_ALREADY_DEFINED, v->line, {v->content}});
	}
	variables.back().back().insert(pair<string, SemanticVar*>(
		v->content,
		new SemanticVar(v->content, VarScope::LOCAL, type, 0, value, vd, current_function())
	));
	return variables.back().back().at(v->content);
}

void SemanticAnalyser::add_function(Function* l) {
	functions.push_back(l);
}

map<string, SemanticVar*>& SemanticAnalyser::get_local_vars() {
	return variables.back().back();
}

void SemanticAnalyser::add_error(SemanticError ex) {
	errors.push_back(ex);
}

} // end of namespace ls
