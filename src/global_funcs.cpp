/*=============================================================================
    Copyright (c) 2018-2019 Marian Klein (cvm)

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#include <cstdarg>
#include "libcvm/global_funcs.h"
#include <iostream>

#include <cstdarg>
#include <cmath>
#include <chrono>
#include <iomanip>
#include "libcvm/global_funcs_utils.h"
#include <boost/chrono.hpp>
#include "libcvm/ast.hpp"

double myplus(double a,double b)
{
	return a+b;
}

ADAPT(myplus);

//this is unsafe, as compiler does not check the type and count of passed arguments, this exposes application stack directly
int64_t myplus_raw(int64_t* args)
{
	double arg0=*(double*)args;
	double arg1=*(double*)++args;
	arg0+=arg1;
	return reinterpret_cast<int64_t&>(arg0);
}

void do_something()
{
	std::cout << "do something" << std::endl;
}

int64_t now()
{
	static std::chrono::steady_clock::time_point start=std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point nw=std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::nanoseconds>(nw - start).count();
}

int64_t tsc()
{
	register
		unsigned int eax asm ("eax");
	  register
		unsigned int edx asm ("edx");
	   asm volatile("rdtsc" : "=a" (eax), "=d" (edx));
	  return ((unsigned long)eax) | (((unsigned long)edx) << 32);
}

//ADAPT(now);

int64_t unow()
{
	static auto start=boost::chrono::process_user_cpu_clock::now();
	//static std::chrono::steady_clock::time_point start=std::chrono::steady_clock::now();
	auto nw=boost::chrono::process_user_cpu_clock::now();
	//std::cout << nw << std::endl;
	auto diff=nw - start;

	return diff.count();//boost::chrono::duration_cast<boost::chrono::microseconds>(diff).count();

}

//ADAPT(unow);


void setprecision(int64_t a)
{
	std::cout << std::setprecision(a);
}
ADAPT(setprecision)




//Demonstration of overloading of 3 global functions with the same name 'test', but with different signature
int64_t test()
{
	std::cout << "test():" << std::endl;
	return 78;
}


int64_t test(double a,int64_t i)
{
	std::cout << "test(double,int64_t):" << a << "," << i << std::endl;
	return a+i;
}

//to specify which overload you need to cast
const auto f_test=adapt((int64_t (*)(double ,int64_t ))test);

double test(double a,double b)
{
	std::cout << "test(double,double):"  << a << "," << b << std::endl;
	return a+b;
}

//to specify which overload you need to cast
const auto f_test2=adapt((double (*)(double ,double ))test);

/* Not supported
int64_t test(int64_t a,...)//variadic arguments not working yet
{
	std::cout  << "int64_t test(int64_t ,...):" << std::endl;
	std::cout  << "   val=" << a << std::endl;
	return a+1;
}*/



//Demonstration of various types of functions,Functors,labdas, plain/global/templated,
template<typename T>
int print2args_(double val,T vali)
{
	std::cout << "val=" << val << std::endl;
	std::cout << "vali=" << vali << std::endl;
	return 351;
}
const auto print2args=adapt( print2args_<int64_t>);

struct Printer2Args
{

	int operator()(double val,int64_t vali) const
	{
		std::cout << "vald=" << val << std::endl;
		std::cout << "vali=" << vali << std::endl;
		return 1;
	}

};


const auto print2args_s=adapt( Printer2Args() );

const auto print2args_l=adapt(

		[](double d,int64_t l) -> int{
				std::cout << "lambda:vald=" << d << std::endl;
				std::cout << "lambda:vali=" << l << std::endl;
				return 111;
		}
);

//variadic function with same types arguments  signature "double func(double...)"
inline double sum_double(int64_t count,const int64_t* ptr)
{
    double sum = 0;
    for (int i = 0; i < count; ++i) {
        sum += *(double*)ptr++;
    }
    return sum;
}

const auto f_sum_double=adapt_variadic(sum_double);

//the same thing without wrapping
int64_t sum_double2(int64_t count,const int64_t* ptr)
{
    double sum = 0;
    for (int i = 0; i < count; ++i) {
        sum += *(double*)ptr++;
    }
    return *(int64_t*)&sum;
}

int64_t add_any(int64_t count,const int64_t* ptr,const int64_t* ptr_types)
{
    double sum = 0;
    for (int i = 0; i < count; ++i) {
    	TypeID tp=TO_TYPEID(*ptr_types++);
        sum += (tp==DOUBLE) ? *(double*)ptr++ : (double)*ptr++;//either double or int64_t
    }
    return *(int64_t*)&sum;
}

inline int64_t sum_int64(int64_t count,const int64_t* ptr)
{
    int64_t sum = 0;
    for (int i = 0; i < count; ++i) {
        sum += *ptr++;
    }
    return sum;
}

const auto f_sum_int64=adapt_variadic(sum_int64);

//demonstration of fixed signature prefix and variadic tail
inline double sum_double_from_to(int64_t count,const int64_t* ptr,int64_t from,int64_t to)
{
    double sum = 0;
    if (to<count)
    	count=to;
    ptr+=from;
    for (int i = from; i < count; ++i) {
        sum += *(double*)ptr++;
    }
    return sum;
}

const auto f_sum_double_from_to=adapt_variadic(sum_double_from_to);




void print(int64_t* data,int64_t count,const int64_t* ptr,const int64_t* ptr_types)
{
	TypeID tp;
	double *lhs_dptr=0;
	const int64_t *pci;
	//lhs_i =  *pc++;//count
	//frame_ptr_save=frame_ptr;//
	for (int i=0;i<count;i++)
	{
		tp=TO_TYPEID(*ptr_types++);
		if (tp==DOUBLE)
		{
			lhs_dptr=(double*)(ptr+i);

			double integral;
			if (std::modf(*lhs_dptr, &integral) == 0)
			{
				auto pr=std::cout.precision();
				std::cout << std::fixed << std::setprecision(1) << integral;//always print at least 5.0
				std::cout.precision(pr);
			}
			else {
				std::cout << std::fixed <<  *lhs_dptr;
			}
		}
		else if (tp==INT64)
		{
			pci=ptr+i;
			std::cout << *pci << std::flush;
		}
		else if (tp==STR8)
		{
			std::cout << str8{ptr[i]}.toString()
					//int64_t2str8a(ptr+i)
					<< std::flush;
		}
		else if (tp==CSTRING)
		{
					std::cout << (char*)&data[ptr[i]]
							//int64_t2str8a(ptr+i)
							<< std::flush;
		}
		else if (tp==TYPEID)
		{
			std::cout << TO_TYPEID(ptr[i]).pretty_name()
					//int64_t2str8a(ptr+i)
					<< std::flush;
		}
		else
		{
			std::cout << "(type:" << tp.pretty_name() << ",val:" << *(ptr+i) << ")";
		}

	}
	std::cout << std::endl;
}

//note the variadic part of arguments list is defined by first three parameters here at implementation
// str8 is part of fixed signature and it is at the end
//print_sep(" , ",3,5.7,"text"); " , " will be in s8
void print_sep(int64_t* data,int64_t count,const int64_t* ptr,const int64_t* ptr_types,str8 s8)
{
	TypeID tp;
	double *lhs_dptr=0;
	const int64_t *pci;
	//lhs_i =  *pc++;//count
	//frame_ptr_save=frame_ptr;//
	for (int i=0;i<count;i++)
	{
		tp=TO_TYPEID(*ptr_types++);
		if (tp==DOUBLE)
		{
			lhs_dptr=(double*)(ptr+i);

			double integral;
			if (std::modf(*lhs_dptr, &integral) == 0)
			{
				auto pr=std::cout.precision();
				std::cout << std::fixed << std::setprecision(1) << integral;//always print at least 5.0
				std::cout.precision(pr);
			}
			else {
				std::cout << std::fixed <<  *lhs_dptr;
			}
		}
		else if (tp==INT64)
		{
			pci=ptr+i;
			std::cout << *pci << std::flush;
		}
		else if (tp==STR8)
		{
			std::cout <<  str8{ptr[i]}.toString() << std::flush;
		} else
		{
			std::cout << "(type:" << tp.pretty_name() << ",val:" << *(ptr+i) << ")";
		}

		std::cout << s8.toString();

	}
	std::cout << std::endl;
};

const auto f_print_sep=adapt_variadic_with_types(print_sep);

//double stddev(int64_t count, ...)
//in script: you use  stddev2(32,5.3,2.1,3.2);//32 is fixed compile time part, the rest is run-time variadic part
//note order is reverted here from the one in call site:
double stddev(int64_t count,const int64_t* ptr)
{

    double sum = 0;
    double sum_sq = 0;
    for (int i = 0; i < count; ++i) {
        double num = *(double*)ptr++;
        sum += num;
        sum_sq += num*num;
    }

    return std::sqrt(sum_sq/count - (sum/count)*(sum/count));

}

const auto f_stddev=adapt_variadic(stddev);


double variance(int64_t count,const int64_t* ptr)
{
    double sum = 0;
    double sum_sq = 0;
    for (int i = 0; i < count; ++i) {
        double num = *(double*)ptr++;
        sum += num;
        sum_sq += num*num;
    }
    return sum_sq/count - (sum/count)*(sum/count);
}


const auto f_variance=adapt_variadic(variance);

/*
int64_t typeid_(str8 a)
{
	return *(int64_t*)&getTypeID(a.toString()).type_info();
}
ADAPT(typeid_);*/

int64_t typeid_(int64_t* data,int64_t count,const int64_t* ptr,const int64_t* ptr_types)
{
	return (count>0) ? *ptr_types : 0;
}

const std::map<func_signature,func_ptr> global_functions{

		//registering function with adapter/wrapper to expose desired signature of the used C/C++ function
		FUNC(myplus),

		//registering function with adapter/wrapper to enforce desired signature and exposed to user under name 'plus'
		FUNC_NAME("plus",myplus),

		//FUNC_NAME("typeid",typeid_),

		//here compiler checks signature, it is verbose ,but safe as long as the implementation of myplus_raw respects registered signature
		// note that you don't use adapter so you need to reinterpret return value of type double to int64_t type
		// and input arguments from int64_t type
		//C signature: double add2(double,double);
		{{"add2",{DOUBLE,DOUBLE},DOUBLE,fixed_signature},{(void*)myplus_raw}},

		//the same as above for variable number of the arguments of the same type 'double'
		//and return value double, see also sum_alt bellow,
		// note that you don't use adapter so you need to reinterpret return value of type double to int64_t type
		// signature: double addN(double...)
		{{"addN",{DOUBLE},DOUBLE,varcount},{(void*)sum_double2} },

		//very useful function, demonstration of passing array of types from program to C
		//to display content of array of various types correctly
		{{"print",{},VOID,pass_types},{(void*)print} },

		//demonstration of matching and calling function with fixed signature prefix and with variadic arguments tail.
		//here we want to pass in the first argument a separator of type str8  to display either comma or space between variadic values
		{{"print_sep",{STR8},VOID,pass_types},{(void*)entry_void_variadic_with_types<f_print_sep>}},
		//{regfunc("print_sep",static_cast<void(*)(str8)>(0),entry_void_variadic_with_types<f_print_sep>,pass_types)},

		{{"typeid",{},TYPEID,pass_types},{(void*)typeid_}},

		//adds all double and int arguments interleave and retuns as double
		{{"add_any",{},DOUBLE,pass_types},{(void*)add_any} },

		//this is unsafe, as compiler does not check the type and count of passed arguments, this exposes application stack directly
		//compiler will be happy when you pass by mistake one argument only or int64_t, second argument is return type
		FUNC_UNCHECKED(myplus_raw,double),

		//exposing unsafe function under different name
		FUNC_NAME_UNCHECKED("add",myplus_raw,double),

		// registering global functions with signature  void f(void)  don't need wrapper/adapter, speed up execution
		FUNC_VOID_NO_ARGS(do_something),

		// global functions with signature  int64_t f(void)  don't need wrapper/adapter, speed up execution
		FUNC_NO_ARGS(now),
		// exposing the same function under different name
		FUNC_NAME_NO_ARGS("now_nano",now),
		FUNC_NO_ARGS(tsc),
		FUNC_NO_ARGS(unow),

		//no return value
		FUNC_VOID(setprecision),


		//  overloading 3 global functions with the same name test, but with different signature
		FUNC_NO_ARGS(test),
		//third argument/function is called,
		//the second argument/function  is never called directly and it is always used just to detect
		// and register the function signature only, so we can safely pass 0
		{ regfunc("test",static_cast<int64_t(*)(double,int64_t)>(0),entry<f_test>) },
		{ regfunc("test",static_cast<double(*)(double,double)>(0),entry<f_test2>) },


		//demo of adapting and registering global function with exposed name different with implementation name
		{ regfunc("print2args",print2args_<int64_t>,entry<print2args>) },

		//demo of registering Functor, note the signature is taken from operator()
		{ regfunc("print2args_s",Printer2Args(),entry<print2args_s>) },

		//demo of registering Functor 2, not instantiating and passing functor, passing null function pointer to convey function signature
		{ regfunc("print2args_s2",static_cast<int64_t(*)(double,int64_t)>(0),entry<print2args_s>) },

		//demo of registering lambda passing null, function pointer to convey function signature
		{ regfunc("print2args_l",static_cast<int64_t(*)(double,int64_t)>(0),entry<print2args_l>) },

		//demo for overloading function name 'sum' for types double and int64, also demo for variadic argument counts
		{ regfunc("sum",static_cast<double(*)(double)>(0),entry_variadic<f_sum_double>,varcount) },
		{ regfunc("sum",static_cast<int64_t(*)(int64_t)>(0),entry_variadic<f_sum_int64>,varcount) },

		//alternativelly variadic functions with no fixed function signature prefix you can call without wrapper/adapter
		//with return value adapted inside function
		{ regfunc("sum_alt",static_cast<double(*)(double)>(0),sum_double2,varcount) },

		//demonstration of fixed signature prefix and arbitrary number of
		//arguments all of the same type 'double', first two arguments  passed are from and to indexes
		{ regfunc("sum",static_cast<double(*)(int64_t,int64_t,double)>(0),entry_variadic<f_sum_double_from_to>,varcount) },

		//what is preference?  varargs, varcount?
		{ regfunc("stddev",static_cast<double(*)()>(0),entry_variadic<f_stddev>,varargs) },
		{ regfunc("stddev",static_cast<double(*)(double)>(0),entry_variadic<f_stddev>,varcount) },
		{ regfunc("variance",static_cast<double(*)(double)>(0),entry_variadic<f_variance>,varcount) }

};

extern std::string global_signatures_with_name(const std::string& fname)
{
	std::string out;
	for(const auto& func:global_functions)
	{
		if (func.first.name==fname)
		{
			out+="\n  candidate:"+func.first.signature();
		}
	}
	return out;
}

