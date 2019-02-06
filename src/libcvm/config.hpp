/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman (spirit 3 , calc9 example)
    			  2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_X3_CALC9_CONFIG_HPP)
#define BOOST_SPIRIT_X3_CALC9_CONFIG_HPP


#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/auxiliary/eol.hpp>
#include "error_handler.hpp"

namespace client { namespace parser
{
    typedef std::string::const_iterator iterator_type;
    using boost::spirit::x3::standard::space;
    using boost::spirit::x3::standard::lit;
    using boost::spirit::x3::char_;
    using boost::spirit::x3::eol;
    using boost::spirit::x3::eoi;
//    Skipper block_comment, single_line_comment, skipper;

    const auto block_comment = "/*" >> *(char_ - "*/") > "*/";

    const auto space_and_comments = space | block_comment | (lit("//") >> *(char_ - eol) >> (eol|eoi));
    typedef decltype(space_and_comments) space_and_comments_type;


    typedef x3::phrase_parse_context<space_and_comments_type>::type phrase_context_type;
    typedef error_handler<iterator_type> error_handler_type;

    typedef x3::context<
        error_handler_tag
      , std::reference_wrapper<error_handler_type>
      , phrase_context_type>
    context_type;
}}

#endif
