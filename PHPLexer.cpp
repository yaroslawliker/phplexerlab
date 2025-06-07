#include <iostream>
#include <string>
#include <list>


enum class TokenType {
    KEYWORD,
    OPERATOR,
    IDENTIFIER,
    INTEGER,
    FLOAT,
    STRING,
    BOOLEAN,
    // Named so because NULL is a nave in C++
    NUL
};



struct Token{    
    TokenType type;
    std::string value;

    Token(TokenType t, const std::string& v) : type(t), value(v){}
};

class PHPLexer
{
private:
    std::string sourceCode;
    size_t curPos; // Currect position
    size_t sourceCodelength;
    bool trace = false; // If true, prints debug information

    // List of all keywords
    std::string keywords[11] = {
         "if", "else",
         "do", "while", "for", "foreach", "break", "continue",
         "function", "return", "echo"
    };

    // List of operators written as keywords
    std::string keywordOperators[3] = {
        "and", "or", "xor"
    };

    // Operators (except the ones written as keywords)
    // Source: https://www.w3schools.com/php/php_operators.asp
    std::string operators[33] = {
        "+", "-", "*", "/", "%", // Arithmetic operators
        "=", "+=", "-=", "*=", "/=", "%=", // Assignment operators
        "==", "===" "!=", "!==", "<", ">", "<=", ">=", "<=>", // Comparison operators
        "<>" // Not equals for arrays
        "&&", "||", "!", // Logical operators
        "&", "|", "^", "~", "<<", ">>", // Bitwise operators
        ".=", ".", // String operators
        "?", ":", "??" // Conditional operators
    }; // "+-*/%=&|^~<>!?:.";


public:

    void setSourceCode(std::string code) {

        // Added \0 to the end if needed
        if (code.at(code.length()-1) != '\0') {
            code += '\0'; 
        }
        
        sourceCode = code;
        curPos = 0;
        sourceCodelength = code.length();
    };

    void setTrace(bool t) {
        trace = t;
    }

    std::list<Token> getTokens() {
        std::list<Token> tokens;

        while (curPos < sourceCodelength) {

            char ch = sourceCode[curPos];

            if (ch == '$') {
                tokens.push_back(extractIdenetifier());
            } 
            else if (isAbleToExtractBoolean(tokens)) { /* Check the doc string on isAbleToExtractBoolean method*/ }
            else if (isalpha(ch) || ch == '_') {
                tokens.push_back(extractKeywordOrKeywordOperator());
            } 
            else if (ch == '"' || ch == '\'') {
                tokens.push_back(extractString());
            } 
            else if (isdigit(ch)) {
                tokens.push_back(extractIntegerOrFloat());
            } else if (isOperatorSymbol(ch)) {
                tokens.push_back(extractOperator());
            }
        

            curPos++;
        }
        
        return tokens;
    }

    void raiseError(std::string message, int pos) {

        int wordStartPos = pos;
        int wordEndPos = pos;

        if (pos > 0) {
            wordStartPos--;
        }
        if (pos < sourceCodelength-1) {
            wordEndPos++;
        }

        while (wordStartPos > 0 && sourceCode[wordStartPos] != ' ') {
            wordStartPos--;
        }

        while (wordEndPos < sourceCodelength && sourceCode[wordEndPos] != ' ') {
            wordEndPos++;
        }

        std::string positionStr = " at position: " + std::to_string(pos);


        std::string errorTrace;
        errorTrace += sourceCode.substr(wordStartPos, pos - wordStartPos);
        errorTrace += "<---";
        errorTrace += sourceCode.substr(pos, wordEndPos - pos);

        message += positionStr;
        message += ": " + errorTrace;

        throw std::runtime_error(message);
    }


    // Extracts an indentifier from the current position.
    // Uses Finite Automata to recognize identifiers.
    Token extractIdenetifier() {

        if (trace) {
            std::cout << "Extracting identifier at position: " << curPos << std::endl;
        }

        enum STATE {
            START,
            IDENTIFIER_FIRST, // Checing if the first character is valid for an identifier
            IDENTIFIER,
            ACCEPT
        } state = START;
        
        std::string identifier;

        while (curPos < sourceCodelength && state != ACCEPT) {

            char ch = sourceCode[curPos];

            switch (state)
            {
            case START:
                if (ch == '$') {
                    identifier += ch;
                    state = IDENTIFIER_FIRST;
                } else {
                    // Handle error or unexpected character
                    raiseError("Expected '$' at the start of identifier", curPos);
                }
                break;
            
            case IDENTIFIER_FIRST:
                if (isalpha(ch) || ch == '_') {
                    identifier += ch;
                    state = IDENTIFIER;
                } else {
                    // Handle error or unexpected character
                    raiseError("Invalid first character in identifier: ", curPos);
                }
                break;

            case IDENTIFIER:
                if (isalnum(ch) || ch == '_') {
                    identifier += ch;
                } else {
                    state = ACCEPT;
                }
                break;

            
            }  
        curPos++;    
         
        }

        curPos--; // Step back to reprocess the current character
        Token token(TokenType::IDENTIFIER, identifier);
        return token; 
    }

    // Extracts a keyword from the currect position
    // Uses almost Finite Automata to recognize keywords
    Token extractKeywordOrKeywordOperator() {

        if (trace) {
            std::cout << "Extracting keyword at position: " << curPos << std::endl;
        }

        enum STATE {
            START,
            KEYWORD,
            END
        } state = START;

        std::string potentialKeyword;

        while (curPos < sourceCodelength && state != END) {
            char ch = sourceCode[curPos];

            switch (state)
            {
            case START:
                if (isalpha(ch) || ch == '_') {
                    potentialKeyword += ch;
                    state = KEYWORD;
                } else {
                    raiseError("Expected a letter or underscore at the start of keyword", curPos);
                }
                break;

            case KEYWORD:
                if (isalnum(ch) || ch == '_') {
                    potentialKeyword += ch;
                } else {
                    state = END;
                }
                break;
            }
            curPos++;
        }

        curPos--; // Step back to reprocess the current character

        // Checking for keywords
        for(std::string keyword: keywords) {
            if (potentialKeyword == keyword) {
                return Token(TokenType::KEYWORD, potentialKeyword);
            }
        }

        // Checking for operators written as keywords (e.g and, or, xor)
        for(std::string keywordOperator: keywordOperators) {
            if (potentialKeyword == keywordOperator) {
                return Token(TokenType::OPERATOR, potentialKeyword);
            }
        }
        
        raiseError("Unrecognized keyword: ", curPos);
    }

    // Extracts a string from the current position
    // Uses (alomst) Finite Automata to recognize strings:
    // has quotChar memory slot to keep it simple
    Token extractString() {

        if (trace) {
            std::cout << "Extracting string at position: " << curPos << std::endl;
        }

        enum STATE {
            START,
            STRING_CONTENT,
            END
        } state = START;

        // Value of the string including quotes
        std::string strValue;

        char quoteChar = '\0'; 

        while (curPos < sourceCodelength && state != END) {
            char ch = sourceCode[curPos];

            switch (state)
            {
            case START:
                if (ch == '"') {
                    quoteChar = '"';
                } else if (ch == '\'') {
                    quoteChar = '\'';
                } else {
                    // Handle error or unexpected character
                    raiseError("Expected a quote character to start string", curPos);
                }

                strValue += ch;
                state = STRING_CONTENT;
                break;
            
            case STRING_CONTENT:
                if (ch == '\0') {
                    raiseError("Unterminated string literal", curPos);
                }

                if (ch == quoteChar) { 
                    state = END;
                }
                // Adding ch no matter it's part of the content or an ending quote
                strValue += ch;
            }

            curPos++;
        }

        return Token(TokenType::STRING, strValue);
    }

    // Extracts a number (integer or float) from the current position
    // Uses Finite Automata
    Token extractIntegerOrFloat() {

        if (trace) {
            std::cout << "Extracting number at position: " << curPos << std::endl;
        }

        enum STATE {
            START,
            LEADING_ZERO, // Has leadng zero e.g. 01 or 0.1
            INTEGER_PART, // Haven't meet a point yet
            FLOAT, // Met a point
            ACCEPT_INTEGER,
            ACCEPT_FLOAT
        } state = START;

        std::string value;

        while (curPos < sourceCodelength && state != ACCEPT_INTEGER && state != ACCEPT_FLOAT) {
            char ch = sourceCode[curPos];

            switch (state)
            {
            case START:
                if (ch == '0')  {
                    state = LEADING_ZERO;
                } else if (isdigit(ch)) {
                    state = INTEGER_PART;
                } else {
                    // Never reached if the method is called properly
                    raiseError("Expected a digit at the start of number", curPos);
                }
                value += ch;
                break;
            
            case LEADING_ZERO:
                if (ch == '.') {
                    state = FLOAT;
                    value += ch;
                } else {
                    raiseError("Leading zero must be followed by a decimal point", curPos);
                }
                break;
            
            case INTEGER_PART:
                if (ch == '.') {
                    state = FLOAT;
                    value += ch;
                } else if (!isdigit(ch)) {
                    state = ACCEPT_INTEGER;
                } else {
                    value += ch;
                }

                break;
            
            case FLOAT:
                if (isdigit(ch)) {
                    value += ch;
                } else {
                    state = ACCEPT_FLOAT;
                }
                break;
            }
        
            curPos++;
        }

        curPos--; // Step back to reprocess the current character

        if (state == ACCEPT_INTEGER) {
            return Token(TokenType::INTEGER, value);
        } else if (state == ACCEPT_FLOAT) {
            return Token(TokenType::FLOAT, value);
        }
    }

    // Working with booleans:
    // 1. Tries to extract a boolean value
    // 2. If a boolean value is found, adds approriate token to the list
    // 3. Returns true if a boolean value was found, false otherwise
    bool isAbleToExtractBoolean(std::list<Token>& tokens) {

        if (trace) {
            std::cout << "Checking for boolean at position: " << curPos << std::endl;
        }

        size_t startPos = curPos;
        std::string value;

        char ch;
        // Exctraction a word
        while (curPos < sourceCodelength) {
            ch = sourceCode[curPos];

            if (!isalpha(ch)) {
                break; // Stop if we hit a non-alphabetic character
            }

            value += ch; 
            curPos++;
        }

        if (value == "true" || value == "false") {
            curPos--; // Step back to reprocess the current character
            Token token(TokenType::BOOLEAN, value);
            tokens.push_back(token);
            return true;
        }

        // No boolean value found 
        curPos = startPos; // Resetting the position to start 
        return false;       
    }

    // Checks if the character is an operator symbol like +, =, ? ect
    bool isOperatorSymbol(char ch) {
        const std::string operatorSymbols = "+-*/%=&|^~<>!?:.";

        return operatorSymbols.find(ch) != std::string::npos;
    }

    // Extracts an operator from the current position
    // Uses Finite Automata to recognize operators
    Token extractOperator() {
        
        // I think this method can be implemented 5 times shorter withou using Finite Automata,
        // but let it be :D, trying to do accroding to the lab rules where possible

        enum STATE {
            START,

            // --- Starting states ---
            // Choosing automata path depending on the first character
            ARYTHMETIC_FIRST, // Also includes string operator ., becouse it is handeled like arythmetic operators
            LESS_FIRST,
            GREATER_FIRST,
            ASSIGNMENT_FIRST,
            NOT_FIRST,
            LOGICAL,
            // On one-element operators without possible continuation goes to ACCEPT

            // --- Processing states ---
            LESS_EQUAL, // After COMPARISON_FIRST, may result in <= or <=>
            DOUBLE_EQUAL, // After ASSIGNMENT_FIRST
            NOT_EQUAL,
        
            ACCEPT
        } state = START;

        std::string operatorValue;
        char ch;

        while(curPos < sourceCodelength && state != ACCEPT) {
            ch = sourceCode[curPos];

            switch (state)
            {
                case START:
                    if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '.') {
                        state = ARYTHMETIC_FIRST;
                    }
                    else if (ch == '<') {
                        state = LESS_FIRST;
                    }
                    else if (ch == '>') {
                        state = GREATER_FIRST;
                    }
                    else if (ch == '=') {
                        state = ASSIGNMENT_FIRST;
                    }
                    else if (ch == '!') {
                        state = NOT_FIRST;
                    }
                    else if (ch == '&' || ch == '|') { // Excepts ~ and ^
                        state = LOGICAL;
                    }
                    // --- Single character operators ---
                    else if (ch == ':' || ch == '~' || ch == '^') {
                        state = ACCEPT;
                    }
                    else {
                        // Would never be reached, if I didn't mess up in the state-transmission above and if method is called properly
                        raiseError("Unexpected start character for operator: ", curPos);
                    }
                    operatorValue += ch;
                    break;

                case ARYTHMETIC_FIRST:
                    if (!isOperatorSymbol(ch)) {
                        state == ACCEPT;
                    } else if (ch == '=') {
                        state = ACCEPT;;
                        curPos--; // Compensating the end-of-function curPos--
                        operatorValue += ch;
                    } else {
                        raiseError("Unexpected character in arithmetic operator: ", curPos);
                    }
                    break;

                case LESS_FIRST:
                    if (!isOperatorSymbol(ch)) { // Just <
                        state = ACCEPT;
                    } 
                    if (ch == '=') { 
                        operatorValue += ch; // <=
                        state = LESS_EQUAL;
                    }
                    else if (ch == '<' || ch == '>') { // << or <>
                        operatorValue += ch;
                        curPos++; // Compensating the end-of-function curPos--
                        state = ACCEPT;
                    } else {
                        raiseError("Unexpected character in less operator: ", curPos);
                    }
                    break;
                case LESS_EQUAL:
                    if (ch == '>') { // <=>
                        operatorValue += ch;
                        curPos++; // Compensating the end-of-function curPos--
                        state = ACCEPT; 
                    } else if (!isOperatorSymbol(ch)) { // <=
                        state = ACCEPT;
                    } else {
                        raiseError("Unexpected character in less equal operator: ", curPos);
                    }
                    break;

                case GREATER_FIRST:
                    if (!isOperatorSymbol(ch)) { // Just >
                        state = ACCEPT;
                    } 
                    if (ch == '=' || ch == '>') { // >= or >>
                        operatorValue += ch;
                        curPos++; // Compensating the end-of-function curPos--
                        state = ACCEPT;
                    } else {
                        raiseError("Unexpected character in greater operator: ", curPos);
                    }
                    break;

                case ASSIGNMENT_FIRST:
                    if (ch == '=') {
                        operatorValue += ch;
                        state = DOUBLE_EQUAL; // Could be a comparison operator
                    } else if (!isOperatorSymbol(ch)) {
                        state = ACCEPT;
                    } else {
                        raiseError("Unexpected character in assignment operator: ", curPos);
                    }
                    break; 
                case DOUBLE_EQUAL:
                    if (ch == '=') { // === met
                        operatorValue += ch;
                        curPos++; // Compenstating the end-of-function curPos--
                        state = ACCEPT; 
                    } else if (!isOperatorSymbol(ch)) { // == 
                        state = ACCEPT;
                    } else {
                        raiseError("Unexpected character in double equal operator: ", curPos);
                    }
                    break;

                case NOT_FIRST:
                    if (ch == '=') {
                        operatorValue += ch;
                        state = NOT_EQUAL;
                    } else if (!isOperatorSymbol(ch)) { // Just ! 
                        state = ACCEPT;
                    } else {
                        raiseError("Unexpected character in not operator: ", curPos);
                    }
                    break;
                case NOT_EQUAL:
                    if (ch == '=') { // !== met
                        operatorValue += ch;
                        curPos++; // Compenstating the end-of-function curPos--
                        state = ACCEPT; 
                    } else if (!isOperatorSymbol(ch)) { // !=
                        state = ACCEPT;
                    } else {
                        raiseError("Unexpected character in not equal operator: ", curPos);
                    }
                    break;

                case LOGICAL:
                    if (!isOperatorSymbol(ch)) { // Bitwise | or &
                        state = ACCEPT;
                    } else if (ch == operatorValue.at(0)) { // Used non-FA techique to avoid doubling state
                        operatorValue += ch; // && or ||
                        curPos++; // Compenstating the end-of-function curPos--
                        state = ACCEPT; 
                    } else {
                        raiseError("Unexpected character in logical operator: ", curPos);
                    } 
                    break;
            }

            curPos++;
        }

        curPos--; // Step back to reprocess the current character
        return Token(TokenType::OPERATOR, operatorValue);
    }

    /*
    std::string operators[33] = {
        "+", "-", "*", "/", "%", // Arithmetic operators
        "=", "+=", "-=", "*=", "/=", "%=", // Assignment operators
        "==", "===" "!=", "!==", "<", ">", "<=", ">=", "<=>", // Comparison operators
        "<>" // Not equals for arrays
        "&&", "||", "!", // Logical operators
        "&", "|", "^", "~", "<<", ">>", // Bitwise operators
        ".=", ".", // String operators
        "?", ":", "??" // Conditional operators
    };
    */

};