/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman (spirit 3 , calc9 example)
    			  2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_X3_CALC9_STATEMENT_HPP)
#define BOOST_SPIRIT_X3_CALC9_STATEMENT_HPP

#include <boost/spirit/home/x3.hpp>
#include "ast.hpp"

namespace client
{
    namespace x3 = boost::spirit::x3;
    namespace parser
    {
        struct statement_class;
        typedef x3::rule<statement_class, ast::statement_list> statement_type;
        typedef statement_type::id statement_id;
        BOOST_SPIRIT_DECLARE(statement_type);

        struct statement_list_class;
        typedef x3::rule<statement_list_class, ast::statement_list> statement_list_type;
        BOOST_SPIRIT_DECLARE(statement_list_type);
    }
    
    parser::statement_type const& statement();
    parser::statement_list_type const& statement_list();
}


#endif
