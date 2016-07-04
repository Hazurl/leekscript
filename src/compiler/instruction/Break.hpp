#ifndef BREAK_HPP
#define BREAK_HPP

#include "../../compiler/instruction/Instruction.hpp"
#include "../../compiler/value/Expression.hpp"

namespace ls {

class Break : public Instruction {
public:

	char value;

	Break();
	virtual ~Break();

	virtual void print(std::ostream&) const override;

	virtual void analyse(SemanticAnalyser*, const Type& req_type) override;

	virtual jit_value_t compile_jit(Compiler&, jit_function_t&, Type) const override;
};

}

#endif