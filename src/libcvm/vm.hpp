/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman (spirit 3 , calc9 example)
    			  2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#if !defined(BOOST_SPIRIT_X3_CALC9_VM_HPP)
#define BOOST_SPIRIT_X3_CALC9_VM_HPP

#include <vector>
#include <inttypes.h>
#include <map>
#include <iostream>
#include "types.h"

//Relational OperatorS ROP
#define ROP_S(PR,LHS,RHS,TYPEC) PR ## TYPEC ## LHS ## _ ## TYPEC ## RHS


#define ROPS(PREFIX,TYPEC) \
	 ROP_S(PREFIX,s,s,TYPEC), \
	 ROP_S(PREFIX,s,c,TYPEC), \
	 ROP_S(PREFIX,s,v,TYPEC), \
	 ROP_S(PREFIX,v,c,TYPEC), \
	 ROP_S(PREFIX,v,v,TYPEC), \
	 ROP_S(PREFIX,v,nv,TYPEC)

#define STRINGIFY(A) #A
#define STRINGIFY_(B) STRINGIFY(B)
#define ROP_S_(PR,LHS,RHS,TYPEC) STRINGIFY_(ROP_S(PR,LHS,RHS,TYPEC))

#define ROPS_(PREFIX,TYPEC) \
	 ROP_S_(PREFIX,s,s,TYPEC), \
	 ROP_S_(PREFIX,s,c,TYPEC), \
	 ROP_S_(PREFIX,s,v,TYPEC), \
	 ROP_S_(PREFIX,v,c,TYPEC), \
	 ROP_S_(PREFIX,v,v,TYPEC), \
	 ROP_S_(PREFIX,v,nv,TYPEC)

//#define CAT_(A,B) A##B
//#define CAT(A,B) CAT_(A,B)

#define ROPS2(PREFIX,TYPEC) \
	 &&ROP_S(PREFIX,s,s,TYPEC), \
	 &&ROP_S(PREFIX,s,c,TYPEC), \
	 &&ROP_S(PREFIX,s,v,TYPEC), \
	 &&ROP_S(PREFIX,v,c,TYPEC), \
	 &&ROP_S(PREFIX,v,v,TYPEC), \
	 &&ROP_S(PREFIX,v,nv,TYPEC)

#define INSTR_SIZES 1,2,2,3,3,3
#define REPEAT12(A) A,A,A,A ,A,A,A,A, A,A,A,A
#define REL_INSTR_SIZES REPEAT12(INSTR_SIZES)

#define EROPS \
	ROPS(op_eq_s_,i), \
	ROPS(op_eq_s_,d), \
	ROPS(op_neq_s_,i), \
	ROPS(op_neq_s_,d), \
	ROPS(op_lt_s_,i), \
	ROPS(op_lt_s_,d), \
	ROPS(op_lte_s_,i), \
	ROPS(op_lte_s_,d), \
	ROPS(op_gt_s_,i), \
	ROPS(op_gt_s_,d), \
	ROPS(op_gte_s_,i), \
	ROPS(op_gte_s_,d)

#define EROPS_ \
	ROPS_(op_eq_s_,i), \
	ROPS_(op_eq_s_,d), \
	ROPS_(op_neq_s_,i), \
	ROPS_(op_neq_s_,d), \
	ROPS_(op_lt_s_,i), \
	ROPS_(op_lt_s_,d), \
	ROPS_(op_lte_s_,i), \
	ROPS_(op_lte_s_,d), \
	ROPS_(op_gt_s_,i), \
	ROPS_(op_gt_s_,d), \
	ROPS_(op_gte_s_,i), \
	ROPS_(op_gte_s_,d)


#define ELAB \
	ROPS2(l_eq_s_,i), \
	ROPS2(l_eq_s_,d), \
	ROPS2(l_neq_s_,i), \
	ROPS2(l_neq_s_,d), \
	ROPS2(l_lt_s_,i), \
	ROPS2(l_lt_s_,d), \
	ROPS2(l_lte_s_,i), \
	ROPS2(l_lte_s_,d), \
	ROPS2(l_gt_s_,i), \
	ROPS2(l_gt_s_,d), \
	ROPS2(l_gte_s_,i), \
	ROPS2(l_gte_s_,d)


namespace client
{
    ///////////////////////////////////////////////////////////////////////////
    //  The Virtual Machine
    ///////////////////////////////////////////////////////////////////////////




    enum byte_code
    {
    	//op_start=10,

		op_exit,  		//
		op_nop,         //  no exec

		op_stk_adj,     // adjust the stack (for args and locals)

		op_stk_adj_chk,   // adjust the stack (for args and locals) with check the adjustment will not cross end of stack space

		op_stk_assert,	//if remaining stack space is less than specified throw exception with stack content and program counter

		op_less_assert, //if value on the top of stack is more or equal than value provided then throw an exception

		op_0_less_assert, //if value on the top of stack is less then zero or more or equal than value provided then throw an exception

		op_tsc,			// put the CPU time stamp counter on the stack
		//op_print,		//prints the stack with specified list of types, only instruction with variable length

		op_ucall,		// UDF user defined functions call
		op_return_void,  // return from UDF
		op_return_int,   // return from UDF

		op_call,        // function call T f(args,...) with fixed signiture non-variadic
		op_callv,       // function with no return type type void func(args...);
		op_call0reg,    // function call with zero arguments T func();
		op_call0,    	// function call with zero arguments T func();
		op_callv0reg,   // function with no return type and zero args void func();
		op_callv0,		// function with no return type and zero args void func(); optimized/direct call
		op_callvar,		// variadic C/C++ function call, with runtime variable argument count and arbitrary multiple variadic types
		op_callv_var,	// variadic C/C++ function call, with runtime variable argument count and arbitrary multiple variadic types
		op_callvararr,	// variadic C/C++ function call, with runtime variable argument count and single variadic  type (variadic array)
		op_callv_var_arr,// variadic C/C++ function call, with runtime variable argument count and single variadic  type (variadic array)


        op_load,        //  load a local variable onto top of stack
        op_store,       //  store from the top of the stack to the local variable
		op_deref,		//  take top of stack as an offset from frame pointer (FP) in stack and replace it with the value containing it
						//  stack[top]=FP[stack[top]]
		op_deref_assign,//  uses top 2 stack entries and does something like:
						//  FP[stack[top]]=stack[top-1]; POP;

		op_load_ds,     //  load a global variable in data segment(ds) onto top of stack
		op_store_ds,    //  store from the top of the stack to the global variable in data segment(ds)
		op_deref_ds,	//  take top of stack as an location of value in data segment and replace it with the value containing it
						//  stack[top]=data[stack[top]]
		op_deref_ds_assign,//  uses top 2 stack entries and does something like:
						//data[stack[top]]=stack[top-1]; POP;

		op_int,         //  push constant integer into the stack
		op_double,   	// 	push constant double into the stack
		op_str8,		//  push constant string (up to 8 chars) into the stack
		op_true,        //  push constant 0 into the stack
	    op_false,       //  push constant 1 into the stack

		op_set,			//  set variable from prog / constant
		op_set_double,	//  set variable from prog / constant
		op_mov,			//  dst:=src

        op_jump,        //  jump to an absolute position in the code

		op_jump_if,     //  skip absolutely if top stack is true

		op_jump_if_not, //  jump to an absolute position in the code if top stack
                        //  evaluates to false

		op_jump_and_keep_stack_if,

		op_jump_and_keep_stack_if_not,

		op_neg,         //  negate the top stack int entry : i:=-i

		op_negd,        //                       double    : d:=-d

        op_not,         //  boolean negate the top stack entry
        op_and,         //  logical and top two stack entries
        op_or,          //  logical or top two stack entries

		op_cast_int2double, //
		op_cast_double2int, //

		op_inc,
		op_dec,

		op_add_s_is_is, //  add top two stack int entries
		op_add_s_iv_iv, //  add top two stack int entries

		op_add_r_ds_ds,
		op_add_s_ds_ds, //  add top two stack double entries
		op_add_r_dv_dv,
		op_add_s_dv_dv, //
		op_add_s_dv_dv_,
		op_add_s_dg_dg,
		op_add_v_ds_dc,
		op_add_v_ds_dv,

		op_sub_s_is_is,
		op_sub_s_iv_iv, //  add top two stack int entries

		op_sub_s_ds_ds,
		op_sub_s_dv_dv,
		op_sub_s_dg_dg,
		op_sub_v_ds_dc,
		op_sub_v_ds_dv,

		op_mul_s_is_is,
		op_mul_s_iv_iv, //  add top two stack int entries

		op_mul_s_ds_ds,
		op_mul_s_dv_dv,
		op_mul_s_dv_dv_,
		op_mul_s_dg_dg,
		op_mul_v_dr_dc,
		op_mul_v_ds_dc,
		op_mul_v_ds_dv,

		op_div_s_is_is,
		op_div_s_iv_iv, //  add top two stack int entries

		op_div_s_ds_ds,
		op_div_s_dv_dv,
		op_div_s_dg_dg,
		op_div_v_ds_dc,
		op_div_v_ds_dv,

		EROPS,


		op_last

    };

    extern const int instr_size[op_last];
    //sizes are needed to iterate safely over the program skip all instructions to look for jumps and to shrink/optimize
    //we assume,there are no gaps and garbage data in program

    extern const char* instr_name[op_last];


    struct stack_overflow_exception:std::exception
    {
 	   stack_overflow_exception(size_t pc0):pc(pc0){}
 	   const char* what() const noexcept{
 		   return "stack overflow";
 	   }
 	   size_t pc;
    };

    struct array_index_top_bound_exception:std::exception
    {
 	   array_index_top_bound_exception(size_t pc0,const std::string& arr0,int64_t wrong_index0)
 			   :pc(pc0),array_name(arr0),wrong_index(wrong_index0){}
 	   const char* what() const noexcept{
 		   return "array index top bound";
 	   }
 	   size_t pc;
 	   std::string array_name;
 	   int64_t wrong_index;
    };

    struct array_index_range_exception:std::exception
    {
 	   array_index_range_exception(size_t pc0,const std::string& arr0,int64_t wrong_index0)
 			   :pc(pc0),array_name(arr0),wrong_index(wrong_index0){}
 	   const char* what() const noexcept{
 		   return "array index range";
 	   }
 	   size_t pc;
 	   std::string array_name;
 	   int64_t wrong_index;
    };

    class vmachine
    {
    public:

        vmachine(std::vector<int64_t> const& code0,std::vector<int64_t>& data_segment,unsigned stackSize = 4096)
          : code(code0),data(data_segment),stack_(stackSize)
        {
        	//std::vector<int64_t> fake_exit{0};
        	//const std::map<std::string, std::pair<int64_t,TypeID > > vars;
        	//execute1(fake_exit,fake_exit.begin(),stack.begin(),vars);// to initiate calculated goto static array
        }

        void print(const std::map<std::string, std::pair<int64_t,TypeID > >& variables)
        {
			for (auto const& p : variables)
			{
				std::cout << p.second.second << "  " << p.first << ": ";
				if (p.second.second==DOUBLE)
						std::cout << *(double*)(void*)&stack_[p.second.first];
				else
						std::cout << stack_[p.second.first];
				std::cout << " ";
			}
			std::cout << std::endl;
        }

        int64_t execute1(
            //std::vector<int64_t> const& code            // the program code
           std::vector<int64_t>::const_iterator pc     // program counter
          , std::vector<int64_t>::iterator frame_ptr    // start of arguments and locals
		  //, const std::map<std::string, std::pair<int64_t,TypeID > >& vars
        );

        int64_t execute()
        {
            return execute1(code.begin(), stack_.begin());
        };

        std::vector<int64_t> const& get_stack() const { return stack_; };

    private:
        std::vector<int64_t> const& code;
        std::vector<int64_t>& data;//data segment
        std::vector<int64_t> stack_;


    };
}

#endif

