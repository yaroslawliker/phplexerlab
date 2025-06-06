#include <iostream>
#include <string>
#include <list>


enum class TokenType {
    KEYWORD,
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

    std::string keywords[11] = {
         "if", "else",
         "do", "while", "for", "foreach", "break", "continue",
         "function", "return", "echo"
    };


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
                tokens.push_back(extractKeyword());
            } 
            else if (ch == '"' || ch == '\'') {
                tokens.push_back(extractString());
            } 
            else if (isdigit(ch)) {
                tokens.push_back(extractIntegerOrFloat());
            } 
        

            curPos++;
        }
        
        return tokens;
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
                    throw std::runtime_error("Expected '$' at the start of identifier");             
                }
                break;
            
            case IDENTIFIER_FIRST:
                if (isalpha(ch) || ch == '_') {
                    identifier += ch;
                    state = IDENTIFIER;
                } else {
                    // Handle error or unexpected character
                    throw std::runtime_error(std::string("Invalid first character in identifier: ") + ch);
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
    Token extractKeyword() {

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
                    throw std::runtime_error("Expected a letter or underscore at the start of keyword");
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

        for(std::string keyword: keywords) {
            if (potentialKeyword == keyword) {
                return Token(TokenType::KEYWORD, potentialKeyword);
            }
        }

        throw std::runtime_error("Unrecognized keyword: " + potentialKeyword);
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
                    throw std::runtime_error("Expected a quote character to start string");
                }

                strValue += ch;
                state = STRING_CONTENT;
                break;
            
            case STRING_CONTENT:
                if (ch == '\0') {
                    throw std::runtime_error("Unterminated string literal");
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
                    throw std::runtime_error("Expected a digit at the start of number");
                }
                value += ch;
                break;
            
            case LEADING_ZERO:
                if (ch == '.') {
                    state = FLOAT;
                    value += ch;
                } else {
                    throw std::runtime_error("Leading zero must be followed by a decimal point");
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
};