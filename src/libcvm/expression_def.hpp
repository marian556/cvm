/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman (spirit 3 , calc9 example)
    			  2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_X3_CALC9_EXPRESSION_DEF_HPP)
#define BOOST_SPIRIT_X3_CALC9_EXPRESSION_DEF_HPP

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include "ast.hpp"
#include "ast_adapted.hpp"
#include "expression.hpp"
#include "common.hpp"
#include "error_handler.hpp"

namespace client { namespace parser
{
    using x3::uint_;
    using x3::int64;
    using x3::double_;
    using x3::char_;
    using x3::bool_;
    using x3::raw;
    using x3::lexeme;
    using namespace x3::ascii;

    x3::symbols<> keywords;
    ////////////////////////////////////////////////////////////////////////////
    // Tokens
    ////////////////////////////////////////////////////////////////////////////

    x3::symbols<ast::optoken> equality_op;
    x3::symbols<ast::optoken> relational_op;
    x3::symbols<ast::optoken> logical_op;
    x3::symbols<ast::optoken> additive_op;
    x3::symbols<ast::optoken> multiplicative_op;
    x3::symbols<ast::optoken> unary_op;


    void add_keywords()
    {
        static bool once = false;
        if (once)
            return;
        once = true;

        logical_op.add
            ("&&", ast::op_and)
            ("||", ast::op_or)
            ;

        equality_op.add
            ("==", ast::op_equal)
            ("!=", ast::op_not_equal)
            ;

        relational_op.add
            ("<", ast::op_less)
            ("<=", ast::op_less_equal)
            (">", ast::op_greater)
            (">=", ast::op_greater_equal)
            ;

        additive_op.add
            ("+", ast::op_plus)
            ("-", ast::op_minus)
            ;

        multiplicative_op.add
            ("*", ast::op_times)
            ("/", ast::op_divide)
            ;

        unary_op.add
			("++", ast::op_inc)
			("--", ast::op_dec)
            ("+", ast::op_positive)
            ("-", ast::op_negative)
            ("!", ast::op_not)
            ;
        keywords.add
              ("var")
              ("true")
              ("false")
              ("if")
              ("else")
              ("while")
			  ("func")
			  ("def")
			  ("typeid")
			  ("not")
			  ("auto")
			  ("cast")
              ;

    }

    ////////////////////////////////////////////////////////////////////////////
    // Main expression grammar
    ////////////////////////////////////////////////////////////////////////////

    struct equality_expr_class;
    struct relational_expr_class;
    struct logical_expr_class;
    struct additive_expr_class;
    struct multiplicative_expr_class;
    struct unary_expr_class;
    struct primary_expr_class;
    struct function_call2_class;
    struct variable_rhs_class;
    struct cast_expression_class;
    struct subscript_class;
   // struct expression_list_class;

    typedef x3::rule<equality_expr_class, ast::expression> equality_expr_type;
    typedef x3::rule<relational_expr_class, ast::expression> relational_expr_type;
    typedef x3::rule<logical_expr_class, ast::expression> logical_expr_type;
    typedef x3::rule<additive_expr_class, ast::expression> additive_expr_type;
    typedef x3::rule<multiplicative_expr_class, ast::expression> multiplicative_expr_type;
    typedef x3::rule<unary_expr_class, ast::operand> unary_expr_type;
    typedef x3::rule<primary_expr_class, ast::operand> primary_expr_type;
    typedef x3::rule<function_call2_class,ast::function_call> function_call2_type;
    typedef x3::rule<variable_rhs_class, ast::variable> variable_rhs_type;
    typedef x3::rule<cast_expression_class, ast::cast_expression> cast_expression_type;
    typedef x3::rule<subscript_class,ast::subscript> subscript_type;
    //typedef x3::rule<expression_list_class, ast::expression_list> expression_list_type;


    expression_type const expression = "expression";
    expression_list_type const expression_list = "expression_list";
    equality_expr_type const equality_expr = "equality_expr";
    relational_expr_type const relational_expr = "relational_expr";
    logical_expr_type const logical_expr = "logical_expr";
    additive_expr_type const additive_expr = "additive_expr";
    multiplicative_expr_type const multiplicative_expr = "multiplicative_expr";
    unary_expr_type const unary_expr = "unary_expr";
    primary_expr_type const primary_expr = "primary_expr";
    function_call2_type const function_call2="funcion_call2";
    variable_rhs_type const variable_rhs="variable_rhs";
    cast_expression_type const cast_expression="cast_expression";
    subscript_type const subscript="subscript";

    auto const logical_expr_def =
            equality_expr
        >> *(logical_op > equality_expr)
        ;

    auto const equality_expr_def =
            relational_expr
        >> *(equality_op > relational_expr)
        ;

    auto const relational_expr_def =
            additive_expr
        >> *(relational_op > additive_expr)
        ;

    auto const additive_expr_def =
            multiplicative_expr
        >> *(additive_op > multiplicative_expr)
        ;

    auto const multiplicative_expr_def =
            unary_expr
        >> *(multiplicative_op > unary_expr)
        ;

    auto const unary_expr_def =
            primary_expr
        |   (unary_op > primary_expr)
        ;

    auto const argument_list_def 		= (quote|expression)  % ',';

    auto const function_call2_def 		= identifier  >> '(' >> -argument_list_def >> ')' ;

    auto const expression_list_def		= lit("{") >> expression  % ',' >> lit("}");

    auto const variable_rhs_def			= (!keywords >> identifier);

    auto const cast_expression_def		= lit("cast<") >> identifier >> lit(">") >> lit("(") >> expression >> lit(")");

    auto const subscript_def			= identifier >> '[' >> expression >> ']';

    typedef x3::real_parser<double,x3::strict_real_policies<double> > double_strict_type;
    double_strict_type const double_strict_ = {};

    auto const primary_expr_def =

		 	double_strict_
		|	int64
        |   bool_
		|   function_call2
		|   quote
		|   cast_expression
		|   subscript
        |   variable_rhs
        |   ('(' >> expression >> ')')
        ;

    auto const expression_def = logical_expr;

    BOOST_SPIRIT_DEFINE(
        expression
	  , expression_list
	  , cast_expression
      , logical_expr
      , equality_expr
      , relational_expr
      , additive_expr
	  , function_call2
      , multiplicative_expr
      , unary_expr
      , primary_expr
	  , variable_rhs
	  , subscript
    );

    //struct unary_expr_class : error_handler_base, x3::annotate_on_success {};
    //struct primary_expr_class : error_handler_base, x3::annotate_on_success {};

    struct equality_expr_class:error_handler_base, x3::annotate_on_success {};
    struct relational_expr_class:error_handler_base, x3::annotate_on_success {};;
    struct logical_expr_class:error_handler_base, x3::annotate_on_success {};;
    struct additive_expr_class:error_handler_base, x3::annotate_on_success {};;
    struct multiplicative_expr_class:error_handler_base, x3::annotate_on_success {};;
    struct unary_expr_class:error_handler_base, x3::annotate_on_success {};;
    struct primary_expr_class:error_handler_base, x3::annotate_on_success {};;
    struct function_call2_class:error_handler_base, x3::annotate_on_success {};
    struct variable_rhs_class:error_handler_base, x3::annotate_on_success {};
    struct cast_expression_class:error_handler_base, x3::annotate_on_success {};
    struct expression_list_class:error_handler_base, x3::annotate_on_success {};
    struct subscript:error_handler_base, x3::annotate_on_success {};

}}

namespace client
{
    parser::expression_type const& expression()
    {
        parser::add_keywords();
        return parser::expression;
    }
    parser::expression_list_type const& expression_list()
    {
           //parser::add_keywords();
        return parser::expression_list;
    }
    parser::primary_expr_type const& primary_expression()
    {
        //parser::add_keywords();
        return parser::primary_expr;
    }
}

#endif
