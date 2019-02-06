/*=============================================================================
    Copyright (c) 2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#ifndef GLOBAL_FUNCS_UTILS_H_
#define GLOBAL_FUNCS_UTILS_H_

#include <inttypes.h>
#include "types.h"
#include "global_funcs.h"

template<typename T>
inline constexpr int64_t retcast(T val);

template<>
inline constexpr int64_t retcast(int64_t val)
{
	return val;
}



template<>
inline constexpr int64_t retcast(int val)
{
	return val;
}

template<>
inline constexpr int64_t retcast(double val)
{
	return reinterpret_cast<int64_t&>(val);
}

template<typename T>
inline constexpr T argcast(int64_t val);

template<>
inline constexpr int64_t argcast<int64_t>(int64_t val)
{
	return val;
}

template<>
inline constexpr const int64_t* argcast(int64_t val)
{
	return reinterpret_cast<const int64_t*>(val);
}

template<>
inline constexpr int argcast<int>(int64_t val)
{
	return val;
}

template<>
inline constexpr double argcast<double>(int64_t val)
{
	return reinterpret_cast<const double&>(val);
}

template<>
inline constexpr str8 argcast<str8>(int64_t val)
{
	return reinterpret_cast<const str8&>(val);;
}

//Fixed signature adapter/wrapper from  arguments in array of intt64_t to array of arguments with types in parameter pack ArgsT
template <typename F,typename RetT,typename ... ArgsT>
struct adapter_func
{
	 adapter_func(F f)
		 : f(f) {}

	 template <std::size_t... Ints>
	 inline int64_t dispatch(const int64_t* args, std::index_sequence<Ints...>) const
	 {
		 return retcast(f(argcast<ArgsT>(args[Ints])...));
	 }


	 inline int64_t operator()(const int64_t* args) const
	 {
		 return dispatch(args, std::make_index_sequence<sizeof...(ArgsT)>());
	 }

	 F f;
};


//like above but no return value
template <typename F,typename ... ArgsT>
struct adapter_func_void
{
	 adapter_func_void(F f)
		 : f(f) {}

	 template <std::size_t... Ints>
	 inline void dispatch(const int64_t* args, std::index_sequence<Ints...>) const
	 {
		 f(argcast<ArgsT>(args[Ints])...);
	 }


	 inline void operator()(const int64_t* args) const
	 {
		  dispatch(args, std::make_index_sequence<sizeof...(ArgsT)>());
	 }

	 F f;
};

//this passes variadic arguments /array only but no type information, (can be used when type is noted in first arguments
//or when compiler enforces agreed type
template <typename F,typename RetT,typename ... ArgsT>
struct adapter_func_variadic
{
	 adapter_func_variadic(F f)
		 : f(f) {}

	 template <std::size_t... Ints>
	 inline int64_t dispatch(int64_t cnt,const int64_t* args, std::index_sequence<Ints...>) const
	 {
		 return retcast(f(cnt-sizeof...(ArgsT),args+sizeof...(ArgsT),argcast<ArgsT>(args[Ints])...));
	 }


	 inline int64_t operator()(int64_t cnt,const int64_t* args) const
	 {
		 return dispatch(cnt,args, std::make_index_sequence<sizeof...(ArgsT)>());
	 }

	 F f;
};

//like above but no return value
template <typename F,typename ... ArgsT>
struct adapter_func_void_variadic
{
	 adapter_func_void_variadic(F f)
		 : f(f) {}

	 template <std::size_t... Ints>
	 inline void dispatch(int64_t cnt,const int64_t* args, std::index_sequence<Ints...>) const
	 {
		 f(cnt-sizeof...(ArgsT),args+sizeof...(ArgsT),argcast<ArgsT>(args[Ints])...);
	 }


	 inline void operator()(int64_t cnt,const int64_t* args) const
	 {
		 dispatch(cnt,args, std::make_index_sequence<sizeof...(ArgsT)>());
	 }

	 F f;
};

//this passes variadic arguments /array only but no type information, (can be used when type is noted in first arguments
//or when compiler enforces agreed type
template <typename F,typename RetT,typename ... ArgsT>
struct adapter_func_variadic_with_types_array
{
	 adapter_func_variadic_with_types_array(F f)
		 : f(f) {}

	 template <std::size_t... Ints>
	 inline int64_t dispatch(int64_t cnt,const int64_t* args,const int64_t* type_ids, std::index_sequence<Ints...>) const
	 {
		 return retcast(f(cnt,args+sizeof...(ArgsT),type_ids,argcast<ArgsT>(args[Ints])...));
	 }

	 inline int64_t operator()(int64_t cnt,const int64_t* args,const int64_t* type_ids) const
	 {
		 return dispatch(cnt,args,type_ids, std::make_index_sequence<sizeof...(ArgsT)>());
	 }

	 F f;
};


template <typename F,typename ... ArgsT>
struct adapter_func_void_variadic_with_types_array
{
	 adapter_func_void_variadic_with_types_array(F f)
		 : f(f) {}

	 template <std::size_t... Ints>
	 inline void dispatch(int64_t* data,int64_t cnt,const int64_t* args,const int64_t* type_ids, std::index_sequence<Ints...>) const
	 {
		 f(data,cnt,args+sizeof...(ArgsT),type_ids,argcast<ArgsT>(args[Ints])...);
	 }

	 inline void operator()(int64_t* data,int64_t cnt,const int64_t* args,const int64_t* type_ids) const
	 {
		 dispatch(data,cnt,args,type_ids, std::make_index_sequence<sizeof...(ArgsT)>());
	 }

	 F f;
};


template <typename ... ArgsT,class RetT,class classT>
inline constexpr auto adapt_sg_operator_func_call(classT obj,RetT (classT::*)(ArgsT...) const)//second argument is only to detect signature ArgsT
{

	return sg<ArgsT...>();//we don't need f,
}

template <typename ... ArgsT,class RetT>
inline constexpr auto adapt_sg(RetT f(ArgsT...))
{
	return sg<ArgsT...>();
}

template <class classT>
inline constexpr auto adapt_sg(classT obj)
{
	return adapt_sg_operator_func_call(obj,&classT::operator());
}

template <typename ... ArgsT,class RetT,class classT>
inline constexpr auto adapt_ret_operator_func_call(classT obj,RetT (classT::*)(ArgsT...) const)//second argument is only to detect signature ArgsT
{

	return getTypeID<RetT>();//we don't need f,
}

template <typename ... ArgsT,class RetT>
inline constexpr auto adapt_ret(RetT f(ArgsT...))
{
	return getTypeID<RetT>();
}

template <class classT>
inline constexpr auto adapt_ret(classT obj)
{
	return adapt_ret_operator_func_call(obj,&classT::operator());
}

template <typename ... ArgsT,class RetT,class classT>
inline constexpr auto adapt_operator_func_call(classT obj,RetT (classT::*)(ArgsT...) const)//second argument is only to detect signature ArgsT
{
	//return adapter_func2<classT,RetT (classT::*)(ArgsT...),ArgsT...>(obj,f);
	return adapter_func<classT,RetT,ArgsT...>(obj);//we don't need f,
}

template <typename ... ArgsT,class classT>
inline constexpr auto adapt_operator_func_call(classT obj,void (classT::*)(ArgsT...) const)//second argument is only to detect signature ArgsT
{
	//return adapter_func2<classT,RetT (classT::*)(ArgsT...),ArgsT...>(obj,f);
	return adapter_func_void<classT,ArgsT...>(obj);//we don't need f,
}

template <typename ... ArgsT,class RetT>
inline constexpr auto adapt(RetT f(ArgsT...))
{
	return adapter_func<RetT (*)(ArgsT...),RetT,ArgsT...>(f);
}

template <typename ... ArgsT,class RetT>
inline constexpr auto adapt(RetT f(ArgsT...,...))//variadic functions
{
	return adapter_func_variadic<RetT (*)(ArgsT...,...),RetT,ArgsT...>(f);
}

template <typename ... ArgsT>
inline constexpr auto adapt(void f(ArgsT...))
{
	return adapter_func_void<void (*)(ArgsT...),ArgsT...>(f);
}

template <class classT>
inline constexpr auto adapt(classT obj)
{
	return adapt_operator_func_call(obj,&classT::operator());
}


template <typename ... ArgsT,class RetT>
inline  auto adapt_variadic(RetT (*f)(int64_t,const int64_t*,ArgsT...))//variadic functions
{
	return adapter_func_variadic<RetT (*)(int64_t,const int64_t*,ArgsT...),RetT,ArgsT...>(f);
}

template <typename ... ArgsT>
inline  auto adapt_variadic(void (*f)(int64_t,const int64_t*,ArgsT...))//variadic functions
{
	return adapter_func_void_variadic<void (*)(int64_t,const int64_t*,ArgsT...),ArgsT...>(f);
}

template <typename ... ArgsT,class RetT>
inline  auto adapt_variadic_with_types(RetT (*f)(int64_t*,int64_t,const int64_t*,const int64_t*,ArgsT...))//variadic functions
{
	return adapter_func_variadic_with_types_array<RetT (*)(int64_t*,int64_t,const int64_t*,const int64_t*,ArgsT...),RetT,ArgsT...>(f);
}

template <typename ... ArgsT>
inline  auto adapt_variadic_with_types(void (*f)(int64_t*,int64_t,const int64_t*,const int64_t*,ArgsT...))//variadic functions
{
	return adapter_func_void_variadic_with_types_array<void (*)(int64_t*,int64_t,const int64_t*,const int64_t*,ArgsT...),ArgsT...>(f);
}



template <auto& t>
inline constexpr int64_t entry(const int64_t* args)
{
	return t(args);
}


template <auto& t>
inline constexpr void entry_void(const int64_t* args)
{
	t(args);
}

template <auto& t>
inline constexpr int64_t entry_variadic(int64_t cnt,const int64_t* args)
{
	return t(cnt,args);
}

template <auto& t>
inline constexpr void entry_void_variadic(int64_t cnt,const int64_t* args)
{
	t(cnt,args);
}

template <auto& t>
inline constexpr int64_t entry_variadic_with_types(int64_t* data,int64_t cnt,const int64_t* args,const int64_t* types)
{
	return t(data,cnt,args,types);
}

template <auto& t>
inline constexpr void entry_void_variadic_with_types(int64_t* data,int64_t cnt,const int64_t* args,const int64_t* types)
{
	 t(data,cnt,args,types);
}

template <typename FuncT,typename WrappedFuncT>
inline constexpr auto regfunc(const std::string& fname,FuncT func,WrappedFuncT wrapped_func,FunctionSignaturePolicy fsp=fixed_signature)
{
	return std::make_pair(func_signature{fname,adapt_sg(func),adapt_ret(func),fsp},func_ptr{(void*)wrapped_func});
}


#define ADAPT(A) const auto f_##A=adapt(A);
#define FUNC(A) { regfunc(#A,A,entry<f_##A>) }
#define FUNC_NAME(B,A) { regfunc(B,A,entry<f_##A>) }
#define FUNC_VOID(A) { regfunc(#A,A,entry_void<f_##A>) }
#define FUNC_NAME_VOID(B,A) { regfunc(B,A,entry_void<f_##A>) }


#define FUNC_UNCHECKED(A,RETTYPE) {{#A,{},getTypeID<RETTYPE>(),name_only},{(void*)static_cast<int64_t(*)(int64_t*)>(A)}}
#define FUNC_NAME_UNCHECKED(B,A,RETTYPE) {{B,{},getTypeID<RETTYPE>(),name_only},{(void*)static_cast<int64_t(*)(int64_t*)>(A)}}
#define FUNC_NO_ARGS(A) {{#A,{},INT64,fixed_signature},{(void*)static_cast<int64_t(*)(void)>(A)}}
#define FUNC_NAME_NO_ARGS(B,A) {{B,{},INT64,fixed_signature},{(void*)static_cast<int64_t(*)(void)>(A)}}
#define FUNC_VOID_NO_ARGS(A) {{#A,{},VOID,fixed_signature},{(void*)static_cast<void(*)(void)>(A)}}
#define FUNC_NAME_VOID_NO_ARGS(B,A) {{B,{},VOID,fixed_signature},{(void*)static_cast<void(*)(void)>(A)}}

//std::map<func_signature,func_ptr> global_functions2;

#endif /* GLOBAL_FUNCS_UTILS_H_ */
