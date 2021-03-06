/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman (spirit 3 , calc9 example)
    			  2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#include "expression_def.hpp"
#include "config.hpp"

namespace client { namespace parser
{
    BOOST_SPIRIT_INSTANTIATE(expression_type, iterator_type, context_type);
    BOOST_SPIRIT_INSTANTIATE(expression_list_type, iterator_type, context_type);
    BOOST_SPIRIT_INSTANTIATE(primary_expr_type, iterator_type, context_type);
}}
