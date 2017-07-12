#ifndef TEST_HPP_
#define TEST_HPP_

#include <iostream>
#include <string>
#include <set>
#include "../src/vm/VM.hpp"
#include "../src/vm/value/LSNumber.hpp"

#define LIST_TEST(X) \
	X(test_general, "gen") \
	X(test_types, "type") \
	X(test_booleans, "bool") \
	X(test_numbers, "num") \
	X(test_strings, "str") \
	X(test_arrays, "arr") \
	X(test_intervals, "int") \
	X(test_map, "map") \
	X(test_set, "set") \
	X(test_objects, "obj") \
	X(test_functions, "fun") \
	X(test_classes, "cla") \
	X(test_loops, "loop") \
	X(test_operators, "optr") \
	X(test_references, "ref") \
	X(test_exceptions, "ex") \
	X(test_operations, "opti") \
	X(test_system, "sys") \
	X(test_json, "json") \
	X(test_files, "file") \
	X(test_doc, "doc") \
	X(test_utils, "util")
	

class Test {
private:
	ls::VM vm;
	ls::VM vmv1;
	int total;
	int success_count;
	double exeTime;
	int obj_created;
	int obj_deleted;
	int mpz_obj_created;
	int mpz_obj_deleted;
	static std::vector<std::string> failed_tests;

public:

	class Input;

	Test();
	virtual ~Test();

	int all();
	int selected(std::set<std::string> tests_selected);
	void header(std::string);
	void section(std::string);

	Input code(const std::string& _code);
	Input code_v1(const std::string& _code);
	Input file(const std::string& file_name);
	Input file_v1(const std::string& file_name);

#define F(func, str) void func();
	LIST_TEST(F)
#undef F

	class Input {
	public:
		Test* test;
		std::string name;
		std::string code;
		bool file;
		bool v1;
		float compilation_time = 0;
		float execution_time = 0;
		long int operation_limit = ls::VM::DEFAULT_OPERATION_LIMIT;
		ls::VM::Result result;

		Input(Test* test, const std::string& name, const std::string& code,
			bool file = false, bool v1 = false) : test(test), name(name), code(code), file(file), v1(v1) {};
		void works();
		void equals(std::string expected);
		template <typename T>
		void almost(T expected, T delta = 1e-10);
		template <typename T>
		void between(T a, T b);
		void semantic_error(ls::SemanticError::Type error, std::vector<std::string> params);
		void syntaxic_error(ls::SyntaxicalError::Type error, std::vector<std::string> params);
		void lexical_error(ls::LexicalError::Type error);
		void operations(int ops);
		void exception(ls::vm::Exception, std::vector<ls::vm::exception_frame> frames = {{"main", 1}});
		void output(std::string expected);
		void quine();
		Input& timeout(int ms);
		Input& ops_limit(long int ops);

		ls::VM::Result run(bool display_errors = true);
		void pass(std::string expected);
		void fail(std::string expected, std::string actuel);
	};

	
};




#endif
