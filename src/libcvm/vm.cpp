/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman (spirit 3 , calc9 example)
    			  2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#include "vm.hpp"
#include <boost/assert.hpp>
#include <inttypes.h>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <setjmp.h>
#include "global_funcs.h"
//#define DEBUG
namespace client
{
   const int instr_size[]={//one unit is 8 bytes int64_t
    		1,//op_exit,  		//
    		1,//op_nop,         //  no exec

    		2,//op_stk_adj,     // adjust the stack (for args and locals)
			2,//op_stk_adj_chk,   // adjust the stack (for args and locals) with check the adjustment will not cross end of stack space

			2,//op_stk_assert,	//if remaining stack space is less than specified throw exception with stack content and program counter

			2,//op_less_assert, //if value on the top of stack is more or equal than value provided then throw an exception

			2,//op_0_less_assert, //if value on the top of stack is less then zero or more or equal than value provided then throw an exception

			1,//op_tsc			// put the CPU time stamp counter on the stack

			//2,//+cnt  op_print,		//prints the stack with specified list of types, only instruction with variable length

			3,//op_ucall
			1,//op_return_void //
			1,//op_return 		//

			3,//op_call,        // function call to C built-in functions
			3,//op_callv,       // function call to C with no return type  void func(args...);
			2,//op_call0reg,    // function call with zero arguments T func();
			2,//op_call0        // function call with zero arguments T func();optimized/direct call
			2,//op_callv0reg,   // function with no return type and zero args void func();
			2,//op_callv0,	    // function with no return type and zero args void func(); optimized/direct call
			4,//+cnt op_callvar  // variadic C/C++ function call, with runtime variable argument count of variable type
			4,//+cnt op_callv_var // like above with void return
			3,//op_callvararr	// variadic C/C++ function call, with runtime variable argument count
			3,//op_callv_var_arr

    		2,//op_load,        //  load a variable onto stack
    		2,//op_store,       //  store a variable onto stack
			1,//op_deref,		//  take top of stack as an offset from frame pointer in stack and replace it with the value containing it
			1,//op_deref_assign //  uses top 2 stack entries and does something like:
								//  FP[stack[top]]=stack[top-1]; POP;

			2,//op_load_ds,     //  load a global variable in data segment(ds) onto top of stack
			2,//op_store_ds,    //  store from the top of the stack to the global variable in data segment(ds)
			1,//op_deref_ds,	//  take top of stack as an location of value in data segment and replace it with the value containing it
			1,//op_deref_ds_assign,//  uses top 2 stack entries and does something like:
									//data[stack[top]]=stack[top-1]; POP;

    		2,//op_int,         //  push constant integer into the stack
    		2,//op_double,   	// 	push constant double into the stack
			2,//op_str8,		//  push constant string (up to 8 chars) into the stack
    		1,// op_true,        //  push constant 0 into the stack
    		2,// op_false,       //  push constant 1 into the stack

    		3,//	op_set,			//  set variable from prog / constant
    		3,//	op_set_double,	//  set variable from prog / constant
    		3,//	op_mov,			//  dst:=src

    		2,//    op_jump,        //  jump to an absolute position in the code

    		2,//	op_jump_if,     //  skip absolutely if top stack is true

    		2,//	op_jump_if_not, //  jump to an absolute position in the code if top stack
    		                        //  evaluates to false

			2,//    op_jump_and_keep_stack_if

    		2,//	op_jump_and_keep_stack_if_not,

    		1,//	op_neg,         //  negate the top stack int entry : i:=-i

    		1,//	op_negd,        //                       double    : d:=-d

    		1,//    op_not,         //  boolean negate the top stack entry
    		1,//    op_and,         //  logical and top two stack entries
    		1,//    op_or,          //  logical or top two stack entries

			1,//   "op_cast_int2double", //
			1,// "op_cast_double2int", //

    		2,//	op_inc,
    		2,//	op_dec,
			1,// op_add_s_is_is, //  add top two stack int entries
			3, //op_add_s_iv_iv, //  add top two stack int entries

			1, //op_add_r_ds_ds,
			1, //op_add_s_ds_ds, //  add top two stack double entries
			3, //op_add_r_dv_dv
			3, //op_add_s_dv_dv, //
			3, //op_add_s_dv_dv_,
			3, //op_add_s_dg_dg,
			3, //op_add_v_ds_dc,
			3, //op_add_v_ds_dv,

			1, //op_sub_s_is_is,
			3, //op_sub_s_iv_iv, //  add top two stack int entries

			1, //op_sub_s_ds_ds,
			3, //op_sub_s_dv_dv,
			3, //op_sub_s_dg_dg,
			3, //op_sub_v_ds_dc,
			3, //op_sub_v_ds_dv,

			1, //op_mul_s_is_is,
			3, //op_mul_s_iv_iv, //  add top two stack int entries

			1, //op_mul_s_ds_ds,
			3, //op_mul_s_dv_dv,
			3, //op_mul_s_dv_dv_,
			3, //op_mul_s_dg_dg,
			3, //op_mul_v_dr_dc
			3, //op_mul_v_ds_dc,
			3, //op_mul_v_ds_dv,

			1, //op_div_s_is_is,
			3, //op_div_s_iv_iv, //  add top two stack int entries

			1, //op_div_s_ds_ds,
			3, //op_div_s_dv_dv,
			3, //op_div_s_dg_dg,
			3, //op_div_v_ds_dc,
			3, //op_div_v_ds_dv,
			REL_INSTR_SIZES
    };

   const char* instr_name[]={//one unit is 8 bytes int64_t
       		"op_exit",  		//
       		"op_nop",         //  no exec

       		"op_stk_adj",     // adjust the stack (for args and locals)
			"op_stk_adj_chk",   // adjust the stack (for args and locals) with check the adjustment will not cross end of stack space
			"op_stk_assert",	//if remaining stack space is less than specified throw exception with stack content and program counter
			"op_less_assert",   //if value on the top of stack is more or equal than value provided then throw an exception
			"op_0_less_assert", //if value on the top of stack is less then zero or more or equal than value provided then throw an exception

			"op_tsc",		// put the CPU time stamp counter on the stack
   			//"op_print",		//prints the stack with specified list of types, only instruction with variable length

   			"op_ucall",
			"op_return_void",
   			"op_return", 		//

   			"op_call",        // function call to C built-in functions
   			"op_callv",       // function with no return type type void func(args...);
   			"op_call0reg",       // function call with zero arguments T func();
   			"op_call0",     // function call with zero arguments T func();optimized/direct call
   			"op_callv0reg",      // function with no return type and zero args void func();
   			"op_callv0",	// function with no return type and zero args void func(); optimized/direct call
			"op_callvar",      // variadic C/C++ function call, with runtime variable argument count of variable type
			"op_callv_var",	   // like above with void return
			"op_callvararr",   // variadic C/C++ function call, with runtime variable argument count
			"op_callv_var_arr", // like above with void return

       		"op_load",        //  load a variable onto stack
       		"op_store",       //  store a variable onto stack
			"op_deref",		//  take top of stack as an offset from frame pointer in stack and replace it with the value containing it
			"op_deref_assign",//  uses top 2 stack entries and does something like:
							  //  FP[stack[top]]=stack[top-1]; POP;

			"op_load_ds",     //  load a global variable in data segment(ds) onto top of stack
			"op_store_ds",    //  store from the top of the stack to the global variable in data segment(ds)
			"op_deref_ds",	//  take top of stack as an location of value in data segment and replace it with the value containing it
			"op_deref_ds_assign",//  uses top 2 stack entries and does something like:
									//data[stack[top]]=stack[top-1]; POP;
       		"op_int",         //  push constant integer into the stack
       		"op_double",   	// 	push constant double into the stack
   			"op_str8",		//  push constant string (up to 8 chars) into the stack
       		"op_true",        //  push constant 0 into the stack
       		"op_false",       //  push constant 1 into the stack

       		"op_set",			//  set variable from prog / constant
       		"op_set_double",	//  set variable from prog / constant
       		"op_mov",			//  dst:=src

       		"op_jump",        //  jump to an absolute position in the code

       		"op_jump_if",     //  skip absolutely if top stack is true

       		"op_jump_if_not", //  jump to an absolute position in the code if top stack
       		                        //  evaluates to false

			"op_jump_and_keep_stack_if",

       		"op_jump_and_keep_stack_if_not",

       		"op_neg",         //  negate the top stack int entry : i:=-i

       		"op_negd",        //                       double    : d:=-d

       		"op_not",         //  boolean negate the top stack entry
       		"op_and",         //  logical and top two stack entries
       		"op_or",          //  logical or top two stack entries

			"op_cast_int2double", //
			"op_cast_double2int", //

       		"op_inc",
       		"op_dec",
   			"op_add_s_is_is", //  add top two stack int entries
   			"op_add_s_iv_iv", //  add top two stack int entries

   			"op_add_r_ds_ds",
   			"op_add_s_ds_ds", //  add top two stack double entries
   			"op_add_r_dv_dv",
   			"op_add_s_dv_dv", //
   			"op_add_s_dv_dv_",
   			"op_add_s_dg_dg",
   			"op_add_v_ds_dc",
   			"op_add_v_ds_dv",

   			"op_sub_s_is_is",
   			"op_sub_s_iv_iv", //  add top two stack int entries

   			"op_sub_s_ds_ds",
   			"op_sub_s_dv_dv",
   			"op_sub_s_dg_dg",
   			"op_sub_v_ds_dc",
   			"op_sub_v_ds_dv",

   			"op_mul_s_is_is",
   			"op_mul_s_iv_iv", //  add top two stack int entries

   			"op_mul_s_ds_ds",
   			"op_mul_s_dv_dv",
   			"op_mul_s_dv_dv_",
   			"op_mul_s_dg_dg",
   			"op_mul_v_dr_dc",
   			"op_mul_v_ds_dc",
   			"op_mul_v_ds_dv",

   			"op_div_s_is_is",
   			"op_div_s_iv_iv", //  add top two stack int entries

   			"op_div_s_ds_ds",
   			"op_div_s_dv_dv",
   			"op_div_s_dg_dg",
   			"op_div_v_ds_dc",
   			"op_div_v_ds_dv",
			EROPS_

//   			REL_INSTR_SIZES
       };
/*
    inline volatile unsigned long tsc()
    { register
   		unsigned int eax asm ("eax");
   	  register
   	  	unsigned int edx asm ("edx");
   	   asm volatile("rdtsc" : "=a" (eax), "=d" (edx));
   	  return ((unsigned long)eax) | (((unsigned long)edx) << 32);
   	}*/



    int64_t vmachine::execute1(
      //  std::vector<int64_t> const& code,
       std::vector<int64_t>::const_iterator pc_
      , std::vector<int64_t>::iterator frame_ptr_
	  //, const std::map<std::string, std::pair<int64_t,TypeID > >& vars
    )
    {
    	 //jmp_buf env;
    	//setjmp(env);
    	//longjmp(env,1);
        int64_t lhs_i=0;
        int64_t tmp;

        double *lhs_dptr=0;

        //double *rhs_dptr=0;
        int64_t* pci=0;


static void* dispatch_table[] = {
		&&exit_label,  		//
		&&l_nop,         //  no exec

		&&l_stk_adj,     // adjust the stack (for args and locals)
		&&l_stk_adj_chk,   // adjust the stack (for args and locals) with check the adjustment will not cross end of stack space
		&&l_stk_assert,	//if remaining stack space is less than specified throw exception with stack content and program counter
		&&l_less_assert,   //if value on the top of stack is more or equal than value provided then throw an exception
		&&l_0_less_assert, //if value on the top of stack is less then zero or more or equal than value provided then throw an exception

		&&l_tsc, 		// put the CPU time stamp counter on the stack
		//&&l_print,

		&&l_ucall,
		&&l_return_void,
		&&l_return,

		// registered C/C++ functions calls
		&&l_call,        // C function call with fixed signature
		&&l_callv,       // C function with no return type type void func(args...);
		&&l_call0reg,    // function call with zero arguments T func();
		&&l_call0,    	 // function call with zero arguments T func();
		&&l_callv0reg,    // function with no return type and zero args void func();
		&&l_callv0,		  // function with no return type and zero args void func(); optimized/direct call
		&&l_callvar,      // variadic C/C++ function call, with runtime variable argument count
		&&l_callv_var,	  // variadic C/C++ function call, with runtime variable argument count and arbitrary multiple variadic types
		&&l_callvararr,      // variadic C/C++ function call, with runtime variable argument count
		&&l_callv_var_arr,// variadic C/C++ function call, with runtime variable argument count and single variadic  type (variadic array)


		&&l_load,
		&&l_store,
		&&l_deref,		//  take top of stack as an offset from frame pointer in stack and replace it with the value containing it
		&&l_deref_assign,//  uses top 2 stack entries and does something like:
								//  FP[stack[top]]=stack[top-1]; POP;

		&&l_load_ds,     //  load a global variable in data segment(ds) onto top of stack
		&&l_store_ds,    //  store from the top of the stack to the global variable in data segment(ds)
		&&l_deref_ds,	//  take top of stack as an location of value in data segment and replace it with the value containing it
		&&l_deref_ds_assign,//  uses top 2 stack entries and does something like:
						//data[stack[top]]=stack[top-1]; POP;

		&&l_int,         //  push constant integer into the stack
		&&l_double,   	// 	push constant double into the stack
		&&l_str8,
		&&l_true,        //  push constant 0 into the stack
		&&l_false,       //  push constant 1 into the stack

		&&l_set,			//  set variable from prog / constant
		&&l_set_double,	//  set variable from prog / constant
		&&l_mov,			//  dst:=src

		&&l_jump,        //  jump to an absolute position in the code

		&&l_jump_if,     //  skip absolutely if top stack is true

		&&l_jump_if_not,     //  jump to an absolute position in the code if top stack
		                //  evaluates to false
		&&l_jump_and_keep_stack_if,

		&&l_jump_and_keep_stack_if_not,//  used to short circuit expr2  in   (expr1 && expr2)
		&&l_neg,
		&&l_negd,

        &&l_not,         //  boolean negate the top stack entry
        &&l_and,         //  logical and top two stack entries
        &&l_or,          //  logical or top two stack entries

		&&l_cast_int2double, //
		&&l_cast_double2int, //

		&&l_inc,
		&&l_dec,

		&&l_add_s_is_is,
		&&l_add_s_iv_iv,


		&&l_add_r_ds_ds,
		&&l_add_s_ds_ds,
		&&l_add_r_dv_dv,
		&&l_add_s_dv_dv, //
		&&l_add_s_dv_dv_,
		&&l_add_s_dg_dg,
		&&l_add_v_ds_dc,
		&&l_add_v_ds_dv,

		&&l_sub_s_is_is,
		&&l_sub_s_iv_iv,

		&&l_sub_s_ds_ds,
		&&l_sub_s_dv_dv,
		&&l_sub_s_dg_dg,
		&&l_sub_v_ds_dc,
		&&l_sub_v_ds_dv,

		&&l_mul_s_is_is,
		&&l_mul_s_iv_iv,

		&&l_mul_s_ds_ds,
		&&l_mul_s_dv_dv,
		&&l_mul_s_dv_dv_,
		&&l_mul_s_dg_dg,
		&&l_mul_v_dr_dc,
		&&l_mul_v_ds_dc,
		&&l_mul_v_ds_dv,

		&&l_div_s_is_is,
		&&l_div_s_iv_iv,

		&&l_div_s_ds_ds,
		&&l_div_s_dv_dv,
		&&l_div_s_dg_dg,
		&&l_div_v_ds_dc,
		&&l_div_v_ds_dv,

		ELAB,

};


#ifdef DEBUG

#define FIRST_DISPATCH() {std::cout << "-------------new-frame ------------" << std::endl << "[" << (pc-&code[0]) << "->" << *pc << "," << instr_name[*pc] << "]" << std::endl; } \
	goto *dispatch_table[*pc++]

#define DISPATCH() {std::cout << "[" << (pc-&code[0]) << "->" << *pc << "," << instr_name[*pc] << "]" << std::endl; } \
	goto *dispatch_table[*pc++]

#else

#define FIRST_DISPATCH()  goto *dispatch_table[*pc++]
#define DISPATCH()  goto *dispatch_table[*pc++]

#endif

register
	int64_t* frame_ptr asm ("r15")
 			=&*frame_ptr_;

 //int64_t* frame_ptr_save=0;// r10 is volatile accross calls, we need to save it

 register
	const int64_t* pc asm ("r12")
             =&*pc_;
//        register
//	const int64_t* pc_end //asm ("r12")
//   	 	 	 =&*pc_+code.size();
 register
	const int64_t* pc_beg asm ("r14")
 			=&code[0];
 register
	int64_t* stack_ptr asm ("r13")
 			=&*frame_ptr_;

 register
 	 double tmp_reg asm("xmm3")=0.0;

// int64_t (*pf)(int64_t*)=0;

FIRST_DISPATCH();

l_nop:         //  no exec
 	DISPATCH();

l_stk_adj_chk:   // adjust the stack (for args and locals) with check the adjustment will not cross end of stack space
				// we should check here the total stack use after locals don't get exceeded
	if (size_t(stack_ptr-&*stack_.begin()+*pc)>=stack_.size())
	{
		throw stack_overflow_exception(pc+1-&*code.begin());
	}
	//fallthrough
l_stk_adj:     // adjust the stack (for args and locals)
	stack_ptr +=  *pc++;
	DISPATCH();

l_stk_assert:	//if remaining stack space is less than specified throw exception with stack content and program counter
	if (size_t(stack_ptr-&*stack_.begin()+*pc++)>=stack_.size())
	{
		throw stack_overflow_exception(pc-&*code.begin());
	}
	DISPATCH();

l_less_assert:   //if value on the top of stack is more or equal than value provided then throw an exception
	if (stack_ptr[-1] >= *pc++)
	{
		throw array_index_top_bound_exception(pc-&*code.begin(),"?",stack_ptr[-1]);
	}
	DISPATCH();
l_0_less_assert: //if value on the top of stack is less then zero or more or equal than value provided then throw an exception
	lhs_i=stack_ptr[-1];
	if ((lhs_i < 0)||(lhs_i >= *pc++))
	{
		throw array_index_range_exception(pc-&*code.begin(),"?",lhs_i);
	}
	DISPATCH();

//https://en.m.wikibooks.org/wiki/X86_Disassembly/Functions_and_Stack_Frames
l_ucall://argc, addr
//  N arguments
//		arg0,arg1...argN-1, argCount, caller_addr, caller_FP, local
//		-(N+2)			-2		-2	    -1			0			1     2

    lhs_i = *pc++;
    int64_t jump = *pc++;
    //int64_t st_off=(stack_ptr-frame_ptr);

    *stack_ptr++=lhs_i;
    *stack_ptr++=(int64_t)(pc-pc_beg);//saving return address to stack, so return instruction knows where to jump back
    *stack_ptr++=(int64_t)(frame_ptr-&*stack_.begin());//saving current frame pointer onto stack

    //stack_ptr-=2;
    //stack_ptr-=lhs_i;

    frame_ptr=stack_ptr-1;

    pc = pc_beg+jump;//jump
/*

    // a function call is a recursive call to execute
    // func f() -> void { f(); }  crashes due to hardware stack (not this VM stack ) overflow (under no local variables condition),
	//we cannot recursively call execute1
    lhs_i = *pc++;
    int jump = *pc++;
    int st_off=(stack_ptr-frame_ptr);
    int64_t r = execute1(
       // code,
       code.begin() + jump
      , frame_ptr_ + st_off - lhs_i
	 // , vars
    );

    // cleanup after return from function
    stack_ptr[-lhs_i] = r;      //  get return value
    stack_ptr -= (lhs_i - 1);   //  the stack will now contain
                                //  the return value
              */

	DISPATCH();

l_return:
#ifdef DEBUG
	std::cout << "ret:" << stack_ptr[-1]  << " " <<
	*(double*)&stack_ptr[-1] << "d " << std::endl;
#endif
	//pc = pc_beg+stack_ptr[-1];
	//  N arguments
	//		arg0,   arg1.. ,arg2,   .argN-1, argCount, caller_addr, caller_FP, local					    ret_val
	//		-(N+2)			          -3		-2	    -1			0 (FP)		1     2                      (sp-1)   (sp)
	//              new_sp
	lhs_i=frame_ptr[-2];
	frame_ptr[-2-lhs_i]=stack_ptr[-1];
	stack_ptr=frame_ptr-lhs_i-1;
	pc = pc_beg+frame_ptr[-1];
	frame_ptr = &stack_[0]+frame_ptr[0];

	DISPATCH();
	//return stack_ptr[-1];
l_return_void:
#ifdef DEBUG
	std::cout << "ret:" << stack_ptr[-1]  << " " <<
	*(double*)&stack_ptr[-1] << "d " << std::endl;
#endif
	//pc = pc_beg+stack_ptr[-1];
	//  N arguments
	//		arg0,   arg1.. ,arg2,   .argN-1, argCount, caller_addr, caller_FP, local					    ret_val
	//		-(N+2)			          -3		-2	    -1			0 (FP)		1     2                      (sp-1)   (sp)
	//              new_sp

	stack_ptr=frame_ptr-frame_ptr[-2];
	pc = pc_beg+frame_ptr[-1];
	frame_ptr = &stack_[0]+frame_ptr[0];

	DISPATCH();
	//return stack_ptr[-1];

// C/C++ functions calls
l_call:
	stack_ptr-=*pc++;
	tmp=((int64_t (*)(int64_t*))*pc++)(stack_ptr);
	*stack_ptr++=tmp;
	DISPATCH();

l_callv:       // function with no return type type void func(args...);
	stack_ptr-=*pc++;
	((int64_t (*)(int64_t*))*pc++)(stack_ptr);
	DISPATCH();

l_call0reg:       // function call with zero arguments T func();
	*stack_ptr++=((int64_t(*)())(((std::pair<func_signature,func_ptr>*)(*pc++))->second.p))();
	DISPATCH();

l_call0:       // function call with zero arguments T func();
	*stack_ptr++=((int64_t(*)())*pc++)();
	DISPATCH();

l_callv0reg:      // function with no return type and zero args void func();
	((void (*)(void))(((std::pair<func_signature,func_ptr>*)(*pc++))->second.p))();
	DISPATCH();

l_callv0:	// function with no return type and zero args void func(); optimized/direct call
	((void(*)())*pc++)();
	DISPATCH();

l_callvar:
	stack_ptr-=*pc++;//moving to the beginning of all arguments
	lhs_i=*pc++;//this is size of variadic part only
	tmp=((int64_t (*)(int64_t*,int64_t,int64_t*,const int64_t*))pc[lhs_i])(&data[0],lhs_i,stack_ptr,pc);
	pc+=lhs_i;
	++pc;
	*stack_ptr++=tmp;
	DISPATCH();

l_callv_var:
	stack_ptr-=*pc++;//moving to the beginning of all arguments
	lhs_i=*pc++;//this is size of variadic part only
	((void (*)(int64_t*,int64_t,int64_t*,const int64_t*))pc[lhs_i])(&data[0],lhs_i,stack_ptr,pc);
	pc+=lhs_i;
	++pc;
	DISPATCH();

l_callvararr:
	lhs_i=*pc++;
	stack_ptr-=lhs_i;
	// first argument is variadic type , second is total count (variadic and non-variadic) arguments and third pointer stack of arguments
	tmp=((int64_t (*)(int64_t,int64_t*))*pc++)(lhs_i,stack_ptr);
	*stack_ptr++=tmp;
	DISPATCH();

l_callv_var_arr:
	lhs_i=*pc++;
	stack_ptr-=lhs_i;
	// first argument is variadic type , second is total count (variadic and non-variadic) arguments and third pointer stack of arguments
	((int64_t (*)(int64_t,int64_t*))*pc++)(lhs_i,stack_ptr);
	DISPATCH();



l_load:
    *stack_ptr++ = frame_ptr[*pc++];
    DISPATCH();

l_store:
    --stack_ptr;
    frame_ptr[*pc++] = stack_ptr[0];
    DISPATCH();

l_deref:		//  take top of stack as an offset from frame pointer in stack and replace it with the value containing it
	stack_ptr[-1]=frame_ptr[stack_ptr[-1]];
	DISPATCH();

l_deref_assign://  uses top 2 stack entries and does something like:
							//  FP[stack[top]]=stack[top-1]; POP;POP;
	stack_ptr-=2;
	frame_ptr[stack_ptr[1]]=stack_ptr[0];
	DISPATCH();

l_load_ds:
	*stack_ptr++ = data[*pc++];
	DISPATCH();

l_store_ds:
	--stack_ptr;
	lhs_i=stack_ptr[0];
	data[*pc++] = lhs_i;
	DISPATCH();

l_deref_ds:	//  take top of stack as an location of value in data segment and replace it with the value containing it
	stack_ptr[-1]=data[stack_ptr[-1]];
	DISPATCH();

l_deref_ds_assign://  uses top 2 stack entries and does something like:
					//data[stack[top]]=stack[top-1]; POP;POP;
	stack_ptr-=2;
	data[stack_ptr[1]]=stack_ptr[0];
	DISPATCH();

l_double:
l_int:
l_str8:
	*stack_ptr++ = *pc++;
	DISPATCH();

l_true:
	*stack_ptr++ = true;
	DISPATCH();

l_false:
	*stack_ptr++ = false;
	DISPATCH();

l_set:			//  set variable from prog / constant
l_set_double://  set variable from prog / constant
	lhs_i=*pc++;
    frame_ptr[lhs_i] = *pc++;
    DISPATCH();

l_mov:			//  dst:=src
	lhs_i=*pc++;
    frame_ptr[lhs_i]= frame_ptr[*pc++];
    DISPATCH();

//strangely, when the next l_tsc block is moved at any other spot up the timing gets 15% worse even if tsc is never used,
//investigate gcc code optimization, memory allignement?
l_tsc:
/*
	{ register
 		unsigned int eax asm ("eax");
 	  register
 	  	unsigned int edx asm ("edx");
 	   asm volatile("rdtsc" : "=a" (eax), "=d" (edx));
 	  *stack_ptr++=((unsigned long)eax) | (((unsigned long)edx) << 32);
 	}*/
	*stack_ptr++ = 0;//tsc();
	DISPATCH();

l_jump:        //  jump to an absolute position in the code
	pc = pc_beg+*pc;
	DISPATCH();

l_jump_if:     //  skip absolutely if top stack is true
	--stack_ptr;
	if (bool(stack_ptr[0]))
		pc = pc_beg+*pc;
	else
		++pc;
	DISPATCH();

l_jump_if_not:     //  jump to an absolute position in the code if top stack
                //  evaluates to false
	--stack_ptr;
	if (!bool(stack_ptr[0]))
         pc = pc_beg+*pc;
    else
         ++pc;
    DISPATCH();

l_jump_and_keep_stack_if:     //  used to short circuit expr2  in   (expr1 && expr2)
    				//  evaluates to false
	if (bool(stack_ptr[-1]))
		 pc = pc_beg+*pc;
	else
	{
		 ++pc;
		--stack_ptr;
	}
	DISPATCH();

l_jump_and_keep_stack_if_not:     //  used to short circuit expr2  in   (expr1 && expr2)
				//  evaluates to false
	if (!bool(stack_ptr[-1]))
		 pc = pc_beg+*pc;
	else
	{
		 ++pc;
	    --stack_ptr;
	}
	DISPATCH();

l_neg:
	stack_ptr[-1] = -stack_ptr[-1];
	DISPATCH();

l_negd:
	*(double*)&stack_ptr[-1] = -*(double*)&stack_ptr[-1];
	DISPATCH();

l_not:
    stack_ptr[-1] = !bool(stack_ptr[-1]);
    DISPATCH();

l_and:
	--stack_ptr;
	stack_ptr[-1] = bool(stack_ptr[-1]) && bool(stack_ptr[0]);
	DISPATCH();

l_or:
	--stack_ptr;
	stack_ptr[-1] = bool(stack_ptr[-1]) || bool(stack_ptr[0]);
	DISPATCH();

l_cast_int2double:
	*(double*)&stack_ptr[-1]=(double)stack_ptr[-1];
	DISPATCH();

l_cast_double2int:
	stack_ptr[-1]=(int64_t)*(double*)&stack_ptr[-1];
	DISPATCH();

l_inc:
	++(frame_ptr[*pc++]);
	DISPATCH();

l_dec:
	--(frame_ptr[*pc++]);
	DISPATCH();


//arithmetics
l_add_s_is_is:
	--stack_ptr;
	stack_ptr[-1] += stack_ptr[0];
	DISPATCH();

l_add_s_iv_iv:
	lhs_i=*pc++;
    stack_ptr++[0] = frame_ptr[lhs_i]+frame_ptr[*pc++];
	DISPATCH();

l_add_r_ds_ds:
	tmp_reg=*(double*)(void*)&stack_ptr[-1] + *(double*)(void*)&stack_ptr[0];
	DISPATCH();

l_add_s_ds_ds:
	--stack_ptr;
    *(double*)(void*)&stack_ptr[-1] += *(double*)(void*)&stack_ptr[0];
	DISPATCH();

l_add_r_dv_dv:
	lhs_i=*pc++;
	tmp_reg = *(double*)(void*)&frame_ptr[lhs_i]+*(double*)(void*)&frame_ptr[*pc++];
	DISPATCH();

l_add_s_dv_dv:
	lhs_i=*pc++;
    *(double*)(void*)&stack_ptr++[0] = *(double*)(void*)&frame_ptr[lhs_i]+*(double*)(void*)&frame_ptr[*pc++];
	DISPATCH();

l_add_s_dv_dv_:
//	{
	//int64_t* pci=0;
	pci=const_cast<int64_t*>(&*pc);
	++*(pci-1);
	*(int64_t**)pci=&frame_ptr[*pci];
	++pci;
	*(int64_t**)pci=&frame_ptr[*pci];
	//fallthrough
	//}

l_add_s_dg_dg:
	lhs_dptr=(double*)*pc++;
	//rhs_dptr=(double*)*pc++;
	*(double*)(void*)&stack_ptr++[0]= *lhs_dptr +  *(double*)*pc++;
	DISPATCH();

l_add_v_ds_dc:
	lhs_i=*pc++;
	*(double*)&frame_ptr[lhs_i]=*(double*)(void*)&(--stack_ptr)[0] + *(double*)(void*)&(*pc++);
	DISPATCH();

l_add_v_ds_dv:
	lhs_i=*pc++;
	*(double*)&frame_ptr[lhs_i]=*(double*)(void*)&(--stack_ptr)[0] + *(double*)(void*)&frame_ptr[*pc++];
	DISPATCH();

l_sub_s_is_is:
	--stack_ptr;
	stack_ptr[-1] -= stack_ptr[0];
	DISPATCH();

l_sub_s_iv_iv:
	lhs_i=*pc++;
	stack_ptr++[0] = frame_ptr[lhs_i]-frame_ptr[*pc++];
	DISPATCH();

l_sub_s_ds_ds:
	--stack_ptr;
	*(double*)&stack_ptr[-1] -= *(double*)&stack_ptr[0];
	DISPATCH();

l_sub_s_dv_dv:
	lhs_i=*pc++;
	*(double*)(void*)&stack_ptr++[0] = *(double*)(void*)&frame_ptr[lhs_i] - *(double*)(void*)&frame_ptr[*pc++];
	DISPATCH();

l_sub_s_dg_dg:
	lhs_dptr=(double*)*pc++;
	//rhs_dptr=(double*)*pc++;
	*(double*)(void*)&stack_ptr++[0]= *lhs_dptr -  *(double*)*pc++;
	DISPATCH();

l_sub_v_ds_dc:
	lhs_i=*pc++;
	*(double*)&frame_ptr[lhs_i]=*(double*)(void*)&(--stack_ptr)[0] - *(double*)(void*)&(*pc++);
	DISPATCH();

l_sub_v_ds_dv:
	lhs_i=*pc++;
	*(double*)&frame_ptr[lhs_i]=*(double*)(void*)&(--stack_ptr)[0] - *(double*)(void*)&frame_ptr[*pc++];
	DISPATCH();

l_mul_s_is_is:
	--stack_ptr;
	stack_ptr[-1] *= stack_ptr[0];
	DISPATCH();

l_mul_s_iv_iv:
	lhs_i=*pc++;
	stack_ptr++[0] = frame_ptr[lhs_i]*frame_ptr[*pc++];
	DISPATCH();

l_mul_s_ds_ds:
	--stack_ptr;
    *(double*)(void*)&stack_ptr[-1] *= *(double*)(void*)&stack_ptr[0];
	DISPATCH();

l_mul_s_dv_dv:
	lhs_i=*pc++;
	*(double*)(void*)&stack_ptr++[0] = *(double*)(void*)&frame_ptr[lhs_i] * *(double*)(void*)&frame_ptr[*pc++];
	DISPATCH();

l_mul_s_dv_dv_:
//	{
	//int64_t* pci=0;
	pci=const_cast<int64_t*>(&*pc);
	++*(pci-1);//manipulate the code to the faster
	*(int64_t**)pci=&frame_ptr[*pci];
	++pci;
	*(int64_t**)pci=&frame_ptr[*pci];
	//}
	//fallthrough

l_mul_s_dg_dg:
	lhs_dptr=(double*)*pc++;
	//rhs_dptr=(double*)*pc++;
	*(double*)(void*)&stack_ptr++[0]= *lhs_dptr *  *(double*)*pc++;
	DISPATCH();

l_mul_v_dr_dc:
	lhs_i=*pc++;
	*(double*)&frame_ptr[lhs_i]=tmp_reg * *(double*)(void*)&(*pc++);
	DISPATCH();

l_mul_v_ds_dc:
	lhs_i=*pc++;
	*(double*)&frame_ptr[lhs_i]=*(double*)(void*)&(--stack_ptr)[0] * *(double*)(void*)&(*pc++);
	DISPATCH();

l_mul_v_ds_dv:
	lhs_i=*pc++;
	*(double*)&frame_ptr[lhs_i]=*(double*)(void*)&(--stack_ptr)[0] * *(double*)(void*)&frame_ptr[*pc++];
	DISPATCH();

l_div_s_is_is:
	--stack_ptr;
	stack_ptr[-1] /= stack_ptr[0];
	DISPATCH();

l_div_s_iv_iv:
	lhs_i=*pc++;
	stack_ptr++[0] = frame_ptr[lhs_i]/frame_ptr[*pc++];
	DISPATCH();

l_div_s_ds_ds:
	--stack_ptr;
	*(double*)&stack_ptr[-1] /= *(double*)&stack_ptr[0];
	DISPATCH();

l_div_s_dv_dv:
	lhs_i=*pc++;
	*(double*)(void*)&stack_ptr++[0] = *(double*)(void*)&frame_ptr[lhs_i] / *(double*)(void*)&frame_ptr[*pc++];
	DISPATCH();

l_div_s_dg_dg:
	lhs_dptr=(double*)*pc++;
	//rhs_dptr=(double*)*pc++;
	*(double*)(void*)&stack_ptr++[0]= *lhs_dptr /  *(double*)*pc++;
	DISPATCH();

l_div_v_ds_dc:
	lhs_i=*pc++;
	*(double*)&frame_ptr[lhs_i]=*(double*)(void*)&(--stack_ptr)[0] / *(double*)(void*)&(*pc++);
	DISPATCH();

l_div_v_ds_dv:
	lhs_i=*pc++;
	*(double*)&frame_ptr[lhs_i]=*(double*)(void*)&(--stack_ptr)[0] / *(double*)(void*)&frame_ptr[*pc++];
	DISPATCH();


//relational

#define REL_OPER_S_S_S_(OP,CAST) \
 	 --stack_ptr; \
	stack_ptr[-1] = bool(CAST stack_ptr[-1] OP CAST stack_ptr[0]);	\
	DISPATCH();

#define REL_OPER_S_S_C_(OP,CAST) \
	stack_ptr[-1] = bool(CAST stack_ptr[-1] OP CAST *pc++);	\
	DISPATCH();

#define REL_OPER_S_S_V_(OP,CAST) \
	stack_ptr[-1] = bool(CAST stack_ptr[-1] OP CAST frame_ptr[*pc++]);	\
	DISPATCH();

#define REL_OPER_S_V_C_(OP,CAST) \
	lhs_i=*pc++; \
	*stack_ptr++ = bool(CAST frame_ptr[lhs_i] OP CAST *pc++);\
	DISPATCH();

#define REL_OPER_S_V_V_(OP,CAST) \
	lhs_i=*pc++; \
	*stack_ptr++ = bool(CAST frame_ptr[lhs_i] OP CAST frame_ptr[*pc++]);\
	DISPATCH();

#define REL_OPER_S_V_NV_(OP,CAST) \
	lhs_i=*pc++; \
	*stack_ptr++ = bool(CAST frame_ptr[lhs_i] OP - CAST frame_ptr[*pc++]);\
	DISPATCH();

//is the following safe , does twice pc++ evaluate from left to right
#define REL_OPER_S_G_G_(OP,PCAST) \
	*stack_ptr++= bool(* PCAST *pc++ OP  * PCAST *pc++);\
	DISPATCH();

#define REL_OPER_S_G_C_(OP,PCAST,CAST) \
	*stack_ptr++= bool(* PCAST *pc++ OP  CAST *pc++);\
	DISPATCH();

#define REL_OPER_S_IS_IS(LABEL_PREFIX,OP) \
 LABEL_PREFIX ## s_is_is: \
	REL_OPER_S_S_S_(OP,)

#define REL_OPER_S_IS_IC(LABEL_PREFIX,OP) \
 LABEL_PREFIX ## s_is_ic: \
	REL_OPER_S_S_C_(OP,)

#define REL_OPER_S_IS_IV(LABEL_PREFIX,OP) \
 LABEL_PREFIX ## s_is_iv: \
	REL_OPER_S_S_V_(OP,)

#define REL_OPER_S_IV_IC(LABEL_PREFIX,OP) \
 LABEL_PREFIX ## s_iv_ic: \
	REL_OPER_S_V_C_(OP,)

#define REL_OPER_S_IV_IV(LABEL_PREFIX,OP) \
 LABEL_PREFIX ## s_iv_iv: \
	REL_OPER_S_V_V_(OP,)

#define REL_OPER_S_IV_NIV(LABEL_PREFIX,OP) \
 LABEL_PREFIX ## s_iv_inv: \
	REL_OPER_S_V_NV_(OP,)

#define REL_OPER_S_DS_DS(LABEL_PREFIX,OP) \
 LABEL_PREFIX ## s_ds_ds: \
	REL_OPER_S_S_S_(OP,*(double*)&)

#define REL_OPER_S_DS_DC(LABEL_PREFIX,OP) \
 LABEL_PREFIX ## s_ds_dc: \
	REL_OPER_S_S_C_(OP,*(double*)&)

#define REL_OPER_S_DS_DV(LABEL_PREFIX,OP) \
 LABEL_PREFIX ## s_ds_dv: \
	REL_OPER_S_S_V_(OP,*(double*)&)

#define REL_OPER_S_DV_DC(LABEL_PREFIX,OP) \
 LABEL_PREFIX ## s_dv_dc: \
	REL_OPER_S_V_C_(OP,*(double*)&)

#define REL_OPER_S_DV_DV(LABEL_PREFIX,OP) \
 LABEL_PREFIX ## s_dv_dv: \
	REL_OPER_S_V_V_(OP,*(double*)&)

#define REL_OPER_S_DV_NDV(LABEL_PREFIX,OP) \
 LABEL_PREFIX ## s_dv_dnv: \
	REL_OPER_S_V_NV_(OP,*(double*)&)


#define REL_OPER_S_I(LABEL_PREFIX,OP) \
	REL_OPER_S_IS_IS(LABEL_PREFIX,OP); \
	REL_OPER_S_IS_IC(LABEL_PREFIX,OP); \
	REL_OPER_S_IS_IV(LABEL_PREFIX,OP); \
	REL_OPER_S_IV_IC(LABEL_PREFIX,OP); \
	REL_OPER_S_IV_IV(LABEL_PREFIX,OP); \
	REL_OPER_S_IV_NIV(LABEL_PREFIX,OP); \

#define REL_OPER_S_D(LABEL_PREFIX,OP) \
	REL_OPER_S_DS_DS(LABEL_PREFIX,OP); \
	REL_OPER_S_DS_DC(LABEL_PREFIX,OP); \
	REL_OPER_S_DS_DV(LABEL_PREFIX,OP); \
	REL_OPER_S_DV_DC(LABEL_PREFIX,OP); \
	REL_OPER_S_DV_DV(LABEL_PREFIX,OP); \
	REL_OPER_S_DV_NDV(LABEL_PREFIX,OP); \

REL_OPER_S_I(l_eq_,==)
REL_OPER_S_D(l_eq_,==)
REL_OPER_S_I(l_neq_,!=)
REL_OPER_S_D(l_neq_,!=)
REL_OPER_S_I(l_lt_,<)
REL_OPER_S_D(l_lt_,<)
REL_OPER_S_I(l_lte_,<=)
REL_OPER_S_D(l_lte_,<=)
REL_OPER_S_I(l_gt_,>)
REL_OPER_S_D(l_gt_,>)
REL_OPER_S_I(l_gte_,>=)
REL_OPER_S_D(l_gte_,>=)


/*

l_lt_s_iv_ic_:
	pci=const_cast<int64_t*>(&*pc);
	++*(pci-1);//manipulate the code to the faster
	*(int64_t**)pci=&frame_ptr[*pci];

l_lt_s_ig_ic:
	lhs_iptr=(int64_t*)*pc++;
	*stack_ptr++ = bool(*lhs_iptr < *pc++);
	DISPATCH();

l_gt_s_dv_dv_:
	pci=const_cast<int64_t*>(&*pc);
	++*(pci-1);//manipulate the code to the faster
	*(int64_t**)pci=&frame_ptr[*pci];
	++pci;
	*(int64_t**)pci=&frame_ptr[*pci];

l_gt_s_dg_dg:
	lhs_dptr=(double*)*pc++;
	rhs_dptr=(double*)*pc++;
	*stack_ptr++ = bool(*lhs_dptr > *rhs_dptr);
	DISPATCH();
*/

        exit_label:
//        if (pc != pc_end)
  //      {
    //    	std::cout << "Warning:program stopped beyond the end (as opposed to at the end)" << std::endl;
      //  }
        return -1;
    }
}
