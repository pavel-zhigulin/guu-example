#include <iostream>
#include <memory>
#include <stdexcept>

#include "guu/lexer.h"
#include "guu/parser.h"
#include "guu/interpreter.h"


using namespace std::string_literals;

using namespace Guu;

int main()
{
    const std::string text =
            "sub foo   \n"
            "set a 1\n"
            "sub main\n"
            "call foo\n"
            "print 1\n\n\n"
            "   print a\n"
            "set a 2\n"
            "set b a\n"
            "call walle  \n"
            "sub walle\n"
            "print a\n"
            "print b\n"
            ;

    try
    {
        std::cout << "PROGRAM:" << std::endl;
        std::cout << text << std::endl << std::endl;

        std::cout << "Parsing...";

        std::unique_ptr<AST::Node> ast;
        try
        {
            auto p = Parser(Tokenizer(text));
            ast = p.buildAST();
        }
        catch(...)
        {
            std::cout << "FAIL" << std::endl;
            throw;
        }
        std::cout << "OK" << std::endl;

        std::cout << std::endl;
        std::cout << "Running..." << std::endl;
        Interpreter interpreter(std::move(ast));
        interpreter.interpret();
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }


    return 0;
}
