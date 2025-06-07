#include <iostream>
#include <list>
#include <string>
#include "PHPLexer.cpp"

void coutTokens(const std::list<Token>& tokens) {

    for (const auto& token : tokens) {

        if (token.type == TokenType::COMMENT) {
            std::cout << "Comment: " << token.value << std::endl;
        }
        else if (token.type == TokenType::IDENTIFIER) {
            std::cout << "Identifier: " << token.value << std::endl;
        } else if (token.type == TokenType::KEYWORD) {
            std::cout << "Keyword: " << token.value << std::endl;
        } else if (token.type == TokenType::INTEGER) {
            std::cout << "Integer: " << token.value << std::endl;
        } else if (token.type == TokenType::FLOAT) {
            std::cout << "Float: " << token.value << std::endl;
        } else if (token.type == TokenType::STRING) {
            std::cout << "String: " << token.value << std::endl;
        } else if (token.type == TokenType::BOOLEAN) {
            std::cout << "Boolean: " << token.value << std::endl;
        } else if (token.type == TokenType::NUL) {
            std::cout << "Null: " << token.value << std::endl;
        } else if (token.type == TokenType::OPERATOR) {
            std::cout << "Operator: " << token.value << std::endl;
        }
        else if (token.type == TokenType::PUNCTUATION) {
            std::cout << "Punctuation: " << token.value << std::endl;
        } 
        else if (token.type == TokenType::END_OF_FILE) {
            std::cout << "End of file." << std::endl;
        }
        else {
            std::cout << "(Map token type with id" << static_cast<int>(token.type) << "): " << token.value << std::endl;
        }
    }
}


int main() {
    

    PHPLexer lexer;

    lexer.setSourceCode("// This is an example to test all stuff\n\
$var1 if \"hello\" or (true >= false) do 'how are you' && else NULL,\n\
$_my_var2::123 <=> 123. /* Just testing different stuff */ 45.67 > -0.89 ??;");
    // lexer.setTrace(true); // Enable tracing for debugging

    // Testing all operators
    // lexer.setSourceCode("+ = * / % = += -= *= /= %= == === != !== < > <= >= <=> <> && || ! & | ^ ~ << >> .= . ? : ?? @");

    // Testing all punctuation symbols
    // lexer.setSourceCode("; , :: => -> ?-> ... [ ] { } ()");

    // Testing comments
    // lexer.setSourceCode("// This is comment one\n $num = 123; # This is comment two\n /* This is a multi-line comment\n that spans multiple lines */\n $str = \"Hello, World!\";");


    std::list<Token> tokens = lexer.getTokens();

    coutTokens(tokens);

    return 0;
}

