// PKU Software Design (RJSJ) Mini-Lisp Test Library
// Copyright (C) 2023 @pku-software at GitHub.com

// To use this library, follow the instructions below:
// https://pku-software.github.io/project-doc/appendix/rjsj_test.html

#pragma once
#ifndef RJSJ_MINI_LISP_TEST_H
#define RJSJ_MINI_LISP_TEST_H

// Comment below line if you want to manually control do testing or not.
#define RJSJ_TEST_ENABLED 1

#include <cmath>
#if __has_include(<concepts>)
#include <concepts>
#define RMLT_INTERNAL_CONCEPT_ENABLED
#endif
#include <deque>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif

namespace rjsj_mini_lisp_test {

using namespace std::literals;

/*************
 * TOKENIZER *
 *************/

struct LParenToken {};

struct RParenToken {};

struct DotToken {};

struct Bool {
    bool value;
    friend bool operator==(const Bool& a, const Bool& b) {
        return a.value == b.value;
    }
    std::string toString() const {
        return value ? "#t" : "#f";
    }
};

struct Num {
    double value;
    friend bool operator==(const Num& a, const Num& b) {
        double err = std::max(1e-6, 1e-4 * (a.value + b.value) / 2);
        return std::abs(a.value - b.value) < err;
    }
    std::string toString() const {
        return std::to_string(value);
    }
};

struct Str {
    std::string value;
    friend bool operator==(const Str& a, const Str& b) {
        return a.value == b.value;
    }
    std::string toString() const {
        std::stringstream ss;
        ss << std::quoted(value);
        return ss.str();
    }
};

struct Sym {
    std::string name;
    friend bool operator==(const Sym& a, const Sym& b) {
        return a.name == b.name;
    }
    std::string toString() const {
        return name;
    }
};

struct OtherToken {
    std::string text;
};  // procedure etc.

using Token = std::variant<LParenToken, RParenToken, DotToken, Bool, Num, Str, Sym, OtherToken>;

inline std::optional<Token> tokenFromChar(char c) {
    switch (c) {
        case '(': return LParenToken{};
        case ')': return RParenToken{};
        case '.': return DotToken{};
        case '`': throw std::runtime_error("Quasiquote not supported");
        case ',': throw std::runtime_error("Unquote not supported");
        default: return std::nullopt;
    }
}
const std::set<char> TOKEN_END{'(', ')', '\'', '`', ',', '"'};

inline std::deque<Token> tokenize(const std::string& input) {
    int pos = 0;
    auto nextToken = [&]() -> std::optional<Token> {
        while (pos < input.size()) {
            auto c = input[pos];
            if (c == ';') {
                while (pos < input.size() && input[pos] != '\n') {
                    pos++;
                }
            } else if (std::isspace(c) || c == '\'') {
                pos++;
            } else if (auto token = tokenFromChar(c)) {
                pos++;
                return token;
            } else if (c == '"') {
                std::string string;
                pos++;
                while (pos < input.size()) {
                    if (input[pos] == '"') {
                        pos++;
                        return Token{Str{string}};
                    } else if (input[pos] == '\\') {
                        if (pos + 1 >= input.size()) {
                            throw std::runtime_error("Unexpected end of string literal");
                        }
                        auto next = input[pos + 1];
                        if (next == 'n') {
                            string += '\n';
                        } else {
                            string += next;
                        }
                        pos += 2;
                    } else {
                        string += input[pos];
                        pos++;
                    }
                }
                throw std::runtime_error("Unexpected end of string literal");
            } else {
                int start = pos;
                do {
                    pos++;
                } while (pos < input.size() && !std::isspace(input[pos]) &&
                         !TOKEN_END.contains(input[pos]));
                auto text = input.substr(start, pos - start);
                if (text == ".") {
                    return Token{DotToken{}};
                } else if (text == "#t") {
                    return Token{Bool{true}};
                } else if (text == "#f") {
                    return Token{Bool{false}};
                } else if (std::isdigit(text[0]) || text[0] == '+' || text[0] == '-' ||
                           text[0] == '.') {
                    try {
                        return Token{Num{std::stod(text)}};
                    } catch (std::invalid_argument& e) {
                    }
                } else if (text[0] == '#') {
                    return Token{OtherToken{text}};
                }
                return Token{Sym{text}};
            }
        }
        return std::nullopt;
    };
    std::deque<Token> tokens;
    while (auto token = nextToken()) {
        tokens.push_back(*token);
    }
    return tokens;
}

/**********
 * PARSER *
 **********/

struct Null {
    friend bool operator==(const Null&, const Null&) {
        return true;
    }
    std::string toString() const {
        return "()";
    }
};

struct Proc {
    friend bool operator==(const Proc&, const Proc&) {
        return true;
    }
    std::string toString() const {
        return "#<procedure>";
    }
};

struct Pair;

using Value = std::variant<Null, Bool, Num, Str, Sym, Proc, Pair>;

template <typename... Ts>
std::string toString(const std::variant<Ts...>& value) {
    return std::visit([](const auto& v) { return v.toString(); }, value);
}

template <typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};

template <typename... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

struct Pair {
    std::unique_ptr<Value> car;
    std::unique_ptr<Value> cdr;
    friend bool operator==(const Pair& a, const Pair& b) {
        return *a.car == *b.car && *a.cdr == *b.cdr;
    }
    std::string toString() const;
};

struct OutputCdr {
    std::stringstream& ss;
    void operator()(const Value& value) {
        std::visit(Overloaded{
                       [&](const Null&) { ss << ")"; },
                       [&](const Pair& pair) {
                           ss << " " << toString(*pair.car);
                           OutputCdr{ss}(*pair.cdr);
                       },
                       [&](const auto&) { ss << " . " << toString(value) << ")"; },
                   },
                   value);
    }
};

inline std::string Pair::toString() const {
    std::stringstream ss;
    ss << "(" << ::rjsj_mini_lisp_test::toString(*car);
    OutputCdr{ss}(*cdr);
    return ss.str();
}

inline Token nextToken(std::deque<Token>& tokens) {
    if (tokens.empty()) {
        throw std::runtime_error("Unexpected end of input");
    }
    auto token = tokens.front();
    tokens.pop_front();
    return token;
}

inline std::unique_ptr<Value> parse(std::deque<Token>& tokens);

inline Value parseList(std::deque<Token>& tokens) {
    if (tokens.empty()) {
        throw std::runtime_error("Unexpected end of input");
    }
    if (auto token = std::get_if<RParenToken>(&tokens.front())) {
        tokens.pop_front();
        return Null{};
    }
    auto car = parse(tokens);
    std::unique_ptr<Value> cdr;
    if (auto token = std::get_if<DotToken>(&tokens.front())) {
        tokens.pop_front();
        cdr = parse(tokens);
        auto paren = nextToken(tokens);
        if (!std::holds_alternative<RParenToken>(paren)) {
            throw std::runtime_error("Expected ')'");
        }
    } else {
        cdr = std::make_unique<Value>(parseList(tokens));
    }
    return Pair{std::move(car), std::move(cdr)};
}

inline std::unique_ptr<Value> parse(std::deque<Token>& tokens) {
    auto token = nextToken(tokens);
    auto value = std::visit(Overloaded{
                                [&](LParenToken) { return parseList(tokens); },
                                [](Bool x) { return Value{x}; },
                                [](Num x) { return Value{x}; },
                                [](Str x) { return Value{x}; },
                                [](Sym x) { return Value{x}; },
                                [](OtherToken) { return Value{Proc{}}; },
                                [](auto) -> Value { throw std::runtime_error("Unexpected token"); },
                            },
                            token);
    return std::make_unique<Value>(std::move(value));
}

inline Value buildValueFromStr(const std::string& str) {
    auto tokens = tokenize(str);
    auto value = parse(tokens);
    return std::move(*value);
}

template <typename T>
struct IsCompleteImpl {
    template <typename U>
    static auto test(U*) -> std::integral_constant<bool, sizeof(U) == sizeof(U)>;
    static auto test(...) -> std::false_type;
    using type = decltype(test((T*)0));
};

// Whether T is a complete type.
template <typename T>
constexpr int IsComplete = IsCompleteImpl<T>::type::value;

/**************
 * CONTROLLER *
 **************/

struct Cases {
    const char* name;
    std::vector<std::pair<std::string, std::optional<std::string>>> cases;
};

#ifdef RMLT_INTERNAL_CONCEPT_ENABLED
template <typename T>
concept TestCtx =
    std::movable<T> && std::default_initializable<T> && requires(T env, const std::string& str) {
        { env.eval(str) } -> std::convertible_to<std::string>;
    };
#define RMLT_INTERNAL_TEST_ENV TestCtx
#else
#define RMLT_INTERNAL_TEST_ENV typename
#endif

template <RMLT_INTERNAL_TEST_ENV E>
class TestController {
private:
    std::vector<Cases> cases;
    E env;

public:
    TestController(std::vector<Cases> cases) : cases{std::move(cases)} {
#ifdef _WIN32
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode{};
        GetConsoleMode(hOut, &mode);
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
        SetConsoleMode(hOut, mode);
#endif
    }

    bool test() {
        std::vector<std::pair<int, int>> nums;
        bool allResult = true;
        for (auto case_ : cases) {
            env = E{};
            int successNum = 0;
            std::cout << "\033[1mTesting " << case_.name << "\033[0m\n";
            for (const auto& [input, output] : case_.cases) {
                std::cout << input;
                try {
                    auto result = env.eval(input);
                    std::cout << "\033[36m => " << result << "\033[0m";
                    auto got = buildValueFromStr(result);
                    if (output) {
                        auto expected = buildValueFromStr(*output);
                        if (expected == got) {
                            std::cout << " \033[32mok\033[0m\n";
                            successNum++;
                        } else {
                            std::cout << " \033[31mbad\033[0m \033[90m[expected "
                                      << toString(expected) << "]\033[0m\n";
                        }
                    } else {
                        std::cout << " \033[32mok\033[0m\n";
                        successNum++;
                    }
                } catch (std::exception& e) {
                    std::cout << " \033[31mbad\033[0m";
                    if (output) {
                        auto expected = buildValueFromStr(*output);
                        std::cout << " \033[90m[expected " << toString(expected) << "]\033[0m";
                    }
                    std::cout << "\nException thrown: " << e.what() << std::endl;
                }
            }
            bool ac = successNum == case_.cases.size();
            std::cout << (ac ? "\033[1;32mTest Passed\033[0m\n" : "\033[1;31mTest failed\033[0m\n");
            nums.emplace_back(successNum, case_.cases.size());
            allResult &= ac;
        }
        std::cout << "\033[1mSummary\033[0m\n+------------+-----------+\n| NAME       | RESULT    "
                     "|\n+------------+-----------+\n";
        int totalSuccessNum = 0, totalTotalNum = 0;
        for (int i = 0; i < cases.size(); i++) {
            auto [successNum, totalNum] = nums[i];
            std::cout << "| " << std::left << std::setw(10) << cases[i].name << " | " << std::right
                      << std::setw(4) << successNum << "/" << std::left << std::setw(4) << totalNum
                      << " |\n";
            totalSuccessNum += successNum;
            totalTotalNum += totalNum;
        }
        std::cout << "+------------+-----------+\n| " << std::left << std::setw(10) << "Total"
                  << " | " << std::right << std::setw(4) << totalSuccessNum << "/" << std::left
                  << std::setw(4) << totalTotalNum << " |\n+------------+-----------+\n";
        return allResult;
    }
};

}  // namespace rjsj_mini_lisp_test

/***********************
 * PP-META-PROGRAMMING *
 ***********************/

#if defined(_MSC_VER) && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)
#error Traditional MSVC preprocessing mode is not supported. Please enable /Zc:preprocessor flag.
#endif

#define PP_REMOVE_PARENS(T) PP_REMOVE_PARENS_IMPL T
#define PP_REMOVE_PARENS_IMPL(...) __VA_ARGS__
#define PP_COMMA() ,
#define PP_LPAREN() (
#define PP_RPAREN() )
#define PP_EMPTY()
#define PP_CONCAT(A, B) PP_CONCAT_IMPL(A, B)
#define PP_CONCAT_IMPL(A, B) A##B
#define PP_INC(N) PP_CONCAT(PP_INC_, N)
#define PP_INC_0 1
#define PP_INC_1 2
#define PP_INC_2 3
#define PP_INC_3 4
#define PP_INC_4 5
#define PP_INC_5 6
#define PP_INC_6 7
#define PP_INC_7 8
#define PP_INC_8 9
#define PP_INC_9 10
#define PP_INC_10 11
#define PP_INC_11 12
#define PP_INC_12 13
#define PP_INC_13 14
#define PP_INC_14 15
#define PP_INC_15 16
#define PP_NOT(N) PP_CONCAT(PP_NOT_, N)
#define PP_BOOL(N) PP_CONCAT(PP_BOOL_, N)
#define PP_BOOL_0 0
#define PP_BOOL_1 1
#define PP_BOOL_2 1
#define PP_BOOL_3 1
#define PP_BOOL_4 1
#define PP_BOOL_5 1
#define PP_BOOL_6 1
#define PP_BOOL_7 1
#define PP_BOOL_8 1
#define PP_BOOL_9 1
#define PP_BOOL_10 1
#define PP_BOOL_11 1
#define PP_BOOL_12 1
#define PP_BOOL_13 1
#define PP_BOOL_14 1
#define PP_BOOL_15 1
#define PP_IF(PRED, THEN, ELSE) PP_CONCAT(PP_IF_, PP_BOOL(PRED))(THEN, ELSE)
#define PP_IF_1(THEN, ELSE) THEN
#define PP_IF_0(THEN, ELSE) ELSE
#define PP_COMMA_IF(N) PP_IF(N, PP_COMMA, PP_EMPTY)()
#define PP_NOT_0 1
#define PP_NOT_1 0
#define PP_AND(A, B) PP_CONCAT(PP_AND_, PP_CONCAT(A, B))
#define PP_AND_00 0
#define PP_AND_01 0
#define PP_AND_10 0
#define PP_AND_11 1
#define PP_GET_N(N, ...) PP_CONCAT(PP_GET_N_, N)(__VA_ARGS__)
#define PP_GET_N_0(_0, ...) _0
#define PP_GET_N_1(_0, _1, ...) _1
#define PP_GET_N_2(_0, _1, _2, ...) _2
#define PP_GET_N_3(_0, _1, _2, _3, ...) _3
#define PP_GET_N_4(_0, _1, _2, _3, _4, ...) _4
#define PP_GET_N_5(_0, _1, _2, _3, _4, _5, ...) _5
#define PP_GET_N_6(_0, _1, _2, _3, _4, _5, _6, ...) _6
#define PP_GET_N_7(_0, _1, _2, _3, _4, _5, _6, _7, ...) _7
#define PP_GET_N_8(_0, _1, _2, _3, _4, _5, _6, _7, _8, ...) _8
#define PP_GET_N_9(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, ...) _9
#define PP_GET_N_10(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, ...) _10
#define PP_GET_N_11(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, ...) _11
#define PP_GET_N_12(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, ...) _12
#define PP_GET_N_13(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, ...) _13
#define PP_GET_N_14(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, ...) _14
#define PP_GET_N_15(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, ...) _15
#define PP_GET_N_16(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
                    ...)                                                                       \
    _16
#define PP_IS_EMPTY(...)                                                                   \
    PP_AND(PP_AND(PP_NOT(PP_HAS_COMMA(__VA_ARGS__)), PP_NOT(PP_HAS_COMMA(__VA_ARGS__()))), \
           PP_AND(PP_NOT(PP_HAS_COMMA(PP_COMMA_V __VA_ARGS__)),                            \
                  PP_HAS_COMMA(PP_COMMA_V __VA_ARGS__())))
#define PP_HAS_COMMA(...) \
    PP_GET_N_16(__VA_ARGS__, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0)
#define PP_COMMA_V(...) ,
#define PP_VA_OPT_COMMA(...) PP_COMMA_IF(PP_NOT(PP_IS_EMPTY(__VA_ARGS__)))
#define PP_NARG(...)                                                                              \
    PP_GET_N(16, __VA_ARGS__ PP_VA_OPT_COMMA(__VA_ARGS__) 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, \
             5, 4, 3, 2, 1, 0)
#define PP_FOR_EACH(DO, CTX, ...) \
    PP_CONCAT(PP_FOR_EACH_, PP_NARG(__VA_ARGS__))(DO, CTX, 0, __VA_ARGS__)
#define PP_FOR_EACH_0(DO, CTX, IDX, ...)
#define PP_FOR_EACH_1(DO, CTX, IDX, VAR, ...) DO(VAR, IDX, CTX)
#define PP_FOR_EACH_2(DO, CTX, IDX, VAR, ...) \
    DO(VAR, IDX, CTX)                         \
    PP_FOR_EACH_1(DO, CTX, PP_INC(IDX), __VA_ARGS__)
#define PP_FOR_EACH_3(DO, CTX, IDX, VAR, ...) \
    DO(VAR, IDX, CTX)                         \
    PP_FOR_EACH_2(DO, CTX, PP_INC(IDX), __VA_ARGS__)
#define PP_FOR_EACH_4(DO, CTX, IDX, VAR, ...) \
    DO(VAR, IDX, CTX)                         \
    PP_FOR_EACH_3(DO, CTX, PP_INC(IDX), __VA_ARGS__)
#define PP_FOR_EACH_5(DO, CTX, IDX, VAR, ...) \
    DO(VAR, IDX, CTX)                         \
    PP_FOR_EACH_4(DO, CTX, PP_INC(IDX), __VA_ARGS__)
#define PP_FOR_EACH_6(DO, CTX, IDX, VAR, ...) \
    DO(VAR, IDX, CTX)                         \
    PP_FOR_EACH_5(DO, CTX, PP_INC(IDX), __VA_ARGS__)
#define PP_FOR_EACH_7(DO, CTX, IDX, VAR, ...) \
    DO(VAR, IDX, CTX)                         \
    PP_FOR_EACH_6(DO, CTX, PP_INC(IDX), __VA_ARGS__)
#define PP_FOR_EACH_8(DO, CTX, IDX, VAR, ...) \
    DO(VAR, IDX, CTX)                         \
    PP_FOR_EACH_7(DO, CTX, PP_INC(IDX), __VA_ARGS__)
#define PP_FOR_EACH_9(DO, CTX, IDX, VAR, ...) \
    DO(VAR, IDX, CTX)                         \
    PP_FOR_EACH_8(DO, CTX, PP_INC(IDX), __VA_ARGS__)
#define PP_FOR_EACH_10(DO, CTX, IDX, VAR, ...) \
    DO(VAR, IDX, CTX)                          \
    PP_FOR_EACH_9(DO, CTX, PP_INC(IDX), __VA_ARGS__)
#define PP_FOR_EACH_11(DO, CTX, IDX, VAR, ...) \
    DO(VAR, IDX, CTX)                          \
    PP_FOR_EACH_10(DO, CTX, PP_INC(IDX), __VA_ARGS__)
#define PP_FOR_EACH_12(DO, CTX, IDX, VAR, ...) \
    DO(VAR, IDX, CTX)                          \
    PP_FOR_EACH_11(DO, CTX, PP_INC(IDX), __VA_ARGS__)
#define PP_FOR_EACH_13(DO, CTX, IDX, VAR, ...) \
    DO(VAR, IDX, CTX)                          \
    PP_FOR_EACH_12(DO, CTX, PP_INC(IDX), __VA_ARGS__)
#define PP_FOR_EACH_14(DO, CTX, IDX, VAR, ...) \
    DO(VAR, IDX, CTX)                          \
    PP_FOR_EACH_13(DO, CTX, PP_INC(IDX), __VA_ARGS__)
#define PP_FOR_EACH_15(DO, CTX, IDX, VAR, ...) \
    DO(VAR, IDX, CTX)                          \
    PP_FOR_EACH_14(DO, CTX, PP_INC(IDX), __VA_ARGS__)
#define PP_FOR_EACH_16(DO, CTX, IDX, VAR, ...) \
    DO(VAR, IDX, CTX)                          \
    PP_FOR_EACH_15(DO, CTX, PP_INC(IDX), __VA_ARGS__)

#define RMLT_INTERNAL_CASE_PREFIXED(name) PP_CONCAT(rjsj_mini_lisp_test_, name)

#define RMLT_INTERNAL_FOREACH_ADD_PREFIX(VAR, IDX, CTX) \
    PP_COMMA_IF(IDX) RMLT_INTERNAL_CASE_PREFIXED(VAR)

#ifdef RJSJ_TEST_NO_EXIT
#define RMLT_INTERNAL_EXIT(...) static_cast<void>(__VA_ARGS__)
#else
#define RMLT_INTERNAL_EXIT(...) std::exit(__VA_ARGS__)
#endif

/**************
 * MAIN MACRO *
 **************/

#if defined(RJSJ_TEST_ENABLED) && RJSJ_TEST_ENABLED

#define RJSJ_TEST(CTX_TYPE, ...)                                             \
    do {                                                                     \
        rjsj_mini_lisp_test::TestController<CTX_TYPE> controller(            \
            {PP_FOR_EACH(RMLT_INTERNAL_FOREACH_ADD_PREFIX, , __VA_ARGS__)}); \
        RMLT_INTERNAL_EXIT(controller.test() ? 0 : 1);                       \
    } while (0)

#else

#define RJSJ_TEST(...)

#endif

#define RMLT_BEGIN_CASES(NAME)                                                  \
    static const rjsj_mini_lisp_test::Cases RMLT_INTERNAL_CASE_PREFIXED(NAME) { \
        #NAME, {
#define RMLT_CASE(input, ...) {input, PP_IF(PP_IS_EMPTY(__VA_ARGS__), std::nullopt, __VA_ARGS__)},
#define RMLT_END_CASES(...) \
    }                       \
    }                       \
    ;

/********************
 * CASES DEFINITION *
 ********************/

RMLT_BEGIN_CASES(Lv2)
RMLT_CASE("#f", "#f")
RMLT_CASE("#t", "#t")
RMLT_CASE("42", "42")
RMLT_CASE("+42", "42")
RMLT_CASE("-42", "-42")
RMLT_CASE("3.14", "3.14")
RMLT_CASE("\"abc\"", "\"abc\"")
RMLT_CASE("\"ab\\\"c\"", "\"ab\\\"c\"")
RMLT_END_CASES()

RMLT_BEGIN_CASES(Lv2Only)
RMLT_CASE("map", "map")
RMLT_CASE("+", "+")
RMLT_CASE("eq?", "eq?")
RMLT_CASE("(a . (b . c))", "(a b . c)")
RMLT_CASE("(a b c)", "(a b c)")
RMLT_CASE("(a b . c)", "(a b . c)")
RMLT_CASE("'\"abc\"", "(quote \"abc\")")
RMLT_CASE("(#t . (#f . ()))", "(#t #f)")
RMLT_END_CASES()

RMLT_BEGIN_CASES(Lv3)
RMLT_CASE("(define x 42)")
RMLT_CASE("x", "42")
RMLT_CASE("(define y x)")
RMLT_CASE("y", "42")
RMLT_CASE("(define x #t)")
RMLT_CASE("x", "#t")
RMLT_CASE("y", "42")
RMLT_CASE("(define y \"abc\")")
RMLT_CASE("y", "\"abc\"")
RMLT_END_CASES()

RMLT_BEGIN_CASES(Lv4)
RMLT_CASE("+", "#<proc>")
RMLT_CASE("(+ 1 2)", "3")
RMLT_CASE("(+)", "0")
RMLT_CASE("(+ 1 2 3 4 5)", "15")
RMLT_CASE("(define x (+ 1 2))")
RMLT_CASE("x", "3")
RMLT_CASE("(+ x 4)", "7")
RMLT_CASE("(define add +)")
RMLT_CASE("(add 1 2 3)", "6")
RMLT_CASE("(+ 1 (add 2 3) 4)", "10")
RMLT_CASE("print")
RMLT_CASE("(print 42)")
RMLT_END_CASES()

RMLT_BEGIN_CASES(Lv5)
RMLT_CASE("'42", "42")
RMLT_CASE("'#t", "#t")
RMLT_CASE("'\"abc\"", "\"abc\"")
RMLT_CASE("'()", "()")
RMLT_CASE("'(+ 1 2)", "(+ 1 2)")
RMLT_CASE("(quote (+ x 2))", "(+ x 2)")
RMLT_CASE("''x", "(quote x)")
RMLT_CASE("(if '() \"Yea\" \"Nay\")", "\"Yea\"")
RMLT_CASE("(if #f \"Yea\" \"Nay\")", "\"Nay\"")
RMLT_CASE("(define false #f)")
RMLT_CASE("(if false \"Ok\" \"Emm\")", "\"Emm\"")
RMLT_CASE("(and 1 2 false 3)", "#f")
RMLT_CASE("(and 1 2 3)", "3")
RMLT_CASE("(and 1 2 '())", "()")
RMLT_CASE("(and)", "#t")
RMLT_CASE("(and #f (/ 1 0))", "#f")
RMLT_CASE("(or #f #f 1 2)", "1")
RMLT_CASE("(or 1 2 3)", "1")
RMLT_CASE("(or #f #f #f)", "#f")
RMLT_CASE("(or)", "#f")
RMLT_CASE("(or 3 (/ 1 0))", "3")
RMLT_CASE("(if (or #f #f #f) 1 2)", "2")
RMLT_CASE("(lambda (x) (+ x x))", "#<proc>")
RMLT_CASE("(define (double x) (+ x x))")
RMLT_CASE("double", "#proc")
RMLT_END_CASES()

RMLT_BEGIN_CASES(Lv5Extra)
RMLT_CASE("(* 2 3)", "6")
RMLT_CASE("(> 3 2)", "#t")
RMLT_CASE("(if (> 3 2) \"Correct\" \"Bad\")", "\"Correct\"")
RMLT_CASE("(*)", "1")
RMLT_CASE("(* 1 2 3 4 5)", "120")
RMLT_END_CASES()

RMLT_BEGIN_CASES(Lv6)
RMLT_CASE("(define (square x) (* x x))")
RMLT_CASE("(square 21)", "441")
RMLT_CASE("(define (sum-of-squares x y) (+ (square x) (square y)))")
RMLT_CASE("(sum-of-squares 3 4)", "25")
RMLT_CASE("(define (add-partial x) (lambda (y) (+ x y)))")
RMLT_CASE("(define add42 (add-partial 42))")
RMLT_CASE("add42", "#<proc>")
RMLT_CASE("(add42 36)", "78")
RMLT_CASE("(define add-pi (add-partial 3.14))")
RMLT_CASE("(add-pi 2.71)", "5.85")
RMLT_CASE("(define (a-plus-abs-b a b) ((if (> b 0) + -) a b))")
RMLT_CASE("(a-plus-abs-b 3 -2)", "5")
RMLT_CASE("(define (print-twice x) (print x) (print x))")
RMLT_CASE("(print-twice 42)")
RMLT_END_CASES()

RMLT_BEGIN_CASES(Lv7)
RMLT_CASE("(define n -5)")
RMLT_CASE("(cond ((< n 0) -1) ((> n 0) 1) (else 0))", "-1")
RMLT_CASE("(cond (#f 2) (#t 3))", "3")
RMLT_CASE("(cond (#f 2) (else 3))", "3")
RMLT_CASE("(cond (#f 2) (#t))", "#t")
RMLT_CASE("(begin 1)", "1")
RMLT_CASE("(begin 1 2)", "2")
RMLT_CASE("(begin (print 1) (print 2) (print 3))")
RMLT_CASE("(let ((x 5) (y 10)) (print x) (print y) (+ x y))", "15")
RMLT_CASE("`(11 45 ,(* 2 7))", "(11 45 14)")
RMLT_END_CASES()

RMLT_BEGIN_CASES(Lv7Lib)
RMLT_CASE("(apply + '(1 2 3))", "6")
RMLT_CASE("(display \"Hello\")")
RMLT_CASE("(displayln \"Hello\")")
RMLT_CASE("error", "#<proc>")
RMLT_CASE("(eval '(cons 1 (cons 2 '())))", "(1 2)")
RMLT_CASE("(eval '(+ 1 2))", "3")
RMLT_CASE("exit", "#<proc>")
RMLT_CASE("(newline)")
RMLT_CASE("(atom? 42)", "#t")
RMLT_CASE("(atom? #t)", "#t")
RMLT_CASE("(atom? \"abc\")", "#t")
RMLT_CASE("(atom? 'atom?)", "#t")
RMLT_CASE("(atom? '())", "#t")
RMLT_CASE("(atom? atom?)", "#f")
RMLT_CASE("(atom? +)", "#f")
RMLT_CASE("(atom? '(1 2 3))", "#f")
RMLT_CASE("(atom? (list 1 2 3))", "#f")
RMLT_CASE("(boolean? #t)", "#t")
RMLT_CASE("(boolean? #f)", "#t")
RMLT_CASE("(boolean? 42)", "#f")
RMLT_CASE("(boolean? '())", "#f")
RMLT_CASE("(boolean? '(1 2 3))", "#f")
RMLT_CASE("(boolean? \"abc\")", "#f")
RMLT_CASE("(boolean? +)", "#f")
RMLT_CASE("(integer? 42)", "#t")
RMLT_CASE("(integer? 0)", "#t")
RMLT_CASE("(integer? -42)", "#t")
RMLT_CASE("(integer? 3.14)", "#f")
RMLT_CASE("(integer? -1.41)", "#f")
RMLT_CASE("(integer? #f)", "#f")
RMLT_CASE("(integer? +)", "#f")
RMLT_CASE("(integer? '())", "#f")
RMLT_CASE("(number? 42)", "#t")
RMLT_CASE("(number? 0)", "#t")
RMLT_CASE("(number? -42)", "#t")
RMLT_CASE("(number? 3.14)", "#t")
RMLT_CASE("(number? -1.41)", "#t")
RMLT_CASE("(number? #t)", "#f")
RMLT_CASE("(number? +)", "#f")
RMLT_CASE("(number? '())", "#f")
RMLT_CASE("(number? \"abc\")", "#f")
RMLT_CASE("(number? '(1 2 3))", "#f")
RMLT_CASE("(list? '(1 2 3))", "#t")
RMLT_CASE("(list? '(1 . 2))", "#f")
RMLT_CASE("(list? '())", "#t")
RMLT_CASE("(list? 42)", "#f")
RMLT_CASE("(list? \"abc\")", "#f")
RMLT_CASE("(list? #t)", "#f")
RMLT_CASE("(null? '())", "#t")
RMLT_CASE("(null? 42)", "#f")
RMLT_CASE("(null? \"\")", "#f")
RMLT_CASE("(null? 0)", "#f")
RMLT_CASE("(null? #f)", "#f")
RMLT_CASE("(pair? '(1 2 3))", "#t")
RMLT_CASE("(pair? '(1 . 2))", "#t")
RMLT_CASE("(pair? '(1))", "#t")
RMLT_CASE("(pair? '())", "#f")
RMLT_CASE("(pair? 42)", "#f")
RMLT_CASE("(pair? \"abc\")", "#f")
RMLT_CASE("(pair? #t)", "#f")
RMLT_CASE("(pair? +)", "#f")
RMLT_CASE("(procedure? +)", "#t")
RMLT_CASE("(procedure? procedure?)", "#t")
RMLT_CASE("(procedure? '())", "#f")
RMLT_CASE("(procedure? 42)", "#f")
RMLT_CASE("(procedure? \"abc\")", "#f")
RMLT_CASE("(procedure? #f)", "#f")
RMLT_CASE("(procedure? '(1 2 3))", "#f")
RMLT_CASE("(procedure? (lambda (x) x))", "#t")
RMLT_CASE("(procedure? '+)", "#f")
RMLT_CASE("(string? \"abc\")", "#t")
RMLT_CASE("(string? \"\")", "#t")
RMLT_CASE("(string? '())", "#f")
RMLT_CASE("(string? 42)", "#f")
RMLT_CASE("(string? #f)", "#f")
RMLT_CASE("(string? +)", "#f")
RMLT_CASE("(string? '(a b c))", "#f")
RMLT_CASE("(symbol? 'a)", "#t")
RMLT_CASE("(symbol? '+)", "#t")
RMLT_CASE("(symbol? '())", "#f")
RMLT_CASE("(symbol? 42)", "#f")
RMLT_CASE("(symbol? '42)", "#f")
RMLT_CASE("(symbol? #f)", "#f")
RMLT_CASE("(symbol? +)", "#f")
RMLT_CASE("(symbol? '(a b c))", "#f")
RMLT_CASE("(symbol? \"abc\")", "#f")
RMLT_CASE("(append '(1 2 3) '(a b c) '(\"foo\" \"bar\"))", "(1 2 3 a b c \"foo\" \"bar\")")
RMLT_CASE("(append)", "()")
RMLT_CASE("(append (list 1 2 3))", "(1 2 3)")
RMLT_CASE("(append '(1 2 3) '())", "(1 2 3)")
RMLT_CASE("(append '() (list 1 2 3))", "(1 2 3)")
RMLT_CASE("(append '(1 2 3) '() '(a b c))", "(1 2 3 a b c)")
RMLT_CASE("(car '(1 2 3))", "1")
RMLT_CASE("(car '(1 . 2))", "1")
RMLT_CASE("(car '(1))", "1")
RMLT_CASE("(cdr '(1 2 3))", "(2 3)")
RMLT_CASE("(cdr '(1 . 2))", "2")
RMLT_CASE("(cdr '(1))", "()")
RMLT_CASE("(cons 1 2)", "(1 . 2)")
RMLT_CASE("(cons 1 '())", "(1)")
RMLT_CASE("(cons '() 2)", "(() . 2)")
RMLT_CASE("(cons '() '())", "(())")
RMLT_CASE("(cons 1 (cons 2 (cons 3 '())))", "(1 2 3)")
RMLT_CASE("(cons 1 (cons 2 (cons 3 4)))", "(1 2 3 . 4)")
RMLT_CASE("(length '(1 2 3))", "3")
RMLT_CASE("(length '(6 5.0 sym \"str\" (1 2)))", "5")
RMLT_CASE("(length '())", "0")
RMLT_CASE("(length (list (+ 1 2)))", "1")
RMLT_CASE("(list 1 2 3)", "(1 2 3)")
RMLT_CASE("(list)", "()")
RMLT_CASE("(list (list 1 2) (list 3 4))", "((1 2) (3 4))")
RMLT_CASE("(map - '(1 2 3))", "(-1 -2 -3)")
RMLT_CASE("(map abs '(1 -2 3 -4))", "(1 2 3 4)")
RMLT_CASE("(map (lambda (x) (* x x)) '(1 2 3 4 5))", "(1 4 9 16 25)")
RMLT_CASE("(filter odd? '(1 2 3 4 5))", "(1 3 5)")
RMLT_CASE("(filter even? '(1 2 3 4 5))", "(2 4)")
RMLT_CASE("(filter (lambda (x) (> x 3)) '(1 2 3 4 5))", "(4 5)")
RMLT_CASE("(reduce + '(1 2 3 4 5))", "15")
RMLT_CASE("(reduce * '(1 2 3 4 5))", "120")
RMLT_CASE("(- 3.14)", "-3.14")
RMLT_CASE("(- 3.14 1.59)", "1.55")
RMLT_CASE("(/ 4)", "0.25")
RMLT_CASE("(/ 7 2)", "3.5")
RMLT_CASE("(abs -1.41)", "1.41")
RMLT_CASE("(abs 3.14)", "3.14")
RMLT_CASE("(abs 0)", "0")
RMLT_CASE("(expt 2 3)", "8")
RMLT_CASE("(expt 2 0)", "1")
RMLT_CASE("(expt 2 -4)", "0.0625")
RMLT_CASE("(expt 4 0.5)", "2")
RMLT_CASE("(expt 4 -0.5)", "0.5")
RMLT_CASE("(modulo 10 3)", "1")
RMLT_CASE("(modulo -10 3)", "2")
RMLT_CASE("(modulo 10 -3)", "-2")
RMLT_CASE("(modulo -10 -3)", "-1")
RMLT_CASE("(remainder 10 3)", "1")
RMLT_CASE("(remainder -10 3)", "-1")
RMLT_CASE("(remainder 10 -3)", "1")
RMLT_CASE("(remainder -10 -3)", "-1")
RMLT_CASE("(eq? '(1 2 3) '(1 2 3))", "#f")
RMLT_CASE("(define x '(1 2 3))")
RMLT_CASE("(eq? x x)", "#t")
RMLT_CASE("(define y x)")
RMLT_CASE("(eq? x y)", "#t")
RMLT_CASE("(define z '(1 2 3))")
RMLT_CASE("(eq? x z)", "#f")
RMLT_CASE("(eq? '() '())", "#t")
RMLT_CASE("(eq? '() 'a)", "#f")
RMLT_CASE("(eq? 'a 'a)", "#t")
RMLT_CASE("(eq? 'a 'b)", "#f")
RMLT_CASE("(eq? 1 1)", "#t")
RMLT_CASE("(equal? '(1 2 3) '(1 2 3))", "#t")
RMLT_CASE("(equal? '(1 2 3) '(1 2 4))", "#f")
RMLT_CASE("(equal? '(1 2 3) '(1 2))", "#f")
RMLT_CASE("(equal? '(1 2 3) '(1 2 . 3))", "#f")
RMLT_CASE("(equal? '(1 2 3) '(1 2 . (3)))", "#t")
RMLT_CASE("(equal? '(1 2 3) '(1 2 . (3 . ())))", "#t")
RMLT_CASE("(equal? 42 42)", "#t")
RMLT_CASE("(equal? 42 43)", "#f")
RMLT_CASE("(equal? 'a 'a)", "#t")
RMLT_CASE("(equal? 'a 'b)", "#f")
RMLT_CASE("(equal? 'a '())", "#f")
RMLT_CASE("(equal? '() '())", "#t")
RMLT_CASE("(equal? '() 'a)", "#f")
RMLT_CASE("(equal? + 42)", "#f")
RMLT_CASE("(equal? \"abc\" \"abc\")", "#t")
RMLT_CASE("(equal? \"abc\" \"def\")", "#f")
RMLT_CASE("(equal? \"abc\" 42)", "#f")
RMLT_CASE("(equal? #f '())", "#f")
RMLT_CASE("(equal? #f #f)", "#t")
RMLT_CASE("(equal? #f #t)", "#f")
RMLT_CASE("(not #f)", "#t")
RMLT_CASE("(not #t)", "#f")
RMLT_CASE("(not 0)", "#f")
RMLT_CASE("(not 1)", "#f")
RMLT_CASE("(not '())", "#f")
RMLT_CASE("(not '(1 2 3))", "#f")
RMLT_CASE("(not 'a)", "#f")
RMLT_CASE("(not +)", "#f")
RMLT_CASE("(not \"abc\")", "#f")
RMLT_CASE("(= 42 42)", "#t")
RMLT_CASE("(= 42 43)", "#f")
RMLT_CASE("(= 3.14 3.14)", "#t")
RMLT_CASE("(= 3.14 (/ 314 100))", "#t")
RMLT_CASE("(= 3.14 -3.14)", "#f")
RMLT_CASE("(< 1 2)", "#t")
RMLT_CASE("(< (abs -2) (abs 1))", "#f")
RMLT_CASE("(< 1 1)", "#f")
RMLT_CASE("(<= 1 1)", "#t")
RMLT_CASE("(<= 1 2)", "#t")
RMLT_CASE("(>= 1 2)", "#f")
RMLT_CASE("(>= 2 1)", "#t")
RMLT_CASE("(> 2 1)", "#t")
RMLT_CASE("(> 2 2)", "#f")
RMLT_CASE("(even? 0)", "#t")
RMLT_CASE("(even? -1)", "#f")
RMLT_CASE("(odd? -1)", "#t")
RMLT_CASE("(zero? 0)", "#t")
RMLT_CASE("(zero? 1)", "#f")
RMLT_END_CASES()

RMLT_BEGIN_CASES(Sicp)
// Copied from Berkley CS 61A Proj. 4, extra cases removed.
// These are examples from several sections of "The Structure
// and Interpretation of Computer Programs" by Abelson and Sussman.
// License: Creative Commons share alike with attribution
// 1.1.1
RMLT_CASE("10", "10")
RMLT_CASE("(+ 137 349)", "486")
RMLT_CASE("(- 1000 334)", "666")
RMLT_CASE("(* 5 99)", "495")
RMLT_CASE("(/ 10 5)", "2")
RMLT_CASE("(+ 2.7 10)", "12.7")
RMLT_CASE("(+ 21 35 12 7)", "75")
RMLT_CASE("(* 25 4 12)", "1200")
RMLT_CASE("(+ (* 3 5) (- 10 6))", "19")
RMLT_CASE("(+ (* 3 (+ (* 2 4) (+ 3 5))) (+ (- 10 7) 6))", "57")
// 1.1.2
RMLT_CASE("(define size 2)")
RMLT_CASE("size", "2")
RMLT_CASE("(* 5 size)", "10")
RMLT_CASE("(define pi 3.14159)")
RMLT_CASE("(define radius 10)")
RMLT_CASE("(* pi (* radius radius))", "314.159")
RMLT_CASE("(define circumference (* 2 pi radius))")
RMLT_CASE("circumference", "62.8318")
// 1.1.4
RMLT_CASE("(define (square x) (* x x))")
RMLT_CASE("(square 21)", "441")
RMLT_CASE("(define square (lambda (x) (* x x)))")
RMLT_CASE("(square 21)", "441")
RMLT_CASE("(square (+ 2 5))", "49")
RMLT_CASE("(square (square 3))", "81")
RMLT_CASE("(define (sum-of-squares x y) (+ (square x) (square y)))")
RMLT_CASE("(sum-of-squares 3 4)", "25")
RMLT_CASE("(define (f a) (sum-of-squares (+ a 1) (* a 2)))")
RMLT_CASE("(f 5)", "136")
// 1.1.6
RMLT_CASE("(define (abs x) (cond ((> x 0) x) ((= x 0) 0) ((< x 0) (- x))))")
RMLT_CASE("(abs -3)", "3")
RMLT_CASE("(abs 0)", "0")
RMLT_CASE("(abs 3)", "3")
RMLT_CASE("(define (a-plus-abs-b a b) ((if (> b 0) + -) a b))")
RMLT_CASE("(a-plus-abs-b 3 -2)", "5")
// 1.1.7
RMLT_CASE(
    "(define (sqrt-iter guess x) (if (good-enough? guess x) guess (sqrt-iter (improve guess x) "
    "x)))")
RMLT_CASE("(define (improve guess x) (average guess (/ x guess)))")
RMLT_CASE("(define (average x y) (/ (+ x y) 2))")
RMLT_CASE("(define (good-enough? guess x) (< (abs (- (square guess) x)) 0.001))")
RMLT_CASE("(define (sqrt x) (sqrt-iter 1.0 x))")
RMLT_CASE("(sqrt 9)", "3.00009155413138")
RMLT_CASE("(sqrt (+ 100 37))", "11.704699917758145")
RMLT_CASE("(sqrt (+ (sqrt 2) (sqrt 3)))", "1.7739279023207892")
RMLT_CASE("(square (sqrt 1000))", "1000.000369924366")
// 1.1.8
RMLT_CASE(
    "(define (sqrt x) (define (good-enough? guess) (< (abs (- (square guess) x)) 0.001)) (define "
    "(improve guess) (average guess (/ x guess))) (define (sqrt-iter guess) (if (good-enough? "
    "guess) guess (sqrt-iter (improve guess)))) (print (sqrt-iter 1.0)) (sqrt-iter 1.0))")
RMLT_CASE("(sqrt 9)", "3.00009155413138")
RMLT_CASE("(sqrt (+ 100 37))", "11.704699917758145")
RMLT_CASE("(sqrt (+ (sqrt 2) (sqrt 3)))", "1.7739279023207892")
RMLT_CASE("(square (sqrt 1000))", "1000.000369924366")
// 1.3.1
RMLT_CASE("(define (cube x) (* x x x))")
RMLT_CASE("(define (sum term a next b) (if (> a b) 0 (+ (term a) (sum term (next a) next b))))")
RMLT_CASE("(define (inc n) (+ n 1))")
RMLT_CASE("(define (sum-cubes a b) (sum cube a inc b))")
RMLT_CASE("(sum-cubes 1 10)", "3025")
RMLT_CASE("(define (identity x) x)")
RMLT_CASE("(define (sum-integers a b) (sum identity a inc b))")
RMLT_CASE("(sum-integers 1 10)", "55")
// 1.3.2
RMLT_CASE("((lambda (x y z) (+ x y (square z))) 1 2 3)", "12")
RMLT_CASE(
    "(define (f x y) (let ((a (+ 1 (* x y))) (b (- 1 y))) (+ (* x (square a)) (* y b) (* a b))))")
RMLT_CASE("(f 3 4)", "456")
RMLT_CASE("(define x 5)")
RMLT_CASE("(+ (let ((x 3)) (+ x (* x 10))) x)", "38")
RMLT_CASE("(let ((x 3) (y (+ x 2))) (* x y))", "21")
// 2.1.1
RMLT_CASE(
    "(define (add-rat x y) (make-rat (+ (* (numer x) (denom y)) (* (numer y) (denom x))) (* (denom "
    "x) (denom y))))")
RMLT_CASE(
    "(define (sub-rat x y) (make-rat (- (* (numer x) (denom y)) (* (numer y) (denom x))) (* (denom "
    "x) (denom y))))")
RMLT_CASE("(define (mul-rat x y) (make-rat (* (numer x) (numer y)) (* (denom x) (denom y))))")
RMLT_CASE("(define (div-rat x y) (make-rat (* (numer x) (denom y)) (* (denom x) (numer y))))")
RMLT_CASE("(define (equal-rat? x y) (= (* (numer x) (denom y)) (* (numer y) (denom x))))")
RMLT_CASE("(define x (cons 1 (cons 2 '())))")
RMLT_CASE("(car x)", "1")
RMLT_CASE("(cdr x)", "(2)")
RMLT_CASE("(define x (list 1 2))")
RMLT_CASE("(define y (list 3 4))")
RMLT_CASE("(define z (cons x y))")
RMLT_CASE("(car (car z))", "1")
RMLT_CASE("(car (cdr z))", "3")
RMLT_CASE("z", "((1 2) 3 4)")
RMLT_CASE("(define (make-rat n d) (list n d))")
RMLT_CASE("(define (numer x) (car x))")
RMLT_CASE("(define (denom x) (car (cdr x)))")
RMLT_CASE("(define (cat-rat x) (+ (* (numer x) 10) (denom x)))")
RMLT_CASE("(define one-half (make-rat 1 2))")
RMLT_CASE("(cat-rat one-half)", "12")
RMLT_CASE("(define one-third (make-rat 1 3))")
RMLT_CASE("(cat-rat (add-rat one-half one-third))", "56")
RMLT_CASE("(cat-rat (mul-rat one-half one-third))", "16")
RMLT_CASE("(cat-rat (add-rat one-third one-third))", "69")
RMLT_CASE("(define (gcd a b) (if (= b 0) a (gcd b (remainder a b))))")
RMLT_CASE("(define (make-rat n d) (let ((g (gcd n d))) (list (/ n g) (/ d g))))")
RMLT_CASE("(cat-rat (add-rat one-third one-third))", "23")
RMLT_CASE("(define one-through-four (list 1 2 3 4))")
RMLT_CASE("one-through-four", "(1 2 3 4)")
RMLT_CASE("(car one-through-four)", "1")
RMLT_CASE("(cdr one-through-four)", "(2 3 4)")
RMLT_CASE("(car (cdr one-through-four))", "2")
RMLT_CASE("(cons 10 one-through-four)", "(10 1 2 3 4)")
RMLT_CASE("(cons 5 one-through-four)", "(5 1 2 3 4)")
RMLT_CASE(
    "(define (map proc items) (if (null? items) '() (cons (proc (car items)) (map proc (cdr "
    "items)))))")
RMLT_CASE("(map abs (list -10 2.5 -11.6 17))", "(10 2.5 11.6 17)")
RMLT_CASE("(map (lambda (x) (* x x)) (list 1 2 3 4))", "(1 4 9 16)")
RMLT_CASE("(define (scale-list items factor) (map (lambda (x) (* x factor)) items))")
RMLT_CASE("(scale-list (list 1 2 3 4 5) 10)", "(10 20 30 40 50)")
RMLT_CASE(
    "(define (count-leaves x) (cond ((null? x) 0) ((not (pair? x)) 1) (else (+ (count-leaves (car "
    "x)) (count-leaves (cdr x))))))")
RMLT_CASE("(define x (cons (list 1 2) (list 3 4)))")
RMLT_CASE("(count-leaves x)", "4")
RMLT_CASE("(count-leaves (list x x))", "8")
// 2.2.3
RMLT_CASE("(define (odd? x) (= 1 (remainder x 2)))")
RMLT_CASE(
    "(define (filter predicate sequence) (cond ((null? sequence) '()) ((predicate (car sequence)) "
    "(cons (car sequence) (filter predicate (cdr sequence)))) (else (filter predicate (cdr "
    "sequence)))))")
RMLT_CASE("(filter odd? (list 1 2 3 4 5))", "(1 3 5)")
RMLT_CASE(
    "(define (accumulate op initial sequence) (if (null? sequence) initial (op (car sequence) "
    "(accumulate op initial (cdr sequence)))))")
RMLT_CASE("(accumulate + 0 (list 1 2 3 4 5))", "15")
RMLT_CASE("(accumulate * 1 (list 1 2 3 4 5))", "120")
RMLT_CASE("(accumulate cons '() (list 1 2 3 4 5))", "(1 2 3 4 5)")
RMLT_CASE(
    "(define (enumerate-interval low high) (if (> low high) '() (cons low (enumerate-interval (+ "
    "low 1) high))))")
RMLT_CASE("(enumerate-interval 2 7)", "(2 3 4 5 6 7)")
RMLT_CASE(
    "(define (enumerate-tree tree) (cond ((null? tree) '()) ((not (pair? tree)) (list tree)) (else "
    "(append (enumerate-tree (car tree)) (enumerate-tree (cdr tree))))))")
RMLT_CASE("(enumerate-tree (list 1 (list 2 (list 3 4)) 5))", "(1 2 3 4 5)")
// 2.3.1
RMLT_CASE("(define a 1)")
RMLT_CASE("(define b 2)")
RMLT_CASE("(list a b)", "(1 2)")
RMLT_CASE("(list 'a 'b)", "(a b)")
RMLT_CASE("(list 'a b)", "(a 2)")
RMLT_CASE("(car '(a b c))", "a")
RMLT_CASE("(cdr '(a b c))", "(b c)")
RMLT_CASE(
    "(define (memq item x) (cond ((null? x) #f) ((equal? item (car x)) x) (else (memq item (cdr "
    "x)))))")
RMLT_CASE("(memq 'apple '(pear banana prune))", "#f")
RMLT_CASE("(memq 'apple '(x (apple sauce) y apple pear))", "(apple pear)")
RMLT_CASE(
    "(define (my-equal? x y) (cond ((pair? x) (and (pair? y) (my-equal? (car x) (car y)) "
    "(my-equal? (cdr x) (cdr y)))) ((null? x) (null? y)) (else (equal? x y))))")
RMLT_CASE("(my-equal? '(1 2 (three)) '(1 2 (three)))", "#t")
RMLT_CASE("(my-equal? '(1 2 (three)) '(1 2 three))", "#f")
RMLT_CASE("(my-equal? '(1 2 three) '(1 2 (three)))", "#f")
// Peter Norvig tests (http://norvig.com/lispy2.html)
RMLT_CASE("(define double (lambda (x) (* 2 x)))")
RMLT_CASE("(double 5)", "10")
RMLT_CASE("(define compose (lambda (f g) (lambda (x) (f (g x)))))")
RMLT_CASE("((compose list double) 5)", "(10)")
RMLT_CASE("(define apply-twice (lambda (f) (compose f f)))")
RMLT_CASE("((apply-twice double) 5)", "20")
RMLT_CASE("((apply-twice (apply-twice double)) 5)", "80")
RMLT_CASE("(define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1))))))")
RMLT_CASE("(fact 3)", "6")
// RMLT_CASE("(fact 50)", "30414093201713378043612608166064768844377641568960512000000000000")
RMLT_CASE(
    "(define (combine f) (lambda (x y) (if (null? x) '() (f (list (car x) (car y)) ((combine f) "
    "(cdr x) (cdr y))))))")
RMLT_CASE("(define zip (combine cons))")
RMLT_CASE("(zip (list 1 2 3 4) (list 5 6 7 8))", "((1 5) (2 6) (3 7) (4 8))")
RMLT_CASE(
    "(define riff-shuffle (lambda (deck) (begin (define take (lambda (n seq) (if (<= n 0) (quote "
    "()) (cons (car seq) (take (- n 1) (cdr seq)))))) (define drop (lambda (n seq) (if (<= n 0) "
    "seq (drop (- n 1) (cdr seq))))) (define mid (lambda (seq) (/ (length seq) 2))) ((combine "
    "append) (take (mid deck) deck) (drop (mid deck) deck)))))")
RMLT_CASE("(riff-shuffle (list 1 2 3 4 5 6 7 8))", "(1 5 2 6 3 7 4 8)")
RMLT_CASE("((apply-twice riff-shuffle) (list 1 2 3 4 5 6 7 8))", "(1 3 5 7 2 4 6 8)")
RMLT_CASE("(riff-shuffle (riff-shuffle (riff-shuffle (list 1 2 3 4 5 6 7 8))))",
          "(1 2 3 4 5 6 7 8)")
// Additional tests
RMLT_CASE("(apply square '(2))", "4")
RMLT_CASE("(apply + '(1 2 3 4))", "10")
RMLT_CASE("(apply (if #f + append) '((1 2) (3 4)))", "(1 2 3 4)")
RMLT_CASE("(define (loop) (loop))")
RMLT_CASE("(cond (#f (loop)) (12))", "12")
RMLT_CASE("((lambda (x) (display x) (newline) x) 2)", "2")
RMLT_CASE("(let ((x 2)) ((begin (define x (+ x 1)) +) 3 (begin (define x (+ x 1)) x)))", "7")
// Scheme Implementations
RMLT_CASE("(define (len s) (if (eq? s '()) 0 (+ 1 (len (cdr s)))))")
RMLT_CASE("(len '(1 2 3 4))", "4")
RMLT_END_CASES()

#undef RMLT_BEGIN_CASES
#undef RMLT_CASE
#undef RMLT_END_CASES

#endif
