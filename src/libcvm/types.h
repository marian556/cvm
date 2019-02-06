/*=============================================================================
    Copyright (c) 2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#ifndef TYPES_H_
#define TYPES_H_

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <boost/type_index.hpp>
#include <boost/any.hpp>


typedef boost::typeindex::type_index TypeID;

#define TO_TYPEID(A) TypeID(*(TypeID::type_info_t*)A)

template <typename T>
int64_t reinterpret_as_int64(T val)
{
	return *(int64_t*)&val;
}

template <typename T>
T reinterpret_as(int64_t val)
{
	return *(T*)&val;
}

#define TO_DOUBLE(A) reinterpret_as<double>(A)
#define TO_INT64(A) reinterpret_as_int64(A)

template <typename T>
constexpr TypeID getTypeID()
{
	return boost::typeindex::type_id<T>();
}

struct no_type{};

struct str8 {

	std::string toString() const
	{

		if (ch[0]==0)
			return std::string();
		if ((ch[7]==0)||(ch[6]==0)||(ch[5]==0)||(ch[4]==0)||(ch[3]==0)||(ch[2]==0)||(ch[1]==0))
			return std::string(ch);
		return std::string(ch,8);
	}

	union {
		int64_t val;
		char ch[8];
	};
};

struct vm_addr {
	int64_t addr;
};

struct stack_addr {
	int64_t addr;
};

struct data_addr {
	int64_t addr;
};

struct cstring {
	data_addr val;
};

#define NO_TYPE  getTypeID<no_type>()
#define VM_ADDR  getTypeID<vm_addr>()
#define ST_ADDR  getTypeID<stack_addr>()
#define DATA_ADDR  getTypeID<data_addr>()
#define VOID     getTypeID<void>()
#define BOOL     getTypeID<bool>()

#define INT    	 getTypeID<int>()
#define UINT     getTypeID<unsigned int>()
#define INT32    getTypeID<int32_t>()
#define INT64    getTypeID<int64_t>()
#define UINT32   getTypeID<uint32_t>()
#define UINT64   getTypeID<uint64_t>()

#define DOUBLE   getTypeID<double>()
#define STR8	 getTypeID<str8>()
#define CSTRING  getTypeID<cstring>()
#define TYPEID	 getTypeID<TypeID>()
#define VECTOR_INT64 getTypeID<std::vector<int64_t>*>()
#define VECTOR_DOUBLE getTypeID<std::vector<double>*>()
#define VECTOR_ANY getTypeID<std::vector<boost::any>*>()

template <int i=0>//this will merge multiple definitions in different translation units
const TypeID getTypeID(const std::string& type_name)
{
	static const std::map<std::string,TypeID> mp{
		{"typeinfo",TYPEID},
		{"void",VOID},
		{"bool",BOOL},
		{"int",INT64},//unlike C++ , where int is 32 bit for both 32bit and 64 architecture
		{"long",INT64},
		{"uint",UINT64},//unlike C++ , where int is 32 bit for both 32bit and 64 architecture
		{"ulong",UINT64},
		{"int32",INT32},
		{"int64",INT64},
		{"uint32",UINT32},
		{"uint64",UINT64},
		{"double",DOUBLE},
		{"str8",STR8},
		{"cstring",CSTRING},
		{"vector_int64",VECTOR_INT64},
		{"vector_double",VECTOR_DOUBLE},
		{"vector_any",VECTOR_INT64}
	};
	auto it=mp.find(type_name);
	if (it==mp.end())
		return NO_TYPE;
	return it->second;
}


template<int i = 0>
constexpr std::vector<TypeID> sg_()
{
    return std::vector<TypeID>();
}

template<typename FirstT, typename ... RestT>
constexpr std::vector<TypeID> sg_()
{
	std::vector<TypeID> out{sg_<RestT...>()};
	out.push_back(getTypeID<FirstT>());
    return out;
}

template<typename ... ArgsT>
constexpr std::vector<TypeID> sg()
{
	std::vector<TypeID> out{sg_<ArgsT...>()};
	std::reverse(out.begin(),out.end());
	return out;
}


#endif /* TYPES_H_ */
