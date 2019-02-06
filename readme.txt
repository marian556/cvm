
This project is a customizable,  lightweight and fast C like general purpose virtual machine and scripting language with 
fast binding/calls to C/C++.

For build see build.txt.
For examples see files examples/*.cvm

Note: This is an experimental work in progress project that in current state will not work on Windows with Visual Studio 
compiler due to lack of "calculated goto" language feature in windows compiler. 
Only tested on GNU/Linux/ubuntu 18.04.01 LTS (bionic) with 64 bit g++ (Ubuntu 7.3.0-16ubuntu3) 7.3.0


Motivation ,usage and design goal:

  1) Demonstrating the parsing of a 'complex' language with X3 spirit and demonstrating compilaton and Virtual machine design.
  
  2) Demonstrating function recursions as a teaching tool for students in a simplistic language
  
  3) Main usage 1:
    Fast general purpose numerical calculations VM language (see examples/bisec_sqrt7_func.cvm)
    15x Faster and safer than general purpose scripting languages python,lua (and C#??, not compared yet) for numeric algorithms, 
    only twice as slow as the same routine compiled in C
  
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
        
Limitation: untested on windows and likely does not work on windows, only Linux/ubuntu
            Uses calculated goto in C++ in virtual machine: "goto *&&label"; which is supported by compiler gcc,clang,icc but not in microsoft mvc)

Speed (excluding compiling part) 
Achieved 15x speed up over python and lua for one numerical algorithm 'sqrt' bellow (0.45 seconds for cscript vs 7.5  seconds for python or lua)
 

Language Features:

1) Comments
like this documentation
a=5;//one line comment like this

2) Supported types:
int( same as int64) , double, str8 (string 8 characters or shorter)
no pointers and arrays yet, no objects, no dynamic memory allocation,

3) Variables 
are autodeclared on the first assignemnt by default:

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
This file should be best placed in your application that is using this module.. You can implement 
security policy by defining sets of allowed functions.


11) Future work will probably be to support tuples and make type system more robust. 
    Possibly supporting runttime type . Adding support for microsoft compiler 
    workaround for lacking 'calculated goto' in vm.

