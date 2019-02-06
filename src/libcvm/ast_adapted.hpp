/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman (spirit 3 , calc9 example)
    			  2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_X3_CALC9_AST_ADAPTED_HPP)
#define BOOST_SPIRIT_X3_CALC9_AST_ADAPTED_HPP

#include "ast.hpp"
#include <boost/fusion/include/adapt_struct.hpp>

BOOST_FUSION_ADAPT_STRUCT(client::ast::unary,
    operator_, operand_
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::operation,
    operator_, operand_
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::expression,
    first, rest
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::cast_expression,
    type, expr
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::variable_binary_operation,
		lhs,operator_,rhs
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::variable_declaration,
    assign
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::inc,
    var
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::dec,
    var
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::assignment,
    lhs, rhs
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::assignment_list,
    lhs, rhs
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::array_element_assignment,
		lhs,lhs_index,rhs
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::subscript,
    name, index
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::if_statement,
    condition, then
	, else_
)
BOOST_FUSION_ADAPT_STRUCT(client::ast::function_def_statement,
		name,arguments,return_type,body
)


BOOST_FUSION_ADAPT_STRUCT(client::ast::while_statement,
    condition, body
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::function_call2 ,
	name,arguments
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::function_call ,
	name,arguments
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::func_name,
		name//,arguments,return_type,body
)


BOOST_FUSION_ADAPT_STRUCT(client::ast::argument_decl,
		type1,name
)

BOOST_FUSION_ADAPT_STRUCT(client::ast::quote,
		str
)
#endif
