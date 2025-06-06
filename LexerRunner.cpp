#include <iostream>
#include <list>
#include <string>
#include "PHPLexer.cpp"

void coutTokens(const std::list<Token>& tokens) {

    for (const auto& token : tokens) {

        if (token.type == TokenType::IDENTIFIER) {
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
            std::cout << "Null: ";
        } else {
            std::cout << "(Map token type with id" << static_cast<int>(token.type) << "): " << token.value << std::endl;
        }
    }
}


int main() {
    

    PHPLexer lexer;

    lexer.setSourceCode("$var1 if else $_my_var2");
    std::list<Token> tokens = lexer.getTokens();

    coutTokens(tokens);

    return 0;
}

