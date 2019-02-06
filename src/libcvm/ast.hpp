/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman (spirit 3 , calc9 example)
    			  2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_X3_CALC9_AST_HPP)
#define BOOST_SPIRIT_X3_CALC9_AST_HPP

#include <boost/spirit/home/x3/support/ast/variant.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/optional.hpp>
#include <list>
#include "types.h"

namespace client { namespace ast
{
    ///////////////////////////////////////////////////////////////////////////
    //  The AST
    ///////////////////////////////////////////////////////////////////////////
    namespace x3 = boost::spirit::x3;

    struct nil {};
    struct unary;
    struct expression;
    struct expression_list;
    struct cast_expression;
    struct function_call;
    struct function_call2;
    struct subscript;

    struct variable : x3::position_tagged
    {
        variable(std::string const& name = "") : name(name) {}
        std::string name;
    };

    struct quote : x3::position_tagged
    {
           // quote(std::string const& name = "") : str(name) {}
            std::string str;
    };

    struct str8:x3::position_tagged
    {
    	str8(char val0[8]):val(*(uint64_t*)val0){};
    	str8(int64_t val0):val(val0){}
    	const char* toCharPtr() const {
    		return (char*)&val;
    	}

    	std::string toString() const {
    		std::string out;
    		out.resize(9);
    		*(int64_t*)&out[0]=val;
    		return out;
    	 }

    	int64_t val;
    };

    struct operand :
        x3::variant<
            nil
		  , bool //bool added to remove ambiguity to set bool between int and double
		  , int32_t
		  , uint32_t
          , int64_t
		  , uint64_t
		  , double
		  , str8
          , variable
		  , quote
          , x3::forward_ast<unary>
          , x3::forward_ast<expression>
          , x3::forward_ast<expression_list>
          , x3::forward_ast<cast_expression>
          , x3::forward_ast<function_call>
          , x3::forward_ast<subscript>
        >
    {
        using base_type::base_type;
        using base_type::operator=;
    };

    enum optoken
    {
        op_plus,
        op_minus,
        op_times,
        op_divide,
        op_positive,
        op_negative,
		op_inc,
		op_dec,
        op_not,
        op_equal,
        op_not_equal,
        op_less,
        op_less_equal,
        op_greater,
        op_greater_equal,
        op_and,
        op_or,
		op_bit_and,
		op_bit_or,
		op_bit_not,
		op_bit_xor,
		op_bit_lshift,
		op_bit_rshift
    };

    struct unary:x3::position_tagged
    {
        optoken operator_;
        operand operand_;
    };

    struct operation : x3::position_tagged
    {
        optoken operator_;
        operand operand_;
    };


    struct expression : x3::position_tagged
    {
        operand first;
        std::list<operation> rest;
    };

    struct expression_list:x3::position_tagged,
	std::list<operand> {};

    struct cast_expression:x3::position_tagged
	{
    	std::string type;
    	expression expr;
	};


    struct variable_binary_operation: x3::position_tagged  //not strictly necessary, just optimization
    {
    	variable lhs;
    	optoken operator_;
    	variable rhs;
    };

    struct assignment : x3::position_tagged
    {
        variable lhs;
        expression rhs;
   //     operand rhs;
    };

    struct array_element_assignment : x3::position_tagged
    {
         variable lhs;
         operand lhs_index;
         operand rhs;
    };

    struct assignment_list : x3::position_tagged
    {
            variable lhs;
            expression_list rhs;
    };

    struct variable_declaration:x3::position_tagged
    {
        assignment assign;
    };

    struct inc:x3::position_tagged
    {
    	variable var;
    };

    struct dec:x3::position_tagged
    {
        variable var;
    };

    struct if_statement;
    struct while_statement;
    struct statement_list;
    struct function_def_statement;

    struct statement :
        x3::variant<
            variable_declaration
          , assignment
		  , assignment_list
		  , array_element_assignment
		  //, expression
		  , inc
		  , dec
          , boost::recursive_wrapper<if_statement>
          , boost::recursive_wrapper<while_statement>
          , boost::recursive_wrapper<statement_list>
          , boost::recursive_wrapper<function_def_statement>

         , boost::recursive_wrapper<function_call>
        , boost::recursive_wrapper<function_call2>//not used yet, meant to be rhs call (as oposed to lhs)
        >,x3::position_tagged
    {
        using base_type::base_type;
        using base_type::operator=;
    };

    struct statement_list :x3::position_tagged,
    		std::list<statement> {};

    struct if_statement:x3::position_tagged
    {
        expression condition;
        statement then;
        boost::optional<statement> else_;
    };

    struct while_statement:x3::position_tagged
    {
        expression condition;
        statement body;
    };

    struct argument_decl:x3::position_tagged
    {
    	std::string type1;
    	std::string name;
    };

    struct func_name : x3::position_tagged
    {
           //variable(std::string const& name = "") : name(name) {}
        std::string name;
    };

    struct function_def_statement : x3::position_tagged
    {
    	std::string name;
    	std::list<argument_decl> arguments;
    	std::string return_type;

    	statement_list body;
    };

    typedef operand argument;
    struct function_call : x3::position_tagged
	{
    	/*
		bool operator == (const function_call& other) const
		{
			return name==other.name && arguments == other.arguments;
		}*/

	   std::string name;
	   std::list< argument > arguments;
	};

    struct subscript : x3::position_tagged
    {
    	   std::string name;
    	   //variable name;
    	   operand index;
    };

    struct function_call2 : x3::position_tagged
	{

    	function_call2(){}
       //function_call2(std::string&& nm):name(nm){}
       function_call2(const std::string& nm):name(nm){}
	   std::string name;
	   std::list< argument_decl > arguments;
	};

    // print functions for debugging
    inline std::ostream& operator<<(std::ostream& out, nil)
    {
        out << "nil";
        return out;
    }

    inline std::ostream& operator<<(std::ostream& out, variable const& var)
    {
        out << var.name; return out;
    }
}}
/*
template <>
struct typeinfo<client::ast::str8>
{
	const types value=str8_e;
	const char* name="str8";
};*/
/*
template <>
constexpr types getTypeID<client::ast::str8>()
{
	return str8_e;
}*/
#endif
