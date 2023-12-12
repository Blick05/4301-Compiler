// CJ BLICK
// CS 4301
// Stage 0

#include <stage0.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <string>
#include <map>
#include <iomanip>
#include <utility>
#include <ctime>

using namespace std;

//static varaibles
static char prevCh;
static int intCounter = 0;
static int boolCounter = 0;
static int progCounter = 0;

string Compiler::genInternalName(storeTypes stype) const{
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
void Compiler::vars() //token should be "var"
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
void Compiler::beginEndStmt() //token should be "begin"
{ 
	if (token != "begin") {
		processError("keyword \"begin\" expected");
	}
	if (nextToken() != "end") {
		processError("keyword \"end\" expected");
	}
	if (nextToken() != ".") {
		processError("period expected");
	}
	nextToken();
	code("end", ".");
}

// constStmts() - production 6
void Compiler::constStmts() //token should be NON_KEY_ID
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
void Compiler::varStmts() //token should be NON_KEY_ID
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
string Compiler::ids() //token should be NON_KEY_ID
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
			processError("reference to undefined constant");
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
			processError("reference to undefined constant");
		}
	}
	return value;
}

// code()
void Compiler::code(string op, string operand1, string operand2)
{
	// currently there are only two emit statement that stage 0 can go through
	if (op == "program") {
		emitPrologue(operand1);
	}
	else if (op == "end") {
		emitEpilogue();
	}
	else {
		processError("compiler error since function code should not be called with illegal arguments");
	}
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
		if (symbolTable.at(external).getMode() == CONSTANT && symbolTable.at(external).getDataType() != PROG_NAME){ // if constant and not prog_name prob could remove progname but just in case
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
		if (symbolTable.at(external).getMode() == VARIABLE && symbolTable.at(external).getDataType() != PROG_NAME){
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
	return (s == "program"  || s == "true" || s == "false" || s == "not" || s == "var" || s == "begin" || s == "end" || s == "boolean" || s == "const" || s == "integer" );
}  // determines if s is a keyword

bool Compiler::isSpecialSymbol(char c) const{
	return (c == '=' || c == ':' || c == ',' || c == ';' || c == '.' || c == '+' || c == '-') ;
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


