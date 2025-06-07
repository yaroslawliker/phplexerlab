#include <iostream>
#include <string>
#include <list>


enum class TokenType {
    COMMENT,
    KEYWORD,
    OPERATOR,
    IDENTIFIER,
    PUNCTUATION,
    INTEGER,
    FLOAT,
    STRING,
    BOOLEAN,
    // Named so because NULL is a name in C++
    NUL,
    END_OF_FILE
};

struct Token{    
    TokenType type;
    std::string value;

    Token(TokenType t, const std::string& v) : type(t), value(v){}
};

// Custom exception
class LexerException: public std::runtime_error {
public:
    explicit LexerException(const std::string& message) 
    : runtime_error(message) { }
};

// Class reads sourceCode of PHP script and translates it into tokens,
// giving tokens types and values (original)
// Usage: first call setSourceCode method, then retrieve tokens via getTokens() method
class PHPLexer
{
private:
    std::string sourceCode;
    size_t curPos; // Currect position
    size_t line; // Number of lines
    size_t sourceCodelength; // Extracted to evoid multiple invoking sourceCode.length()
    bool trace = false; // If true, prints debug information

    // List of all keywords
    std::string keywords[11] = {
         "if", "else",
         "do", "while", "for", "foreach", "break", "continue",
         "function", "return", "echo"
         // Add keywords if needed
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
    };

public:

    // Sets the input sourceCode
    void setSourceCode(std::string code) {
                
        sourceCode = code;
        curPos = 0;
        line = 1;
        sourceCodelength = code.length();
    };

    // If the lexer should print messages to the console
    void setTrace(bool t) {
        trace = t;
    }
    
    // Retrieving tokens from the sourceCode
    // Should be called after invoking setSourceCode() method
    // Returns a list of Toknes always ending with END_OF_FILE token
    // May throw LexerExcetion
    std::list<Token> getTokens() {
        std::list<Token> tokens;

        while (curPos < sourceCodelength) {

            char ch = sourceCode[curPos];

            if (ch == '\n') {
                line++;
            }

            // Methods named like extract... return approriate token or through a
            // LexerException error. If token was found, such methods always
            // leave curPos pointing on the last symbol of the lexeme

            // Methods named like isAbleToExtract... try to extract an appropriate token:
            // If token was found, then add it in the tokens list, sets curPos on the
            //     last symbol of the token and return true. No other routes are checked
            // Otherwise just resests curPos to the position before method was called
            //     and return false, so other routes are checked

            // The demand of leave curPos on the last symbol is important to
            // continue processing next tokens after this cycle's curPos++ is executed.

            // !!! The order of routes is important
            
            // --- Routes ---
            if (ch == '$') {
                tokens.push_back(extractIdenetifier());
            }
            else if ( (ch == '/' || ch == '#') && isAbleToExtractComment(tokens)) {  /* Check the doc string on isAbleToExtractComment*/ }
            else if (isAbleToExtractPunctuation(tokens)) { /* Check the doc string on isAbleToExtractPunctuation method*/ }
            else if (isAbleToExtractBoolean(tokens)) { /* Check the doc string on isAbleToExtractBoolean method*/ }
            else if (isalpha(ch) || ch == '_') {
                tokens.push_back(extractKeyword_KeywordOperator_Null());
            }
            else if (ch == '"' || ch == '\'') {
                tokens.push_back(extractString());
            } 
            else if (isdigit(ch)) {
                tokens.push_back(extractIntegerOrFloat());
            } 
            else if (isOperatorSymbol(ch)) {
                tokens.push_back(extractOperator());
            }
        
            curPos++;
        }

        tokens.push_back(Token(TokenType::END_OF_FILE, ""));
        
        return tokens;
    }

    // Helping method to raise error.
    // Takes the beggining and the end of the words, find "broken" spot and
    // puts it in the thrown LexerException.
    // If curPos is on the whitespace, then takes two near words
    void raiseError(std::string message, int pos) {

        int wordStartPos = pos;
        int wordEndPos = pos;

        // Looking for start and end of the word
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

        // Showing the position
        std::string positionStr = " at position: " + std::to_string(pos);

        // Finidng the trace
        std::string errorTrace;
        errorTrace += sourceCode.substr(wordStartPos, pos - wordStartPos);
        errorTrace += "<---";
        errorTrace += sourceCode.substr(pos, wordEndPos - pos);

        // Getting everything up
        message += positionStr;
        message += ": " + errorTrace;

        throw LexerException(message);
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
                    // Should never be reached if the method is called properly
                    raiseError("Expected '$' at the start of identifier", curPos);
                }
                break;
            
            case IDENTIFIER_FIRST:
                if (isalpha(ch) || ch == '_') {
                    identifier += ch;
                    state = IDENTIFIER;
                } else {
                    // Handle an unexpected character
                    raiseError("Invalid first character in identifier: ", curPos);
                }
                break;

            case IDENTIFIER:
                if (isalnum(ch) || ch == '_') {
                    identifier += ch;
                } else {
                    state = ACCEPT;
                    curPos--; // Making curPos to point to the last symbol of the token
                }
                break;

            
            }  
        curPos++;    
         
        }

        curPos--; // Compensate the last cycle curPos++ execution
        Token token(TokenType::IDENTIFIER, identifier);
        return token; 
    }

    // Extracts a keyword, a keyword operators ('and', 'or', 'xor') or 'NULL' from the currect position
    // Uses almost Finite Automata to recognize keywords
    Token extractKeyword_KeywordOperator_Null() {

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
                    curPos--; // Making curPos to point to the last symbol of the token
                    state = END;
                }
                break;
            }
            curPos++;
        }

        curPos--; // Compensating last cycle curPos++ execution

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

        // Checking for null
        if (potentialKeyword == "NULL") {
            return Token(TokenType::NUL, potentialKeyword);
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
                    // Would never be reached if the method is called properly
                    raiseError("Expected a quote character to start string", curPos);
                }

                strValue += ch;
                state = STRING_CONTENT;
                break;
            
            case STRING_CONTENT:
                if (ch == quoteChar) { // Used non-FA trick to avoid doubling states
                    state = END;
                } else if (curPos == sourceCodelength-1 || ch == '\n') {
                    raiseError("Unterminated string literal", curPos);
                }
                
                // Adding ch no matter it's part of the content or an ending quote
                strValue += ch;
            }

            curPos++;
        }

        curPos--; // Compensating the last cycle's curPos++ execution

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
            INTEGER_PART, // Haven't meet a point yet (may result in Integer or Float)
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
                    curPos--; // Leave curPos on the end of the token
                } else {
                    value += ch;
                }

                break;
            
            case FLOAT:
                if (isdigit(ch)) {
                    value += ch;
                } else {
                    state = ACCEPT_FLOAT;
                    curPos--; // Leave curPos on the end of the token
                }
                break;
            }
        
            curPos++;
        }

        curPos--; // Compensate the while's last curPos++ execution

        // INTEGER_PART and ACCEPT_INTEGER are needed ending of the file
        if (state == ACCEPT_INTEGER || state == INTEGER_PART) {
            return Token(TokenType::INTEGER, value);
        } else if (state == ACCEPT_FLOAT || state == FLOAT) {
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

        while (curPos < sourceCodelength) {
            ch = sourceCode[curPos];

            if (!isalpha(ch)) {
                break; // Stop if we hit a non-alphabetic character
            }

            value += ch; 
            curPos++;
        }

        if (value == "true" || value == "false") {
            curPos--; // Compensate the while's last curPos++ execution
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
        const std::string operatorSymbols = "+-*/%=&|^~<>!?:.@";

        return operatorSymbols.find(ch) != std::string::npos;
    }

    // Extracts an operator from the current position
    // Uses Finite Automata to recognize operators
    Token extractOperator() {
        
        // I think this method can be implemented 5 times shorter withou using Finite Automata,
        // but let it be :D, trying to do accroding to the lab rules where possible

        if (trace) {
            std::cout << "Extracting operator at position: " << curPos << std::endl;
        }

        enum STATE {
            START,

            // --- Starting states ---
            // Choosing automata path depending on the first character
            ARYTHMETIC_FIRST, // Also includes string operator ., becouse it is handeled like arythmetic operators
            LESS_FIRST,
            GREATER_FIRST,
            ASSIGNMENT_FIRST,
            NOT_FIRST,
            LOGICAL, // For |, ||, &, &&
            QESTION_MARK,

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
                    // Single character operators
                    else if (ch == ':' || ch == '~' || ch == '^' || ch == '@') {
                        state = ACCEPT;
                    }
                    else if (ch == '?') {
                        state = QESTION_MARK;
                    }
                    else {
                        // Would never be reached, if I didn't mess up in the state-transmission above and if method is called properly
                        raiseError("Unexpected start character for operator: ", curPos);
                    }
                    operatorValue += ch;
                    break;

                case ARYTHMETIC_FIRST:
                    if (!isOperatorSymbol(ch)) {
                        state = ACCEPT;
                        curPos--;
                    } else if (ch == '=') {
                        state = ACCEPT;
                        operatorValue += ch;
                    } else {
                        raiseError("Unexpected character in arithmetic operator: ", curPos);
                    }
                    break;

                case LESS_FIRST:
                    if (!isOperatorSymbol(ch)) { // Just <
                        curPos--; // Step back to reprocess the current character
                        state = ACCEPT;
                    } 
                    else if (ch == '=') { 
                        operatorValue += ch; // <=
                        state = LESS_EQUAL;
                    }
                    else if (ch == '<' || ch == '>') { // << or <>
                        operatorValue += ch;
                        state = ACCEPT;
                    } else {
                        raiseError("Unexpected character in less operator: ", curPos);
                    }
                    break;
                case LESS_EQUAL:
                    if (ch == '>') { // <=>
                        operatorValue += ch;
                        state = ACCEPT; 
                    } else if (!isOperatorSymbol(ch)) { // <=
                        state = ACCEPT;
                        curPos--;
                    } else {
                        raiseError("Unexpected character in less equal operator: ", curPos);
                    }
                    break;

                case GREATER_FIRST:
                    if (!isOperatorSymbol(ch)) { // Just >
                        state = ACCEPT;
                        curPos--; // Step back to reprocess the current character
                    } else if (ch == '=' || ch == '>') { // >= or >>
                        operatorValue += ch;
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
                        curPos--; // Step back to reprocess the current character
                    } else {
                        raiseError("Unexpected character in assignment operator: ", curPos);
                    }
                    break; 
                case DOUBLE_EQUAL:
                    if (ch == '=') { // === met
                        operatorValue += ch;
                        state = ACCEPT; 
                    } else if (!isOperatorSymbol(ch)) { // == 
                        curPos--; // Step back to reprocess the current character
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
                        curPos--; // Step back to reprocess the current character
                    } else {
                        raiseError("Unexpected character in not operator: ", curPos);
                    }
                    break;
                case NOT_EQUAL:
                    if (ch == '=') { // !== met
                        operatorValue += ch;
                        state = ACCEPT; 
                    } else if (!isOperatorSymbol(ch)) { // !=
                        state = ACCEPT;
                        curPos--; // Step back to reprocess the current character
                    } else {
                        raiseError("Unexpected character in not equal operator: ", curPos);
                    }
                    break;

                case LOGICAL:
                    if (!isOperatorSymbol(ch)) { // Bitwise | or &
                        state = ACCEPT;
                        curPos--; // Step back to reprocess the current character
                    } else if (ch == operatorValue.at(0)) { // Used non-FA techique to avoid doubling state
                        operatorValue += ch; // && or ||
                        state = ACCEPT; 
                    } else {
                        raiseError("Unexpected character in logical operator: ", curPos);
                    } 
                    break;

                case QESTION_MARK:
                    if (!isOperatorSymbol(ch)) {
                        state = ACCEPT; // Just ?
                        curPos--; // Step back to reprocess the current character
                    } else if (ch == '?') { // ?: met
                        operatorValue += ch;
                        state = ACCEPT; 
                    } else {
                        raiseError("Unexpected character in question mark operator: ", curPos);
                    }

                    break;
            }

            curPos++;
        }

        curPos--; // Compensate the while's last curPos++ execution
        return Token(TokenType::OPERATOR, operatorValue);
    }

    // Checks if the ch is one of the symbles of the punctuation tokens.
    bool isPunctuationSymbol(char ch) {

        const char punctuationSymbols[15] = {
            ';', ',', '.',
            ':',
            '=', '?', '-', '>', '.',
            '[', ']', '{', '}', '(', ')'
        };

        for (char symbol : punctuationSymbols) {
            if (ch == symbol) {
                return true;
            }
        }
        return false;
    }

    // Working with punctuation symbols:
    // 1. Checks if the current position is a punctuation symbol
    // 2. If it is, extracts the punctuation symbol(s) and adds a token to the list
    // 3. Returns true if a punctuation symbol was found, false otherwise
    bool isAbleToExtractPunctuation(std::list<Token>& tokens) {

        if (trace) {
            std::cout << "Checking for punctuation at position: " << curPos << std::endl;
        }

        std::string value;
        bool isPunctuation = true;

        char ch = sourceCode[curPos];
        value += ch;

        // Punctuations:
        //     ; ,
        //     ::
        //     => -> ?-> ...
        //     [ ] { } ( )
    
        // : or ::
        if (ch == ':') {
            if (curPos + 1 < sourceCodelength && sourceCode[curPos + 1] == ':') {
                value = "::";
                curPos++; // Go to the end of the token
            } else {
                isPunctuation = false; // ':' is an operator, not punctuation
            }
        } 
        // => or ->
        else if (ch == '=' || ch == '-') {
            if (curPos + 1 < sourceCodelength && sourceCode[curPos + 1] == '>') {
                value += ">";
                curPos++; // Go to the end of the token
            } else {
                isPunctuation = false; // '=' and '-' are operators, not punctuation
            }
        }
        // ?->
        else if (ch == '?') {
            if (curPos + 2 < sourceCodelength && sourceCode[curPos + 1] == '-' && sourceCode[curPos + 2] == '>') {
                curPos += 2; // Go to the end of the token
                value = "?->";
            } else {
                isPunctuation = false; // '?' is an operator, not punctuation
            }
        }
        // ...
        else if (ch == '.') {
            if (curPos + 2 < sourceCodelength && sourceCode[curPos + 1] == '.' && sourceCode[curPos + 2] == '.') {
                value = "...";
                curPos += 2; // Go to the end of the token
            } else {
                isPunctuation = false; // '.' is an operator, not punctuation
            }
        // Single character punctuation symbols e.g. '(', ']', ',' ect
        } 
        // Ommit '>', '?' operators
        else if (ch == '>' || ch == '?') {
            isPunctuation = false;
        }
        else if (!isPunctuationSymbol(ch)) {
            isPunctuation = false; // Not a punctuation symbol
        }

        if (isPunctuation) {
            tokens.push_back(Token(TokenType::PUNCTUATION, value));
            return true;
        } else {
            return false;
        }

    }

    // Working with comments:
    // 1. Checks if the current position is the start of a comment
    // 2. If it is, extracts the comment and adds a token to the list
    // 3. Returns true if a comment was found, false otherwise
    // The method uses Finite Automata to recognize comments
    bool isAbleToExtractComment(std::list<Token>& tokens) {

        if (trace) {
            std::cout << "Checking for comment at position: " << curPos << std::endl;
        }

        enum STATE {
            START,
            SINGLE_DASH,
            INLINE_COMMENT,
            MULTI_LINE_COMMENT,
            MULTI_LINE_COMMENT_END,
            ACCEPT,
            DECLINE
        } state = START;

        std::string commentValue;

        while (curPos < sourceCodelength && state != ACCEPT && state != DECLINE) {
            char ch = sourceCode[curPos];

            switch (state)
            {
                case START:
                    if (ch == '/') {
                        state = SINGLE_DASH;
                    } else if (ch == '#') {
                        state = INLINE_COMMENT;
                    }
                     else {
                        // Never reached if the method is called properly
                        raiseError("Expected '/' at the start of comment", curPos);
                    }
                    commentValue += ch;
                    break;

                case SINGLE_DASH:
                    if (ch == '/') {
                        state = INLINE_COMMENT;
                    } else if (ch == '*') {
                        state = MULTI_LINE_COMMENT;
                    } else {
                        curPos--;
                        state = DECLINE; // Not a comment
                    }
                    commentValue += ch;

                    break;

                case INLINE_COMMENT:
                    if (ch == '\n' || curPos == sourceCodelength-1) {
                        state = ACCEPT;
                        curPos--;
                    } else {
                        commentValue += ch;
                    }
                    break;

                case MULTI_LINE_COMMENT:
                    if (ch == '*') {
                        state = MULTI_LINE_COMMENT_END;
                    } else if (curPos == sourceCodelength-1) {
                        raiseError("Unterminated multi-line comment", curPos);
                    }
                    commentValue += ch;
                    break;

                case MULTI_LINE_COMMENT_END:
                    if (ch == '/') {
                        state = ACCEPT; // End of multi-line comment
                    } else if (curPos == sourceCodelength-1) {
                        raiseError("Unterminated multi-line comment", curPos);
                    } else {
                        state = MULTI_LINE_COMMENT; // Continue multi-line comment
                    }
                    commentValue += ch;
                
                    break;
            }

            curPos++;
        }

        curPos--; // Step back to leave curPos the last character of the token
        if (state == ACCEPT) {
            tokens.push_back(Token(TokenType::COMMENT, commentValue));
            return true;
        } else if (state == DECLINE) {
            return false;
        }

    }

};