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

    std::list<Token> getTokens() {
        std::list<Token> tokens;

        while (curPos < sourceCodelength) {

            char ch = sourceCode[curPos];

            if (ch == '$') {
                tokens.push_back(extractIdenetifier());
            }

            curPos++;
        }
        
        return tokens;
    }

    // Extracts and indentifier from the current position.
    // Uses Finite Automata to recognize identifiers.
    Token extractIdenetifier() {

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
};