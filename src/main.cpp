/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman (spirit 3 , calc9 example)
    			  2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
///////////////////////////////////////////////////////////////////////////////
//
//  Now we'll introduce boolean expressions and control structures.
//  Is it obvious now what we are up to? ;-)
//
//  [ JDG April 9, 2007 ]       spirit2
//  [ JDG February 18, 2011 ]   Pure attributes. No semantic actions.
//  [ JDG June 6, 2014 ]        Ported from qi calc8 example.
//
///////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/auxiliary/eol.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/chrono.hpp>
#include <chrono>
#include <iomanip>
#include <utility>
#include <typeinfo>
#include "libcvm/types.h"

#include "libcvm/ast.hpp"
#include "libcvm/vm.hpp"
#include "libcvm/compiler.hpp"
#include "libcvm/expression.hpp"
//#include "statement.hpp"
#include "libcvm/error_handler.hpp"
#include "libcvm/config.hpp"
#include <functional>

///////////////////////////////////////////////////////////////////////////////
//  Main program
///////////////////////////////////////////////////////////////////////////////


inline std::string load(boost::filesystem::path p)
   {
       boost::filesystem::ifstream file(p);
       if (!file)
           return "";
       std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
       return contents;
   }

int64_t now2()
{
	static std::chrono::steady_clock::time_point start=std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point nw=std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::nanoseconds>(nw - start).count();
};


int main(int argc, char *argv[])
{
	bool optimize=false;
	bool execute=true;
	bool show_asm=false;
	bool show_timing_parsing=false;
	bool show_timing_compile=false;
	bool show_timing_optimize=false;
	bool show_timing_execute=false;
	bool show_optimized_asm=false;//implies optimization
	bool glob_evaluation_at_compilation=false;//if true , compiler basically becomes JIT compiler/interpreter (no execution is needed afterwards,because no VM code is emitted)
											//similar to concept constexpr functions in C++14, result is data segment
	bool void_func_glob_init=true;//functions returning void are not executed in global scope at compilation,code is emitted though (similar to C)
	bool glob_var_decl_only=false;//if true attempts to modify global variable in global scope fails compilations (similar to C)
	bool stack_space_check_at_func_start=true;//
	bool print_vars=false;
	bool autodeclare_vars_on_first_assignment=true;
	bool array_index_bounds_check=true;
	bool assert_stack_size_before_variadic_function_call=true;// only way for variadic functions to make sure parameters don't stack overflow
	bool assert_stack_size_at_function_start=true;

	std::string filename;

	for (int i=1;i<argc;i++)
	{
		if (std::string(argv[i])=="-o")
			optimize=true;
		else if (std::string(argv[i])=="-ne")
			execute=false;
		else if ((std::string(argv[i])=="--no-stack-space-check")||(std::string(argv[i])=="-nsc"))
			stack_space_check_at_func_start=false;
		else if ((std::string(argv[i])=="--no-array-index-check")||(std::string(argv[i])=="-naic"))
			array_index_bounds_check=false;
		else if ((std::string(argv[i])=="--no-void-func-glob-init")||(std::string(argv[i])=="-nvfgi"))
			void_func_glob_init=false;
		else if ((std::string(argv[i])=="--glob-var-decl-only")||(std::string(argv[i])=="-gvdo"))
			glob_var_decl_only=true;
		else if ((std::string(argv[i])=="--no-function-stack-check")||(std::string(argv[i])=="-nfsc"))
			assert_stack_size_at_function_start=false;
		else if ((std::string(argv[i])=="--no-variadic-func-stack-check")||(std::string(argv[i])=="-nvfsc"))
			assert_stack_size_before_variadic_function_call=false;
		else if (std::string(argv[i])=="-a")
			show_asm=true;
		else if (std::string(argv[i])=="-tp")
			show_timing_parsing=true;
		else if (std::string(argv[i])=="-tc")
			show_timing_compile=true;
		else if (std::string(argv[i])=="-to")
			show_timing_optimize=true;
		else if (std::string(argv[i])=="-te")
			show_timing_execute=true;
		else if (std::string(argv[i])=="-t")
		{
			show_timing_parsing=true;
			show_timing_compile=true;
			show_timing_optimize=true;
			show_timing_execute=true;
		}
		else if (std::string(argv[i])=="-oa")
		{
			optimize=true;
			show_optimized_asm=true;
		}
		else if (std::string(argv[i])=="-v")
		{
			print_vars=true;
		}
		else if (std::string(argv[i])=="--no-vars-auto-decl")
		{
			autodeclare_vars_on_first_assignment=false;
		}
		else if ((std::string(argv[i])=="--compile-init-global-data")||(std::string(argv[i])=="-cigd"))
		{
			glob_evaluation_at_compilation=true;
		}
		else if (argv[i][0]!='-')
			 filename=argv[i];
		else
		{
			 std::cerr << "Error:Unrecognized option:" << argv[i] << std::endl;
			 return -1;
		}
	}

	if (filename.empty())
	{

		std::cerr << "Error:Source file not specified" << std::endl;
		return -3;
	}

	if (!boost::filesystem::exists(filename))
	{
		 std::cerr << "File '"<< filename << "' does not exist." << std::endl;
		 return -4;
	}

	std::chrono::steady_clock::time_point t0=std::chrono::steady_clock::now();
	//boost::chrono::time_point t0u=boost::chrono::process_user_cpu_clock::now();

    std::string source=load(filename);


    using client::parser::iterator_type;
    iterator_type iter(source.begin());
    iterator_type end(source.end());



    client::code_gen::program program;                      // Our VM program
    client::vmachine vm(program(),program.data());                                    // Our virtual machine

    client::ast::statement_list ast;                        // Our AST
    //client::ast::expression ast;

    using boost::spirit::x3::with;
    using client::parser::error_handler_type;
    error_handler_type error_handler(iter, end, std::cerr); // Our error handler

    // Our compiler
    client::code_gen::compilation_context cctx;
    client::code_gen::compiler compile(program, error_handler,cctx,vm);

    compile.auto_declare_vars_on_first_assignment=autodeclare_vars_on_first_assignment;
    compile.evaluate_expressions_in_global_scope=glob_evaluation_at_compilation;
	compile.void_func_glob_init=void_func_glob_init;//functions returning void are not executed in global scope at compilation,code is emitted though (similar to C)
	compile.glob_var_decl_only=glob_var_decl_only;//if true attempts to modify global variable in global scope fails compilations (similar to C)
	compile.stack_space_check_at_func_start=stack_space_check_at_func_start;//
	compile.array_index_bounds_check=array_index_bounds_check;
	compile.assert_stack_size_before_variadic_function_call=assert_stack_size_before_variadic_function_call;
	compile.assert_stack_size_at_function_start=assert_stack_size_at_function_start;


    // Our parser
    auto const parser =
        // we pass our error handler to the parser so we can access
        // it later on in our on_error and on_sucess handlers
        with<client::parser::error_handler_tag>(std::ref(error_handler))
        [
            client::statement_list()
		// client::expression()
        ];
    //client::statement();
//    using boost::spirit::x3::standard::space;

    using client::parser::space_and_comments;
//    Skipper block_comment, single_line_comment, skipper;

//    auto sp = space | (lit("//") >> *(char_ - eol) >> (eol|eoi));
//    block_comment = "/*" >> *(block_comment | char_ - "*/") > "*/"

    //auto sp= space | lit("//");// >> lit - ~boost::spirit::x3::eol;



    std::chrono::steady_clock::time_point t0a=std::chrono::steady_clock::now();
    //boost::chrono::process_cpu_clock::time_point t0ap=boost::chrono::process_cpu_clock::now();
    //boost::chrono::process_user_cpu_clock::time_point t0au=boost::chrono::process_user_cpu_clock::now();

    bool success = phrase_parse(iter, end, parser, space_and_comments, ast);

    if (!success || (iter != end))
    {
        std::cerr << "Parse failure\n";
        std::string str(iter,end);
        std::cerr << str << std::endl;
        return -2;
    }

    std::chrono::steady_clock::time_point t1=std::chrono::steady_clock::now();


    if (show_timing_parsing)
    {
      auto usecs0=std::chrono::duration_cast<std::chrono::nanoseconds>(t0a - t0).count();
      std::cout << "Timing loading:" << usecs0 << " nsecs" << std::endl;
      auto usecsp=std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0a).count();
      std::cout << "Timing parsing:" << usecsp << " nsecs" << std::endl;
      auto usecs=std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
      std::cout << "Timing loading+parsing:" << usecs << " nsecs" << std::endl;

    }


	//boost::apply_visitor(vis(),ast.first);

	if (!compile.start(ast))
	{
		std::cerr << "Compile failure\n";
		return -1;
	}


	std::chrono::steady_clock::time_point t2=std::chrono::steady_clock::now();
	if (show_timing_compile)
	{
		  auto nsecs=std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
		  auto nsecs02=std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t0).count();
		  std::cout << "Timing compiling:" << nsecs << " nsecs " << std::endl;
		  std::cout << "Timing loading+parsing+compiling:" << nsecs02 << " nsecs " << std::endl;
	}

	//adjust data segment size to keep all the global variables
	program.adjust_data_segment();

	if (show_asm)
	{
		std::cout << "-------------------------\n";
		std::cout << "Assembler----------------\n\n";
	   program.print_assembler();
	}

	if (optimize)
	{
	  program.optimize();
	  std::chrono::steady_clock::time_point t3=std::chrono::steady_clock::now();
	  if (show_timing_optimize)
	  {
		  auto usecs=std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
		  auto usecs02=std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t0).count();
		  std::cout << "Timing optimizing:" << usecs << " nsecs " << std::endl;
		  std::cout << "Timing loading+parsing+compiling+optimizing:" << usecs02 << " nsecs " << std::endl;
	  }
//              program.optimize();
	}

	if (show_optimized_asm)
	{
	  std::cout << "-------------------------\n";
	  std::cout << "Optimized Assembler----------------\n\n";
	  program.print_assembler();
	}

	if (execute)
	{
	  std::cout << std::setprecision(2);
	  std::chrono::steady_clock::time_point t23=std::chrono::steady_clock::now();

	  try {
		vm.execute();
	  }
	  catch (client::stack_overflow_exception& exc)
	  {
		  std::cerr << "stack overflow exception at location:" << exc.pc << std::endl;
	  }
	  catch (client::array_index_range_exception& exc)
	  {
		  std::cerr << "index range exception at location:" << exc.pc << " with index value "
				  << exc.wrong_index << std::endl;
	  }
	  catch (std::exception& exc)
	  {
		  std::cerr << "exception of program running in VM:"<< exc.what() << std::endl;
	  }
	  catch (...)
	  {
		  std::cerr << "unknown exception of program running in VM:" << std::endl;
	  }

	  //boost::chrono::process_cpu_clock::time_point t1p=boost::chrono::process_cpu_clock::now();
	  //auto dff=t1p-t0ap;
	 // auto myr=dff.count();
   // auto dff2=dff.count();
//      auto ms=boost::chrono::duration_cast<boost::chrono::microseconds>(dff).count();

	//std::cout << "t1p=" << myr << std::endl;

	  if (show_timing_execute)
	  {
		std::chrono::steady_clock::time_point t4=std::chrono::steady_clock::now();
		auto nsecs=std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t23).count();
		auto nsecs02=std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t0).count();
		std::cout << "Timing execution:" << nsecs << " nsecs " << std::endl;
		std::cout << "Timing loading+parsing+compiling+(optimization)+execution:" << nsecs02 << " nsecs " << std::endl;
	  }
	}

	if (print_vars)
	{
		std::cout << "-------------------------\n";
		std::cout << "Results------------------\n\n";
		std::cout << std::setprecision(20);
		program.print_variables(vm.get_stack());
		program.print_variables_global();
	}



    //std::cout << "-------------------------\n\n";
    return 0;
}
