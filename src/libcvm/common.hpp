/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman (spirit 3 , calc9 example)
    			  2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_X3_CALC9_COMMON_HPP)
#define BOOST_SPIRIT_X3_CALC9_COMMON_HPP

#include <boost/spirit/home/x3.hpp>
#include "error_handler.hpp"
#include "ast.hpp"

namespace client { namespace parser
{
    using x3::raw;
    using x3::lexeme;
    using x3::alpha;
    using x3::alnum;
    using x3::lexeme;
    using x3::char_;

    extern x3::symbols<> keywords;



    struct identifier_class;
    typedef x3::rule<identifier_class, std::string> identifier_type;
    identifier_type const identifier = "identifier";

    auto const identifier_def = raw[lexeme[(alpha | '_') >> *(alnum | '_')]];



    struct quote_class;
    typedef
            x3::rule<quote_class, ast::quote>
        quote_type;
    quote_type const
   		quote = "quote";
    auto const quote_def =lexeme['"' >>  *(~char_('"')) ]>> '"';

    struct quote_class:client::parser::error_handler_base, x3::annotate_on_success {};;

    BOOST_SPIRIT_DEFINE(identifier
    		,quote);

}}

#endif
