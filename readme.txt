
This project is a customizable,  lightweight and fast C like general purpose virtual machine and scripting language with 
fast binding/calls to C/C++. It is a bytecode interpreter, and the speed gain over CPython comes from typed instructions and no object boxing.

For build see build.txt.
For examples see files examples/*.cvm

All four are now measured pinned to the same P-core:
Time per iteration: C about 2 ns, Numba about 5.5 ns, cvm about 29 ns (median of 7), CPython about 87 ns (median of 9).
cvm is about 3x faster than CPython and about 15x slower than compiled C. 

				ns/iteration				vs C				vs CPython
C				about 1.9 (1.7 to 2.2)		1x					about 46x faster
Numba @njit		about 5.5 (5.4 to 5.7)		about 3x slower		about 16x faster
cvm				about 29 median (24 to 34)	about 15x slower	about 3x faster
CPython 3.13	about 87 median (82 to 90)	about 46x slower	1x


Note:
This work uses computed goto with gcc compiler, Visual Studio compiler msvc lacks computed goto so there is a 30% slower 
alternative solution using switch/branching in virtual machine. It should work on Microsoft Visual Studio , but it was not yet tested.
The table above is for cvm using  computed goto in virtual machine(about 1.5x faster than using a switch, from 30% fewer instructions, measured with perf stat)
So for Microsoft Visual Studio compiler msvc you expect cvm about 2 time faster than CPython.

Project tested on GNU/Linux/ubuntu 18.04.01 LTS (bionic) with 64 bit g++ (Ubuntu 7.3.0-16ubuntu3) 7.3.0 
and debian/trixie GNU/Linux 13.
Benchmark: bisection for sqrt(7), 3.1M inner iterations, pinned to a P-core (taskset -c 2), GCC 14.2, Python 3.13. 
Retested for timing in Nov 2026 on commodity laptop (with CPU 13th Gen Intel(R) Core(TM) i7-1355U)
with 5GHz P core:
(stocks) marian@linlap:~/devel/cvm/cvm/examples$  lscpu --extended 
CPU NODE SOCKET CORE L1d:L1i:L2:L3 ONLINE    MAXMHZ   MINMHZ       MHZ
  0    0      0    0 0:0:0:0          yes 5000.0000 400.0000  798.8510
  1    0      0    0 0:0:0:0          yes 5000.0000 400.0000 1386.8960
  2    0      0    1 4:4:1:0          yes 5000.0000 400.0000  757.6870
  3    0      0    1 4:4:1:0          yes 5000.0000 400.0000 1266.4130
  4    0      0    2 8:8:2:0          yes 3700.0000 400.0000 1299.0150
  5    0      0    3 9:9:2:0          yes 3700.0000 400.0000 1339.2600
  6    0      0    4 10:10:2:0        yes 3700.0000 400.0000 1323.8320
  7    0      0    5 11:11:2:0        yes 3700.0000 400.0000 1275.5150
  8    0      0    6 12:12:3:0        yes 3700.0000 400.0000 1208.8070
  9    0      0    7 13:13:3:0        yes 3700.0000 400.0000 1208.7830
 10    0      0    8 14:14:3:0        yes 3700.0000 400.0000 1160.9700
 11    0      0    9 15:15:3:0        yes 3700.0000 400.0000 1139.7660





Motivation ,usage and design goal:

  1) Demonstrating the parsing of a 'complex' language with X3 spirit and demonstrating compilaton and Virtual machine design.
  
  2) Demonstrating function recursions as a teaching tool for students in a simplistic language
  
  3) Main usage 1:
    Fast general purpose numerical calculations VM language (see examples/bisect_sqrt7_func.cvm)
     3 times Faster and safer than general purpose scripting languages python,lua (and C#??, not compared yet) for numeric algorithms, 
    
  
  4) Main usage 2: 
    Program in this language can be treated as an advanced and safe configuration file (master high level program) for other projects written in C/C++ , as this language can call C/C++ functions. Also it can be a plugin/module in master program written in C/C++ as this 'scriptable' can be called from C/C++ applications.

    Big complex C/C++ application software often have an expensive/lengthy build time (hours,days) and deployment. To avoid such an ordeal with any change/iteration/bug fix it is desirable to have a configurable/scriptable plugin for a frequently changed part. Any scripting language such as python and lua would fit for this purpose. When there is an additional need for a tight control of the security and execution speed for numerical processing as often is in finance industry then those languages are too slow and too general/complex and hence too risky.
    This project tries to solve the problem by offering numerically fast lightweight simplistic language with configurable security properties.
     
    You can disable powerfull but dangerous parts of this language such as unbounded 'while' loops (that can take unlimited time) and user defined (recursive) functions (that can overflow stack) or forward functions declarations (potential for unbound calling circles),  and keep 'safe' bounded execution of a well chosen set of global functions pre-implemented in C/C++;
    
    The flexibility and configuration power comes from having expressions,nested functions 
    (either user defined or implemented in C/C++) and conditional execution.(if else)
    
    In some application high level modules/entry points pre-implemented C/C++ application functions 
    can be chained up arbitrarily (by nested functions) after laborious/costly deployment by the advanced user of this applications and this language.
    Example:
    
    var0=0;
    var1=func1(func2(var2,var3),func3(a,b));
    var2=func3(var4,var6);
    if (var0)
        var3=something;
    else
        var3=something_else;
    

How does it work?
Source is compiled behind the scenes once into custom byte code, then optionally optimized (switch -o) 
        and immediately interpreted/executed in custom VM. Overall from outside 
        it behaves like a scripting language without exposing intermediate state of 'byte code' to user.
        
Limitation: untested on windows , but should work on windows too, tested on debian (Nov 2026),  compler gcc 14.2.0 
            Uses computed goto in C++ in virtual machine: "goto *&&label"; which is supported by compiler gcc,clang,icc but not in microsoft msvc)
            fallback with switch, that allows to get it working on msvc compiler

Speed (excluding compiling part) 
Achieved 3x speed up over python and lua for one numerical algorithm 'sqrt' 
 

Language Features:

1) Comments
like this documentation
a=5;//one line comment like this

2) Supported types:
int( same as int64) , double, str8 (string 8 characters or shorter)
no pointers , no objects, no dynamic memory allocation,

3) Variables 
are automatically declared on the first assignement by default:

count=5;//int64
eps=0.00001;//double

but this feature can be disabled in command line and then you have to 'declare' them like :

auto count=5;
auto eps=0.0000001;

or

var count=5;
var eps=0.0000001;

4) Expressions:
over the types double and int for operators +,-,*,/ and ( )
bool expressions: &&,||


5) Conditional execution:

if (bool_expresson)
    statement;
or
if (bool_expression)
        statement;
else
        statement;
         
6) Unbounded loop: 
    Note: This feature can be disabled if needed for security reasons:
   while (expression)
   {
        statements;
   }

   
7) global User defined functions (UDF) with name overloading by their signature
 Note: This feature can be disabled if needed for security reasons:
func test(double a,int 5) -> double
{
    return (0.4);//unlike C , in this language braces are mandatory
}

func test(double a) -> double
{
    return (0.5);
}

8) For safety by default the compilation of the arithmetic and logical expressions with mixed types 
(double, int)  will fail.
You can use cast<T>(expr) for conversion between arithmetic types int and double like that
for example for casting variable a of double type to int   cast<int>(a)
    
9) Fixed size arrays/lists supported. (see examples/list.cvm). Tuples with different element types not yet supported.
	a={5.6,7.0,3.2};
	a[2]=2.1;
	
10) You can extend or security limit bindings to C/C++ in global_funcs.cpp. 
This file should be best placed in your application that is using this module. You can implement 
security policy by defining sets of allowed functions.


11) Future work will probably be to support tuples and make type system more robust. 
    Possibly supporting runtime type .

