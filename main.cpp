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
    const std::string example = R"delim(
fn main(args: str[N]) -> int {
    int x = 3;
    int[3] y = [1,2,3];
    str dquot_str = "some_text";
    str squot_str = 'some_text';
    str[3] strings = ["struct", 'struct', "\"escaped_str\"", '\'escaped_str\''];
}
)delim";
    (void)example;

    const std::string program = R"delim(
fn main(args: str[N], test : int, aa : hello) -> int {

}
)delim";

    try
    {
        std::cout << "PROGRAM:" << std::endl;
        std::cout << program << std::endl << std::endl;

        std::cout << "Parsing...";

        std::unique_ptr<AST::Node> ast;
        try
        {
            auto p = Parser(Tokenizer(program));
            ast    = p.buildAST();
        } catch(...)
        {
            std::cout << "FAIL" << std::endl;
            throw;
        }
        std::cout << "OK" << std::endl;

        AST::Printer p(std::cout);

        p.print(*ast);

        // std::cout << std::endl;
        // std::cout << "Running..." << std::endl;
        // Interpreter interpreter(std::move(ast));
        // interpreter.stepInto();
        // interpreter.stepInto();
        // interpreter.printVars();
        // interpreter.bt();

        // std::cout << std::endl;

        // interpreter.stepOver();
        // interpreter.printVars();
        // interpreter.bt();

        // interpreter.interpret();
    } catch(const std::runtime_error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }

    return 0;
}
