/*=============================================================================
    Copyright (c) 2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#ifndef PROGRAM_H_
#define PROGRAM_H_

#include <inttypes.h>
#include <vector>
#include <string>
#include "types.h"
#include "global_funcs.h"

namespace client { namespace code_gen
{
///////////////////////////////////////////////////////////////////////////
//  The Program
///////////////////////////////////////////////////////////////////////////

  struct VarInfo{
	int64_t location;//it is positive value for global data,  it can be negative as it is offset from frame pointer for local data in function
	size_t  size;//1 for simple value,  >1 for arrays/tuples
	TypeID  tid;
	bool 	initialized;
	bool 	is_const;
  };

  struct program
    {
        void op(int64_t a);
        void op(int64_t a, int64_t b);
        void op(int64_t a, int64_t b, int64_t c);

        int64_t& operator[](std::size_t i) { return code[i]; }
        int operator[](std::size_t i) const { return code[i]; }
        void clear() { code.clear(); variables.clear(); }
        std::size_t size() const { return code.size(); }
        std::vector<int64_t> const& operator()() const { return code; }
        std::vector<int64_t>& data()  { return data_;}

        std::size_t nvars() const { return variables.size(); }
        std::size_t nvars_global() const { return variables_global.size(); }



        void init_fp_offset(int64_t offs) {
        	current_stack_allocation_frame_pointer_offset=offs;
        }
        int64_t fp_offset() {
               return 	current_stack_allocation_frame_pointer_offset;
        }

        int64_t const* find_var_fp_offset(std::string const& name) const;
        VarInfo * find_var_info(std::string const& name);
        VarInfo& add_var(std::string const& name,TypeID tid,size_t size=1,bool initialized=false);

        int64_t const* find_var_global(std::string const& name) const;
        VarInfo * find_var_info_global(std::string const& name);
        VarInfo& add_var_global(std::string const& name,TypeID tid,size_t size=1,bool initialized=false);

        void adjust_data_segment()
        {
        	data_.resize(allocated_global_data_size);
        }

        void print_variables(std::vector<int64_t> const& stack) const;
        void print_variables_global() const;

        void print_assembler();
        void optimize();

        std::string signatures_with_name(const std::string& fname) const
        {
        	std::string out;
        	for(const auto& func:functions)
        	{
        		if (func.first.name==fname)
        			out+="\n  candidate:"+func.first.signature();
        	}
        	return out;
        }

        typedef std::map<std::string, VarInfo > vars_map_type;
        vars_map_type variables;//local variables on the stack
        vars_map_type variables_global;//global variables in data segment

        struct func_desc
        {
        	int64_t start;
        	TypeID  return_type;
        	vars_map_type locals;
        	int64_t stack_requirement;//minimal size of stack space needed for this function to work safely
        };
        std::map<func_signature, func_desc > functions;
        std::map<int64_t,func_signature> functions_reverse;


    private:
        size_t allocated_global_data_size=0;
        int64_t current_stack_allocation_frame_pointer_offset=0;//we can also start from negative values, -15,14,....
        std::vector<int64_t> code;
        std::vector<int64_t> data_;
    };

}
}

#endif /* PROGRAM_H_ */
