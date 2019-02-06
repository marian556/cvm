/*=============================================================================
    Copyright (c) 2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#include <iostream>
#include <set>
#include <boost/lexical_cast.hpp>
#include "vm.hpp"
#include "program.h"
#include "types.h"

namespace client { namespace code_gen
{
	//const char * typenames[]={"int64","double"};

    void program::op(int64_t a)
    {
        code.push_back(a);
    }

    void program::op(int64_t a, int64_t b)
    {
        code.push_back(a);
        code.push_back(b);
    }

    void program::op(int64_t a, int64_t b, int64_t c)
    {
        code.push_back(a);
        code.push_back(b);
        code.push_back(c);
    }

    int64_t const* program::find_var_fp_offset(std::string const& name) const
    {
        auto i = variables.find(name);
        if (i == variables.end())
            return 0;
        return &i->second.location;
    }

    VarInfo * program::find_var_info(std::string const& name)
    {
           auto info = variables.find(name);
           if (info == variables.end())
               return 0;
           return &info->second;
    }
/*
    std::size_t program::add_var(std::string const& name,TypeID tid)
    {
        std::size_t n = variables.size();
        variables[name] = VarInfo{int(n),tid,false};
        return n;
    }*/
    VarInfo& program::add_var(std::string const& name,TypeID tid,size_t size,bool init)
    {

          auto it_bool=variables.emplace(name,VarInfo{int(current_stack_allocation_frame_pointer_offset),size,tid,init});
          current_stack_allocation_frame_pointer_offset += size;
          return it_bool.first->second;
    }

    int64_t const * program::find_var_global(std::string const& name) const
    {
        auto i = variables_global.find(name);
        if (i == variables_global.end())
            return 0;
        return &i->second.location;
    }

    VarInfo * program::find_var_info_global(std::string const& name)
    {
           auto i = variables_global.find(name);
           if (i == variables_global.end())
               return 0;
           return &i->second;
    }

    VarInfo& program::add_var_global(std::string const& name,TypeID tid,size_t size,bool init)
    {
  	   // static std::size_t last=0;
  	   // if (variables_global.empty())
  		//  last=0;
        //std::size_t n = last;client
        auto it_bool=variables_global.emplace(name,VarInfo{(int64_t)allocated_global_data_size,size,tid,init});
        allocated_global_data_size += size;
        return it_bool.first->second;


        //std::size_t n = variables_global.size();
        //auto it_bool=variables_global.emplace(name,VarInfo{int(n),tid,init});
        //return it_bool.first->second;
    }
/*
    std::string int64_t2str8_(const int64_t *pval)
    {
 	   std::string out;
 	   out.resize(9);
 	   *(int64_t*)&out[0]=*pval;
 	   char *pch=&out[0];
 	   while (*pch)
 		   pch++;
 	   int n=pch-&out[0];
 	   out.resize(n);
 	   return out;
    }
*/
    void program::print_variables(std::vector<int64_t> const& stack) const
    {
        for (auto const& p : variables)
        {
            std::cout << p.second.tid.pretty_name() << "  " << p.first << "=";
            if (p.second.tid==DOUBLE)
            	std::cout << *(double*)(void*)&stack[p.second.location];
            else if (p.second.tid==INT64)
            	std::cout << stack[p.second.location];
            else if (p.second.tid==STR8)
            	std::cout << str8{stack[p.second.location]}.toString();
            else
            	std::cout << "unknown" ;

            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    void program::print_variables_global() const
    {
    	std::cout << "globals:" << std::endl;
        for (auto const& p : variables_global)
        {

            std::cout << p.second.tid.pretty_name() << "  " << p.first << "=";
             if (p.second.tid==DOUBLE)
             	std::cout << *(double*)(void*)&data_[p.second.location];
             else if (p.second.tid==INT64)
             	std::cout << data_[p.second.location];
             else if (p.second.tid==STR8)
             	std::cout << str8{data_[p.second.location]}.toString();
             else
             	std::cout << "unknown" ;

            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    byte_code rel_op_is_is_to_iv_ic(byte_code instr)
    {
		if (instr==op_lt_s_is_is)
			return op_lt_s_iv_ic;

		if (instr==op_lte_s_is_is)
			return op_lte_s_iv_ic;

		if (instr==op_gt_s_is_is)

			return op_gt_s_iv_ic;

		if (instr==op_gte_s_is_is)
			return op_gte_s_iv_ic;

		if (instr==op_eq_s_is_is)
			return op_eq_s_iv_ic;

		if (instr==op_neq_s_is_is)
			return op_neq_s_iv_ic;

		if (instr==op_neq_s_is_is)
			return op_neq_s_iv_ic;

		return (byte_code)0;
    }



    byte_code rel_op_ds_ds_to_dv_dc(byte_code instr)
    {
		if (instr==op_lt_s_ds_ds)
			return op_lt_s_dv_dc;

		if (instr==op_lte_s_ds_ds)
			return op_lte_s_dv_dc;

		if (instr==op_gt_s_ds_ds)

			return op_gt_s_dv_dc;

		if (instr==op_gte_s_ds_ds)
			return op_gte_s_dv_dc;

		if (instr==op_eq_s_ds_ds)
			return op_eq_s_dv_dc;

		if (instr==op_neq_s_ds_ds)
			return op_neq_s_dv_dc;

		if (instr==op_neq_s_ds_ds)
			return op_neq_s_dv_dc;

		return (byte_code)0;
    }

    byte_code rel_op_is_is_to_iv_iv(byte_code instr)
    {
		if (instr==op_lt_s_is_is)
			return op_lt_s_iv_iv;

		if (instr==op_lte_s_is_is)
			return op_lte_s_iv_iv;

		if (instr==op_gt_s_is_is)

			return op_gt_s_iv_iv;

		if (instr==op_gte_s_is_is)
			return op_gte_s_iv_iv;

		if (instr==op_eq_s_is_is)
			return op_eq_s_iv_iv;

		if (instr==op_neq_s_is_is)
			return op_neq_s_iv_iv;

		if (instr==op_neq_s_is_is)
			return op_neq_s_iv_iv;

		return (byte_code)0;
    }

    byte_code arith_op_is_is_to_iv_iv(byte_code instr)
    {
		if (instr==op_add_s_is_is)
			return op_add_s_iv_iv;

		if (instr==op_sub_s_is_is)
			return op_sub_s_iv_iv;

		if (instr==op_mul_s_is_is)
			return op_mul_s_iv_iv;

		if (instr==op_div_s_is_is)
			return op_div_s_iv_iv;

		return (byte_code)0;
    }


    byte_code rel_op_ds_ds_to_dv_dv(byte_code instr)
    {
		if (instr==op_lt_s_ds_ds)
			return op_lt_s_dv_dv;

		if (instr==op_lte_s_ds_ds)
			return op_lte_s_dv_dv;

		if (instr==op_gt_s_ds_ds)

			return op_gt_s_dv_dv;

		if (instr==op_gte_s_ds_ds)
			return op_gte_s_dv_dv;

		if (instr==op_eq_s_ds_ds)
			return op_eq_s_dv_dv;

		if (instr==op_neq_s_ds_ds)
			return op_neq_s_dv_dv;

		if (instr==op_neq_s_ds_ds)
			return op_neq_s_dv_dv;

		return (byte_code)0;
    }

    byte_code arith_op_ds_ds_to_dv_dv(byte_code instr)
    {
		if (instr==op_add_s_ds_ds)
			return op_add_s_dv_dv;

		if (instr==op_sub_s_ds_ds)
			return op_sub_s_dv_dv;

		if (instr==op_mul_s_ds_ds)
			return op_mul_s_dv_dv;

		if (instr==op_div_s_ds_ds)
			return op_div_s_dv_dv;

		return (byte_code)0;
    }

    byte_code arith_op_ds_ds_to_v_ds_dv(byte_code instr)
    {
  		if (instr==op_add_s_ds_ds)
  			return op_add_v_ds_dv;

  		if (instr==op_sub_s_ds_ds)
  			return op_sub_v_ds_dv;

  		if (instr==op_mul_s_ds_ds)
  			return op_mul_v_ds_dv;

  		if (instr==op_div_s_ds_ds)
  			return op_div_v_ds_dv;

  		return (byte_code)0;
     }

    byte_code arith_op_ds_ds_to_v_ds_dc(byte_code instr)
    {
    	if (instr==op_add_s_ds_ds)
        	return op_add_v_ds_dc;

    	if (instr==op_sub_s_ds_ds)
        	return op_sub_v_ds_dc;

    	if (instr==op_mul_s_ds_ds)
    		return op_mul_v_ds_dc;

    	if (instr==op_div_s_ds_ds)
    	    return op_div_v_ds_dc;

    	return (byte_code)0;
    }








    //optimizer


    inline constexpr bool is_code_pointers_instruction(byte_code instr)
    {
    	return (instr==op_jump)||(instr==op_jump_if_not)
    	    		   ||(instr==op_jump_if)||
    				   (instr==op_jump_and_keep_stack_if_not)|| (instr==op_jump_and_keep_stack_if)||(instr==op_ucall);
    }

    void program::optimize()
    {
    	//std::cout << "__________________________" << std::endl;
    	//print_assembler();
    	//auto pc = code.begin();
    	int64_t * code_=&code.front();
    	int n=code.size();
    	std::map<int64_t,int64_t> old_to_new_labels;
    	std::vector<int64_t> dest_jumps;
    	int dst=0;
    	int64_t tmp1;

    	byte_code instr=(byte_code)0;
    	for (int src=0;src<n;)
    	{
    		instr=(byte_code)code_[src];
    		if (is_code_pointers_instruction(instr))
    		{
    			if ((instr==op_jump_and_keep_stack_if)&&(code_[code_[src+1]]==op_jump_if_not))
    			{
    				code_[src]=op_jump_if;
    				code_[src+1]+=2;
    			}
    			if ((instr==op_jump_and_keep_stack_if_not)&&(code_[code_[src+1]]==op_jump_if))
    			{
    				code_[src]=op_jump_if_not;
    				code_[src+1]+=2;
    			}

    		   if (instr==op_ucall)
    			   ++src;
    		   old_to_new_labels[code_[++src]]=-1;

    		   ++src;
    		}
    		else
    		{
			   src+=instr_size[instr];
			  // if (instr==op_print)//only this instruction has variable size
			   //		src+=code[src-1];

			  if ((instr==op_callvar)||(instr==op_callv_var))//only this instruction has variable size
			  		src+=code[src-2];//  instr,argc,vargc,varg0_type,varg1_type(src points here)  , so vargc is at src-2

    		}

    	};

    	auto label_iter=old_to_new_labels.begin();
    	auto elabel_iter=old_to_new_labels.end();

    	auto fr_it=functions_reverse.begin();
    	auto fr_eit=functions_reverse.end();

    	std::map<int64_t,func_signature> functions_reverse_new;

    	for (int src=0;src<n;)
    	{

    		if ((label_iter!=elabel_iter) && label_iter->first==src)
    		{
    			label_iter++->second=dst;

    		}

    		if ( (fr_it!=fr_eit) && fr_it->first==src)
    		{
    			auto& val=functions[fr_it->second];
    			val.start=dst;

    			functions_reverse_new[dst]=fr_it->second;
    			++fr_it;

    		}

    		instr=(byte_code)code[src];


    		if (is_code_pointers_instruction(instr))
			{
    			code[dst++]=code[src++];
    			if (instr==op_ucall)
    				code[dst++]=code[src++];//ucall nargs
    			if (code[src]<src)
    				code[dst++]=old_to_new_labels[code[src++]];
    			else
    			{
    			  dest_jumps.push_back(dst);//this will be set later
    			  code[dst++]=code[src++];
    			}
    			continue;
			}


    		if (instr==client::op_nop)
    		{
    			src++;
    			continue;
    		}

    		// VAR
    		byte_code ninstr=(byte_code)0;
			if ((instr==client::op_load)&&(src+3<n))// VAR
			{
				instr=(byte_code)code[src+2];


				if (src+4<n)
				{
					byte_code instr4=(byte_code)code[src+4];
					if (instr==op_int)
						ninstr=rel_op_is_is_to_iv_ic(instr4); // st = var OP int
						// missing instructions op_<arith>_iv_ic
					else
					if (instr==op_double)
						ninstr=rel_op_ds_ds_to_dv_dc(instr4); // st = var OP double
						// missing instructions op_<arith>_dv_dc
					else
					if (instr==op_load)// st = VAR OP VAR   either for int or double
					{
						 ninstr=rel_op_ds_ds_to_dv_dv(instr4);
						 if (!ninstr)
							ninstr=rel_op_is_is_to_iv_iv(instr4);
						 if (!ninstr)
							 ninstr=arith_op_ds_ds_to_dv_dv(instr4);
						 if (!ninstr)
						 	 ninstr=arith_op_is_is_to_iv_iv(instr4);
					}


				}

				if (ninstr)
				{
					code[dst]=ninstr;
					code[dst+1]=code[src+1];
					code[dst+2]=code[src+3];

					src+=5;
					dst+=3;
					continue;

				}

				if ((code[src+3]==op_store)&&(src+4<n)) // var = st OP var2
				{
					  ninstr=arith_op_ds_ds_to_v_ds_dv(instr);
					   // missing instructions for int op_<arith>_s_is_iv

					  if (ninstr)
					  {
							code[dst]=ninstr;
							tmp1=code[src+1];
							code[dst+1]=code[src+4];
							code[dst+2]=tmp1;

							src+=5;
							dst+=3;
							continue;
					  }

				}


				if ((src+5<n)&&(code[src+2]==client::op_load)&&(code[src+4]==(client::op_negd))
						&&(code[src+5]==(client::op_lt_s_ds_ds)))
				{
					code[dst]=op_lt_s_dv_dnv;
					code[dst+1]=code[src+1];
					code[dst+2]=code[src+3];

					src+=6;
					dst+=3;
					continue;
				}


				if (code[src+2]==client::op_store)
				{
					code[dst]=client::op_mov;
					tmp1=code[src+1];
					code[dst+1]=code[src+3];
					code[dst+2]=tmp1;

					src+=4;
					dst+=3;
					continue;
				}

				instr=(byte_code)code[src];

			} // end LOAD/Var
			else

			if ((instr==client::op_double)&&(src+3<n)) // var = stack ARITH_OP double
			{
				if ((code[src+3]==client::op_store)&&(src+4<n))
				{

//					instr=(byte_code)code[src+2];
					ninstr=arith_op_ds_ds_to_v_ds_dc((byte_code)code[src+2]);
					//unlikely we need assign bool  var= stack REL_OP double
					if (ninstr)
					{
						code[dst]=ninstr;
						tmp1=code[src+1];
						code[dst+1]=code[src+4];
						code[dst+2]=tmp1;

						src+=5;
						dst+=3;
						continue;
					}
				}

				if (code[src+2]==client::op_store)
				{
					code[dst]=client::op_set_double;
					tmp1=code[src+1];
					code[dst+1]=code[src+3];
					code[dst+2]=tmp1;

					src+=4;
					dst+=3;
					continue;
				}

		//		instr=(byte_code)code[src];
			}

			if ((src+3<n)&&(instr==client::op_int)&&(code[src+2]==client::op_store))
				{
					code[dst]=client::op_set;
					tmp1=code[src+1];
					code[dst+1]=code[src+3];
					code[dst+2]=tmp1;

					src+=4;
					dst+=3;
					continue;
				}

			if ((instr==op_add_s_dv_dv)&&(code[src+3]==op_mul_v_ds_dc))
			{
				code[dst]=op_add_r_dv_dv;
				code[dst+1]=code[src+1];
				code[dst+2]=code[src+2];
				code[dst+3]=op_mul_v_dr_dc;
				code[dst+4]=code[src+4];
				src+=5;
				dst+=5;
				continue;
			}

			int ncopy=instr_size[instr];

			if ((instr==op_callvar)||(instr==op_callv_var))//only this instruction has variable size
					ncopy+=code[src+2];//  instr(src points here),argc,vargc,varg0_type,varg1_type  , so vargc is at src+2


			for (int i=0;i<ncopy;i++)
				code[dst++]=code[src++];//all other instructions including arguments, we move in-place

    	}

    	code.resize(dst);

    	int jsize=dest_jumps.size();
    	for (int i=0;i<jsize;i++)
    	{
    		code[dest_jumps[i]]=old_to_new_labels[code[dest_jumps[i]]];//this should be known here
    		if (code[dest_jumps[i]]<0)
    			code[dest_jumps[i]]=code.size();
    	}

    	functions_reverse=functions_reverse_new;

    	//std::cout << "________optimized__________________" << std::endl;
        //print_assembler();

    }










    //printer



    void program::print_assembler()
    {

        auto pc = code.begin();

        std::map<int,std::string> globals;//(variables_global.size());
        std::map<int,std::string> locals_;//(variables.size());
        //locals_.resize(100);
        //typedef std::pair<std::string, int > pair;

        for (auto const& p : variables_global)
        {
            globals[p.second.location] = p.first;
            std::cout << "global       "
                << p.first << ":" << p.second.tid.pretty_name() << ", @" << p.second.location << std::endl;
        }

        std::map<std::size_t, std::string> lines;
        std::set<std::size_t> jumps;
        auto it=functions_reverse.begin();

        auto getvarname=[&](int64_t v) ->std::string {
        	if (v>=(int64_t)locals_.size())
        		return "("+boost::lexical_cast<std::string>(v)+")";
        	return locals_[v];
        };

        auto getvarname_ds=[&](int64_t v) ->std::string {
                	if (v>=(int64_t)globals.size())
                		return "("+boost::lexical_cast<std::string>(v)+")";
                	return globals[v];
        };

        int64_t lhs_i=0;
        //types rt5=void_e;
        //func_ptr fptr5;
        while (pc != code.end())
        {
            std::string line;
            std::size_t address = pc - code.begin();

            it=functions_reverse.find(address);
            if (it!=functions_reverse.end())
            {
                  line += " " + it->second.signature() + ":\n";
                  program::vars_map_type& variables_=functions[it->second].locals;
                  locals_.clear();
                  //locals_.resize(variables_.size());
                  for (auto const& p : variables_)
                  {
                      locals_[p.second.location] = p.first;
                      line += "      local       "
                          + p.first + "   :   " + p.second.tid.pretty_name() + ", @" + boost::lexical_cast<std::string>(p.second.location) + "\n";
                  }
                  line += "\n";
            }

           // std::pair<func_signature,func_ptr>* pp;


            //line += std::string(instr_name[*pc]) + " ";
            switch (*pc++)
            {
            	case op_exit:
            		line += "      exit";
            		break;

               	case op_nop:
                		line += "      nop";
                		break;

            	case op_stk_adj:
                    line += "      op_stk_adj  ";
                    line += boost::lexical_cast<std::string>(*pc++);
                    break;

            	case op_stk_adj_chk:
					line += "      op_stk_adj_chk  ";
					line += boost::lexical_cast<std::string>(*pc++);
					break;

            	case op_stk_assert:
            		line += "      op_stk_assert";
            		line += "  " + boost::lexical_cast<std::string>(*pc++);
            		break;

            	case op_ucall:
            		line += "      ucall";
            		line += "  nargs:" + boost::lexical_cast<std::string>(*pc++);
            		lhs_i=*pc;
            		line += "  offset:" + boost::lexical_cast<std::string>(lhs_i);
            		it=functions_reverse.find(lhs_i);
            		if (it!=functions_reverse.end())
            			line += " " + it->second.name;
            		jumps.insert(lhs_i);
            		++pc;
            		break;

            	case op_return_void:
            		line += "      return_void";
            		break;

            	case op_return_int:
					line += "      return_int";
					break;

            	case op_call:
            		line += "      call";
            		line += "  (args:" + boost::lexical_cast<std::string>(*pc++);
            		line += ")  " + boost::lexical_cast<std::string>((void*)(*pc++));//address
            		//pp=(std::pair<func_signature,func_ptr>*)(*pc++);
            		//line += "  " + pp->first.name;
            		//pp=(std::pair<func_signature,func_ptr>*)(*pc++);
            		//fptr5=pp->second;
            		//rt5=fptr5.return_type;

            		//line += std::string("  ret:") + types_names[rt5];
            		break;

            	case op_callv:       // function with no return type type void func(args...);
            		line += "      callv";
            		line += "  (args:" + boost::lexical_cast<std::string>(*pc++);
            		line += ")  " + boost::lexical_cast<std::string>((void*)(*pc++));//address
            		//line += "  " + ((std::pair<func_signature,func_ptr>*)(*pc++))->first.name;
            		break;

            	case op_call0reg:       // function call with zero arguments T func();
            		line += "      call0reg";
            		line += "  " + ((std::pair<func_signature,func_ptr>*)(*pc++))->first.name;
            		line += std::string("  ret:") + ((std::pair<func_signature,func_ptr>*)(*pc++))->first.return_type.pretty_name();
            		break;

            	case op_call0:       // function call with zero arguments T func();
					line += "      call0";
					line += "  " + boost::lexical_cast<std::string>((void*)(*pc++));
					line += std::string("  ret:int64_t");
					break;

            	case op_callv0reg:      // function with no return type and zero args void func();
            		line += "      callv0reg";
            		line += "  " + ((std::pair<func_signature,func_ptr>*)(*pc++))->first.name;
            		break;

            	case op_callv0:	// function with no return type and zero args void func(); optimized/direct call
            		line += "      callv0";
            		line += "  " + boost::lexical_cast<std::string>((void*)(*pc++));
            		break;

            	case op_callvar:
            		line += "      callvar";
            		lhs_i=*pc++;//all arguments count
            		line += "  (args:" + boost::lexical_cast<std::string>(lhs_i);
            		lhs_i=*pc++;//variadic arguments count
            		line += "  variadic args cnt:" + boost::lexical_cast<std::string>(lhs_i) + ")  " ;
            		for (int i=0;i<lhs_i;i++)
            				line += TO_TYPEID(*pc++).pretty_name()+ " ";
            		          //line +=  boost::typeindex::type_index(*(boost::typeindex::type_index::type_info_t*)*pc++).pretty_name() + ",";
            		line += boost::lexical_cast<std::string>((void*)(*pc++));//address
            		break;

            	case op_callv_var:
            		line += "      callv_var";
            		lhs_i=*pc++;//all arguments count
            		line += "  (args:" + boost::lexical_cast<std::string>(lhs_i);
            		lhs_i=*pc++;//variadic arguments count
            		line += "  variadic args cnt:" + boost::lexical_cast<std::string>(lhs_i) + ")  " ;
            		for (int i=0;i<lhs_i;i++)
            				line += TO_TYPEID(*pc++).pretty_name()+ " ";
            		          //line +=  boost::typeindex::type_index(*(boost::typeindex::type_index::type_info_t*)*pc++).pretty_name() + ",";
            		line += boost::lexical_cast<std::string>((void*)(*pc++));//address
            		break;

                case op_neg:
                    line += "      op_neg";
                    break;

                case op_inc:
                    line += "      op_inc     ";
                    line += getvarname(*pc++);
                    break;

                case op_dec:
                    line += "      op_dec     ";
                    line += getvarname(*pc++);
                    break;

                case op_negd:
                    line += "      op_neg_d";
                    break;

                case op_not:
                    line += "      op_not";
                    break;

                case op_add_s_is_is:
                    line += "      op_add";
                    break;

                case op_add_r_ds_ds:
                	line += "      op_addd -> r";
                	break;

                case op_add_s_ds_ds:
					line += "      op_addd";
					break;

                case op_sub_s_is_is:
                    line += "      op_sub";
                    break;

                case op_sub_s_ds_ds:
                	line += "      op_subd";
                	break;

                case op_mul_s_is_is:
                    line += "      op_mul";
                    break;

                case op_mul_s_ds_ds:
                    	line += "      op_muld";
                    	break;

                case op_mul_v_dr_dc:
                	line += "      op     ";
                	line += getvarname(*pc++);
                	line += " := reg * "+boost::lexical_cast<std::string>(*(double*)(void*)&*pc++);
                	break;

                case op_mul_v_ds_dc:
                	line += "      op     ";
                	line += getvarname(*pc++);
                	line += " := pop * "+boost::lexical_cast<std::string>(*(double*)(void*)&*pc++);
                	break;

                case op_div_s_is_is:
                    line += "      op_div";
                    break;

                case op_div_s_ds_ds:
                    	line += "      op_divd";
                    	break;



                case op_eq_s_is_is:
                    line += "      op_eq";
                    break;

                case op_eq_s_ds_ds:
                    line += "      op_eqd";
                    break;

                case op_neq_s_is_is:
                    line += "      op_neq";
                    break;
                case op_neq_s_ds_ds:
				   line += "      op_neqd";
				   break;

                case op_lt_s_is_is:
                    line += "      op_lt";
                    break;

                case op_lt_s_ds_ds:
					 line += "      op_ltd";
					 break;

                case op_lte_s_is_is:
                    line += "      op_lte";
                    break;

                case op_lte_s_ds_ds:
					line += "      op_lted";
					break;

                case op_gt_s_is_is:
                    line += "      op_gt";
                    break;

                case op_gt_s_ds_ds:
				   line += "      op_gtd";
				   break;

                case op_gte_s_is_is:
                    line += "      op_gte";
                    break;

                case op_gte_s_ds_ds:
				   line += "      op_gted";
				   break;

                case op_and:
                    line += "      op_and";
                    break;

                case op_or:
                    line += "      op_or";
                    break;

                case op_load:
                    line += "      op_load     ";
                    line += getvarname(*pc++);
                    break;

                case op_store:
                    line += "      op_store    ";
                    line += getvarname(*pc++);
                    break;

                case op_load_ds:
                    line += "      op_load_ds     ";
                    line += getvarname_ds(*pc++);
                    break;

                case op_store_ds:
                    line += "      op_store_ds    ";
                    line += getvarname_ds(*pc++);
                    break;

                case op_set:
                     line += "      op_set    ";
                     line += getvarname(*pc++);
                     line +=  "  = "+boost::lexical_cast<std::string>(*pc++);
                     break;

                case op_lt_s_dv_dc:
					line += "      op_lt_vc_dd   ";
					line += getvarname(*pc++);
					line += " < "+boost::lexical_cast<std::string>(*(double*)(void*)&*pc++);
					break;
/*
                case op_lt_s_iv_ic_:
					line += "      op_lt_vc_    ";
  					line += boost::lexical_cast<std::string>(locals[*pc++]);
  					line += " < "+boost::lexical_cast<std::string>(*pc++);
  					break;*/
                case op_lt_s_iv_ic:
//                case op_lt|lhs_var|rhs_const:
  					line += "      op_lt_vc    ";
  					line += getvarname(*pc++);
  					line += " < "+boost::lexical_cast<std::string>(*pc++);
  					break;

                case op_lt_s_iv_iv:
					line += "      op_lt_s_iv_iv   ";
					line += getvarname(*pc++);
					line += " < "+getvarname(*pc++);
					break;

                case op_gt_s_dv_dc:
  //              case op_gt|lhs_double|rhs_double|lhs_var|rhs_const:
					line += "      op_gt_vc_dd   ";
					line += getvarname(*pc++);
					line += " > "+boost::lexical_cast<std::string>(*(double*)(void*)&*pc++);
					break;

                case op_gt_s_iv_ic:
  					line += "      op_gt_vc    ";
  					line += getvarname(*pc++);
  					line += " > "+boost::lexical_cast<std::string>(*pc++);
  					break;
/*
                case op_gt_s_dv_dv_:
					line += "      op_gt_vv_dd_   ";
					line += boost::lexical_cast<std::string>(locals[*pc++]);
					line += " > "+boost::lexical_cast<std::string>(locals[*pc++]);
                	break;*/

                case op_gt_s_dv_dv:
//                case op_gt|lhs_double|rhs_double|lhs_var|rhs_var:
					line += "      op_gt_vv_dd   ";
					line += getvarname(*pc++);
					line += " > "+getvarname(*pc++);
					break;

                case op_lt_s_dv_dnv:
  //              case op_lt|lhs_double|rhs_double|lhs_var|rhs_var|rhs_neg:
					line += "      op_lt_vv_dd   ";
					line += getvarname(*pc++);
					line += " < -"+getvarname(*pc++);
					break;

                case op_gt_s_iv_iv:
  					line += "      op_gt_vv    ";
  					line += getvarname(*pc++);
  					line += " > "+getvarname(*pc++);
  					break;

                case op_set_double:
                     line += "      op_set_double    ";
                     line += getvarname(*pc++);
                     line +=  " := "+boost::lexical_cast<std::string>(*(double*)(void*)&*pc++);
                    break;

                case op_mov:
                    line += "      op_mov    ";
                    line += getvarname(*pc++);
                    line +=  " :=  "+getvarname(*pc++);
                	break;

                case op_add_s_iv_iv:
                      line += "      push     ";
                      line += getvarname(*pc++);
                      line += " + "+getvarname(*pc++);

                      break;

                case op_sub_s_iv_iv:
                      line += "      push     ";
                      line += getvarname(*pc++);
                      line += " - "+getvarname(*pc++);

                      break;

                case op_mul_s_iv_iv:
                      line += "      push     ";
                      line += getvarname(*pc++);
                      line += " * "+getvarname(*pc++);

                      break;

                case op_div_s_iv_iv:
                      line += "      op_div_vv     ";
                      line += getvarname(*pc++);
                      line += "   "+getvarname(*pc++);

                      break;
                case op_add_r_dv_dv:
                	  line += "      reg=     ";
                	  line += getvarname(*pc++);
                	  line += " + "+getvarname(*pc++);

                	    break;

                case op_add_s_dv_dv:
                      line += "      pushd     ";
                      line += getvarname(*pc++);
                      line += " + "+getvarname(*pc++);
                      break;

                case op_add_s_dv_dv_:
                      line += "      pushd_     ";
                      line += getvarname(*pc++);
                      line += " + "+getvarname(*pc++);

                      break;

                case op_sub_s_dv_dv:
                      line += "      pushd     ";
                      line += getvarname(*pc++);;
                      line += "  - "+getvarname(*pc++);;

                      break;

                case op_mul_s_dv_dv:
                     line += "      pushd     ";
                     line += getvarname(*pc++);;
                     line += "  * "+getvarname(*pc++);;

                     break;
                case op_mul_s_dv_dv_:
                    line += "      pushd_     ";
                     line += getvarname(*pc++);;
                     line += "  * "+getvarname(*pc++);;

                     break;

                case op_div_s_dv_dv:
                      line += "      pushd     ";
                      line += getvarname(*pc++);;
                      line += " / "+getvarname(*pc++);

                      break;

                case op_div_v_ds_dc:
                //case  client::op_div|rhs_const|store_var|lhs_double|rhs_double:
					line += "      op    ";
					line += getvarname(*pc++);
					line += " := pop/"+boost::lexical_cast<std::string>(*(double*)(void*)&*pc++);
					break;

                case op_sub_v_ds_dc:
                //case  client::op_sub|rhs_const|store_var|lhs_double|rhs_double:
					line += "      op     ";
					line += getvarname(*pc++);;
					line += " := pop-"+boost::lexical_cast<std::string>(*(double*)(void*)&*pc++);
					break;

                case	op_sub_v_ds_dv:
    				line += "      op     ";
    					line += getvarname(*pc++);;
    					line += " := pop-"+getvarname(*pc++);;
    					break;

                case op_int:
                    line += "      op_int      ";
                    line += boost::lexical_cast<std::string>(*pc++);
                    break;

                case op_double:
				   line += "      op_double      ";
				   line += boost::lexical_cast<std::string>(*(double*)(void*)&*pc++);
				   break;

                case op_str8:
                	line += "      op_str8      ";
                	line +=  str8{*pc++}.toString();
                	break;

                case op_true:
                    line += "      op_true";
                    break;

                case op_false:
                    line += "      op_false";
                    break;

                case op_jump_and_keep_stack_if:
                case op_jump_and_keep_stack_if_not:
                case op_jump_if_not:
                case op_jump_if:
                case op_jump:
						{
							line += std::string("      ")+ instr_name[*(pc-1)] + "   ";
							std::size_t pos = *pc++;
							line += (pos == code.size()) ? "end" : boost::lexical_cast<std::string>(pos);
							jumps.insert(pos);
						}
						break;

                default:
                	--pc;
                	line += std::string("      ")
                			+ instr_name[*pc] + "(" + boost::lexical_cast<std::string>(*pc)+")  +" +
                			boost::lexical_cast<std::string>(instr_size[*pc]-1) + " args";

                	pc+=instr_size[*pc];
                	break;
            }
            lines[address] = line;
        }

        std::cout << "start:" << std::endl;
        for (auto const& l : lines)
        {
            std::size_t pos = l.first;
            if (jumps.find(pos) != jumps.end())
            {
                std::cout << pos << ':' << std::endl;
            }
            std::cout << l.second << std::endl;
        }

        //if (jumps.find(code.size())!=jumps.end())
        //	if (jumps2)
        	//	(*jumps2)[code.size()]=0;

        std::cout << "end:" << std::endl;
    }
}
}
