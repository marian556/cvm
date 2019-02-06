/*=============================================================================
    Copyright (c) 2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#ifndef GLOBAL_FUNC_H_
#define GLOBAL_FUNC_H_

#include <string>
#include <vector>
#include <map>
#include "types.h"

enum FunctionSignaturePolicy
{
	fixed_signature=0,//compile time signature, arguments have to bind to
	varargs=1,//after fixed signature typed arguments are the arbitrary number of arbitrary type arguments?
	varcount=2,//after fixed signature typed arguments are the arbitrary number of the same type of last signature type arguments?
	pass_types=3,//like varargs but generates array of types of variadic arguments and passes it to called function
	name_only=4
};

struct func_signature
{
	std::string name;
	std::vector<TypeID> arg_types;
	TypeID return_type;
	FunctionSignaturePolicy fsp;

	std::string signature() const
	{
		std::string out=name+"(";
		bool first=true;
		for (auto& tp:arg_types)
		{
			if (!first)
			{
				out += ",";
			}
			first=false;
			out += tp.pretty_name();
//			out	+=types_names[tp];
		}
		if (fsp==varargs)
			out +=",...";
		if (fsp==pass_types)
			out +=",...";
		if (fsp==varcount)
			out +="...";
		out += ")";
		return out;
	}
	bool operator<(const func_signature& other) const
	{
		if (name < other.name)
			return true;
		if (name > other.name)
			return false;
		return arg_types< other.arg_types;
	}
};



struct func_ptr {
	void *p;
//	std::string help;//function human description
};

typedef std::map<func_signature,func_ptr> functions_set;

extern const functions_set global_functions;

extern std::string global_signatures_with_name(const std::string& fname);

//extern int64_t call_glob_func(int64_t n,int64_t* vm_stack, int64_t ptr_map_entry);
//extern void call_glob_func_void(int64_t n,int64_t* vm_stack, int64_t ptr_map_entry);


#endif
