#include <iostream>
#include <variant>
#include <type_traits>
#include <cctype>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <charconv>
#include <sstream>
#include <set>
#include <stack>
#include <optional>

//#include "util/meta_helpers.hpp"
//#include "util/visitor.h"
#include "guu/token.h"
#include "guu/lexer.h"
#include "guu/ast.h"
#include "guu/parser.h"
//#include "guu/interpreter.h"


using namespace std::string_literals;

using namespace Guu;
//using namespace util::meta_helpers;
//using namespace util;


int main()
{
    const std::string text =
            "sub foo\n"
            "set a 1\n"
            "sub main\n"
            "call foo\n"
            "print a\n"
            "set a 2\n"
            "set b a\n"
            "call walle\n"
            "sub walle\n"
            "print a\n"
            "print b\n"
            ;

    try
    {
        std::cout << "PROGRAM:" << std::endl;
        std::cout << text << std::endl << std::endl;

//        std::cout << "TOKENS:" << std::endl;
//        auto t = Tokenizer(text);
//        auto tv = t.getNext();
//        while(tv.type_ != TokenType::EOL)
//        {
//            std::cout << tv << std::endl;
//            tv = t.getNext();
//        }
//        std::cout << std::endl;

        std::cout << "PARSER:" << std::endl;
        auto p = Parser(Tokenizer(text));
        auto ast = p.buildAST();

        AST::Printer printer(std::cout);
        printer.print(*ast);
//
//        Interpreter interpreter(res);
//
//        interpreter.stepInto();
//        interpreter.bt();
//        interpreter.stepOver();
//        interpreter.stepOver();
//        interpreter.stepOver();
//        interpreter.stepOver();
//        interpreter.printVars();
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }


    return 0;
}
