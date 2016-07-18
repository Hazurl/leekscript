#include "Test.hpp"

void Test::test_numbers() {

	header("Numbers");

	// basic numbers
	success("0", "0");
	success("-1", "-1");
	success("-(-1)", "1");
	success("π", "3.141592653589793116");

	// basic operations
	success("0 + 5", "5");
	success("5 + 5", "10");
	success("10 - 3", "7");
	success("5 * 5", "25");
	success("15 / 3", "5");
	success("15 / 2", "7.5");
	success("12 ** 2", "144");
	success("2 ** 5", "32");
	success("2 < 5", "true");
	success("12 < 5", "false");
	success("5 == 12", "false");
	success("12 == 12", "true");
	success("0.2 + 0.1", "0.300000000000000044");

	// with variables
	success("let a = 2 a++", "2");
	success("let a = 2; ++a", "3");
	success("let a = 2 a--", "2");
	success("let a = 2; --a", "1");
	success("let a = 2 a += 5", "7");
	success("let a = 2 a -= 5", "-3");
	success("let a = 2 a *= 5", "10");
	success("let a = 100 a /= 5", "20");
	success("let a = 56 a %= 17", "5");
	success("let a = 15 a **= 2", "225");
	success("let a = 1.5 a * 0.5", "0.75");

	// multiple operations
	success("(33 - 2) / 2", "15.5");
	success("12 < (45 / 4)", "false");
	success("12 == (24 / 2)", "true");
	success("2.5 + 4.7", "7.200000000000000178");
	success("2.5 × 4.7", "11.75");
	success("5 * 2 + 3 * 4", "22");

	// binary
	success("0 & 0", "0");
	success("1 & 0", "0");
	success("1 & 1", "1");
	success("5 & 12", "4");
	success("87619 & 18431", "17987");
	success("let a = 87619 a &= 18431", "17987");
	success("let a = 87619 a &= 18431 a", "17987");
	success("0 | 0", "0");
	success("1 | 0", "1");
	success("1 | 1", "1");
	success("5 | 12", "13");
	success("87619 | 18431", "88063");
	success("let a = 87619 a |= 18431", "88063");
	success("let a = 87619 a |= 18431 a", "88063");
	success("0 ^ 0", "0");
	success("1 ^ 0", "1");
	success("1 ^ 1", "0");
	success("5 ^ 12", "9");
	success("87619 ^ 18431", "70076");
	success("let a = 87619 a ^= 18431", "70076");
	success("let a = 87619 a ^= 18431 a", "70076");
	success("0 << 0", "0");
	success("1 << 0", "1");
	success("123456 << 0", "123456");
	success("0 << 1", "0");
	success("0 << 12", "0");
	success("1 << 8", "256");
	success("123 << 12", "503808");
	success("let a = 123 a <<= 11", "251904");
	success("let a = 123 a <<= 13 a", "1007616");
	success("0 >> 0", "0");
	success("1 >> 0", "1");
	success("123456 >> 0", "123456");
	success("0 >> 1", "0");
	success("0 >> 12", "0");
	success("155 >> 3", "19");
	success("-155 >> 3", "-20");
	success("12345 >> 8", "48");
	success("123123123 >> 5", "3847597");
	success("let a = 123123123 a >>= 6", "1923798");
	success("let a = 123123123 a >>= 7 a", "961899");
	success("155 >>> 3", "19");
	success("-155 >>> 3", "536870892");
	success("let a = -155 a >>>= 4", "268435446");
	success("let a = -155 a >>>= 5 a", "134217723");

	// string conversions
	success("65.char()", "'A'");
	success("Number.char(65)", "'A'");
	success("126.char()", "'~'");
	success("Number.char(128040)", "'🐨'");

	// large numbers
	// success("12344532132423", "12344532132423");

	/*
	 * Number standard library
	 */
	header("Number standard library");
	success("Number", "<class Number>");
	success("Number()", "0");
	success("new Number", "0");
	success("new Number()", "0");
	success("new Number(12)", "12");

	// constants
	success("Number.pi", "3.141592653589793116");
	success("2 × Number.pi", "6.283185307179586232");
	success("Number.e", "2.718281828459045091");
	success("Number.phi", "1.618033988749894903");
	success("Number.epsilon", "0.000000000000000222");

//	success("Number.abs", "<function>");
	success("Number.abs(-12)", "12");
	success("Number.floor(5.9)", "5");
	success("var a = 5 Number.floor(a)", "5");
	success("var a = 5.4 Number.floor(a)", "5");
	success("Number.round(5.7)", "6");
	success("Number.round(5.4)", "5");
	success("Number.ceil(5.1)", "6");
	success("Number.max(5, 12)", "12");

	success("Number.cos(0)", "1");
	success("Number.cos(π)", "-1");
	success_almost("Number.cos(π / 2)", 0.0, std::numeric_limits<double>::epsilon());

	success("Number.sin(0)", "0");
	success_almost("Number.sin(π)", 0.0, std::numeric_limits<double>::epsilon());
	success("Number.sin(π / 2)", "1");
	success("Number.sin(- π / 2)", "-1");
//	success("12.sqrt()", "");

	success("Number.isInteger(12)", "true");
	success("Number.isInteger(0)", "true");
	success("Number.isInteger(-5)", "true");
	success("Number.isInteger(12.9)", "false");
	success("Number.isInteger(-5.2)", "false");
	success("Number.isInteger(π)", "false");
}
