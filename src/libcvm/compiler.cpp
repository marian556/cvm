/*=============================================================================
    Copyright (c) 2001-2014 Joel de Guzman (spirit 3 , calc9 example)
    			  2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#include "compiler.hpp"
#include "vm.hpp"
#include <boost/variant/apply_visitor.hpp>
#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <set>
#include <iostream>
#include <cstring>
#include "global_funcs.h"
#include "types.h"

namespace client { namespace code_gen
{

    compiler::result_type compiler::operator()(int64_t x) const
    {
    	if (evaluate_expressions_in_global_scope&&(!ctx.local_context) )
    	{
    		return {value_calculated,INT64,1,1,x};
    	}
    	program.op(op_int, x);
        return {true,INT64,1,1};
    }

    union mydouble
	{
    	double d;
    	int i[2];
    	int64_t val;
	};

    compiler::result_type compiler::operator()(double x) const
    {
    	mydouble md{x};
    	if (evaluate_expressions_in_global_scope&&(!ctx.local_context) )
    	{
    	    return {value_calculated,DOUBLE,1,1,md.val};
    	}
        program.op(op_double,md.val);
        return {true,DOUBLE,1,1};
    }

    compiler::result_type compiler::operator()(ast::str8 x) const
    {
    	 if (evaluate_expressions_in_global_scope&&(!ctx.local_context) )
    	 {
    	     return {value_calculated,STR8,1,1,x.val};
    	 }
         program.op(op_str8,x.val);
         return {true,STR8,1,1};
    }

    compiler::result_type compiler::operator()(const ast::quote& x) const
    {

    	if (x.str.size()<=8)
    	{
			std::string str=x.str;
			str.resize(8);
			return (*this)((ast::str8) &str[0]);
    	}
    	//error_handler(x, std::string("Strings longer than 8 characters not (yet) supported: \"") + x.str+"\"");
    	std::string varname="__str_"+boost::lexical_cast<std::string>(program.size());
    	int sz0=x.str.size();
    	int sz=1+(sz0>>3);//making space for terminating zero / C string
    	VarInfo& vi=program.add_var_global(varname.c_str(),CSTRING,sz,true);
    	program.data().resize(vi.location+sz);
    	char *dst=(char*)(void*)&(program.data()[vi.location]);
    	strncpy(dst,&x.str[0],x.str.size());
    	((char*)(void*)&(program.data()[vi.location]))[sz0]=0;// terminating C string 0 byte
    	program.op(op_str8,vi.location);
		return {true,CSTRING,1,1};//not yet supported
    }

    compiler::result_type compiler::operator()(bool x) const
    {
    	if (evaluate_expressions_in_global_scope&&(!ctx.local_context) )
    	{
    	    return {value_calculated,INT64,1,1,(int64_t)x};
    	}
        program.op(x ? op_true : op_false);
        return {true,INT64,1,1};
    }

    compiler::result_type compiler::operator()(ast::variable const& x) const
    {
        VarInfo const* p = program.find_var_info(x.name);//do we need to call this in global scope
        if (p == 0)
        {
        	p=program.find_var_info_global(x.name);
        	if (p==0)
        	{
        		error_handler(x, "Undeclared variable: " + x.name);
        		return false;
        	}
        	if (evaluate_expressions_in_global_scope&&(!ctx.local_context)&&(p->initialized) )
        	{
        	    return {value_calculated,p->tid,1,1,program.data()[p->location]};
        	}
        	program.op(op_load_ds, p->location);
        	return {true,p->tid,1,1};
        }
        program.op(op_load, p->location);
        return {true,p->tid,1,1};
    }

    compiler::result_type compiler::operator()(ast::operand const& x) const
    {
    	return boost::apply_visitor(*this, x);
    }

    compiler::result_type compiler::operator()(ast::operation const& x,compiler::result_type lhs_type) const
    {
    	result_type rhs_type=boost::apply_visitor(*this, x.operand_);
    	if (!rhs_type)
    	{
    		//error_handler(x, "Operation not supported. ");
            return false;
    	}
    	//int64_t flags=((lhs_type.type==double_e) ? lhs_double : 0)|((rhs_type.type==double_e) ? rhs_double : 0);
    	bool bothd=((lhs_type.type==DOUBLE)&&(rhs_type.type==DOUBLE));
    	bool bothi=(((lhs_type.type==INT64)||(lhs_type.type==INT))
    			&&((rhs_type.type==INT64)||(rhs_type.type==INT)));

    	if ((!bothd)&&(!bothi))
    	{
    		//error_handler(x, "Operands of mixed type not supported.");, ast::operation does not have its own parsing rule
    		return false;
    	}

		 //1,..........
		 //  1,..........
		 int64_t stack_req=std::max(lhs_type.stack_requirement,1+rhs_type.stack_requirement);

    	if (evaluate_expressions_in_global_scope&&(!ctx.local_context)&&
    			(lhs_type.success==value_calculated)&& (rhs_type.success==value_calculated))
    	{
    		 if (bothd)
    		 {
    			 double lhs=TO_DOUBLE(lhs_type.val);
    			 double rhs=TO_DOUBLE(rhs_type.val);


				 switch (x.operator_)
				 {
					case ast::op_plus:
						return {value_calculated,DOUBLE,stack_req,1,TO_INT64(lhs+rhs)};
					case ast::op_minus:
						return {value_calculated,DOUBLE,stack_req,1,TO_INT64(lhs-rhs)};
					case ast::op_times:
						return {value_calculated,DOUBLE,stack_req,1,TO_INT64(lhs*rhs)};
					case ast::op_divide:
						return {value_calculated,DOUBLE,stack_req,1,TO_INT64(lhs/rhs)};
					case ast::op_equal:
						return {value_calculated,DOUBLE,stack_req,1,lhs==rhs};
					case ast::op_not_equal:
						return {value_calculated,DOUBLE,stack_req,1,lhs!=rhs};
					case ast::op_less:
						return {value_calculated,DOUBLE,stack_req,1,lhs<rhs};
					case ast::op_less_equal:
						return {value_calculated,DOUBLE,stack_req,1,lhs<=rhs};
					case ast::op_greater:
						return {value_calculated,DOUBLE,stack_req,1,lhs>rhs};
					case ast::op_greater_equal:
						return {value_calculated,DOUBLE,stack_req,1,lhs>=rhs};

				//	case ast::op_and: program.op(op_and); break;
				//	case ast::op_or: program.op(op_or); break;
					default: BOOST_ASSERT(0);
						error_handler(x, "Unknown operation val="+ boost::lexical_cast<std::string>(x.operator_));
					   return false;
    		   }
    		 } else
    		 {
    			 int64_t lhs=lhs_type.val;
    			 int64_t rhs=rhs_type.val;

    			 switch (x.operator_)
				 {
					case ast::op_plus:
						return {value_calculated,INT64,stack_req,1,lhs+rhs};
					case ast::op_minus:
						return {value_calculated,INT64,stack_req,1,lhs-rhs};
					case ast::op_times:
						return {value_calculated,INT64,stack_req,1,lhs*rhs};
					case ast::op_divide:
						return {value_calculated,INT64,stack_req,1,lhs/rhs};
					case ast::op_equal:
						return {value_calculated,INT64,stack_req,1,lhs==rhs};
					case ast::op_not_equal:
						return {value_calculated,INT64,stack_req,1,lhs!=rhs};
					case ast::op_less:
						return {value_calculated,INT64,stack_req,1,lhs<rhs};
					case ast::op_less_equal:
						return {value_calculated,INT64,stack_req,1,lhs<=rhs};
					case ast::op_greater:
						return {value_calculated,INT64,stack_req,1,lhs>rhs};
					case ast::op_greater_equal:
						return {value_calculated,INT64,stack_req,1,lhs>=rhs};
					case ast::op_and:
						return {value_calculated,INT64,stack_req,1,lhs&&rhs};
					case ast::op_or:
						return {value_calculated,INT64,stack_req,1,lhs||rhs};
					case ast::op_bit_and:
						return {value_calculated,INT64,stack_req,1,lhs&rhs};
					case ast::op_bit_or:
						return {value_calculated,INT64,stack_req,1,lhs|rhs};
					case ast::op_bit_xor:
						return {value_calculated,INT64,stack_req,1,lhs^rhs};
					case ast::op_bit_lshift:
						return {value_calculated,INT64,stack_req,1,lhs<<rhs};
					case ast::op_bit_rshift:
						return {value_calculated,INT64,stack_req,1,lhs>>rhs};
					default: BOOST_ASSERT(0);
						error_handler(x, "Unknown operation val="+ boost::lexical_cast<std::string>(x.operator_));
					   return false;
			   }

    		 }
    	}

        switch (x.operator_)
        {
            case ast::op_plus:
            	program.op(bothd ? op_add_s_ds_ds : op_add_s_is_is);
            	break;
            case ast::op_minus:
            	program.op(bothd ? op_sub_s_ds_ds : op_sub_s_is_is);
            	break;
            case ast::op_times:
            	program.op(bothd ? op_mul_s_ds_ds : op_mul_s_is_is);
            	break;
            case ast::op_divide:
            	program.op(bothd ? op_div_s_ds_ds : op_div_s_is_is);
            	break;
            case ast::op_equal:
            	program.op(bothd ? op_eq_s_ds_ds : op_eq_s_is_is);
            	break;
            case ast::op_not_equal:
            	program.op(bothd ? op_neq_s_ds_ds : op_neq_s_is_is);
			    break;
            case ast::op_less:
            	program.op(bothd ? op_lt_s_ds_ds : op_lt_s_is_is);
            	break;
            case ast::op_less_equal:
            	program.op(bothd ? op_lte_s_ds_ds : op_lte_s_is_is);
            	break;
            case ast::op_greater:
            	program.op(bothd ? op_gt_s_ds_ds : op_gt_s_is_is);
            	break;
            case ast::op_greater_equal:
            	program.op(bothd ? op_gte_s_ds_ds : op_gte_s_is_is);
            	break;

            case ast::op_and: program.op(op_and); break;
            case ast::op_or: program.op(op_or); break;
            default: BOOST_ASSERT(0);
            	error_handler(x, "Unknown operation val="+ boost::lexical_cast<std::string>(x.operator_));
               return false;
        }
        TypeID outtype=((lhs_type.type==DOUBLE)||(rhs_type.type==DOUBLE)) //not sure about this
        		?
        		DOUBLE :
        		INT64;
        return {true,outtype,stack_req,1};
    }

    compiler::result_type compiler::operator()(ast::unary const& x) const
    {
    	if ((x.operator_==ast::op_inc)||(x.operator_==ast::op_dec))
    	{
    		const ast::variable * pvar=boost::get<ast::variable>(&x.operand_.get());
    		if (!pvar)
    		{
    			error_handler(*pvar, "operand is not a variable " + pvar->name);
    			return false;
    		}
    		VarInfo const* p = program.find_var_info(pvar->name);
			if (p == 0)
			{
				error_handler(*pvar, "Undeclared variable: " + pvar->name);
				return false;
			}
    		program.op((x.operator_==ast::op_inc) ? op_inc : op_dec, p->location);
    		return {true,INT64,0,0};//has zero stack requirement and offset
    	}

    	result_type type=boost::apply_visitor(*this, x.operand_);
    	if (!type)
    	{
    		error_handler(x,"unary operator not supported : operator id=" + x.operator_);
            return false;
    	}

        switch (x.operator_)
        {
            case ast::op_negative: program.op((type.type==DOUBLE)  ? op_negd : op_neg); break;
            case ast::op_not: program.op(op_not); break;
            case ast::op_positive: break;
            default: BOOST_ASSERT(0);
            	error_handler(x,"Unary operator not supported : operator id=" + x.operator_);
                return false;
        }
        return type;
    }

    compiler::result_type compiler::operator()(ast::inc const& x) const
    {
    	VarInfo const* p = program.find_var_info(x.var.name);
		if (p == 0)
		{
			error_handler(x.var, "Undeclared variable: " + x.var.name);
			return false;
		}
		program.op(op_inc, p->location);
		return {true,p->tid,0,0};
    }

    compiler::result_type compiler::operator()(ast::dec const& x) const
    {
    	VarInfo const* p = program.find_var_info(x.var.name);
		if (p == 0)
		{
			error_handler(x.var, "Undeclared variable: " + x.var.name);
			return false;
		}
		program.op(op_dec, p->location);
		return {true,p->tid,0,0};
    }

    compiler::result_type compiler::operator()(ast::subscript const& x) const
    {
    	//return false;

    	compiler::result_type rt=(*this)(x.index);
		if (!rt)
		{
			error_handler(x, "Compiling of rhs expression failed in assignment. ");
			return false;
		}
		if (rt.type!=INT64)
		{
			error_handler(x, "Type of RHS is not an integer type.");
			return false;
		}
		int64_t stack_req=std::max(rt.stack_requirement,2L);
        VarInfo const* p = program.find_var_info(x.name);//do we need to call this in global scope
        if (p == 0)
        {
        	p=program.find_var_info_global(x.name);
        	if (p==0)
        	{
        		error_handler(x, "Undeclared variable: " + x.name);
        		return false;
        	}
        	if (evaluate_expressions_in_global_scope&&(!ctx.local_context)&&(p->initialized)
        			&&(rt.success==value_calculated) )
        	{
        		if (size_t(rt.val)>=p->size)
        		{
            		error_handler(x, "Index beyond the end of array: " + x.name);
            		return false;
        		}
        	    return {value_calculated,p->tid,stack_req,1,program.data()[p->location+rt.val]};
        	}
        	if (array_index_bounds_check)
        		program.op(op_0_less_assert,p->size);
        	program.op(op_int, p->location);
        	program.op(op_add_s_is_is);
        	program.op(op_deref_ds);
        	return {true,p->tid,stack_req,1};
        }
    	if (array_index_bounds_check)
    		program.op(op_0_less_assert,p->size);
        program.op(op_int, p->location);
        program.op(op_add_s_is_is);
        program.op(op_deref);
        return {true,p->tid,stack_req,1};

    }

    const client::ast::expression* collapse(const client::ast::expression* pexpr)
    {

       	while (pexpr->rest.empty())
       	{

       		const client::x3::forward_ast<client::ast::expression> *pf=boost::get<client::x3::forward_ast<client::ast::expression> >(&pexpr->first.get());

       		if (!pf)
       			break;

       		//std::cout << "moved expr," << std::flush;
       		pexpr=pf->get_pointer();
       	}

       	return pexpr;
    }

    compiler::result_type compiler::operator()(ast::variable const& lhs,ast::optoken opt,ast::variable const& rhs) const
    {

    	VarInfo const* p = program.find_var_info(lhs.name);
		if (p == 0)
		{
			error_handler(lhs, "Undeclared variable: " + lhs.name);
			return false;
		}
		VarInfo const* p2 = program.find_var_info(rhs.name);
		if (p2 == 0)
		{
			error_handler(lhs, "Undeclared variable: " + rhs.name);
			return false;
		}

    	bool bothd=((p->tid==DOUBLE)&&(p2->tid==DOUBLE));
		bool bothi=((p->tid==INT64)&&(p2->tid==INT64));

		//TODO: Mixed mode double+int , etc

		if ((!bothd)&&(!bothi))
		{
		    error_handler(lhs, "Operands of mixed type not supported.");
		    return false;
		}

	     switch (opt)
	     {
	            case ast::op_plus:
	            	program.op(bothd ? op_add_s_dv_dv : op_add_s_iv_iv);
	            	break;
	            case ast::op_minus:
	            	program.op(bothd ? op_sub_s_dv_dv : op_sub_s_iv_iv);
	            	break;
	            case ast::op_times:
	            	program.op(bothd ? op_mul_s_dv_dv : op_mul_s_iv_iv);
	            	break;
	            case ast::op_divide:
	            	program.op(bothd ? op_div_s_dv_dv : op_div_s_iv_iv);
	            	break;
	            case ast::op_equal:
	            	program.op(bothd ? op_eq_s_dv_dv : op_eq_s_iv_iv);
	            	break;
	            case ast::op_not_equal:
	            	program.op(bothd ? op_neq_s_dv_dv : op_neq_s_iv_iv);
				    break;
	            case ast::op_less:
	            	program.op(bothd ? op_lt_s_dv_dv : op_lt_s_iv_iv);
	            	break;
	            case ast::op_less_equal:
	            	program.op(bothd ? op_lte_s_dv_dv : op_lte_s_iv_iv);
	            	break;
	            case ast::op_greater:
	            	program.op(bothd ? op_gt_s_dv_dv : op_gt_s_iv_iv);
	            	break;
	            case ast::op_greater_equal:
	            	program.op(bothd ? op_gte_s_dv_dv : op_gte_s_iv_iv);
	            	break;

	            case ast::op_and: program.op(op_and); break;
	            case ast::op_or: program.op(op_or); break;
	            default: BOOST_ASSERT(0);
	            	error_handler(lhs, "Unknown binary operation. val=" + boost::lexical_cast<std::string>(opt));
	            	return false;
	    }


        program.op(p->location);
        program.op(p2->location);

        TypeID outtype=bothd ? DOUBLE : INT64;
        return {true,outtype,1,1};

    }
    compiler::result_type compiler::operator()(ast::cast_expression const& x) const
    {
    	compiler::result_type rt=(*this)(x.expr);
    	if (!rt)
    		return rt;

    	if ((rt.type==DOUBLE)&& ((x.type=="int")||(x.type=="int64")))
    	{
    		program.op(op_cast_double2int);
    		rt.type=INT64;
    	}

    	if ((rt.type==INT64)&& (x.type=="double"))
        {
        	program.op(op_cast_int2double);
        	rt.type=DOUBLE;
        }

    	return rt;

    }

    compiler::result_type compiler::operator()(ast::expression_list const& xl) const
    {
    	return false;
    }

    compiler::result_type compiler::operator()(ast::expression const& x_) const
    {
    	const ast::expression&  x=*collapse(&x_);

    	if (!x.rest.empty())
    	{
			auto it=x.rest.begin();
			++it;
			if (it==x.rest.end())//one element - special case , quite common, lets optimize it with shorter instructions

			{
				ast::operation  op=x.rest.back();

				//(lhs || rhs),   if lhs is nonzero, evaluation of rhs is skipped and stack should be preserved

				if (op.operator_==client::ast::op_or)
				{
					//std::cout << "binary or" << std::endl;
					compiler::result_type rt=boost::apply_visitor(*this, x.first);
					if (!rt)
					{
						error_handler(x_,"first operand in logical 'or' expression failed.");
						return false;
					}
					program.op(op_jump_and_keep_stack_if, 0);                      // we shall fill this (0) in later
					//program.op(op_jump_if, 0);
					std::size_t skip = program.size()-1;            // mark its position
					result_type rhs_type=boost::apply_visitor(*this, op.operand_);
					if (!rhs_type)
					{
						error_handler(op,"second operand in logical 'or' expression failed.");
					    return false;
					}
					program[skip] = int(program.size());       // now we know where to jump to (after the if branch)
					int64_t req=std::max(rt.stack_requirement,rhs_type.stack_requirement);
					return {true,INT64,req,1};

				}

				if (op.operator_==client::ast::op_and)
				{
					//std::cout << "binary or" << std::endl;
					compiler::result_type rt=boost::apply_visitor(*this, x.first);
					if (!rt)
					{
						error_handler(x_," first operand in logical 'and' expression.");
						return false;
					}
					program.op(op_jump_and_keep_stack_if_not, 0);                      // we shall fill this (0) in later
					std::size_t skip = program.size()-1;            // mark its position
					//we should remove top of stack ?
					result_type rhs_type=boost::apply_visitor(*this, op.operand_);
					if (!rhs_type)
					{
						error_handler(x_," second operand in logical 'and' expression.");
					    return false;
					}
					program[skip] = int(program.size());       // now we know where to jump to (after the if branch)
					int64_t req=std::max(rt.stack_requirement,rhs_type.stack_requirement);
					return {true,INT64,req,1};

				}

				if (ctx.local_context)// we don't yet have optimized (macro)instructions working with data segment,
				{// try to optimize  just for the local context inside a function for variables on stack
					const ast::variable *lhs=boost::get<ast::variable>(&x.first.get());
					if (lhs)
					{
						//std::cout << "var=" << lhs->name << std::endl;


					  //const ast::expression&  x2=*collapse(&x_);

					  const ast::variable *rhs=boost::get<ast::variable>(&op.operand_);
					  if (rhs)
						  return (*this)(*lhs,op.operator_,*rhs);
					}
					const client::x3::forward_ast<client::ast::expression> *pf=boost::get<client::x3::forward_ast<client::ast::expression> >(&x.first.get());
					if (pf)
					{
						if (pf->get_pointer()->rest.empty())
						{
						  const ast::variable *lhs=boost::get<ast::variable>(&pf->get_pointer()->first.get());
						  if (lhs)
						  {
							  //std::cout << "var2_lhs=" << lhs->name << std::endl;
							  const client::x3::forward_ast<client::ast::expression> *pf2=boost::get<client::x3::forward_ast<client::ast::expression> >(&op.operand_);
							  if (pf2)
							  {
									if (pf2->get_pointer()->rest.empty())
									{
										const ast::variable *rhs=boost::get<ast::variable>(&pf2->get_pointer()->first.get());
										//std::cout << "var2_rhs=" << rhs->name << std::endl;
										if (rhs)
											return (*this)(*lhs,op.operator_,*rhs);

									}
							  }
						  }

						}
					}
				}
			}
    	}




    	compiler::result_type rt=boost::apply_visitor(*this, x.first);
    	if (!rt)
            return false;
        //bool isdouble=(boost::get<double>(&x.first)!=nullptr);
        for (auto it=  x.rest.begin();it!=x.rest.end();++it)
        {
            rt=(*this)(*it,rt);
            if (!rt)
            {
            	error_handler(x_,"Compilation of an operation in expression failed.");
                return false;
            }
        }
        return rt;
    }


    compiler::result_type compiler::operator()(ast::function_call const& x) const
    {



    	compiler::result_type rt=false;

    	std::vector<TypeID> typesl;
    	std::vector<TypeID> typesl1;
    	//std::vector<compiler::result_type> args;
    	std::vector<int64_t> stack;

/* What to do when some expressions into function can be evaluated and some arguments can not be evaluated during compilation?
 * It happens  when either some modification(and initiation) VM code for the variable in the expression has already been emitted
 * On the first argument expression which cannot be evaluated we need to emit/insert VM instructions for the other arguments
 * that will put evaluated values onto stack in the order before the one unevaluated argument.
 * Alternative solution: always emit code and only if all the arguments expressions are also evaluated in global contexts,
 *  then (either call the function directly on those evaluated arguments or  execute VM from point with those expressions on stack)
 *  and remove that code after.
 *
 *
 */
    	bool are_all_args_evaluated=true;
    	//std::size_t args_start=program.size();

    	int64_t assert_size_location=-1;
    	if (assert_stack_size_before_variadic_function_call&&(x.name!="return")&&((!evaluate_expressions_in_global_scope)||ctx.local_context))
    	{
    		program.op(op_nop,op_nop);//op_stk_assert
    		assert_size_location=program.size()-1;
    	}

    	//int64_t st_req=0;
    	//int64_t cnt=0;
    	int64_t stack_used=0;//how much data was added to stack
    	int64_t stack_required=0;//how much of stack was used (can be bigger than stack_used)

		for (auto it1=  x.arguments.begin();it1!=x.arguments.end();++it1)
		{
    		rt=boost::apply_visitor(*this, *it1);
			if (!rt)
			{
				error_handler(x,"Failed argument calculation.");
				return false;
			}

            stack_required=std::max(stack_required,stack_used+rt.stack_requirement);
            stack_used+=rt.sp_diff;

			//st_req=std::max(st_req,cnt++ +rt.stack_requirement);

			if (rt.success!=value_calculated)
				are_all_args_evaluated=false;

			typesl.push_back(rt.type);

			if (evaluate_expressions_in_global_scope&&(!ctx.local_context)&&are_all_args_evaluated)
				stack.push_back(rt.val);
		 }



       	 if (x.name=="return")
       	 {
       		 if (x.arguments.size()==0)
       		 {
       			program.op(op_return_void);
       		 }

       		 if (x.arguments.size()!=1)
       		 {
       			 error_handler(x, "You can only return exactly one value in function " + x.name );
       		     return false;
       		 }

       		 int64_t return_stack_data_cnt=((x.arguments.size()==0))  ? 0 : 1;
 		     stack_required=std::max(stack_required,return_stack_data_cnt);
 		     stack_used=std::max(stack_used,return_stack_data_cnt);

       		 program.op(op_return_int);
       		 return {true,rt.type,stack_required,stack_used};

       	 }
/*
    	 if (x.name=="tsc")
    	 {
    		 if (!x.arguments.empty())
    		 {
    		     error_handler(x, "tsc -timestamp counter has no arguments");
    		     return false;
    		 }
    		 program.op(op_tsc);
    		 return {true,INT64};
    	 }*/
/*
    	 if ((x.name=="print")&&(!typesl.empty()))//variadic print, saving types list to program
    	 {
    		 program.op(op_print,typesl.size());

    		 for (auto& val:typesl)
    			 program.op(val);

    		 return {true,int64_e};
    	 }*/


    	 auto it0=program.functions.find({x.name,typesl});
    	 if (it0!=program.functions.end())
    	 {


    			//also reserved places for cnt,caller and caller_frame_pointer
    	     stack_required=std::max(stack_required,stack_used+3);
    	     stack_used+=3;

    		 if (evaluate_expressions_in_global_scope&&(!ctx.local_context)&&are_all_args_evaluated)
    		 {
    			 program.optimize();
    			 it0=program.functions.find({x.name,typesl});

    			 stack.resize(4096);
    			 int64_t ret =vm.execute1(program().begin()+it0->second.start,stack.begin());
    			 return {value_calculated,it0->second.return_type,stack_required,stack_used,ret};

    		 }

/*
        	 if (assert_size_location>0)//user defined functions are not yet variadic
        	 {
        		 program[assert_size_location-1]=op_stk_assert;
        		 program[assert_size_location]=stack_required+it0->second.stack_requirement;
        	 }
*/
    		 program.op(op_ucall,typesl.size(),it0->second.start);

    		 return {true,it0->second.return_type,stack_required,stack_used};
    	 }


    	 auto it=global_functions.find({x.name,typesl});

    	 if ((it==global_functions.end())&&(!typesl.empty()))//fallback trying variadic functions  func(T0,...)
    	 {
    		 typesl1.clear();
    		 it=global_functions.find({x.name,{}});
			 if (it==global_functions.end())
			 {
				 auto is_same_type=[&](int i,TypeID tp) ->bool {
				 				    		 for (unsigned int j=i;j<typesl.size();j++)
				 				    		 {
				 				    			if (typesl[j]!=typesl1.back())
				 				    					return false;
				 				    		 }
				 				    		 return true;
				 				      };

				 for(uint i=0;i<typesl.size();i++)
				 {
					 typesl1.push_back(typesl[i]);
					 it=global_functions.find({x.name,typesl1});
					 if (it!=global_functions.end())
					 {
						 if ((it->first.fsp==varargs)||(it->first.fsp==pass_types))
							 break;

						 if (it->first.fsp==varcount)
							 //now check if all remaining variadic arguments have the same type typesl1.back()
						 {
							 if (is_same_type(i,typesl.back()))
								 break;

						 }
					 }
				 }
			 }

    		 if (it!=global_functions.end())
    		 {
    			 if (it->first.fsp==fixed_signature)
    				 it=global_functions.end();
    		 }
    	 }

    	 if (it==global_functions.end())//as a fallback, lets try with name only
    	 {
    		 it=global_functions.find({x.name,{}});
    		 if (it!=global_functions.end())
    		 {
    		    if (it->first.fsp!=name_only)
    		    	it=global_functions.end();
    		 }

    	 }

    	 if (it==global_functions.end())
    	 {
    		 func_signature fs{x.name,typesl};
    		 std::string out="Error: Unknown function with the signature " +  fs.signature();

    		 out += program.signatures_with_name(x.name);
    		 out += global_signatures_with_name(x.name);

    		 error_handler(x,out);
    		 return false;
    	 }

		 int64_t return_stack_data_cnt=(it->first.return_type==VOID)  ? 0 : 1;
		 stack_required=std::max(stack_required,return_stack_data_cnt);
		 stack_used=std::max(stack_used,return_stack_data_cnt);

    	 if (typesl.empty())
    	 {
    		 if (evaluate_expressions_in_global_scope&&(!ctx.local_context))
    		 {
    			 if(it->first.return_type==VOID) {
    				 if (void_func_glob_init)
    					 ((void(*)())it->second.p)();//do we need to call it if has no side effects on data segment?
    				 return {value_calculated,it->first.return_type,stack_required,stack_used,0};//
    			 }
    			 else {
    				 int64_t ret=((int64_t(*)())it->second.p)();
    				 return {value_calculated,it->first.return_type,stack_required,stack_used,ret};
    			 }
    		 }

    		 program.op((it->first.return_type==VOID)  ? op_callv0 : op_call0,(int64_t)(it->second.p));
    	 } else
    		 //some arguments passed
    	 {

    		int64_t ret=0;
    		if (it->first.fsp==pass_types)
    		{

    			int n=it->first.arg_types.size();//fixed_prefix size
    			int variadic_cnt=typesl.size()-n;

    			if (evaluate_expressions_in_global_scope&&(!ctx.local_context))
    			{
    				std::vector<int64_t> typelist;
					for (uint i=n;i<typesl.size();i++)//we record only types after the fixed signature prefix,
						typelist.push_back((int64_t)&typesl[i].type_info());

    				if (it->first.return_type==VOID)
    				{
    					if (void_func_glob_init)
    					{
         					((void (*)(int64_t,int64_t*,const int64_t*))it->second.p)(variadic_cnt,&stack[0],variadic_cnt ? (int64_t*)&typelist[0]: 0);

    					}
    					return {value_calculated,it->first.return_type,stack_required,stack_used,0};
    				}

    				ret=((int64_t (*)(int64_t,int64_t*,const int64_t*))it->second.p)(variadic_cnt,&stack[0],variadic_cnt ? (int64_t*)&typelist[0]: 0);
    				return {value_calculated,it->first.return_type,stack_required,stack_used,ret};
    			}

				 if ((assert_size_location>0)&&variadic_cnt)
				 {
					 program[assert_size_location-1]=op_stk_assert;
					 program[assert_size_location]=stack_required;
				 }
    			program.op((it->first.return_type==VOID) ? op_callv_var : op_callvar,typesl.size(),typesl.size()-n);
    			//we record both the count of all arguments because we need to adjust stack pointer later and
    			// the count of variadic arguments only which we record into program here
    			// so we can quickly skip this call instruction in program
    			//size of op_callvar instruction is  instr+cnt1+cnt2+(types list cnt2 size)+addr  4+variadic size elements
    			for (uint i=n;i<typesl.size();i++)//we record only types after the fixed signature prefix,
    			    program.op((int64_t)&typesl[i].type_info());
    			program.op((int64_t)it->second.p);

    		} else//(it->first.fsp!=pass_types)
    		{
    			bool variadic=((it->first.fsp==varargs)||(it->first.fsp==varcount));

    			int n=it->first.arg_types.size();//fixed_prefix size
    			int variadic_cnt=typesl.size()-n;

    			byte_code bc=((it->first.fsp==varargs)||(it->first.fsp==varcount)) ? op_callvararr : op_call;
    		    byte_code bc_void=((it->first.fsp==varargs)||(it->first.fsp==varcount)) ? op_callv_var_arr : op_callv;
    		    byte_code bc0=(it->first.return_type==VOID) ? bc_void : bc;


    			if (evaluate_expressions_in_global_scope&&(!ctx.local_context))
    			{

    				if (bc0==op_call)
    					ret=((int64_t (*)(int64_t*))it->second.p)(&stack[0]);
    				else if (bc0==op_callv)
    				{
    					if (void_func_glob_init)
					      ((int64_t (*)(int64_t*))it->second.p)(&stack[0]);
    				}
    				else
    					return false;

    				return {value_calculated,it->first.return_type,stack_required,stack_used,ret};

    			}
				 if ((assert_size_location>0)&&variadic_cnt&&variadic)
				 {
					 program[assert_size_location-1]=op_stk_assert;
					 program[assert_size_location]=stack_required;
				 }

    			program.op(bc0,typesl.size(),(int64_t)it->second.p);
    		}
    	 }
    	 return {true,it->first.return_type,stack_required,stack_used};
    }

    compiler::result_type compiler::operator()(ast::assignment_list const& x) const
    {
    	auto sz=x.rhs.size();
    	if (sz<1)
    	{
    		error_handler(x, "Compilation failed. RHS of init expression list has no element. ");
    		return false;
    	}

        byte_code instr=ctx.local_context ? op_store : op_store_ds;

        VarInfo * p = ctx.local_context
                		? program.find_var_info(x.lhs.name) :
                		 program.find_var_info_global(x.lhs.name);

        if ((!p)&&ctx.local_context)//if look up in local context failed lets try the global vars
        {
        	p = program.find_var_info_global(x.lhs.name);
        	if (p)
        		instr=op_store_ds;
        }
        if (p)
        {
        	if (sz>p->size)
        	{
        		error_handler(x, "RHS expression list has more expressions than the variable on LHS.");
        		return false;
        	}
        }
        bool auto_declared=false;
        if ((!p)&&auto_declare_vars_on_first_assignment)
        {
        	p=ctx.local_context ? &program.add_var(x.lhs.name,VOID,sz) : &program.add_var_global(x.lhs.name,VOID,sz);
        	auto_declared=true;
        }

        if (!p)
        {
        	error_handler(x.lhs, "Undeclared variable: " + x.lhs.name);
      		return false;
        }

		VarInfo& vi=*p;

		if (evaluate_expressions_in_global_scope&&(!ctx.local_context))
			program.data().resize(vi.location+sz);


		int cnt=0;
		vi.initialized=true;//if true , all elements of array are initialized/assigned; false otherwise (if at least one element is not initialized)

		compiler::result_type rt0;
		int64_t stack_used=0;//how much data was added to stack
		int64_t stack_required=0;//how much of stack was used (can be bigger than stack_used)

		for (const auto& val:x.rhs)
		{
			compiler::result_type rt=(*this)(val);
			if (!rt)
			{
				//error_handler(val, "Compiling of rhs expression failed in assignment. ");
				return false;
			}
            stack_required=std::max(stack_required,stack_used+rt.stack_requirement);
            stack_used+=rt.sp_diff;

			if (cnt==0)
			{
				if (!auto_declared)
				{
					if (rt.type!=vi.tid)
					{
						error_handler(x, "Type of the first element of the expression list does not match the type of first element of LHS variable.");
						return false;
					}
				}
				rt0=rt;
				vi.tid=rt.type;
			}
			else if ((rt.type!=vi.tid)&&same_types_in_brace_init_expression_list)//is bracelist arrays or tuples?
			{
				//error_handler(val, "Different types in array elements. Tuples are disabled? ");
				return false;
			}
			if (evaluate_expressions_in_global_scope&&(!ctx.local_context)&&(rt.success==value_calculated))
			{
				program.data()[vi.location+cnt++]=rt.val;
			} else
			{
			   program.op(instr, vi.location+cnt++);
			   vi.initialized=false;
			}
		};

		rt0.stack_requirement=stack_required;
		rt0.sp_diff=stack_used;

		return rt0;

    }

    compiler::result_type compiler::operator()(ast::array_element_assignment const& x) const
    {
        VarInfo * p = ctx.local_context
                		? program.find_var_info(x.lhs.name) :
                		 program.find_var_info_global(x.lhs.name);

        if(p&&glob_var_decl_only&&(!ctx.local_context))
        {
          	error_handler(x.lhs, "Non-initiating assignments disabled in global context. ");
          	return false;
        }




    	compiler::result_type rt=(*this)(x.rhs);
    	if (!rt)
    	{
    		error_handler(x.lhs, "Compiling of rhs expression failed in assignment. ");
            return false;
    	}


        byte_code instr=ctx.local_context ? op_deref_assign : op_deref_ds_assign;

        if ((!p)&&ctx.local_context)//if look up in local context failed lets try the global vars
        {
        	p = program.find_var_info_global(x.lhs.name);
        	if (p)
        		instr=op_deref_ds_assign;
        }


        if (!p)
		{
			  error_handler(x.lhs, "Undeclared variable: " + x.lhs.name);
			  return false;
		}


		if (rt.type!=p->tid)
		{
			error_handler(x.lhs, std::string("rhs type '") + rt.type.pretty_name() + "' does NOT match lhs type '"
					+p->tid.pretty_name()+"' for variable " + x.lhs.name );
			return false;
		}

       	int64_t stack_used=rt.sp_diff;//how much data was added to stack
        int64_t stack_required=rt.stack_requirement;//how much of stack was used (can be bigger than stack_used)

		compiler::result_type rt_index=(*this)(x.lhs_index);
		if (!rt_index)
		{
			error_handler(x.lhs, "Compiling of lhs index failed. ");
			return false;
		}

		stack_required=std::max(stack_required,stack_used+rt_index.stack_requirement);
	    stack_used+=rt.sp_diff;

	    if (evaluate_expressions_in_global_scope&&(!ctx.local_context)&&(rt.success==value_calculated)&&
	    				(instr==op_deref_ds_assign))
	    {
		   program.data()[p->location]=rt.val;
		   p->initialized=true;
	    }

		if (array_index_bounds_check)
		   program.op(op_0_less_assert,p->size);

		program.op(op_int, p->location);
		program.op(op_add_s_is_is);
		program.op(instr);
		return {true,p->tid,stack_required,1};

    }

    compiler::result_type compiler::operator()(ast::assignment const& x) const
    {

        VarInfo * p = ctx.local_context
                		? program.find_var_info(x.lhs.name) :
                		 program.find_var_info_global(x.lhs.name);

        if(p&&glob_var_decl_only&&(!ctx.local_context))
        {
          	error_handler(x.lhs, "Non-initiating assignments disabled in global context. ");
          	return false;
        }

    	compiler::result_type rt=(*this)(x.rhs);
    	if (!rt)
    	{
    		error_handler(x.lhs, "Compiling of rhs expression failed in assignment. ");
            return false;
    	}

        byte_code instr=ctx.local_context ? op_store : op_store_ds;

        if ((!p)&&ctx.local_context)//if look up in local context failed lets try the global vars
        {
        	p = program.find_var_info_global(x.lhs.name);
        	if (p)
        		instr=op_store_ds;
        }

        if ((!p)&&auto_declare_vars_on_first_assignment)
        {

			if (rt.type==VOID)
			{
				error_handler(x.lhs, "You cannot assign expression/function returning no type 'void' to a variable. ");
				return false;
			}

			if (ctx.local_context)
			{
			  program.add_var(x.lhs.name,rt.type);
			  auto ploc=program.find_var_fp_offset(x.lhs.name);
			  program.op(op_store,*ploc);
			}
			else
			{
			  VarInfo& vi=program.add_var_global(x.lhs.name,rt.type);
			  if (evaluate_expressions_in_global_scope&&(rt.success==value_calculated))
			  {

					program.data().resize(vi.location+1);
					program.data()[vi.location]=rt.val;
					vi.initialized=true;
					return rt;
			  }

			  program.op(op_store_ds, vi.location);
			}

			return rt;
        }

        if (!p)
        {
			  error_handler(x.lhs, "Undeclared variable: " + x.lhs.name);
			  return false;
        }
        if (rt.type!=p->tid)
        {
        	error_handler(x.lhs, std::string("rhs type '") + rt.type.pretty_name() + "' does NOT match lhs type '"
        			+p->tid.pretty_name()+"' for variable " + x.lhs.name );
        	return false;
        }

	    if (evaluate_expressions_in_global_scope&&(!ctx.local_context)&&(rt.success==value_calculated)&&
	    		(instr==op_store_ds))
	    {
		   program.data()[p->location]=rt.val;
		   p->initialized=true;
	    }
		else
           program.op(instr, p->location);

        return rt;
    }

    compiler::result_type compiler::operator()(ast::variable_declaration const& x) const
    {
        int64_t const* p = ctx.local_context ? program.find_var_fp_offset(x.assign.lhs.name) :
        								program.find_var_global(x.assign.lhs.name);
        if (p != 0)
        {
            error_handler(x.assign.lhs, "Duplicate variable: " + x.assign.lhs.name);
            return false;
        }
        compiler::result_type r = (*this)(x.assign.rhs);
        if (r) // don't add the variable if the RHS fails
        {
        	if (ctx.local_context)
        	{
        		program.add_var(x.assign.lhs.name,r.type);
        		program.op(op_store, *program.find_var_fp_offset(x.assign.lhs.name));
        	} else
        	{
        		program.add_var_global(x.assign.lhs.name,r.type);
        	    program.op(op_store_ds, *program.find_var_global(x.assign.lhs.name));
        	}
        }
        return r;
    }

    compiler::result_type compiler::operator()(ast::statement const& x) const
    {
        return boost::apply_visitor(*this, x);
    }

    compiler::result_type compiler::operator()(ast::statement_list const& x) const
    {
    	compiler::result_type rt;
    	int64_t stack_used=0;//how much data was added to stack
    	int64_t stack_required=0;//how much of stack was used (can be bigger than stack_used)

        for (auto const& s : x)
        {
            rt=(*this)(s);
            if (!rt)
            {
            	error_handler(s, "Compiling of statement failed. ");
                return false;
            }
            stack_required=std::max(stack_required,stack_used+rt.stack_requirement);
            stack_used+=rt.sp_diff;
        }
        rt.stack_requirement=stack_required;
        rt.sp_diff=stack_used;
        return rt;//returns the type of the last statement
    }

    compiler::result_type compiler::operator()(ast::if_statement const& x) const
    {
    	compiler::result_type rt_if=(*this)(x.condition);
        if (!rt_if)
        {
        	error_handler(x, "Compiling of expression for 'if' failed. ");
            return false;
        }
        int64_t st_req=rt_if.stack_requirement;
        int64_t sp_diff=rt_if.sp_diff;

        program.op(op_jump_if_not, 0);                      // we shall fill this (0) in later
        std::size_t skip = program.size()-1;            // mark its position
        compiler::result_type rt_then=(*this)(x.then);
        if (!rt_then)
        {
        	error_handler(x, "Compiling of body statement for 'if' failed. ");
            return false;
        }

        st_req=std::max(st_req,sp_diff+rt_then.stack_requirement);
        sp_diff+=rt_then.sp_diff;

        program[skip] = int(program.size());       // now we know where to jump to (after the if branch)

        if (x.else_)                                    // We got an alse
        {
            program[skip] += 2;                         // adjust for the "else" jump
            program.op(op_jump, 0);                     // we shall fill this (0) in later
            std::size_t exit = program.size()-1;        // mark its position
            compiler::result_type rt_else=(*this)(*x.else_);
            if (!rt_else)
            {
            	error_handler(x, "Compiling of 'else' part statement for 'if' failed. ");
                return false;
            }
            st_req=std::max(st_req,sp_diff+rt_then.stack_requirement);
            sp_diff+=rt_then.sp_diff;
            program[exit] = int(program.size());   // now we know where to jump to (after the else branch)
        }

        return {true,VOID,st_req,sp_diff};
    }

    compiler::result_type compiler::operator()(ast::while_statement const& x) const
    {
        std::size_t loop = program.size();              // mark our position

        compiler::result_type rt_cond=(*this)(x.condition);
        if (!rt_cond)
        {
        	error_handler(x, "Compiling of  expression for 'while' failed. ");
            return false;
        }
        program.op(op_jump_if_not, 0);                      // we shall fill this (0) in later
        std::size_t exit = program.size()-1;            // mark its position
        compiler::result_type rt_body=(*this)(x.body);
        if (!rt_body)
        {
        	error_handler(x, "Compiling of body for 'while' failed. ");
            return false;
        }

        program.op(op_jump,int(loop));         // loop back

        program[exit] = int(program.size());       // now we know where to jump to (to exit the loop)

        int64_t st_req=std::max(rt_cond.stack_requirement,rt_cond.sp_diff+rt_body.stack_requirement);

        return {true,VOID,st_req,rt_cond.sp_diff+rt_body.sp_diff};
    }

    compiler::result_type compiler::start(ast::statement_list const& x) const
    {
        program.clear();
        // op_stk_adj 0 for now. we'll know how many variables we'll have later
        program.op(stack_space_check_at_func_start ? op_stk_adj_chk : op_stk_adj, 0);

        if (!(*this)(x))
        {
            program.clear();
           // error_handler(x, "Compiling of list of statements failed. ");
            return false;
        }
        program[1] = int(program.nvars());              // now store the actual number of local automated variables
        program.op(op_exit);
        return true;
    }

    class IsReturnFunction : public boost::static_visitor<bool>
    {
    public:

    	template <class T>
        bool operator()(const T& val) const
        {
            return false;
        }

        bool operator()(const ast::function_call & fc) const
        {
        	return (fc.name=="return");
        }

    };

    compiler::result_type compiler::operator()(const ast::function_def_statement& funcdef) const
    {

    	//int n=funcdef.name.name.size();
    	//std::size_t here=program.size();
//    	std::cout << "func:" << funcdef.name  << std::endl;
    	program.op(op_jump,0);//skip function

    	std::size_t function_start=program.size();
    	program.op(stack_space_check_at_func_start ? op_stk_adj_chk : op_stk_adj, 0);

    	func_signature fs{funcdef.name};

    	int64_t argc=funcdef.arguments.size();

    	//calling convention, stack layout:
    	//pc = pc_beg+stack_ptr[-1];
    	//  N arguments
    	//		arg0,   arg1.. ,arg2,   .argN-1, argCount, caller_addr, caller_FP, local					    ret_val
    	//		-(N+2)			          -3		-2	    -1			0 (FP)		1     2                      (sp-1)   (sp)
    	//              new_sp


    	program.init_fp_offset(-(argc+2));

    	for (const auto& val:funcdef.arguments)
    	{
    		auto tp=getTypeID(val.type1);
    		if (tp==NO_TYPE)
    		{
    			error_handler(val, "Unknown type for variable: " + val.name);
    			return {false,VOID};
    		}
    		fs.arg_types.push_back(tp);
    		program.add_var(val.name,tp);
    	}


    	auto tp=funcdef.return_type.empty() ? VOID : getTypeID(funcdef.return_type);
    	if (tp==NO_TYPE)
    	{
    		error_handler(funcdef, "Unknown return type : " + funcdef.return_type);
        	return {false,VOID};
    	}

    	program::func_desc fd;
    	fd.start=function_start;
    	fd.return_type=tp;
    	program.functions[fs]=fd;//start of function
    	program.functions_reverse[function_start]=fs;

    	program.add_var("__argc__",INT64);//-2 offset
    	program.add_var("__caller__",VM_ADDR);//-1 offset
    	program.add_var("__caller_frame_ptr__",ST_ADDR); //0 offset

    	int64_t location_assert_stack_size=-1;
    	if (assert_stack_size_at_function_start)
    	{
    		program.op(op_stk_assert,0);
    		location_assert_stack_size=program.size()-1;
    	}

    	ctx.local_context=true;
    	result_type rt=(*this)(funcdef.body);
    	ctx.local_context=false;

    	if (funcdef.body.empty())
    	{
    	   if ((funcdef.return_type!="")&&(funcdef.return_type!="void"))
    	   {
    		   error_handler(funcdef, "You need to specify return(expr); as a last statement in the body of function returning non-void type: " + funcdef.return_type);
    		   return false;
    	   }
   // 	   program.op(op_int,0);
    	   program.op(op_return_void);
    	}
    	else
    	{
    		const ast::statement& st=funcdef.body.back();
    		const auto& st2=st.get();

    		bool ret=boost::apply_visitor(IsReturnFunction(),st2);
    		if ((!ret)&&((funcdef.return_type!="")&&(funcdef.return_type!="void")))
    		{
    			error_handler(funcdef, "You need to specify return(expr); as a last statement in the body of function returning non-void type: " + funcdef.return_type);
    	    	return false;
    		}
    		if (ret)
    		{
    			if (rt.type!=fd.return_type)
    			{
    				error_handler(funcdef, std::string("return expression type '") +rt.type.pretty_name()+ "' does not match declared function type '" + funcdef.return_type+"'");
    				return false;
    			}

    		} else {//this comes when func  declared as void returning


    			program.op(op_return_void);
    		}
    	}

    	program[function_start-1]=program.size();
    	program[function_start+1]=program.fp_offset()-1;//int(program.nvars());  how much space is allocated to locals AFTER the FRAME_POINTER (with index 0)
    	program.functions[fs].locals=program.variables;
    	program.functions[fs].stack_requirement=rt.stack_requirement;
    	program.variables.clear();

      	if (assert_stack_size_at_function_start)
        {
        	program[location_assert_stack_size]=rt.stack_requirement;
        }

    	return {true,fd.return_type,rt.stack_requirement};
    }

    compiler::result_type compiler::operator()(ast::function_call2 const& x) const
    {
    	error_handler(x, "ast::function_call2 not supported");
    	return false;
    }
}}
