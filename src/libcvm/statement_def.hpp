/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman (spirit 3 , calc9 example)
    			  2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(BOOST_SPIRIT_X3_CALC9_STATEMENT_DEF_HPP)
#define BOOST_SPIRIT_X3_CALC9_STATEMENT_DEF_HPP

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/annotate_on_success.hpp>
#include "ast.hpp"
#include "ast_adapted.hpp"
#include "statement.hpp"
#include "expression.hpp"
#include "common.hpp"
#include "error_handler.hpp"

namespace client { namespace parser
{
    using x3::raw;
    using x3::lexeme;
    using namespace x3::ascii;

    //struct statement_list_class;  //this is in statement.hpp
    struct variable_declaration_class;
    struct assignment_class;
    struct assignment_list_class;
    struct array_element_assignment_class;
    struct variable_class;
    struct single_statement_class;
	struct block_statement_class;
	struct if_statement_class;
	struct while_statement_class;
	struct funcion_call_class;
	struct funcion_call2_class;
	struct argument_decl_class;
	struct funcion_def_class;
	struct inc_var_class;
	struct dec_var_class;

    //typedef x3::rule<statement_list_class, ast::statement_list> statement_list_type; //this is in statement.hpp
    typedef x3::rule<variable_declaration_class, ast::variable_declaration> variable_declaration_type;
    typedef x3::rule<assignment_class, ast::assignment> assignment_type;
    typedef x3::rule<variable_class, ast::variable> variable_type;

    typedef x3::rule<assignment_list_class, ast::assignment_list> assignment_list_type;
    typedef x3::rule<array_element_assignment_class, ast::array_element_assignment> array_element_assignment_type;

	typedef x3::rule<single_statement_class,ast::statement> single_statement_type;
	typedef x3::rule<block_statement_class,ast::statement_list> block_statement_type;
	typedef x3::rule<if_statement_class,ast::if_statement> if_statement_type;
	typedef x3::rule<while_statement_class,ast::while_statement> while_statement_type;

	typedef x3::rule<funcion_call_class,ast::function_call> function_call_type;
	typedef x3::rule<funcion_call2_class,ast::function_call2> function_call2_type;
	typedef x3::rule<argument_decl_class,ast::argument_decl> argument_decl_type;
	typedef x3::rule<funcion_def_class,ast::function_def_statement> function_def_type;

	typedef x3::rule<inc_var_class,ast::inc> inc_var_type;
	typedef x3::rule<dec_var_class,ast::dec> dec_var_type;


    statement_type const statement("statement");
    statement_list_type const statement_list("statement_list");
    variable_declaration_type const variable_declaration("variable_declaration");
    assignment_type const assignment("assignment");
    variable_type const variable("variable");

    assignment_list_type const assignment_list("assignment_list");
    array_element_assignment_type const array_element_assignment("array_element_assignement");

	single_statement_type const single_statement("single_statement");
	block_statement_type const block_statement("block_statement");
	if_statement_type const if_statement("if_statement");
	while_statement_type const while_statement("while_statement");

	function_call_type const function_call("funcion_call");
	function_call2_type const function_call2("funcion_call");
	argument_decl_type const argument_decl("argument_decl");
	function_def_type const function_def("function_def_type");

	inc_var_type const inc_var("inc var");
	dec_var_type const dec_var("dec var");

    // Import the expression rule
    namespace { auto const& expression = client::expression(); }
    namespace { auto const& expression_list = client::expression_list(); }
    namespace { auto const& primary_expr = client::primary_expression(); }

    auto const single_statement_def =
    		    block_statement
				| while_statement
				| if_statement
    			| variable_declaration
        		| function_call
			    | assignment
    			| assignment_list
    			| array_element_assignment
				| inc_var
				| dec_var
				| function_def
				;

    auto const statement_list_def       =  +single_statement ;

    auto const block_statement_def 	    = lit('{') >> statement_list >> lit('}');

	auto const argument_list_def 		= (quote|expression)  % ',';

	auto const function_call_def 		= identifier  >> '(' >> -argument_list_def >> ')' >> ';';

	auto const end_of_line_comment		= lexeme[(lit('#')|lit("//")) >> *(x3::omit[~(char_("\r\n"))])];

	auto const argument_decl_def 		= identifier >> identifier;

	auto const argument_decl_list_def   = argument_decl % ',';

	//auto const function_call2_def 		= lit("def") >> identifier  >> '(' >> -argument_decl_list_def >> ')' >> ';';

	auto const function_def_def			= (lit("def")|lit("func")) >> !lit('=') >> identifier  >> '(' >> -argument_decl_list_def >> ')'
										>> -(lit("->") >> identifier)
										>> lit('{') >> statement_list >> lit('}') >> -lit(';');

	auto const inc_var_def				= lit("++") >> variable >> ';';

	auto const dec_var_def				= lit("--") >> variable >> ';';

	auto const variable_declaration_def =
			lexeme[(lit("var")|lit("auto")) >> !(alnum | '_')] // make sure we have whole words
		>   assignment
		;

	auto const assignment_def =
			variable
		>>   '='
		>>   expression//primary_expr  // no '>'  , only '>>' , we don't want to throw exception we want to go to test assignment of expression_list
		>>   ';' ;

	auto const assignment_list_def =
				variable
			>>   '='
			>>   expression_list
			>>   ';';

	auto const array_element_assignment_def = variable >> '[' >> primary_expr >> ']' >> '=' >> primary_expr >> ';';

	auto const while_statement_def			= lit("while") >> lit('(') >> expression >> lit(')') >> single_statement;

   // auto const else_statement_def= lit("else") >> statement;
	auto const if_statement_def		= lit("if") >> lit('(') >> expression >> lit(')') >> single_statement
										 >> -(lit("else") >> single_statement);

	auto const variable_def =  !keywords >> identifier;

    auto const statement_def = statement_list;// this is in calc9 example
    //auto const statement_def = single_statement;// this is mine , if used there are move_to errors

    BOOST_SPIRIT_DEFINE(
        statement
      , statement_list
      , variable_declaration
      , assignment
      , variable

	  , single_statement
	  , block_statement
	  , if_statement
	  , while_statement
	  , function_call
	  , assignment_list
	  , array_element_assignment
	  , function_def
	  , argument_decl
	  , inc_var
	  , dec_var
    );

	  struct statement_class : error_handler_base, x3::annotate_on_success {};
	  struct assignment_class : error_handler_base,x3::annotate_on_success {};
	  struct variable_class : error_handler_base,x3::annotate_on_success {};

      struct statement_list_class: error_handler_base, x3::annotate_on_success {};;
	  struct variable_declaration_class: error_handler_base, x3::annotate_on_success {};;
	  struct function_def_class: error_handler_base, x3::annotate_on_success {};;
	  struct single_statement_class: error_handler_base, x3::annotate_on_success {};
	  struct block_statement_class: error_handler_base, x3::annotate_on_success {};;
	  struct if_statement_class: error_handler_base, x3::annotate_on_success {};;
	  struct while_statement_class: error_handler_base, x3::annotate_on_success {};;
	  struct funcion_call_class: error_handler_base, x3::annotate_on_success {};;
	  struct argument_decl_class: error_handler_base, x3::annotate_on_success {};;
	  struct funcion_def_class: error_handler_base, x3::annotate_on_success {};;
	  struct inc_var_class: error_handler_base, x3::annotate_on_success {};;
	  struct dec_var_class: error_handler_base, x3::annotate_on_success {};;
	  struct array_element_assignment_class:error_handler_base,x3::annotate_on_success {};
}}

namespace client
{
    parser::statement_type const& statement()
    {
        return parser::statement;
    }
    parser::statement_list_type const& statement_list()
    {
            return parser::statement_list;
    }
}

#endif

