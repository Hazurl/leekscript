#ifndef FUNCTION_HPP
#define FUNCTION_HPP

#include <vector>

#include "../lexical/Ident.hpp"
#include "../semantic/SemanticAnalyser.hpp"
#include "Value.hpp"
#include "Block.hpp"

namespace ls {

class SemanticVar;

class Function : public Value {
public:

	bool lambda = false;
	std::vector<std::unique_ptr<Token>> arguments;
	std::vector<bool> references;
	std::vector<Value*> defaultValues;
	std::vector<std::shared_ptr<SemanticVar>> captures;
	Block* body;
	int pos;
	std::map<std::string, std::shared_ptr<SemanticVar>> vars;
	bool function_added;
	Function* parent;
	LSFunction<LSValue*>* ls_fun = nullptr;
	jit_function_t jit_function;
	jit_context_t context = nullptr;
	std::string name;
	Type placeholder_type;
	bool is_main_function = false;
	std::string file;

	Function();
	virtual ~Function();

	void addArgument(Token* token, bool reference, Value* defaultValue);
	int capture(std::shared_ptr<SemanticVar> var);
	Type getReturnType();

	virtual void print(std::ostream&, int indent, bool debug) const override;
	virtual Location location() const override;

	virtual void analyse(SemanticAnalyser*, const Type&) override;

	bool will_take(SemanticAnalyser*, const std::vector<Type>&, int level) override;
	void must_return(SemanticAnalyser*, const Type&) override;
	void analyse_body(SemanticAnalyser*, const Type& req_type);
	void update_function_args(SemanticAnalyser*);

	virtual Compiler::value compile(Compiler&) const override;

	virtual Value* clone() const override;
};

}

#endif
