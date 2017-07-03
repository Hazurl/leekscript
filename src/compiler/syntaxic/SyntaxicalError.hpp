#ifndef SYNTAXICALERROR_HPP
#define SYNTAXICALERROR_HPP

#include <string>
#include "../lexical/Token.hpp"
#include "../../../lib/json.hpp"

namespace ls {

class SyntaxicalError {
public:

	enum Type {
		BLOCK_NOT_CLOSED,
		BREAK_LEVEL_ZERO,
		CONTINUE_LEVEL_ZERO,
		EXPECTED_VALUE,
		UNEXPECTED_TOKEN,
		PATTERN_NOT_MATCHING
	};

	Token* token;
	Type type;
	std::vector<std::string> parameters;

	static std::string build_message(Type, std::vector<std::string> parameters);

	SyntaxicalError(const SyntaxicalError& error);
	SyntaxicalError(Token* token, Type type, std::vector<std::string> parameters);
	virtual ~SyntaxicalError();

	std::string message() const;
	Json json() const;
};

}

#endif
