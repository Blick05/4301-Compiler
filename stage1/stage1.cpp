// CJ BLICK
// CS 4301
// Stage 1

// Tested this out only 110, 114(;found +) and 120 have issues not sure how to fix it whitespace issues and space issues
#include <stage1.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <string>
#include <map>
#include <iomanip>
#include <utility>
#include <ctime>

using namespace std;
string lhs, rhs, sign, temp, label1, label2;
//static varaibles
static char prevCh;
static int intCounter = 0;
static int boolCounter = 0;
static int progCounter = 0;
static int labelCounter = 0;

string Compiler::genInternalName(storeTypes stype) const{ // $
	if (stype == INTEGER) {
		intCounter++;
		return "I" + to_string(intCounter - 1);
	} else if (stype == BOOLEAN) {
		boolCounter++;
		return "B" + to_string(boolCounter - 1);
	} else if (stype == PROG_NAME) {
		progCounter++;
		return "P" + to_string(progCounter - 1);
	}
	return "Error"; // could add an error case 
}
/*
Comments
$ = Good To Go
@ = Still needs work

*/
//Constructor
Compiler::Compiler(char** argv) // Should be good, it takes the arguments ex ./stage0 001.dat 001.lst 001.asm $
{
	sourceFile.open(argv[1]);
	listingFile.open(argv[2]);
	objectFile.open(argv[3]);
}

//Destructor
Compiler::~Compiler() //closes it Should be Good $
{
	sourceFile.close();
	listingFile.close();
	objectFile.close();
}

void Compiler::createListingHeader() // $
{
	time_t now = time(NULL);
	listingFile << "STAGE0:  " << "CJ BLICK       " << ctime(&now) << endl;
	listingFile << "LINE NO." << setw(30) << "SOURCE STATEMENT" << endl;
}

void Compiler::parser() // $
{
	lineNo++;
	listingFile << endl << setw(5) << lineNo << '|' ; // need to setup the first line number 
	nextChar(); // start
	if (nextToken() != "program"){
		processError("keyword \"program\" expected");
	}
	prog();
}

void Compiler::createListingTrailer() // $
{
	listingFile << endl << endl;
	if (errorCount == 1) {
		listingFile << "COMPILATION TERMINATED      " << errorCount << " ERROR ENCOUNTERED" << endl; // if only one or zero error encountered 
	} else {
		listingFile << "COMPILATION TERMINATED      " << errorCount << " ERRORS ENCOUNTERED" << endl; // others
	}
}

void Compiler::processError(string err) // $
{
	
	listingFile << endl << "Error: Line " << lineNo << ": " << err;
	errorCount++;
	createListingTrailer();
	exit(EXIT_FAILURE); // exit(EXIT_FAILURE);
}

//prog() - production 1
void Compiler::prog() //token should be "program" Should be good! $
{ 
	if (token != "program") {
		processError("keyword \"program\" expected");
	}
	progStmt();
	if (token == "const") {
		consts();
	}
	if (token == "var") {
		vars();
	}
	if (token != "begin") {
		processError("keyword \"begin\" expected");
	}
	beginEndStmt();
	if (token != "$") {
		processError("no text may follow \"end\"");
	}
}

//progStmt() - production 2
void Compiler::progStmt() //token should be "program" $
{ 
	string nt;
	if (token != "program") { //
		processError("keyword \"program\" expected");
	}
	nt = nextToken();
	if (!isNonKeyId(token)) { // replaced token != NON_KEY_ID 
		processError("program name expected");
	}
	if (nextToken() != ";") {
		processError("semicolon expected");
	}
	nextToken(); // ?
	if (nt.length() > 15){
		nt = nt.substr(0,15);
	}
	code("program", nt);
	insert(nt,PROG_NAME,CONSTANT,nt,NO,0);
}

// consts() - production 3
void Compiler::consts() //token should be "const" $
{ 
	if (token != "const") { // prob dont need this already
		processError("keyword \"const\" expected");
	}
	if (!isNonKeyId(nextToken())) {// replaced nextToken() is not a NON_KEY_ID 
		processError("non-keyword identifier must follow \"const\"");
	}
	constStmts();
}

// vars() - production 4
void Compiler::vars() //token should be "var" $
{ 
	if (token != "var") {
		processError("keyword \"var\" expected");
	}
	if (!(isNonKeyId(nextToken()))) { // replaced nextToken() is not a NON_KEY_ID
		processError("non-keyword identifier must follow \"var\"");
	}
	varStmts();
}

// beginEndStmt() - production 5
void Compiler::beginEndStmt() //token should be "begin" $
{ 
	if (token != "begin") {
		processError("keyword \"begin\" expected");
	}
	execStmts();
	if (token != "end") {
		processError("non_key_id, \"read\", or \"write\" expected");
		//processError("keyword \"end\" expected");
	}
	if (nextToken() != ".") {
		processError("period expected");
	}
	nextToken();
	code("end", ".");
}

// constStmts() - production 6
void Compiler::constStmts() //token should be NON_KEY_ID $
{ 
	string x,y, temp;
	if (!(isNonKeyId(token))){ // replaced token is not a NON_KEY_ID 
		processError("non-keyword identifier expected");
	}
	x = token;
	if (nextToken() != "=") {
		processError("\"=\" expected");
	}
	y = nextToken();
	if (y != "+" && y != "-" && y != "not" && !isNonKeyId(y) && !isBoolean(y) && !isInteger(y)) { // integer could be changed if there was a problem to isInteger(string s)
	// y is not one of "+","-","not",NON_KEY_ID,"true","false",INTEGER
		processError("token to right of \"=\" illegal");
	}
	if (y == "+" || y == "-") // does the +123123 or -123123
	{
		if (!(isInteger(nextToken()))) { // replaced nextToken() is not an INTEGER 
			processError("integer expected after sign");
		}
		y = y + token;
	}
	if (y == "not") { // does the not true or not false, asd=true; or not asd
		temp = nextToken();
		if (whichType(temp) != BOOLEAN && !isBoolean(temp)) { // replaced nextToken() is not a BOOLEAN checks if temp is acutally a boolean
			processError("boolean expected after “not”");
		}
		if (whichValue(temp) == "true" || token == "true" ) { // if temp is a const it checks if it is either true or false;
			y = "false";
		}
		else {
			y = "true";
		}
	}
	if (nextToken() != ";") {
		processError("semicolon expected");
	}
	if (whichType(y) != INTEGER && whichType(y) != BOOLEAN) { // the data type of y is not INTEGER or BOOLEAN
		processError("data type of token on the right-hand side must be INTEGER or BOOLEAN");
	}
	// may need to add if statement to make sure if y is an nonkeyid make sure it is being used
	insert(x,whichType(y),CONSTANT,whichValue(y),YES,1);
	x = nextToken();
	if (x != "begin" && x != "var" && !(isNonKeyId(x))) { // replaced x is not one of "begin","var",NON_KEY_ID
		processError("non-keyword identifier, \"begin\", or \"var\" expected");
	}
	if (isNonKeyId(x)) { // replaced x is a NON_KEY_ID 
		constStmts();
	}
}

// varStmts() - production 7
void Compiler::varStmts() //token should be NON_KEY_ID $
{ 
	string x,y;
	string temp;
	if (!(isNonKeyId(token))) { // replaced token is not a NON_KEY_ID
		processError("non-keyword identifier expected");
	}
	x = ids(); // gets the variable separated by ,
	if (token != ":") {
		processError("\":\" expected");
	}
	temp = nextToken();
	if (temp != "integer" && temp != "boolean") { // replaced nextToken() is not one of "integer","boolean"
		processError("illegal type follows \":\"");
	}
	y = token;
	if (nextToken() != ";") {
		processError("semicolon expected");
	}
	insert(x,whichType(y),VARIABLE,"",YES,1);
	temp = nextToken();
	if (!(isNonKeyId(temp)) && temp != "begin") { // replaced nextToken() is not one of "begin",NON_KEY_ID
		processError("non-keyword identifier or \"begin\" expected");
	}
	if (isNonKeyId(temp)) { // replaced token is a NON_KEY_ID
		varStmts();
	}
}

// ids() - production 8
string Compiler::ids() //token should be NON_KEY_ID $
{ 
	string temp,tempString;
	if (!(isNonKeyId(token))) { // replaced token is not a NON_KEY_ID
		processError("non-keyword identifier expected");
	}
	tempString = token;
	temp = token;
	if (nextToken() == ",") {
		if (!(isNonKeyId(nextToken()))) { // replaced nextToken() is not a NON_KEY_ID
			processError("non-keyword identifier expected");
		}
		tempString = temp + "," + ids();
	}
	return tempString;
}

// Action Routines
//insert()
 //create symbol table entry for each identifier in list of external names
 //Multiply inserted names are illegal
void Compiler::insert(string externalName,storeTypes inType, modes inMode, string inValue, allocation inAlloc, int inUnits)
{
	string name;
	// string tempName;
	while (!externalName.empty())// name broken from list of external names and put into name != ""
	{
		int comma = externalName.find(','); // variables can be separated by , will output -1 if there is no comma otherwise it gets the first comma
		
		if (comma == -1){ //
			name = externalName;
			externalName.clear();
		}
		else {
			name = externalName.substr(0, comma);
			externalName = externalName.substr(comma + 1, externalName.length());
		}
		if (name.length() > 15){
				name = name.substr(0,15);
		}
		if (symbolTable.find(name) != symbolTable.end()) { // replaced symbolTable[name] is defined
			processError("multiple name definition");
		}
		else if (isKeyword(name)) { //replaced name is a keyword
			processError("illegal use of keyword");
		}
		else { //create table entry
			if (symbolTable.size() >= 256)
				processError("symbolTable overflow -- max 256 entries ");
				
			if (isupper(name[0])) { // name begins with uppercase
				symbolTable.insert(pair<string, SymbolTableEntry>(name, SymbolTableEntry(name,inType,inMode,inValue,inAlloc,inUnits)));
			}
			else {
				symbolTable.insert(pair<string, SymbolTableEntry>(name,SymbolTableEntry(genInternalName(inType),inType,inMode,inValue,inAlloc,inUnits)));
			}
		}
	}
}

// whichType()
storeTypes Compiler::whichType(string name) //tells which data type a name has
{ 
	storeTypes dataType;

	if (name == "integer")
	{
		dataType = INTEGER;
		return dataType;
	}

	if (name == "boolean")
	{
		dataType = BOOLEAN;
		return dataType;
	}
	
	if (isLiteral(name)) { // name is a literal
		if (isBoolean(name)) {
			dataType = BOOLEAN;
		}
		else {
			dataType = INTEGER;
		}
	}
	else { //name is an identifier and hopefully a constant
		if (symbolTable.find(name) != symbolTable.end()) { // symbolTable[name] is defined
			dataType = symbolTable.at(name).getDataType(); // idk why but this does work maybe there is a much simpler one oh whale // there was a simpler one 
		}
		else {
			processError("reference to undefined symbol " + name);
		}
	}
	return dataType;
}

// whichValue()
string Compiler::whichValue(string name) //tells which value a name has
{ 
	string value;
	
	if (isLiteral(name)) {
		value = name;
	}
	else {//name is an identifier and hopefully a constant
		if (symbolTable.find(name) != symbolTable.end()) { // symbolTable[name] is defined and has a value // MAYBE WRONG !!! ??? // now right 
				value = symbolTable.at(name).getValue();  
		}
		else {
			processError("reference to undefined symbol " + name);
		}
	}
	return value;
}

// code()
void Compiler::code(string op, string operand1, string operand2)
{
	// currently there are only two emit statement that stage 0 can go through
	if (op == "program")
		emitPrologue(operand1);
	else if (op == "read")
		emitReadCode(operand1);
	else if (op == "write")
		emitWriteCode(operand1);
	else if (op == "+") // this must be binary '+'
		emitAdditionCode(operand1,operand2);
	else if (op == "-") // this must be binary '-'
		emitSubtractionCode(operand1,operand2);
	else if (op == "neg") // this must be unary '-'
		emitNegationCode(operand1);
	else if (op == "not")
		emitNotCode(operand1);
	else if (op == "*")
		emitMultiplicationCode(operand1,operand2);
	else if (op == "div")
		emitDivisionCode(operand1,operand2);
	else if (op == "mod")
		emitModuloCode(operand1,operand2);
	else if (op == "or")
		emitOrCode(operand1,operand2);
	else if (op == "=")
		emitEqualityCode(operand1,operand2);
	else if (op == ":=")
		emitAssignCode(operand1,operand2);
	else if (op == "<>")
		emitInequalityCode(operand1,operand2);
	else if (op == "<")
		emitLessThanCode(operand1,operand2);
	else if (op == "<=")
		emitLessThanOrEqualToCode(operand1,operand2);
	else if (op == ">")
		emitGreaterThanCode(operand1,operand2);
	else if (op == ">=")
		emitGreaterThanOrEqualToCode(operand1,operand2);
	else if (op == "and")
		emitAndCode(operand1,operand2);
	else if (op == "end") 
		emitEpilogue();
	else
		processError("compiler error since function code should not be called with illegal arguments");
}

// emit()
void Compiler::emit(string label, string instruction, string operands, string comment)
{
	// Turn on left justification in objectFile
    objectFile << left;

    // Output label in a field of width 8
    objectFile << setw(8) << label;

    // Output instruction in a field of width 8
    objectFile << setw(8) << instruction;

    // Output the operands in a field of width 24
    objectFile << setw(24) << operands;

    // Output the comment
    objectFile << comment << endl;
}

// emitPrologue()
void Compiler::emitPrologue(string progName, string operand2)
{
	// Output identifying comments at the beginning of objectFile
    //objectFile << ";" << operand2 << endl; // probably not finished ???!!!
	
	time_t now = time(NULL);
	
	// listingFile << "STAGE0:  " << "CJ BLICK\t" << ctime(&now) << endl; // reference
	objectFile << "; CJ BLICK\t" << ctime(&now);


    // Output the %INCLUDE directives

    objectFile << "%INCLUDE \"Along32.inc\"" << endl;
    objectFile << "%INCLUDE \"Macros_Along.inc\"" << endl << endl;

    emit("SECTION", ".text", "", "");
    emit("global", "_start", "", "; program " + progName);
	objectFile << endl;
    emit("_start:", "", "", "");
}

// emitEpilogue()
void Compiler::emitEpilogue(string operand1, string operand2)
{
	emit("","Exit", "{0}");
	objectFile << endl;
	emitStorage();
}

// emitStorage()
void Compiler::emitStorage()
{
	string external, name, value, comment;
	
	emit("SECTION", ".data","",""); // outputs section .data
	
	map<string, SymbolTableEntry>::iterator itr; 
	
	for (itr = symbolTable.begin(); itr != symbolTable.end(); itr++) {
		external = itr->first; // gets the map<string> which is the external name
		if (symbolTable.at(external).getMode() == CONSTANT && symbolTable.at(external).getAlloc() == YES){ // if constant and not prog_name prob could remove progname but just in case
			value = symbolTable.at(external).getValue();
			if (value == "true" ){
				value = "-1";
			}
			else if (value == "false"){
				value = "0";
			}
			comment = "; " + external;
			emit(symbolTable.at(external).getInternalName(), "dd", value,comment);
		}
	}
	//for those entries in the symbolTable that have 
	//an allocation of YES and a storage mode of CONSTANT
	//{ call emit to output a line to objectFile }
	objectFile << endl;
	emit("SECTION", ".bss","","");
	for (itr = symbolTable.begin(); itr != symbolTable.end(); itr++) {
		external = itr->first;
		if (symbolTable.at(external).getMode() == VARIABLE && symbolTable.at(external).getAlloc() == YES){
			comment = "; " + external;
			emit(symbolTable.at(external).getInternalName(), "resd", "1",comment);
		}
	}
	// for those entries in the symbolTable that have
	// an allocation of YES and a storage mode of VARIABLE
	// { call emit to output a line to objectFile }
}

// Lexical Scanner
// nextToken()
char Compiler::nextChar() //returns the next character or end of file marker
{
	sourceFile.get(ch);
	// read in next character
	if (sourceFile.eof()) {
		ch = END_OF_FILE; //use a special character to designate end of file
		return ch;
	}
	// print to listing file (starting new line if necessary);
	
	if (prevCh == '\n'){
		lineNo++;
		listingFile << endl << setw(5) << lineNo << '|';
	}
	if (ch != '\n'){
		listingFile << ch;
	}
	prevCh = ch;
	return ch;
}

string Compiler::nextToken() {
    token = "";
    char temp;
	int counter;
    while (token == "") {
        if (ch == '{') {
            // Handle comments enclosed within curly braces
            temp = nextChar();
            while (temp != END_OF_FILE && temp != '}') {
                temp = nextChar(); // gets the nextchar until the end of the comment
            }
            if (ch == END_OF_FILE) {
				processError("unexpected end of file");
			} else {
				nextChar();
			}
        } else if (isspace(ch)) { // } and isspace have the same output
            // Skip closing curly brace and whitespace
            nextChar();
		}else if (ch == '}') { // } and isspace have the same output
            // Skip closing curly brace and whitespace
            processError("\'}\' cannot begin token");
        } else if (isSpecialSymbol(ch)) {
            token = ch;
            nextChar();
			if (token == ":" && ch == '='){
				token+= "=";
				nextChar();
			} else if (token == "<" && ch == '>'){
				token+= ">";
				nextChar();
			} else if (token == "<" && ch == '='){
				token+= "=";
				nextChar();
			} else if (token == ">" && ch == '='){
				token+= "=";
				nextChar();
			}
        } else if (islower(ch)) {
            token = ch;
            temp = nextChar();
			counter = 1;
            while ((isdigit(temp) || islower(temp) || temp == '_') && temp != END_OF_FILE) {
				if (counter >= 2){
					if (token[counter-2] == '_' && token[counter-1] == '_'){
						processError("\'_\' must be followed by a letter or number, as per production 6 in the Pascallite Lexicon");
					}
				}
                if (isKeyword(token) && isspace(temp)) {
                    return token;
                }
				
                token += temp;
                temp = nextChar();
				counter++;
            }
			if (ch == END_OF_FILE) {
				processError("unexpected end of file");
			}
        } else if (isdigit(ch)) {
            token = ch;
            temp = nextChar();
            while (isdigit(temp) && temp != END_OF_FILE) {
                token += temp;
                temp = nextChar();
            }
			if (ch == END_OF_FILE) {
				processError("unexpected end of file");
			}
        } else if (ch == END_OF_FILE) {
            token = ch;
        } else {
            processError("illegal symbol");
        }
    }
    return token;
}

bool Compiler::isKeyword(string s) const{
	return (s == "program"  || s == "true" || s == "false" || s == "not" || s == "var" || s == "begin" || s == "end" 
			|| s == "boolean" || s == "const" || s == "integer" 
			|| s == "mod" || s == "div" || s == "and" || s == "or" || s == "read" || s == "write");
}  // determines if s is a keyword

bool Compiler::isSpecialSymbol(char c) const{
	return (c == '=' || c == ':' || c == ',' || c == ';' || c == '.' || c == '+' || c == '-'|| c == '('|| c == ')'|| c == '<'
			|| c == '>'|| c == '*');
} // determines if c is a special symbol

bool Compiler::isNonKeyId(string s) const{
	if (isKeyword(s))
		return false;
	
	if (!islower(s[0]))
		return false;
	
	for (unsigned int a = 0; a < s.length(); a++){
		if(islower(s[a]) || isdigit(s[a]) || s[a] == '_'){
		} else {
			return false;
		}
	}
	if (s[s.length() - 1] == '_')
		return false;
	return true;
} // determines if s is a non_key_id

bool Compiler::isInteger(string s) const{
	for (unsigned int a = 0; a < s.length(); a++){
		if(!isdigit(s[a])){
			return false;
		}
	}
	return true;
}// determines if s is an integer

bool Compiler::isBoolean(string s) const{
	if (s == "true" || s == "false")
		return true;
	return false;
}  // determines if s is a boolean
	
bool Compiler::isLiteral(string s) const{
	if (isInteger(s) || isBoolean(s)|| 
		(s.substr(0, 3) == "not" && isBoolean(s.substr(3, s.length() - 1))) || // for the not True
		(s[0] == '+' && isInteger(s.substr(1, s.length() - 1))) || // for the -123123 or + 123123
		(s[0] == '-' && isInteger(s.substr(1, s.length() - 1)))) { // need to add not and symbols
		return true;
	}
	return false;
}  // determines if s is a literal

// stage1
void Compiler::pushOperator(string name) //push name onto operatorStk
{
	operatorStk.push(name);
}

void Compiler::pushOperand(string name) //push name onto operandStk
{
	if (isLiteral(name) && symbolTable.find(name) == symbolTable.end()){
		name = name.substr(0,15);
		if (name == "true"){
			symbolTable.insert(pair<string, SymbolTableEntry>("true", SymbolTableEntry("TRUE",BOOLEAN,CONSTANT,"-1",YES,1)));
		}else if (name == "false"){
			symbolTable.insert(pair<string, SymbolTableEntry>("false", SymbolTableEntry("FALSE",BOOLEAN,CONSTANT,"0",YES,1)));
		}else
			insert(name, whichType(name), CONSTANT, whichValue(name), YES, 1);
	}
	operandStk.push(name);
}

string Compiler::popOperator() //pop name from operatorStk
{
	if(!operatorStk.empty()){
		temp = operatorStk.top();
		operatorStk.pop();
	}else
		processError("popOperatorcompiler error; operator stack underflow");
	return temp;
//if operatorStk is not empty
//return top element removed from stack;
//else
//processError(compiler error; operator stack underflow)
}

string Compiler::popOperand() //pop name from operandStk
{
	if(!operandStk.empty()){
		temp = operandStk.top();
		operandStk.pop();
	}else
		processError("popOperandcompiler error; operator stack underflow");
	return temp;
//if operandStk is not empty
//return top element removed from stack;
//else
//processError(compiler error; operand1 stack underflow)
}
/* where to call next token 
we can call the next token in execStmts since it starts
*/
void Compiler::execStmts(){ // $
	nextToken();
	//{{NON_KEY_ID,'read','write'}
	if (token == "read" || token == "write" || isNonKeyId(token)){
		execStmt();
		execStmts();
	}
}  
void Compiler::execStmt(){ // $
	if(isNonKeyId(token)){
		assignStmt();
	}else if(token == "read"){
		readStmt();
	}else if(token == "write"){
		writeStmt();
	}else{
		processError("non_key_id, \"read\", or \"write\" expected");
	}
}
void Compiler::assignStmt(){ // $
	pushOperand(token);
	if(nextToken() != ":=")
		processError("\':=\' expected in assignment statement");
	pushOperator(token);
	express();
	if(token != ";")
		processError("one of \"*\", \"and\", \"div\", \"mod\", \")\", \"+\", \"-\", \";\", \"<\", \"<=\", \"<>\", \"=\", \">\", \">=\", or \"or\" expected");
	rhs = popOperand();
	lhs = popOperand();
	sign = popOperator();
	code(sign,rhs,lhs);
	// nextToken();
} 
void Compiler::readStmt(){ // $
	nextToken();
	if(token == "("){
		nextToken();
		if (!isNonKeyId(token))
			processError("non-keyword identifier expected");
		temp = ids();
		if (token != ")")
			processError("')' expected are non_key_id in \"read\"");
		code("read", temp);
		if(nextToken() != ";")
			processError("\";\" expected");
	}else{
      processError("\'(\' expected after read");
    }
} 
void Compiler::writeStmt(){ // $
	nextToken();
	if(token == "("){
		nextToken();
		if (!isNonKeyId(token))
			processError("non-keyword identifier expected");
		temp = ids();
		if (token != ")")
			processError("\',\' or \')\' expected after non-keyword identifier");
		code("write", temp);
		if(nextToken() != ";")
			processError("\";\" expected");
	}else{
      processError(" '(' expected after write");
    }
}  
void Compiler::express(){
	// based on select sets {'not','true','false','(','+','-',INTEGER,NON_KEY_ID
	nextToken();
	if(token=="not" || token=="true" || token== "false" || token== "(" || token== "+" || token== "-" ||
		isInteger(token) || isNonKeyId(token)){
		term(); // here
		expresses();
	} else
		processError("expected non_key_id, integer, \"not\", \"true\", \"false\", '(', '+', or '-'");
} 
void Compiler::expresses(){
	// based on select sets {'<>','=','<=','>=','<','>'}
	if(token == "=" || token == "<>" || token == "<=" || token == ">=" || token == "<" || token == ">"){
		pushOperator(token);
		nextToken();
		term();
		rhs = popOperand();
		lhs = popOperand();
		sign = popOperator();
		code(sign,rhs,lhs);
		expresses();
	}
}  
void Compiler::term(){
	// based on select sets {'not','true','false','(','+','-',INTEGER,NON_KEY_ID}
	if(token=="not" || token=="true" || token== "false" || token== "(" || token== "+" || token== "-" ||
		isInteger(token) || isNonKeyId(token)){
		factor();
		terms();
	}else
		processError("expected non_key_id, integer, \"not\", \"true\", \"false\", '(', '+', or '-'");
}     
void Compiler::terms(){
	// based on select sets {'-','+','or'}
	if (token == "+" || token == "-" || token == "or"){
		pushOperator(token);
		nextToken();
		factor();
		rhs = popOperand();
		lhs = popOperand();
		sign = popOperator();
		code(sign,rhs,lhs);
		terms();
	}
}    
void Compiler::factor(){
	// based on select sets {'not','true','false','(','+','-',INTEGER,NON_KEY_ID}
	if(token=="not" || token=="true" || token== "false" || token== "(" || token== "+" || token== "-" ||
		isInteger(token) || isNonKeyId(token)){
		part();
		factors();
	} else 
		processError("expected non_key_id, integer, \"not\", \"true\", \"false\", '(', '+', or '-'");
	
}     
void Compiler::factors(){
	// based on select sets {'*','div','mod','and'}
	if (token == "*" || token ==  "div" || token ==  "mod" || token ==  "and"){
		pushOperator(token);
		nextToken();
		part();
		rhs = popOperand();
		lhs = popOperand();
		sign = popOperator();
		code(sign,rhs,lhs);
		factors();
	}
}  

void Compiler::part(){
	if (token == "not"){ // 'not' ( 
		nextToken();
		if (token == "("){ // '(' EXPRESS ')' code('not',popOperand)
			express();
			if(token != ")")
				processError("\')\' expected");
			lhs = popOperand();
			code("not",lhs);
		} else if (isBoolean(token)){ // BOOLEANx pushOperand(not x; i.e., 'true' or 'false')
			if (token == "true")
				pushOperand("false");
			else
				pushOperand("true");
		} else if (isNonKeyId(token)){ // NON_KEY_IDx code('not',x)
			code("not",token);
		} else {
			processError("expected \'(\', boolean, or non-keyword id");
		}
	} else if (token == "+"){ // '+'
		nextToken();
		if (token == "("){ // '(' EXPRESS ')'
			express();
			if(token != ")")
				processError("\')\' expected");
		} else if (isInteger(token) || isNonKeyId(token)){ // ( INTEGERx | NON_KEY_IDx ) pushOperand(x)
			pushOperand(token);
		} else {
			processError("expected '(', integer, or non-keyword id; found +");
		}
	}else if (token == "-"){ // '-'
		nextToken();
		if (token == "("){ // '(' EXPRESS ')' code('neg',popOperand) 
			express();
			if(token != ")")
				processError("\')\' expected");
			code("neg", popOperand());
		} else if (isInteger(token)){ // INTEGERx pushOperand('-'+ x)
			pushOperand("-" + token);
		} else if (isNonKeyId(token)){ // NON_KEY_IDx code('neg',x)
			code("neg",token);
		} else {
			processError("expected \'(\', integer, or non_key_id");
		}
	}else if (token == "("){ // '(' EXPRESS ')'
		express();
		if(token != ")")
			processError("\')\' expected");
	}else if (isInteger(token) || isNonKeyId(token) || isBoolean(token)){ // INTEGERx | BOOLEANx | NON_KEY_IDx pushOperand(x)
		pushOperand(token);
	}else{
		processError("\')\' expected");
	}
	nextToken();
}

void Compiler::emitAdditionCode(string operand1,string operand2) //add operand1 to operand2
{
	if(whichType(operand1) != INTEGER ||whichType(operand2) != INTEGER)
		processError("binary '+' requires integer operands");
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand2 && contentsOfAReg != operand1){
		emit("", "mov","[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand2 && contentsOfAReg != operand1)
		contentsOfAReg = "";
	if(contentsOfAReg != operand2 && contentsOfAReg != operand1){
		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; AReg = " + operand2);
		contentsOfAReg = operand2; // maybe dont need this ?!?! <- stupid really need to keep track of eax
	}
	if (contentsOfAReg == operand1)
		emit("","add","eax,[" + symbolTable.at(operand2).getInternalName() +"]", "; AReg = " + operand1 + " + " + operand2);
	else
		emit("","add","eax,[" + symbolTable.at(operand1).getInternalName() +"]", "; AReg = " + operand2 + " + " + operand1);
	if(isTemporary(operand1))
		freeTemp();
	if(isTemporary(operand2))
		freeTemp();
	temp = getTemp();
	contentsOfAReg = temp;
	symbolTable.at(temp).setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}
void Compiler::emitNegationCode(string operand1, string operand2) //add operand1 to operand2
{
	if(whichType(operand1) != INTEGER)
		processError("unary '-' requires integer operands");
	if(isTemporary(contentsOfAReg) && !isTemporary(operand1)){
		emit("", "mov","[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	if(operand1 != contentsOfAReg)
		emit("","mov","eax,[" + symbolTable.at(operand1).getInternalName() + "]","; AReg = " + operand1);
	contentsOfAReg = operand1;
	emit("","neg","eax","; AReg = -AReg");
	if(isTemporary(operand1))
		freeTemp();
	temp = getTemp();
	contentsOfAReg = temp;
	symbolTable.at(temp).setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}
void Compiler::emitSubtractionCode(string operand1,string operand2) //add operand1 to operand2
{
	if(whichType(operand1) != INTEGER ||whichType(operand2) != INTEGER)
		processError("binary '-' requires integer operands");
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand2){
		emit("", "mov","[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand2 && contentsOfAReg != operand1)
		contentsOfAReg = "";
	if(contentsOfAReg != operand2){
		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; AReg = " + operand2);
		contentsOfAReg = operand2;
	}
	if (contentsOfAReg == operand1)
		emit("","sub","eax,[" + symbolTable.at(operand2).getInternalName() +"]", "; AReg = " + operand1 + " - " + operand2);
	else
		emit("","sub","eax,[" + symbolTable.at(operand1).getInternalName() +"]", "; AReg = " + operand2 + " - " + operand1);
	if(isTemporary(operand1))
		freeTemp();
	if(isTemporary(operand2))
		freeTemp();
	temp = getTemp();
	contentsOfAReg = temp;
	symbolTable.at(temp).setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}
void Compiler::emitDivisionCode(string operand1,string operand2) //divide operand2 by operand1
{
	if(whichType(operand1) != INTEGER ||whichType(operand2) != INTEGER) 
		processError("binary 'div' requires integer operands");
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand2){
		emit("", "mov","[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand2)
		contentsOfAReg = "";
	if(operand2 != contentsOfAReg)
		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; AReg = " + operand2);
	emit("", "cdq", "", "; sign extend dividend from eax to edx:eax");
	emit("", "idiv", "dword [" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " div " + operand1);
	if(isTemporary(operand1))
		freeTemp();
	if(isTemporary(operand2))
		freeTemp();
	temp = getTemp();
	contentsOfAReg = temp;
	symbolTable.at(temp).setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}
void Compiler::emitMultiplicationCode(string operand1,string operand2) //divide operand2 by operand1
{
	if(whichType(operand1) != INTEGER ||whichType(operand2) != INTEGER)
		processError("binary '*' requires integer operands");
	if(isTemporary(contentsOfAReg) && operand1 != contentsOfAReg && operand2 != contentsOfAReg){
		emit("", "mov","[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	if(!isTemporary(contentsOfAReg) && operand2 != contentsOfAReg && operand1 != contentsOfAReg)
		contentsOfAReg = "";
	if(operand2 != contentsOfAReg && operand1 != contentsOfAReg){
		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; AReg = " + operand2);
		contentsOfAReg = operand2;
	}
	// finally found the problem on why it was doubling in the asm and Temp was being used in 1011
	if (contentsOfAReg == operand1)
		emit(" ", "imul", "dword [" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " * " + operand2); 
	else 
		emit(" ", "imul", "dword [" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " * " + operand1);
	if(isTemporary(operand1))
		freeTemp();
	if(isTemporary(operand2))
		freeTemp();
	temp = getTemp();
	contentsOfAReg = temp;
	symbolTable.at(temp).setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}
void Compiler::emitModuloCode(string operand1,string operand2) //divide operand2 by operand1
{
	if(whichType(operand1) != INTEGER ||whichType(operand2) != INTEGER)
		processError("binary 'mod' requires integer operands");
	if(isTemporary(contentsOfAReg) && isTemporary(contentsOfAReg) && operand2 != contentsOfAReg){
		emit("", "mov","[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	if(!isTemporary(contentsOfAReg) && operand2 != contentsOfAReg)
		contentsOfAReg = "";
	if(operand2 != contentsOfAReg && operand1 != contentsOfAReg){
		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; AReg = " + operand2);
		contentsOfAReg = operand2;
	}
	emit("", "cdq", "", "; sign extend dividend from eax to edx:eax");
	if (contentsOfAReg == operand1)
		emit("", "idiv", "dword [" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " div " + operand2);
	else
		emit("", "idiv", "dword [" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " div " + operand1);
	emit("", "xchg", "eax,edx", "; exchange quotient and remainder");
	if(isTemporary(operand1))
		freeTemp();
	if(isTemporary(operand2))
		freeTemp();
	temp = getTemp();
	contentsOfAReg = temp;
	symbolTable.at(temp).setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}
void Compiler::emitAndCode(string operand1,string operand2) //and operand1 to operand2
{
	if(whichType(operand1) != BOOLEAN ||whichType(operand2) != BOOLEAN)
		processError("binary 'and' requires boolean operands");
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		emit("", "mov","[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	if (contentsOfAReg != operand1 && contentsOfAReg != operand2){
		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; AReg = " + operand2);
		contentsOfAReg = operand2; 
	}
	if (contentsOfAReg == operand1)
		emit("","and","eax,[" + symbolTable.at(operand2).getInternalName() +"]", "; AReg = " + operand1 + " and " + operand2);
	else
		emit("","and","eax,[" + symbolTable.at(operand1).getInternalName() +"]", "; AReg = " + operand2 + " and " + operand1);
	if(isTemporary(operand1))
		freeTemp();
	if(isTemporary(operand2))
		freeTemp();
	temp = getTemp();
	contentsOfAReg = temp;
	symbolTable.at(temp).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);;
}
void Compiler::emitNotCode(string operand1, string operand2) //and operand1 to operand2
{
	if(whichType(operand1) != BOOLEAN)
		processError("unary 'not' requires boolean operands");
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1){
		emit("", "mov","[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	if (contentsOfAReg != operand1){
		emit("","mov","eax,[" + symbolTable.at(operand1).getInternalName() + "]","; AReg = " + operand1);
		contentsOfAReg = operand1;
	}
	emit("", "not", "eax", "; AReg = !AReg");
	if(isTemporary(operand1))
		freeTemp();
	temp = getTemp();
	contentsOfAReg = temp;
	symbolTable.at(temp).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);;
}

void Compiler::emitOrCode(string operand1,string operand2) //and operand1 to operand2
{
	if(whichType(operand1) != BOOLEAN ||whichType(operand2) != BOOLEAN)
		processError("illegal type");
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		emit("", "mov","[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	if (contentsOfAReg != operand1 && contentsOfAReg != operand2){
		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; AReg = " + operand2);
		contentsOfAReg = operand2;
	}
	if (contentsOfAReg == operand1)
		emit("","or","eax,[" + symbolTable.at(operand2).getInternalName() +"]", "; AReg = " + operand1 + " or " + operand2);
	else
		emit("","or","eax,[" + symbolTable.at(operand1).getInternalName() +"]", "; AReg = " + operand2 + " or " + operand1);
	if(isTemporary(operand1))
		freeTemp();
	if(isTemporary(operand2))
		freeTemp();
	temp = getTemp();
	contentsOfAReg = temp;
	symbolTable.at(temp).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);;
}
void Compiler::emitInequalityCode(string operand1,string operand2) //test whether operand2 equals operand1
{
	if(whichType(operand1) != whichType(operand2))
		processError("binary \'<>\' requires operands of the same type");
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		emit("", "mov","[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	label1 = getLabel();
	label2 = getLabel();
	if(operand1 != contentsOfAReg && operand2 != contentsOfAReg)
		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; AReg = " + operand2);
	if (contentsOfAReg == operand1)
		emit("","cmp","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; compare "+ operand1 +" and " + operand2);
	else	
		emit("","cmp","eax,[" + symbolTable.at(operand1).getInternalName() + "]","; compare "+ operand2 +" and " + operand1);
	emit("","jne",label1,"; if " + operand2 + " <> " + operand1 + " then jump to set eax to TRUE");
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");
	if(symbolTable.find("false") == symbolTable.end())
		symbolTable.insert(pair<string, SymbolTableEntry>("false", SymbolTableEntry("FALSE",BOOLEAN,CONSTANT,"0", YES, 1)));
	emit("","jmp",label2,"; unconditionally jump");
	emit(label1 + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	if(symbolTable.find("true") == symbolTable.end())
		symbolTable.insert(pair<string, SymbolTableEntry>("true", SymbolTableEntry("TRUE",BOOLEAN,CONSTANT,"-1", YES, 1)));
	emit(label2 + ":");
	if(isTemporary(operand1))
		freeTemp();
	if(isTemporary(operand2))
		freeTemp();
	temp = getTemp();
	contentsOfAReg = temp;
	symbolTable.at(temp).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}
void Compiler::emitEqualityCode(string operand1,string operand2) //test whether operand2 equals operand1
{
	if(whichType(operand1) != whichType(operand2))
		processError("binary \'=\' requires operands of the same type");
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		emit("", "mov","[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	label1 = getLabel();
	label2 = getLabel();
	if(operand1 != contentsOfAReg && operand2 != contentsOfAReg){
		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; AReg = " + operand2);	
		contentsOfAReg = operand2;
	}
	if (contentsOfAReg == operand1)
		emit("","cmp","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; compare "+ operand1 +" and " + operand2);
	else
		emit("","cmp","eax,[" + symbolTable.at(operand1).getInternalName() + "]","; compare "+ operand2 +" and " + operand1);
	emit("","je",label1,"; if " + operand2 + " = " + operand1 + " then jump to set eax to TRUE");
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");
	if(symbolTable.find("false") == symbolTable.end())
		symbolTable.insert(pair<string, SymbolTableEntry>("false", SymbolTableEntry("FALSE",BOOLEAN,CONSTANT,"0", YES, 1)));
	emit("","jmp",label2,"; unconditionally jump");
	emit(label1 + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	if(symbolTable.find("true") == symbolTable.end())
		symbolTable.insert(pair<string, SymbolTableEntry>("true", SymbolTableEntry("TRUE",BOOLEAN,CONSTANT,"-1", YES, 1)));
	emit(label2 + ":");
	if(isTemporary(operand1))
		freeTemp();
	if(isTemporary(operand2))
		freeTemp();
	temp = getTemp();
	contentsOfAReg = temp;
	symbolTable.at(temp).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}
void Compiler::emitGreaterThanCode(string operand1,string operand2) //test whether operand2 equals operand1
{
	if(whichType(operand1) != whichType(operand2))
		processError("binary \'<>\' requires operands of the same type");
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		emit("", "mov","[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	label1 = getLabel();
	label2 = getLabel();
	if(operand1 != contentsOfAReg && operand2 != contentsOfAReg){
		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; AReg = " + operand2);	
	}
	if (contentsOfAReg == operand1)
		emit("","cmp","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; compare "+ operand1 +" and " + operand2);
	else
		emit("","cmp","eax,[" + symbolTable.at(operand1).getInternalName() + "]","; compare "+ operand2 +" and " + operand1);
	emit("","jg",label1,"; if " + operand2 + " > " + operand1 + " then jump to set eax to TRUE");
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");
	if(symbolTable.find("false") == symbolTable.end())
		symbolTable.insert(pair<string, SymbolTableEntry>("false", SymbolTableEntry("FALSE",BOOLEAN,CONSTANT,"0", YES, 1)));
	emit("","jmp",label2,"; unconditionally jump");
	emit(label1 + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	if(symbolTable.find("true") == symbolTable.end())
		symbolTable.insert(pair<string, SymbolTableEntry>("true", SymbolTableEntry("TRUE",BOOLEAN,CONSTANT,"-1", YES, 1)));
	emit(label2 + ":");
	if(isTemporary(operand1))
		freeTemp();
	if(isTemporary(operand2))
		freeTemp();
	temp = getTemp();
	contentsOfAReg = temp;
	symbolTable.at(temp).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}
void Compiler::emitLessThanCode(string operand1,string operand2) 
{
	if(whichType(operand1) != whichType(operand2))
		processError("binary \'<\' requires operands of the same type");
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		emit("", "mov","[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	label1 = getLabel();
	label2 = getLabel();
	if(operand1 != contentsOfAReg && operand2 != contentsOfAReg){
		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; AReg = " + operand2);	
	}
	if (contentsOfAReg == operand1)
		emit("","cmp","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; compare "+ operand1 +" and " + operand2);
	else
		emit("","cmp","eax,[" + symbolTable.at(operand1).getInternalName() + "]","; compare "+ operand2 +" and " + operand1);
	emit("","jl",label1,"; if " + operand2 + " < " + operand1 + " then jump to set eax to TRUE");
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");
	if(symbolTable.find("false") == symbolTable.end())
		symbolTable.insert(pair<string, SymbolTableEntry>("false", SymbolTableEntry("FALSE",BOOLEAN,CONSTANT,"0", YES, 1)));
	emit("","jmp",label2,"; unconditionally jump");
	emit(label1 + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	if(symbolTable.find("true") == symbolTable.end())
		symbolTable.insert(pair<string, SymbolTableEntry>("true", SymbolTableEntry("TRUE",BOOLEAN,CONSTANT,"-1", YES, 1)));
	emit(label2 + ":");
	if(isTemporary(operand1))
		freeTemp();
	if(isTemporary(operand2))
		freeTemp();
	temp = getTemp();
	contentsOfAReg = temp;
	symbolTable.at(temp).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}
void Compiler::emitLessThanOrEqualToCode(string operand1,string operand2) 
{
	if(whichType(operand1) != whichType(operand2))
		processError("binary \'<=\' requires operands of the same type");
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		emit("", "mov","[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	label1 = getLabel();
	label2 = getLabel();
	if(operand1 != contentsOfAReg && operand2 != contentsOfAReg){
		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; AReg = " + operand2);	
	}
	if (contentsOfAReg == operand1)
		emit("","cmp","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; compare "+ operand1 +" and " + operand2);
	else
		emit("","cmp","eax,[" + symbolTable.at(operand1).getInternalName() + "]","; compare "+ operand2 +" and " + operand1);
	emit("","jle",label1,"; if " + operand2 + " <= " + operand1 + " then jump to set eax to TRUE");
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");
	if(symbolTable.find("false") == symbolTable.end())
		symbolTable.insert(pair<string, SymbolTableEntry>("false", SymbolTableEntry("FALSE",BOOLEAN,CONSTANT,"0", YES, 1)));
	emit("","jmp",label2,"; unconditionally jump");
	emit(label1 + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	if(symbolTable.find("true") == symbolTable.end())
		symbolTable.insert(pair<string, SymbolTableEntry>("true", SymbolTableEntry("TRUE",BOOLEAN,CONSTANT,"-1", YES, 1)));
	emit(label2 + ":");
	if(isTemporary(operand1))
		freeTemp();
	if(isTemporary(operand2))
		freeTemp();
	temp = getTemp();
	contentsOfAReg = temp;
	symbolTable.at(temp).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}
void Compiler::emitGreaterThanOrEqualToCode(string operand1,string operand2) 
{
	if(whichType(operand1) != whichType(operand2))
		processError("binary \'>=\' requires operands of the same type");
	if(isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2){
		emit("", "mov","[" + symbolTable.at(contentsOfAReg).getInternalName() + "],eax","; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	if(!isTemporary(contentsOfAReg) && contentsOfAReg != operand1 && contentsOfAReg != operand2)
		contentsOfAReg = "";
	label1 = getLabel();
	label2 = getLabel();
	if(operand1 != contentsOfAReg && operand2 != contentsOfAReg){
		emit("","mov","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; AReg = " + operand2);	
	}
	if (contentsOfAReg == operand1)
		emit("","cmp","eax,[" + symbolTable.at(operand2).getInternalName() + "]","; compare "+ operand1 +" and " + operand2);
	else
		emit("","cmp","eax,[" + symbolTable.at(operand1).getInternalName() + "]","; compare "+ operand2 +" and " + operand1);
	emit("","jge",label1,"; if " + operand2 + " >= " + operand1 + " then jump to set eax to TRUE");
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");
	if(symbolTable.find("false") == symbolTable.end())
		symbolTable.insert(pair<string, SymbolTableEntry>("false", SymbolTableEntry("FALSE",BOOLEAN,CONSTANT,"0", YES, 1)));
	emit("","jmp",label2,"; unconditionally jump");
	emit(label1 + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");
	if(symbolTable.find("true") == symbolTable.end())
		symbolTable.insert(pair<string, SymbolTableEntry>("true", SymbolTableEntry("TRUE",BOOLEAN,CONSTANT,"-1", YES, 1)));
	emit(label2 + ":");
	if(isTemporary(operand1))
		freeTemp();
	if(isTemporary(operand2))
		freeTemp();
	temp = getTemp();
	contentsOfAReg = temp;
	symbolTable.at(temp).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}
void Compiler::emitAssignCode(string operand1,string operand2) 
{
	if(symbolTable.find(operand1) == symbolTable.end()) // maybe will not need this
		processError("operand1 reference to undefined symbol " + operand1);
    if(symbolTable.find(operand2) == symbolTable.end())
		processError("operand2 reference to undefined symbol " + operand2);
	if(whichType(operand1) != whichType(operand2))
		processError("incompatible types for operator ':='");
	if(symbolTable.at(operand2).getMode() != VARIABLE)
		processError("symbol on left-hand side of assignment must have a storage mode of VARIABLE");
	if(operand1 == operand2)
		return;
	if(operand1 != contentsOfAReg){
		emit("","mov","eax,[" + symbolTable.at(operand1).getInternalName() + "]","; AReg = " + operand1);
		contentsOfAReg = operand1;
	}
	emit("","mov","[" + symbolTable.at(operand2).getInternalName() + "],eax","; " + operand2 + " = AReg");
	contentsOfAReg = operand2;
	if(isTemporary(operand1))
		freeTemp();
}

void Compiler::emitReadCode(string operand1, string operand2)
{
	string name;
	while (!operand1.empty())// name broken from list of external names and put into name != ""
	{
		int comma = operand1.find(','); // variables can be separated by , will output -1 if there is no comma otherwise it gets the first comma
		if (comma == -1){ //
			name = operand1;
			operand1.clear();
		}
		else {
			name = operand1.substr(0, comma);
			operand1 = operand1.substr(comma + 1, operand1.length());
		}
		if (name.length() > 15){
			name = name.substr(0,15);
		}
		if (symbolTable.find(name) == symbolTable.end()) // replaced symbolTable[name] is defined
			processError("reference to undefined variable \'"+name+"\'");
		if(whichType(name) != INTEGER)
			processError("can't read variables of this type");
		if(symbolTable.at(name).getMode()  != VARIABLE){
			processError("reading in of read-only location \'" + name +"\'");
		}
		emit("", "call", "ReadInt", "; read int; value placed in eax");
		emit("","mov","[" + symbolTable.at(name).getInternalName() + "],eax","; store eax at "+ name);
		contentsOfAReg = name;
	}
}

void Compiler::emitWriteCode(string operand1, string operand2)
{
	//static bool definedStorage = false;
	string name;
	while (!operand1.empty())// name broken from list of external names and put into name != ""
	{
		int comma = operand1.find(','); // variables can be separated by , will output -1 if there is no comma otherwise it gets the first comma
		
		if (comma == -1){ //
			name = operand1;
			operand1.clear();
		}
		else {
			name = operand1.substr(0, comma);
			operand1 = operand1.substr(comma + 1, operand1.length());
		}
		if (name.length() > 15){
			name = name.substr(0,15);
		}
		if (symbolTable.find(name) == symbolTable.end()) { // replaced symbolTable[name] is defined
			processError("reference to undefined variable \'"+name+"\'");
		}
		if (contentsOfAReg != name) { 
			emit("","mov","eax,[" + symbolTable.at(name).getInternalName() + "]","; load "+ name +" in eax");
			contentsOfAReg = name;
		}
		
		if(whichType(name) == BOOLEAN || whichType(name) == INTEGER ){
			emit("", "call", "WriteInt", "; write int in eax to standard out");
			emit("", "call", "Crlf", "; write \\r\\n to standard out");
		} else {
			processError("writecode  has to be boolean or integer");
		}
	}
}

void Compiler::freeTemp() // $
{
	currentTempNo--;
	//??
	if (currentTempNo < -1)
		processError("compiler error, currentTempNo should be ≥ –1");
}

string Compiler::getTemp(){ // $
	currentTempNo++;
	temp = "T" + to_string(currentTempNo);
	if (currentTempNo > maxTempNo){
		symbolTable.insert(pair<string, SymbolTableEntry>(temp, SymbolTableEntry(temp, UNKNOWN, VARIABLE, "", NO, 1)));
		maxTempNo++;
	}
	return temp;
}
bool Compiler::isTemporary(string s) const{
	if(symbolTable.find(s) != symbolTable.end())
      if(s[0] == 'T') // && isdigit(s[1])
         return true;
	return false;
}
string Compiler::getLabel(){
	string temp;
	temp = ".L" + to_string(labelCounter);
	labelCounter++;
	return temp;
}



