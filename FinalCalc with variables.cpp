/*Program evaluates mathematical expressions using the following as input:
[( , ), *, /, +, -] and all real numbers.


This program works by parsing input specifically chars read from cin and converting it into 'Tokens'. The Tokens 
are then subjected to the mathematical grammar shown below:

Expression:
	Term
	Expression + Term
	Expression - Term
	
Term:
	Primary
	Term * Primary
	Term / Primary
	
Primary:
	Number
	+Number
	-Number
	'(' Expression ')'
	
Number:
	Floating-point literal
	

*/


#include "std_lib_facilities.h"


//Token replaces all input read from cin
//using a (type,value) abstraction
struct Token{
	public:
		//data members
		char type;	   //specifies the type of Token: used to distinguish between numbers, and other Tokens
		double value;  //value of the token: for numbers
		string varName; //variable name: stores a variable name defined by user during runtime
	
		//Constructors
		Token(char ch) 			 	:type(ch){}
		Token(char ch, double val)  :type(ch), value(val){}
		Token(char ch, string name)	:type(ch), varName(name){}
};

//Variable stores the variable(name,value) defined by the user (i.e 'x=1;'); during runtime
struct Variable{
	public:
		//Constructors
		double value;
		string name;
		
		//Constructors
		Variable(string str, double val) :name(str),value(val){}
};

//provides tools to handle extraction of Tokens from cin
class Token_Stream{
	public:
		//Constructors
		Token_Stream() :buffer('0'), full(false){}
		
		//Constructors
		Token get();    // get a token from cin
		void putback(Token tok); // putback a token in stream buffer (similar to std::cin.putback())
		void clear(); // clears the full flag allowing a new putback() token to overwrite the current token buffer
		
	private:
		Token buffer;  //where a token is stored when putback(t) is used
		bool full;	//flag to indicate a token was putback() in the buffer
};





//------------------GLOBAL VARIABLE(S)-----------------------
Token_Stream ts; //prepare a stream for reading and handling Tokens
const char Number = '#'; //specifies a Number Token
const char Quit = 'q'; //specifies a Quit Token
const char Enter = ';'; //...
const char Name = 'n';
const char Decl = '$';
const string Prompt = "> "; //used to indicate to user, "require input"
const string Declare = "let"; //specifies the user wants to define a variable
vector<Variable>var_table;// storage for the user-defined variables



//------------------FUNCTION PROTOTYPE(S)-----------------------
//Note: See definitions for descriptions below
double expression();
double primary();
double term();
void calculate();
void clear_up_mess();
void welcome();
string declaration();
string statement();
bool is_declared(string str);
double get_var(string str);
double define_name(string var, double val);



//------------------MAIN-----------------------
int main()
try{
	welcome();
	calculate();
	return 0;
}
catch(...){
	cerr <<"Unknown error" <<endl;
	return 2;
}

//------------------FUNCTION DEFINITION(S)-----------------------

//handles the error handling and prompt
void calculate(){
	
	define_name("pi",3.1415926535);
	define_name("e" ,2.7182818284);
	
	while (true) try{
			cout << Prompt;
			Token t = ts.get();
			if (t.type == Quit){
				cout <<"******************************" <<endl;
				cout << "Program Terminated by user" << endl;
				return;
			}
			ts.putback(t);
			cout << statement() << endl;
			ts.clear(); // expression() attempts to read buffer if it is still full instead of waiting for new input	
		}
		catch(exception& errorString ){
			cerr << "Error: "<< errorString.what() << endl;
			clear_up_mess();
			//clear_up_mess()
		}
	
}

//puts token back into the stream 
void Token_Stream::putback(Token tok){ 
	if(full) error("Buffer Full");
	full = true;
	buffer = tok;
}


void Token_Stream::clear(){
	full = false;
}

//handles all input as Tokens
Token Token_Stream::get(){
	
	if (full){
		full = false;
		return buffer;
	}
	
	char ch;
	cin >> ch;
	if (!cin) error("Token_Stream::get() bad input");
	
	//handle strings i.e variable names, declarations
	if(isalpha(ch)){
		string str;
		str+=ch;
		while(cin.get(ch)){
			if(isalpha(ch)){
				str+=ch;
			}else{
				cin.putback(ch);
				if(str.length()<=1){if (str == "q") return Token(Quit);}
				if (str == Declare) return Token(Decl);
				return Token(Name,str);
			}
		}
	}
	//handle non-strings i.e mathematical expressions
	else{
		//distinguish between operator and number
		switch(ch){
			case Quit: case Enter:
				return Token(ch);
				break;
			case '=':
				return Token('=');
			case '+': case '-': case '*': case '/': case '(': case ')':
				return Token(ch);
				break;
			case '.': case '0': case '1': case '2': case '3': case '4': case '5': case'6': case '7': case '8': case '9':
			{
				cin.unget();
				double val = 0;
				cin >> val;
				if (!cin) error("Token_Stream::get() expected number");
				return Token(Number,val);
			}
			default:
				error("Token_Stream::get() Default");
				break;
		}
	}
}


//handles all real numbers(+/-) as well as parenthesis '(',')'
double primary(){
	
	Token t = ts.get();
	switch(t.type){
		case Name:
			if(is_declared(t.varName)) return get_var(t.varName);
			error("Undefined variable: '"+t.varName+"'");
		case Number:
			return t.value;
		case '-':
			t = ts.get();
			if (t.type != Number){
				ts.putback(t);
				error("expected number after '-'");
			}
			return -t.value; // negative number
		case '+': // this case is unnecessary but maintains (+/-)Num consistency
			t = ts.get();
			if (t.type != Number){
				ts.putback(t);
				error("expected number after '+'");
			}
			return t.value; // negative number		
		case '(':
		{
			double left = expression();
			t = ts.get();
			if (t.type != ')') error("Expected close ')'");
			
			return left;
		}
		default: 
			error("Expected Number or '('");
	}
}

//handles * and / from left to right
double term(){
	
	double left = primary(); // read number or '('
	
	Token t = ts.get(); // read operator '*' or '/' 
	while(true){
		switch(t.type){
			case '*': case '(':
				if (t.type == '(') ts.putback(t); // allows following terms to be evaluated: '4(...' and '4*(...' to be evaluated
				
				left*=primary();
				t = ts.get(); // read operator '*' or '/'
				break;
			case '/':
			{
				t = ts.get(); // grab next token (expected number or Quit)
				if (t.value == 0) error("Division by Zero");
				else ts.putback(t); // put token back to be evaluated
				
				left/=primary();
				t = ts.get(); // read operator '*' or '/'
				break;
			}
			default:
				ts.putback(t);
				return left;
		}
	}
}

//handles addition & subtraction
double expression(){
	
	double left = term(); // read first term (i.e x in x+...+z)
	
	Token t = ts.get(); // read operator '+' or '-'
	while(true){
		switch(t.type){
			case '+':
				left+=term();
				t = ts.get(); // read operator '+' or '-'
				break;
			case '-':
				left-=term();
				t = ts.get(); // read operator '+' or '-'
				break;
			default:
				
				ts.putback(t);
				return left;
		}
	}
}

//clears cin and ts streams for new input
void clear_up_mess(){
	ts.clear(); // clear Token_Stream::buffer
	cin.ignore(50,'\n'); // clear std::cin
	
	//Note: Bjarne's version, used cin>>ch to eat chars(i.e. clearing the stream), but this suffered an issue in the case 
	//when there was already nothing in cin after an error occurred (i.e "-;" causes this), cin would then sit waiting for 
	//something to eat until the user entered ';'
}

//displays introduction prompt
void welcome(){
	cout <<"Welcome to CalcV1.0" << endl;
	cout <<"Revised by Brandon A. (Last Updated: 2019/03/02)" <<endl;
	cout << "Notes: Program is based on Bjarne S. Calculator's Program found in his book:\n\"Programming: Principles & Practive using C++\"" <<endl;
	cout <<"-------------------------------------------------------------" <<endl;
	cout <<"Enter:\n";
	cout <<"'q' to QUIT\n";
	cout <<"';' to submit an expression\n";
	cout <<"Ex: > -1(-1+2)*3; \n";
	cout <<"-------------------------------------------------------------" <<endl;
}

//handles user declarations
string declaration(){

	Token t=ts.get();
	if(t.type == Name){
		string varName = t.varName;
		t = ts.get();
		if (t.type == '='){
			double varValue = expression();
			t=ts.get();
			if (t.type == Enter){
				define_name(varName, varValue);//store var name and value;
				stringstream strs;
				strs << varValue;
				return "Defined: " + varName +"="+ strs.str(); 
			}
			else error("Expected ';'");	
		}
		else error("Expected '='");
	}
	else error("Expected 'variable name'");
}

//distinguish between declaration or expression
string statement(){
	Token t = ts.get();
	
	switch(t.type){
		case Decl: //user is declaring a variable
			return declaration();
			//cout <<declaration() <<endl;
			//return "Declared";
			break;
		default: //user is specifying an expression to evaluate
			ts.putback(t);
			stringstream strs;
			strs << expression();
			return strs.str();
	}
}

//check if var exists in table
bool is_declared(string str){
	for(int i=0; i<var_table.size(); ++i){
		if(var_table[i].name==str) return true;
	}
	return false;
}

//retrieve variable from table
double get_var(string str){
	for(int i=0; i<var_table.size(); ++i){
		if(var_table[i].name==str) return var_table[i].value;
	}
}

//add variable to var_table
double define_name(string var, double val){
    if (is_declared(var)) error(var," declared twice");
    var_table.push_back(Variable(var,val));
    return val;
}