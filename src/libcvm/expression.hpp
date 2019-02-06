/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman (spirit 3 , calc9 example)
    			  2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_X3_CALC9_EXPRESSION_HPP)
#define BOOST_SPIRIT_X3_CALC9_EXPRESSION_HPP

#include <boost/spirit/home/x3.hpp>
#include "ast.hpp"

namespace client
{
    namespace x3 = boost::spirit::x3;
    namespace parser
    {
        struct expression_class;
        typedef x3::rule<expression_class, ast::expression> expression_type;
        BOOST_SPIRIT_DECLARE(expression_type);

        struct expression_list_class;
        typedef x3::rule<expression_list_class, ast::expression_list> expression_list_type;
        BOOST_SPIRIT_DECLARE(expression_list_type);

        struct primary_expr_class;
        typedef x3::rule<primary_expr_class, ast::operand> primary_expr_type;
        BOOST_SPIRIT_DECLARE(primary_expr_type);
    }
    
    parser::expression_type const& expression();
    parser::expression_list_type const& expression_list();
    parser::primary_expr_type const& primary_expression();
}

#endif
