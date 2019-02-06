/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman (spirit 3 , calc9 example)
    			  2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_X3_CALC9_COMPILER_HPP)
#define BOOST_SPIRIT_X3_CALC9_COMPILER_HPP

#include "ast.hpp"
#include "error_handler.hpp"
#include "program.h"
#include "vm.hpp"

namespace client { namespace code_gen
{

    ////////////////////////////////////////////////////////////////////////////
    //  The Compiler
    ////////////////////////////////////////////////////////////////////////////


	enum ResultSuccess
	{
		fail=0,
		type_calculated=1,//type only calculated during compilation
		value_calculated=2 // both value and type calculated during compilation in global scope only,
							// it must be enabled with the flag evaluate_expressions_in_global_scope
							// it tries to calculate value, whenever expression contains variable from stack
							// or not yet initialized variable from data segment then  it
							// reverts to calculating the resulting expression type only and emits
							// VM instruction to do it later
	};
/*
	struct stack_size
	{
		int64_t sz;
	};*/
	typedef int64_t stack_size;
    struct res_type
    {
    	res_type():success{fail},type{INT64},val(0){}

    	res_type(ResultSuccess b,TypeID tid=INT64,stack_size ss=1,int64_t sd=1,int64_t val0=0):
    			success(b),	type(tid),stack_requirement(ss),sp_diff{sd},val(val0){}

    	res_type(bool b,TypeID tid=INT64,stack_size ss=1,int64_t sd=1):
    			success(b ? type_calculated : fail),type(tid),stack_requirement(ss),sp_diff{sd},val(0){}

    	operator bool (){ return success;}

    	ResultSuccess success;
    	TypeID type;
    	int64_t stack_requirement=0;//stack size used/needed to calculate this result by evaluating expression
    	int64_t sp_diff=0;//how instruction/operation changed stack pointer (SP), size of resulting data placed on stack,
    						//this can be less than stack_requirement , always sp_diff <= stack_requirement
    	int64_t val;//this allows calculation of expression during compilation (interpretation)
    				//if enabled in the global scope (useful to initiate data segment)
    				//how do we know if calculation succeeded??

    };

    struct compilation_context
    {
    	compilation_context():local_context{false}{};
        bool local_context;//if true the assignements var=expr; are compiled to allocate/locate variable on stack,
        					// otherwise the data segment is used for variable allocation/
        					// regardless of the value of this flag, the variable on right hand side
        					// is always tried to look up locally first and if not found then globally
        					// function body is local_context, also block braces
    };
    struct compiler
    {
        typedef res_type result_type;
        typedef std::function<
            void(x3::position_tagged, std::string const&)>
        error_handler_type;

        template <typename ErrorHandler>
        compiler(
            client::code_gen::program& program
          , ErrorHandler const& error_handler
		  , compilation_context& ctx0,
		  client::vmachine& vm0)
          : program(program)
          , error_handler(
                [&](x3::position_tagged pos, std::string const& msg)
                { error_handler(pos, msg); }
            ),ctx(ctx0),
			vm(vm0)
        {}

        result_type operator()(ast::nil) const { BOOST_ASSERT(0); return false; }
        result_type operator()(int64_t x) const;
        result_type operator()(uint64_t x) const { return false; }
        result_type operator()(uint32_t x) const { return false; }
        result_type operator()(int32_t x) const { return false; }
        result_type operator()(double x) const;
        result_type operator()(bool x) const;
        result_type operator()(ast::str8 x) const;
        result_type operator()(const ast::quote& x) const;
        result_type operator()(ast::variable const& x) const;
        result_type operator()(ast::operand const& x) const;
        result_type operator()(ast::subscript const& x) const;
        result_type operator()(ast::variable const& lhs,ast::optoken opt,ast::variable const& rhs) const;
        result_type operator()(ast::operation const& x,result_type rt) const;
        result_type operator()(ast::unary const& x) const;
        result_type operator()(ast::expression const& x) const;
        result_type operator()(ast::expression_list const& x) const;
        result_type operator()(ast::cast_expression const& x) const;
        result_type operator()(ast::assignment const& x) const;
        result_type operator()(ast::assignment_list const& x) const;
        result_type operator()(ast::array_element_assignment const& x) const;
        result_type operator()(ast::variable_declaration const& x) const;
        result_type operator()(ast::statement_list const& x) const;
        result_type operator()(ast::statement const& x) const;
        result_type operator()(ast::if_statement const& x) const;
        result_type operator()(ast::while_statement const& x) const;
        result_type operator()(ast::function_call const& x) const;
        result_type operator()(ast::function_call2 const& x) const;
        result_type operator()(ast::inc const& x) const;
        result_type operator()(ast::dec const& x) const;
        result_type operator()(const ast::function_def_statement& funcdef) const;

        result_type start(ast::statement_list const& x) const;

        client::code_gen::program& program;
        error_handler_type error_handler;
        compilation_context& ctx;
        client::vmachine& vm;

        bool auto_declare_vars_on_first_assignment=true;
        bool evaluate_expressions_in_global_scope=true;//if true it computes expressions in global scope at compilation turning into interpreter,
        bool same_types_in_brace_init_expression_list=true;
    	bool void_func_glob_init=true;//functions returning void are not executed in global scope at compilation,code is emitted though (similar to C)
    	bool glob_var_decl_only=false;//if true attempts to modify global variable in global scope fails compilations (similar to C)
    	bool stack_space_check_at_func_start=true;//
    	bool array_index_bounds_check=true;
    	bool assert_stack_size_before_variadic_function_call=true;// only way for variadic functions to make sure parameters don't stack overflow
    	bool assert_stack_size_at_function_start=true;
    };
}}

#endif

