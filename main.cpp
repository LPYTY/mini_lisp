#include <iostream>
#include <string>

#include "./tokenizer.h"
#include "./value.h"
#include "./parser.h"
#include "./eval_env.h"

int main_1();

int main() 
{
    //main_1();
    EvalEnv env;
    int exitCode = 0;
    while (true) 
    {
        try 
        {
            std::cout << ">>> " ;
            std::string line;
            std::getline(std::cin, line);
            if (std::cin.eof()) 
            {
                std::exit(0);
            }
            auto tokens = Tokenizer::tokenize(line);
            Parser parser(std::move(tokens)); // TokenPtr 不支持复制
            auto value = parser.parse();
            auto result = env.eval(std::move(value));
            std::cout << result->toString() << std::endl; // 输出外部表示
        }
        catch (ExitEvent& e)
        {
            exitCode = e.exitCode();
            break;
        }
        catch (SyntaxError& e)
        {
            std::cerr << "SyntaxError: " << e.what() << std::endl;
        }
        catch (LispError& e)
        {
            std::cerr << "LispError: " << e.what() << std::endl;
        }
    }
    return exitCode;
}

#include <iostream>

int main_1() {
    ValuePtr a = std::make_shared<NumericValue>(42);
    ValuePtr b = std::make_shared<BooleanValue>(false);
    ValuePtr c = std::make_shared<SymbolValue>("eq?");
    ValuePtr d = std::make_shared<StringValue>("Hello");
    ValuePtr e = std::make_shared<NilValue>();
    ValuePtr f = std::make_shared<PairValue>(
        c,
        std::make_shared<PairValue>(
            a,
            std::make_shared<PairValue>(d, e)
            )
        );
    std::cout << a->toString() << '\n'
        << b->toString() << '\n'
        << c->toString() << '\n'
        << d->toString() << '\n'
        << e->toString() << '\n'
        << f->toString() << std::endl;
    ValueList v = { a,b,c,d,e,f };
    auto g = ListValue::fromVector(v);
    std::cout << g->toString() << endl;
    return 0;
}
