#include <cstdio>
#include <cstdint>
#include <experimental/array>
#include <variant>

template<typename T, std::size_t capacity>
struct Stack{
    constexpr auto& operator[](std::size_t index) noexcept{
        return array[index];
    }
    constexpr void push(T const& value) noexcept{
        array[size++] = value;
    }
    constexpr auto pop() noexcept{
        return array[--size];
    }
    constexpr auto peek() noexcept{
        return array[size - 1];
    }
    constexpr bool empty() noexcept{
        return size == 0;
    }
    std::size_t size = 0;
    std::array<T, capacity> array;
};

constexpr std::size_t str_to_size_t(char const* str, char const** endptr) noexcept{
    std::size_t i = 0;
    for(; *str != '\0'; ++str){
        char c = *str;
        if('0' <= c && c <= '9'){
            i = i * 10 + (c - '0');
        } else{
            if(endptr){
                *endptr = str;
            }
            return i;
        }
    }
    if(endptr){
        *endptr = str;
    }
    return i;
}

constexpr auto is_space(char c) noexcept{
    return c == ' ';
}

constexpr void skip_whitespace(char const* str, char const** endptr) noexcept{
    auto* line = str;
    while(is_space(*line)){
        ++line;
    }
    if(endptr){
        *endptr = line;
    }
}

template<std::size_t add_precedence, std::size_t mul_precedence>
constexpr auto precedence(char c) noexcept{
    return c == '+' ? add_precedence
        :  c == '*' ? mul_precedence
        :  0;
}

// Shunting Yard and Reverse Polish Notation (RPN)
template<std::size_t add_precedence, std::size_t mul_precedence>
constexpr std::size_t parse_expression(char const* str, char const** endptr) noexcept{
    auto* line = str;

    // Shunting Yard
    auto stack = Stack<std::variant<std::size_t, char>, 100>{};
    auto ops = Stack<char, 10>{};
    bool continue_parsing = true;
    while(continue_parsing && *line != '\0'){
        skip_whitespace(line, &line);
        switch(*line){
            case '+':
            case '*': {
                auto op = *line;
                line += 1;
                auto p = precedence<add_precedence, mul_precedence>(op);
                while(!ops.empty()
                    && p <= precedence<add_precedence, mul_precedence>(ops.peek())){
                    stack.push(ops.pop());
                }
                ops.push(op);
                break;
            }
            case '(': {
                line += 1;
                stack.push(parse_expression<add_precedence, mul_precedence>(line, &line));
                break;
            }
            case ')': {
                line += 1;
                continue_parsing = false;
                break;
            }
            default: {
                stack.push(str_to_size_t(line, &line));
                break;
            }
        }
    }
    if(endptr){
        *endptr = line;
    }
    while(!ops.empty()){
        stack.push(ops.pop());
    }

    // RPN
    auto result = Stack<std::size_t, 100>{};
    for(std::size_t i = 0; i < stack.size; ++i){
        auto item = stack[i];
        std::visit([&](auto elem){
            if constexpr(std::is_same_v<std::decay_t<decltype(elem)>, std::size_t>){
                // expression
                result.push(elem);
            } else{
                // operator
                auto right = result.pop();
                auto left = result.pop();
                switch(elem){
                    case '+': {
                        result.push(left + right);
                        break;
                    }
                    case '*': {
                        result.push(left * right);
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }
        }, item);
    }
    return result.pop();
}

template<std::size_t add_precedence, std::size_t mul_precedence, typename Array>
constexpr auto sum_of_expressions(Array const& array) noexcept{
    std::size_t sum = 0;
    for(auto const* str : array){
        sum += parse_expression<add_precedence, mul_precedence>(str, nullptr);
    }
    return sum;
}

constexpr static auto input = std::experimental::make_array(
    "(8 + 5 + 5) + (8 + (2 * 6) + (6 * 6 * 9 * 8) + 3 + 9 * 8)",
    "(7 + 7 + 3) * 8 * (5 + 9 + 4 + (7 + 8 + 3 * 3 + 9 + 7) + (7 + 6 + 6) * 8) * (4 + 5 * 9)",
    "8 * (3 * (3 + 8 + 4 * 9) + (7 * 9) * (3 * 3 * 3 + 6 * 8 * 2) * 7) + 5 + 2",
    "2 * (2 + 9 + 6 * (5 + 9 * 2 * 2 * 5 * 5) * (6 * 9 + 4 + 8 + 4 * 9))",
    "3 + 2",
    "2 * ((5 * 5 + 9 * 5 * 3 + 7) + (8 * 8 + 4) + 7) * 7",
    "8 * 3 + 4",
    "(5 * 3 * (9 * 9 + 4 * 3) + 2) * (3 + 2 * 2 + (8 + 6) * 3 + 7)",
    "((6 + 4 * 5 + 9 + 7 + 9) + 2 + 4 * 3 + 5) + 5 * 4 + (5 * (2 * 5 + 3 + 8 + 4) * 5 * 8) * 8 + 9",
    "5 * 7 * (2 * 7 * 2 * 4 + 9) + 8 + 3 + 7",
    "2 * (8 + 9 + 5 * 2)",
    "3 * 2 + 6 * (6 + (4 + 9 + 7 * 8 * 5)) * 9",
    "8 + (3 * 5 + (5 + 3) * (2 + 8 + 6) * 2)",
    "(8 + 3 * 2 * (5 * 2 + 3) * 8) + 6 + ((8 + 8 + 4 * 6) * (5 * 4)) + 7",
    "2 * (6 * (3 * 5 + 3) + (2 + 9) * 8) + 6 * 4 * 7",
    "(2 + 7 + 9 * (3 * 3 + 4) + 5 * 5) * 9 * 4",
    "8 + 9 * 5 + 3 * 2 + 7",
    "2 + 9 * 4 + (3 + 9 * (6 + 2 * 2) + 9 * (8 + 8 * 3)) * 8",
    "9 * ((2 + 7) * 4 + 8 + 9 * 9) + 8",
    "(3 * 7 + (8 * 7) + 4 * (3 + 3 * 7 + 7 + 6)) + 2 + 9 * 8",
    "9 * (7 * 6 + (2 + 4 * 2 * 9 + 4)) + 9 * 5 * 3 * (6 + (6 + 8 + 8 + 2 * 5) + 5 * 3)",
    "(6 + 5 * 6) * (3 + 8 * 6) + 8 + 7",
    "(5 * (5 * 2 + 4 * 4 * 5 + 2) * 9 + (2 + 6 * 4 * 2 + 3) * 6) * 4 + ((3 * 7 + 3) + 7) * 8 * 6 * 2",
    "5 + 7 + 9 + 6 * 9 * 3",
    "8 + 2 * 2 * (8 * (4 * 9 + 4 * 8 * 8) * 3)",
    "5 + 5 + 3 + 3 * 7 + (8 + (3 * 9) * 8 * 3)",
    "6 * 8 + (5 * 8 * 4 + (4 + 7 * 4 * 4 * 4)) * 6 * 8",
    "6 * 2 + (2 + 9 * (2 + 2 + 4 * 5 + 6) * (4 * 9)) + 2 * 4 + 6",
    "2 + (8 + (4 + 7 * 5 * 5 + 4)) * 7 + 5 * (2 * 3 * 5 + 6 * 9)",
    "2 * 3 + 5 + (2 + (7 + 5 * 3) * (4 * 3))",
    "(9 + (3 * 7 * 3) * 9) * 6",
    "4 * (2 * 3 * (4 * 3)) * 6 + 7 * 8 * 3",
    "((7 * 6 + 6) + 8) + 8 + (5 + 7 + 4 + 5 + 8) + 5 + 4",
    "(9 * 7 + (3 * 9 + 7 + 5)) + 7 * 2 * (3 * 8 + 6) * 2 + 7",
    "9 * 2 * (5 * (5 + 6 * 6) + 8 * 8 + 7) * (4 + 2) * 7 * 4",
    "3 * (9 + (2 * 6 * 4) * 8 + 9 * (9 * 5 * 9) + (2 + 4)) + 2 + (7 * 3 * 3) + 8 + ((2 + 6 + 7) * 9 * (3 + 6 * 5 + 5) + 7 + 3)",
    "2 * 3 + 8 + 3 + (9 * (3 * 7 * 5))",
    "(2 * (6 * 7 + 9 * 6 + 9) * 6 + 2) + 5 * 9",
    "(5 * 3) * 2 * 4 * 6 + 9 * (6 * 7 + 9 + 5 * 9 + 8)",
    "2 * 3 * ((8 * 8 * 7 * 7) * 2 * 7 * 5 + 4) * (3 * (3 + 4 * 7 + 4) + 7 * (2 * 2) * 8) * (6 + 6 * 7 + 6 * 7)",
    "9 * 6 * 9 + 6 * 4 + ((7 * 5 * 8 * 6 * 3) + 2 + 2)",
    "(9 + 7 * 6 * (3 * 8 * 7) * 9 + 9) * 3",
    "6 * 5 * 9 * 5 * 9 * 9",
    "3 + 9 + 8 * 7 * 8 + 4",
    "(3 * (9 * 5 + 8 + 5 + 9)) * 2 * 7",
    "(2 * 9) + (4 * (6 + 4 * 3 * 7 + 4) + (9 * 9 + 8) * 7 + 3) + 7",
    "(5 * 7 + 2 + 4) * 2 * 8 + 7",
    "((7 + 3) + 7 + 3) + 5 + 2",
    "3 * 4 * 7 + ((2 * 2 + 9 + 5 * 9) * 9 * 7 * (3 * 4 * 9 * 5)) + 9",
    "7 * 7 + 9 * (4 * (8 * 8) + 4 + 2) * 4",
    "7 + (4 + 7 + 9 + 3 + 7) + 8 + 7 * (6 + 8 + 9 + (9 + 2 * 5 + 7) + (4 * 9) * (2 * 6))",
    "5 + 9 * (2 + (7 + 5 * 4 * 3 + 4 + 4) + 8 * 8 * (4 * 8))",
    "6 + ((4 * 4) + (2 + 4 + 5 + 9 + 4 * 7) + (6 + 7 * 4 * 7 * 9 * 2) + 8 * 8 * (7 * 4)) + 4 + 9 + 8",
    "(6 + 4) + (3 * 9 + 3) * 4 * 9 * 9 * 8",
    "(6 * 8) * 8 + 4 * 4",
    "7 * 5 * 9 * 7 + 8 + (3 * 8 * (6 * 4 * 7) * 9 * (8 * 3 * 7 + 4 + 7))",
    "(2 * 8 * (7 + 5 * 3 * 6)) * 5",
    "6 * (8 * 9 * (3 * 9 + 4) * 7 * 3 * 6) + 3 + 6",
    "5 * 7 + 5 + 6 * 4 * (3 * 2 + 4 + 6 * (9 * 4 + 3 * 4 * 3 * 3) + 2)",
    "2 * ((4 + 3 * 8 * 2 + 2 + 8) + (6 * 6 + 5 * 4) + 7 + (4 + 4 + 5 + 3 + 5)) + 8 + 7 + 2 + 6",
    "(5 + 6) + 8 * (8 + 6 + 6 + 8 * 8) * 7 * 9 + 7",
    "(3 * 3 + 4) * 7 + (2 * 8 * (5 + 4 * 3 * 6 * 7 + 6) + 9 * 6 * (5 * 2)) * 8 * 6 * 2",
    "7 * 2",
    "(4 * (4 + 5 + 7 * 9 * 7 * 2) + 5) * 7 * 7 * 3 + 7 + 8",
    "2 * 2 * 6 * 9 * (9 * 5 * 4) + (6 + 2 + 7 * 2 * 6 * 3)",
    "9 + 6 + ((4 * 6 + 7 * 8 + 3) + 9 + 8 * 9 + (4 + 4) + 7) * 6 + 3",
    "5 * 2 + ((4 + 2 * 6) + (7 + 9 * 4)) * 7 + 4 + 2",
    "(8 + 6) * 9 + ((4 + 2 + 8 + 7) + 9 * (6 * 8 + 8 * 7 + 5))",
    "2 * (7 + 6) + 3 * 3 * 9",
    "(4 * 8 + (8 * 4 * 2 * 7 * 8) + 5 + 9) * 7 * 3 * 5 + 7",
    "7 + 8 + 5 * 2 * (3 + 3)",
    "5 + 5 * (4 * 4)",
    "2 * (9 + 2 + 9) * (3 * 7 + 2 + 4)",
    "(9 * (8 * 5 + 5 + 4)) * 7 + 6 * 9 + 8 + 3",
    "(3 + 2 + 4 + 2 + (5 * 3) + 3) + (4 * 5 + (2 * 5 * 5) + 9) * 3 * 4",
    "4 * (5 * 8 + 3 * 5 + 5) + 5 + 4",
    "(6 + 5 * 4 + 9 + 8 * 6) + 3 * 5 + (8 + (2 + 4) + 2 * 7 + 3)",
    "5 * 2 * ((3 + 2 + 2 * 7) + 8 * 8) * 4 + 3 * 8",
    "6 + 4 + (4 * 2 * 3 + 7 + 3 * 9) * 2 + 7 + 7",
    "2 + 3 + (3 * 3 + 8) + 8 + (4 + (5 + 8 + 5 + 4) * 9 + (6 * 8 + 7)) * 6",
    "5 * 2 + 4 * (6 * 6)",
    "8 + 4 + (8 * (5 * 5 * 8 + 3 + 5) + 8 * 5) + ((9 * 3 * 4 * 8) + 9 * 7 * 7) * 3 + ((9 * 5 * 4 * 8) * 2 * 2)",
    "5 * 9 * 6 + (9 + 9 + 8 + 8) * 9",
    "7 * 6 * 8 + 6",
    "3 + 5 + 3 * (5 + 3 * 4 + (8 * 4 * 9 + 3) + 4 * 5) + 4",
    "6 + 7 * 8 * 3",
    "4 + 4 + ((9 * 9 * 5) + 3 * (4 + 9)) * 9 + 9",
    "4 + 5 * 3 + (5 * 6 + 4 * 2 + 8) + ((3 + 3 + 9 + 3 + 3) + 5 + 5 * (7 + 6 + 3 * 7) + 9)",
    "3 * 5 * (2 + 5 * 9 + 3) * 7 + (9 * 6 + 5 * 5) * 7",
    "(9 * 2 * 3) * 5 * 9 * 9 + (2 + 2 + 8) * 4",
    "5 * (7 + 7 + (5 * 2 * 3 + 3 * 4 * 8) * 4 + (7 * 6 + 9 * 3 * 9 * 9)) + 6 + 2 + 6 + 9",
    "(9 + 5 + (9 + 7 + 4 + 7 * 7 + 3) * 3) * (5 + (2 + 2 + 4) * 3 * 6 * 5 + 2) * 3 * 4",
    "9 * (5 + 6 + 9 + 2 + 6 + 4) + 7 + 3 * 2 + (7 * 7 * 8 + 7 * 3 + 2)",
    "((9 + 4 + 7 * 4 + 4 + 7) + 6 + (3 + 7)) * 4 + (8 * 2 + 3) + 2 * 3",
    "(6 + 8) * (7 + 3 + 9 + 2 * 5 * 8)",
    "(9 + 6 * 2 * (6 + 8 + 2 * 9 * 9 * 2)) + 7 * 6",
    "(4 + 9 + (7 * 8 * 8 * 4) + 6) + 6 + (8 + 9 * (9 * 5 * 6) * 5 + (4 * 2 + 2) * 7)",
    "8 * (2 * (5 + 2 + 6 + 9) + 8) * 8 + 8 + 6",
    "6 + 8 * 4 * 2 + 2 * 3",
    "4 * (6 + 6 + (6 * 8 * 6 + 8 * 6 + 5) + 2 * 4) * 2 * 9 + 3",
    "4 * (4 + 4) + (5 + (2 * 3) * (4 + 4 + 7 * 4 + 2 + 3) * 6) * 8 * 4",
    "3 * 2 + (3 * 9 * 9)",
    "((5 * 3 + 7) * 5 + 7 + 6 + 2 + 3) * 5 + (9 + (8 + 4 + 2 * 2 * 9) * 8 + 9 * 5) + 7 * 3 * 9",
    "(3 + 3 * 2 * 5) * 2 * 6",
    "8 + 6 + 2 + 9 * (6 + 5 + 4 * 9 + (4 * 7 + 4 + 9 * 6 + 4)) + 5",
    "5 * 7 * (9 * 5 * (8 + 4 * 2 + 5) + (8 + 6 + 7 * 3)) + 2 * 9",
    "(8 * 7 * 2 * 2 * 8 + 6) + ((8 * 7 * 2 + 4 + 5) * 2 + (7 + 8)) + 3 + (9 * 6 * (6 * 9 + 4) * (8 * 9 * 3 * 2 + 3) + 7 * 5) * 2",
    "(2 + 7 * 9 * 6) * (5 + (7 + 8 * 5 + 5 + 4) * 4 * 4 * 2 + 6) + 3 + 2",
    "(6 * 5 + 3) * 5 + 7 + 9",
    "(3 + 7 + 6) * 4 * (8 + 9 * 9 * 7 * (9 + 2) * 8) * 9 + (8 + 3 * 2 + 4 + 9) * 8",
    "4 + ((7 * 5 * 9 * 5 * 7) * (5 + 4 * 3 + 4 + 3) + (7 + 5) + 9) + 9",
    "6 * ((4 + 9 + 8) + (5 * 4 * 6) * 6) * (4 + 5 * 2 + 8 + 2 * 3) * 8",
    "4 + 4 + 4",
    "8 + (8 + 2 + 8 + 4) + (2 * (7 + 7 + 8) + 7 * 5 * 7 + 6) * 3",
    "(2 + 7 + 7 + 2 * 3) * (7 + (6 + 6 * 4 * 6) + 6 + 3) + 5",
    "6 + ((2 * 8 * 8) + (5 + 2 + 4 * 6 + 7 + 5) * 2 + 3 * 4 + 9) * 3",
    "(4 + 6 + (6 + 7) + (2 + 3 + 3 * 4)) + 4 + 2 + 7",
    "(9 * 3 * 9 + 3 + 2) + (9 * (2 + 2 + 2)) * 6 + 6 * 9",
    "3 * 9 * 8 + 8 * (5 * 6) * (4 * 5 + 3 + 6)",
    "(4 * 9 + 2 * 6) * 4 * (9 + 7 * 3 * 5 + 2) * 3 * 4",
    "(5 + (4 * 5 + 6 * 4) * 5 * 7 * 8 * 4) * 3 + 8",
    "2 + ((8 + 3 + 3 + 8 * 6) + 5 * 6 + (3 * 7 * 4)) * 8 + 4 + 6",
    "6 * 4 * 4 * (3 * 9 + (4 * 4) * 4 + 5) * 5",
    "8 * (8 * (8 * 2 * 7 * 8 * 8))",
    "3 * 9 + 5 * ((6 + 6 + 2 * 3 * 7) * (2 + 9 * 3) + (4 + 5) * 5) * 7 * (7 * 4 + 6 * (8 + 4) + 9)",
    "2 + (2 * 4 + 4) + 8 + (5 + 6 + 3 * 3 * 2)",
    "(5 * (7 + 2 + 5 * 7 * 6) * 8 + (3 * 8) * 4) * 3",
    "(5 * 2 * (4 + 8 * 5 + 8) * 4 + 4 + 2) * 7 + 6 * 8 + ((7 * 7) * 4)",
    "3 + ((9 + 8 * 6 + 8 * 4) + 3 * 6) * 5 * 5 * 4 * 2",
    "(4 * 3) + ((6 * 4 * 5) * (9 + 7 * 4 + 2 + 2)) + (6 + 6 + 3 + 2 * 2 + 4) * 7",
    "5 + (4 + 8 * 5 * 8 * 3) * (6 * 5 * 3 * (2 * 7 + 9 * 3 * 3 * 2) * 3) * 3",
    "((4 + 2 + 3 * 8 * 2) * 4 + 6 * 2) * ((5 * 8 * 6) + (6 * 3 * 8 * 8) * 7 * (6 + 2 * 9))",
    "(4 + 2 + (4 * 7 + 3 + 6 * 7 * 4)) + 6 * 7 * 9",
    "5 + ((2 + 7 + 7 + 3) * 6 * 5) + 6 * 3 * 6 * 7",
    "7 * ((4 * 4 * 4 + 3 + 3) * (4 + 4 + 6 + 3 + 3 * 9)) + 4 * 6 * 8 + ((4 + 6) + 4 * 6 + 6 + 3)",
    "9 * (4 * 4 + 5 + 3 * (9 * 7 * 5)) * 6 + 4 * 2 + 6",
    "(5 * 6 * 4 * 4 + 6 * (5 * 4 + 4 + 5 + 4 * 7)) * 6",
    "((3 + 3) + 7 * 3) * 6 * (2 + 5)",
    "(2 * 3 + 3) + (5 * (3 + 2 + 3 + 4 + 8 * 7) * 7 * (4 * 5 * 7) * 8) * (7 + 2 + 2) * 7 * 2 + 7",
    "3 * 6 * 7 * 5 * (9 * 5 * 5)",
    "2 + 8 + ((6 + 2 * 4 * 2) * 8 * 4) + ((5 + 5 + 6) * 7 + 6 * 9 + 6 + 9)",
    "2 * 7 + 2 * 6 * 6 * ((6 * 5 + 7 + 4 + 4 + 2) * 4 + 8)",
    "5 + (2 + 7 * 7) * 3 + 3",
    "(5 + (9 * 8)) * 6 + 4 + 3 * 6",
    "6 * (8 + 5 + 3 * 5) * 8 + 4 + 8 + (4 * 5 * (6 + 8 + 5 + 2) + 6)",
    "4 * 6 + (8 + 3 + 5) + 8",
    "((5 + 6 * 3 * 5) * (8 + 7 + 6 + 5 * 7 * 7) * 8 * 5) * (3 * 7 + 5 * 8 * 2 * 5) + (2 + 7) * 2",
    "(3 + (8 * 9) + 5 + (8 + 9 * 7)) + 4 * 4 * ((8 * 6 + 9 * 9) + (2 * 6 * 8 + 5) + 8) * 3",
    "3 + 6 * (8 * 9) * 9",
    "9 * 9 + 4 * (8 + 5 * 5 * 7 + 4)",
    "4 * ((4 + 6 * 9 + 9 * 2 * 5) + 2 + 2 * 9 * 6) * 7 + ((8 + 2 * 6 + 3 * 2) + 4) + 6",
    "4 + (5 * 9 + 8) * (8 * (6 + 9 * 9 + 3 * 3) + 9 + 2 * (7 * 2) * (5 + 8 * 5)) + 2",
    "(4 + 5 + 6 + 6 + 8 * 8) + 2 * 7 * 4 * (5 * 2 + 6) * 7",
    "(8 + 2 * 3 + 8 * 2 * 9) * 6 + (6 + 6 * 2 + 6) + 9",
    "5 + 7 + (2 + 5 + 4 + 4 * 2 * 6)",
    "((4 * 7 + 7) * 8 * 9 * 8 + (3 + 7 * 3 * 5) + 9) * 3 * 5",
    "(6 + 9 + 5 + 3 + 5) * ((6 + 5) * 8 + 4 * 2)",
    "3 + 6 * (4 + 8 + 5 + 2) + 8 * ((4 + 6 + 6 + 4) * 4) * ((6 * 6) * 3 * 6 + 8 * 7)",
    "(9 * 2 + 5 * 5 + 5) * (3 * (7 * 3 * 9) * 2 * 5 * 4) * 4",
    "7 * ((4 + 7 * 8 + 2 * 5 * 9) + (3 + 8 * 5 + 8) * 2 + 2 + (2 * 6 * 8) * 7) + (8 * 5) * (2 * 2 * 4 * 5) + 7",
    "7 * 3 * 2 * (3 * (7 + 3 + 3 + 9 * 7) * 5 + 4 * 2 + 7) * 7 + (3 * 7 + 8 * (8 + 2 + 9 * 5) * 9 * 3)",
    "4 + 7 + 9 * (6 + (4 + 9 + 6 + 7 + 4) + (6 * 3) * (3 + 3 + 9 * 4 + 8 * 4))",
    "(2 + 7 * 9 * (8 + 5 + 2 + 5 * 7 * 3) * 9 + 5) * 4 + 7",
    "7 * 8 + 2 + 7 + (3 + 7 * 8 * 2) + (6 + 7 + 3)",
    "3 + 3 + (5 * 4 * (2 + 9 + 5 + 8 + 3 + 7) * 4) * 6 + 8 + (3 * 8 * 9)",
    "5 + (9 + (8 * 5 * 7 + 3 + 2) * 3 + 4 * 6 + 6) + 6 * 9 + 7",
    "4 * (4 * 3 * (4 + 6 + 2 * 8 * 7)) * 5 * 6",
    "5 * 5 * 9 * (9 + (8 + 3 * 4 + 3 + 6 * 6) + 7) + 7",
    "9 * 4 * 6 + (4 + 9 * 6 + 4) * (8 + 5 + 9) * 3",
    "7 + 7 * (3 * 7 * 3 + 3 * (4 * 2 * 6)) + (3 + 7 * 6 + (5 * 6) * 5) * (8 + 5) * 7",
    "7 + 4 + 7 * 2",
    "5 + 7 * 2 * 9 + (4 + (5 * 3 + 6 * 6 + 3 * 3))",
    "4 + (3 + 3 + 7 + 2 * 8) + (6 * 7)",
    "((6 + 8 * 6) * 9) + 8 + (5 + (3 + 6 + 9) * 5) + 3 * 3 + 5",
    "6 * (2 + 6 * (8 * 9 + 3 * 4 * 7) * 9) * 9",
    "(8 * 7 * 6) * 7 + 7 * 6 + 3",
    "3 * ((4 + 4 * 6 * 4 * 4 + 5) * 3 * 6 * (3 * 7 + 8 + 5 + 2) * (5 + 5) + 6) + 8",
    "5 * (6 * (9 + 5 + 7 + 8 * 4) + 9 * 3) * (3 * 8 + 3 + (6 * 4 * 3) * 7 * 7) + 5 * (6 * (7 * 4 + 9)) + (8 * 5)",
    "6 * 8 + ((4 * 4 + 4) + 9 * 4 + 8 + 5) + 6",
    "((5 * 6) * 4 * (6 + 3 + 7) * (5 + 7 + 5 * 5) + 6) * 7",
    "(4 * (2 * 9 * 6 * 8 * 9) * (5 * 4 * 3) + (5 * 7)) + 3 + 7",
    "(5 + (3 + 5 * 5 + 2 + 8 * 5) + 7 * 8 * 6 + 7) * 9 * 3 + 7 * 4",
    "(7 * 4 + 4 * 7 + 4) * 6 + 3",
    "6 + 4",
    "(6 * 3 * 9 + 8 + 7 * 5) + ((5 * 9 * 6 + 6 * 7) * (4 * 7 + 9 * 7 * 8) + 2 + (5 * 7 * 7 + 9 * 5) + 4 + 9) * 5 + 5 + 4 * 6",
    "7 * (9 * 2) * (4 + 7 * (2 * 6 * 6))",
    "(7 * 2 + 4) * 2 + 8 + 7 + 3 * ((4 * 7 * 7 * 6) * (8 * 4 * 7 * 2 * 7 + 5))",
    "4 * 9 * 9 + 2 + (3 + (8 + 8 + 8 + 3 + 9) + (5 * 7 + 5 * 3 + 4 * 8) + 6 * 3 * (3 + 3 * 6 * 8))",
    "3 + (9 * 9 * (5 + 9) + 8 * 6 + 3) * 9 + 5 + (7 + 9)",
    "(5 * 7) + 5 * 4 * 6 * 5 * 7",
    "9 * (4 + 4 + 3 + 3 + 6)",
    "3 * 8 * (9 + 3 + 6 + 6 + 8 + 5) + (4 * 2)",
    "3 + 6 + 4 * 6 * (9 * 8 * (7 + 6 * 8 + 9) * 4) + 9",
    "4 * 6 * (6 * 8 * (2 * 8 * 5 + 5) + 8)",
    "9 * 5 * 4 * 9 + (3 + 9 * 4) + 6",
    "5 + 7 + (2 + 2 * (2 * 8) * 2 + 6 + 8) + (9 * 3) * 7",
    "5 + (2 * 3 + (8 * 8 * 2 * 2 * 7 * 7) + 7 + 8 * 4) * 2 + 4",
    "8 * ((6 + 3 + 6 * 3 * 7) + (6 * 5 + 2 * 3 * 4 + 9) + 2 * (9 + 7 * 7)) + 7",
    "7 * (7 * 5 * 3) + 2 + (2 * 6 * 5 * 4 * 3 + 2)",
    "7 + 8 * 4 * 2 * 9 + (8 * 2 + 7)",
    "((4 * 8) * 5 + 2 + 3) * ((8 + 4) + 5) + (6 * (9 + 4) + (2 + 4 * 8 * 9 + 3 + 6) * 5) * ((9 + 9 * 3 * 5 * 8 * 9) + 9 + 3) + 2",
    "2 * ((8 * 3 + 5 + 7 + 3) * 9 + 7 * (2 * 6 * 6) + (7 * 7 + 4 * 9 * 8) * (9 + 9 + 3)) + (3 * 7 + 5 + (5 + 4 * 7) + (2 + 9 + 3 + 5 + 2 + 2) * 2) + 3 + 3 * 4",
    "(6 + 3 * 9 * (7 + 6) * 6) * 5 + 3 * 9",
    "(6 * 9 * (6 + 7 + 6)) + (7 * 8) * 7 * 3 + 5 + ((3 + 2 + 5 * 5) * 9 * 8)",
    "5 * (8 * 9 * 8 + 9 * (5 + 4 + 9 + 5 + 9 + 4)) + 9 + 4 * 3",
    "(8 * 3 + 8 * 3) + 3 + 9 + 8 * 9",
    "(2 * 9 * 4) * 4 + (2 * 4 * 8 + (6 * 2 * 7 + 6) * 7 * 8) + (5 * 5 * 8 + 8) * 5",
    "(7 * 7 + 2) + 7 * 6",
    "5 + 2 * 6 + 8 + (3 + 4 + 9 + 6)",
    "5 + 8 + 8 + 5 * (3 * 3 + 8 * 9) + 6",
    "4 * 4 * ((4 * 4) * 7 * 3) + 6 + 8",
    "3 * 6 + (8 + (8 + 9 + 4 * 5 * 2) + 5 + (3 + 5)) * 3",
    "5 * 3 + (6 + 5 * 4 * 2 * 2) + 9 * 6",
    "5 + 3 + 8 + 6",
    "7 + (9 + 3) + 6",
    "(9 + 5 * 5) * 7 + 9 * 7 * 4 + 9",
    "7 + (8 * 8 + 9 + 3) + 4 + 9 + 4",
    "8 + ((9 + 3) + 9 + (9 * 9 + 2 + 9 + 7 * 2) * 8 + 5 * 2) * 6",
    "(3 * 7 * 3 * 2) * 2 * ((5 * 8 * 7 * 2 + 2) * 9 + 8 * (4 * 5) * 2) + 9 * 6 * 9",
    "3 + 9 + (3 + 7 + 2) * 3 * 9 * 8",
    "(7 * 9 * 7) + 9 + 5 + ((5 * 8) * 6 * (3 * 8) * (7 * 8 * 6 * 2) + 5 * (5 + 5))",
    "3 * 7 + (6 + 4 * 5 * 3 + 4)",
    "3 * 4 * 2 + (4 + 3 + 3 + 7 * (6 * 4 * 7) * (6 * 4 + 5 * 3 * 2))",
    "6 + (8 * 4 * (2 + 5 * 9 + 3) * 7 * 8)",
    "8 + (9 * 7 + 3) + ((2 * 8 * 4 + 9 * 3 * 5) + 3 + 3) * (3 + 9 * 7 * 3 * 2) + 3",
    "2 * 9 * (5 + 8 + (8 + 3 * 9 * 7 * 2 * 4) * 8) * (5 + 7 * (5 + 7 + 2 * 9 + 8 * 4) * 3 + (7 + 7 + 5)) * 9 + 3",
    "(8 + 9 + 7 + 5) * (8 * (4 + 7 * 4 * 3 * 3 * 9)) * 4",
    "((8 * 7 + 3) * 2 + 2) * (4 + 9 * (4 + 2 + 6 * 4 * 4 * 7)) + 5 * 3",
    "9 + (6 * 6 * 8 + 5 + 6) * 8 * 7 * 2 * 4",
    "7 + 5 * 9 * (3 * 7 + 2) + ((7 + 5 + 5 + 3) * 5) + 9",
    "(9 + 7 * 4 * 8) + 8",
    "8 * 5 + 5 * (3 * 2 * 7)",
    "7 * 8 * 5 + (9 * 8 + 7 * (9 + 5 * 7) + 4 * 4) * 5 + 8",
    "(3 + 9 + 6 * (7 * 5) + 5) + 8 * 5 + 8 + 4",
    "8 * 6 + (6 * (4 * 7 + 2) + 4 * 2) * 2",
    "(3 + 8 * 8) + (6 * 7 + 8 + 2 * 3) * (5 + 5) * (5 * 9 * 2 + 8 + 8 * 9)",
    "9 * (6 + 4 + (7 + 6 + 7 + 9 * 8) * 6) * 6 * 5 * 3 * 9",
    "((3 + 9) + 4 * 3 * 3) * 5 + 6 * (6 + 5 * 5)",
    "3 * 2 + ((3 * 3) + 6 * (4 + 8 * 4 * 3 + 6) + 4) * (9 * 4 + 2 * (5 * 4 + 2 + 4 + 7)) * 2 * (7 * (6 + 8 * 5) + 7 * 8 + 2)",
    "(4 + 8 + 5) * ((3 * 2) * 9 + (5 * 6 * 7 + 6 + 8) + 4 + 9 + 6) + 5 * 6 + 4 * 6",
    "(3 * (3 * 9) * 2 + 6 * 8 + 6) * 5",
    "((9 * 8 * 4 * 4) + 5) + (2 + 8 + 6) * 4",
    "(3 * 9 + 6 + 3 + 6 * 7) + 4 * 7 * 7 + 6 + 9",
    "3 * 3 * 5 + 5 * 9 + (3 * (3 + 3) + (3 + 9 + 5 + 4 + 9))",
    "9 * ((6 + 9 * 5 + 3 + 2 + 4) + (8 + 5 + 4 * 6 * 6)) + (6 * 4 + 9 + 8 + 5 + 7)",
    "9 * (8 * 8 * 4 * 8 + 4) * 7",
    "5 * (5 * (6 * 6 + 4 + 7) * 5 * 2 * 5 + 6) + (4 * 5 + 2 + 6 * 6 * 9) * (2 * (5 * 6 + 7 + 8 + 2))",
    "7 * 7 * 3 * 8 * 7",
    "2 + 2 * 9 * (9 * 2 * 2 * 4 + (3 + 4 * 2 * 9 + 9 * 6) * 8) + 4 + 9",
    "(6 + 7) * 4 + 3 * 6 * (9 + 2 + 3)",
    "4 * (6 * 3 * 4) * ((4 * 3) * 7) * ((8 * 7) + 8 + (6 * 9 * 3))",
    "(4 * 4 + 7 + (6 * 6 + 5 * 8) * (8 + 2 + 3) * 8) + 7",
    "2 * (5 * (3 + 7 * 2) + 5 + 8 * 7) * 6 + (9 + (2 + 6 + 9 * 4 + 3) + (9 * 8 * 5 * 6 * 3 * 4) + 9 + 7 + 8) + 8",
    "((7 + 5 + 2) + 6 + 6 * (4 + 8 + 8) + 5) * (4 * 4 * 9 * (4 * 5) + 2 + 5) * 7",
    "2 * 6 + 7 * (7 + 2 + 5 + (5 * 3 * 4 * 9 * 6 * 7) * 9 + 3) + 5",
    "9 + 6 * 7 + ((8 + 2 * 5 + 3 + 5) + (7 + 2 + 9 * 2) + 5) + 5 * 6",
    "8 * (7 * 3 + 7 + 7 + 8) * 3 * (5 * 4 * 2 + 4 * 2)",
    "7 + 7 + 3 * 8 + ((5 + 3 * 7 + 2 * 4 * 4) * 8 * 4 + 7)",
    "8 + 8 + 4 * 3 + 9 * (2 + 5 + 3 * (6 * 2 + 5 + 2 + 2))",
    "7 * ((2 * 5 + 4 + 9 * 2) + 8 * 8) * 2 * 9 * 3",
    "5 * 8 + ((2 * 9 * 8 * 7 + 5) + 2 * 8 + 7) + 5",
    "(8 * (9 + 5 * 3 * 5) * (3 * 3)) + 5 + 2 * 5 + ((4 * 5 * 5 + 3 * 9 + 7) * 5 * (6 + 9 * 5 * 8) + 7) * 8",
    "(4 + (9 * 9) + (4 + 4 * 6) * 5 * (9 + 2)) * 3 + 9",
    "6 + 3 * 3 * (8 * 3 * 5 * (7 + 4 + 6 * 8 + 6 * 9) + 9)",
    "8 + (6 + 7) * 4 * 6",
    "(4 * 5 * 8 + 3 * 2) + 2 + (7 + 2 + 3 * 7 + 6 + 7) * 5",
    "5 + (2 * 7 * 6 + 2 * 9 * 6) * 2 * 3 + 3",
    "8 + (3 + 6 + 2) + 5 + 7 + 4",
    "5 + 9 + ((4 + 9 * 8 * 2 + 7) * 7 + 6 * 9 + 9) * 7",
    "((8 * 7 * 9 + 8 + 9 + 9) + 2 + 5) + 9 + 3 + 4",
    "8 * 6 * (4 + 2 + (4 + 3 * 5) + (7 * 4 + 9 * 9) + 9 + 4) * 4",
    "9 + 3 * 6 * 6 + (9 * 9 * 4 + (7 + 9 + 5 + 7 + 3)) + ((8 * 9 + 9 + 2) + 8 * (8 * 7 * 3 + 6 + 2 * 4))",
    "6 * 3 * 9 + (7 * (6 * 5 + 2 + 7))",
    "5 + 8 * (3 + 5 * (6 * 9 + 5 + 9 + 5))",
    "4 * 7 * 8 * (4 * (7 + 2 + 7 * 8 + 6 + 6) * 6 + 3)",
    "7 + (4 * 6 * 8 + 9 + (6 + 2 + 4 * 7 * 2))",
    "3 + 7 * 5 * 3 * (9 * 4) + 6",
    "(6 * 8 * 9 + 6 + 5) * 9 + 5 * 4 * 7",
    "9 + 4",
    "3 * (3 * 7 * (3 + 3 * 9 + 3 + 5) + 6)",
    "5 + 3 * (2 * 5 * 3) * 4 * 8",
    "6 + ((8 * 7 * 6 + 2 + 5 + 2) + 2) * 7 + 5",
    "3 + (9 + 2) * (4 + (9 + 8 * 2 + 5 + 5 + 5) + 7) * 4 * 2 + 5",
    "7 + 6 * 7 + 8 * (5 * 7 * 9 * 9 + 2)",
    "(9 * (3 * 8 * 9 * 8 + 7)) * 3 + 2 + 2",
    "(4 + 3 * 7 * (9 + 9)) + 5 + 9",
    "2 * 2 * (7 + 6 + 8 + 5 + 8 * 3) + 9 + (4 + 9 * 5 * 7 + 2 + 7) * 9",
    "3 * 2 * (5 * 8 * (5 + 6) * 7 * 2) + 7",
    "9 * 2 * 7 * (9 * 5 * 8 * (5 * 8 * 4 + 5 + 3))",
    "(8 * (3 + 2 * 4 * 4 + 4 * 2) + 2 + 8 + (7 * 3 * 6 * 6 + 5 + 9) * 9) + 5 + 7 * 9",
    "(5 * 3) + 3 * 2",
    "7 * 2 * 9 * 9 * 8 + (7 + 4 + (3 * 6 + 6 + 2) + 5)",
    "9 + ((6 + 2 * 9 * 2) * (7 * 4 * 6 + 3 * 7) + 5 + 9) + (5 + (5 * 4 * 5 * 8 + 8) + 5 * (3 * 2 * 2 + 4 + 3 * 4))",
    "(6 * (8 + 9 + 3 * 3)) + 7 * 6 * 4 + (9 + 4 * 2) + 4",
    "((2 + 2) + (8 * 7 * 6 + 4 * 2 + 6) + 3 + (5 + 8 + 2) * 2) + 6 * 9 + 2",
    "(3 + 6 + 7 * 2) * 9 + (4 * 9 + (2 + 6 + 4) + 4) + 6 + 9",
    "2 + 3 * 3 + 8 + (4 * 4 + 5 + 4 * 4 + 8) * 6",
    "(9 * 3) + 9",
    "8 * 9 + 4 * (7 * 6 + 3 * 3 + 5) * 8",
    "9 + 6 * 7 + 3 * ((8 + 3) + 9 * 3 + 8 + (3 + 7 + 7 * 4) + 7) + 4",
    "9 * 2 + 9 * ((8 + 7 * 8 + 7 + 7 + 4) + 4 * (9 * 8 + 3) * 2 + 4 + (5 * 5 * 5 + 2 + 2 + 9)) * 7",
    "3 + (8 + (6 * 9 + 6 + 9 + 4 + 9) + 5 * 2 + 6 + 6) + (9 * (8 * 2 + 8 * 8 * 9))",
    "5 * (5 + 9 * (8 * 4)) * (7 * 6 * 8) + 2 + 3",
    "9 * 5",
    "6 + 6 + 9 + 4 + (4 + 9 + (3 + 2) * 2 * 6)",
    "(6 * 4 * 7 * 3 + 7) + 5 * (8 * 4 * (4 + 8 * 4) * 9 * 2 * 3) + 9 + 9 * 2",
    "(8 * 4) + (5 * 4 + 4 * 4) * ((5 * 4 * 3 + 8) * 8 * 3 * 3 * 9 * 5) + 3 * (4 * 9 * 6)",
    "8 * 4 * 6 + 2",
    "(7 + 2 + 3 * (8 + 3)) + 2 + 9 * (2 * 2 * (4 + 6 * 6 + 3 * 2 * 8) + 7 + 7) + 8 * 7",
    "3 * (6 + 4 * (5 * 4 + 5 * 4 * 5 * 2) * (5 * 4) + 9) + 4 + 6 + 4 * (5 + 6 * (2 * 7 + 6 + 2))",
    "9 + 8 + (3 * 6 + 7 + 8 * 9 * 4) * 8 * 9 * ((3 * 8 * 8 + 9 * 7 + 6) * 4 * 2 + (2 * 2 + 4 * 3) + (6 * 6 + 9 + 9 + 8) * (8 + 8 * 4))",
    "5 + (7 * 6 * 6 * (3 * 2 * 3 * 6 * 7 + 7) * 7)",
    "9 * 9 + 3 * ((7 * 6 + 5 + 4) * 8 * (7 * 8 + 5 * 8) * (4 + 4 + 8 * 8 * 7 * 7) + 3 + 8) * 6",
    "2 * ((4 + 2 * 9) * (8 * 3) * 7 + 9) * 6 * 5",
    "(3 * (7 + 7 * 2 + 9) + (3 + 8) * 5 * (2 * 4 + 5 + 6 * 9) * 2) + 5 + 8",
    "2 * (2 + 8 + 5 * 6) * 2 + 7",
    "7 + (6 + 3 + 5 * 6 + 7 * 2)",
    "4 + 2 * 3 * (6 + 3 + 5 * 4 + 2) * 9 * (8 * 7 * 5 * 8)",
    "5 + (4 * 2 * 4 + 3 + 7 + 5) * 7 + 7",
    "8 + 2 + 4 + 7 + 2",
    "9 + 4 * 7",
    "6 + 5 * (5 + 9 * 7 * 6 * 5 + 7) * (7 + (6 * 9 * 7 + 5 + 6 * 4) + (9 * 9 + 7 * 9)) * 4 + 5",
    "7 * 7 * (8 * 3 * 3 + 9) + ((2 + 2) + 7 * 7 + 7 + 7 + 3) * 8",
    "2 + (4 * 7 * (7 * 5 * 4 + 5 + 5 + 5) + 7) + 5 + (6 + (9 + 6 + 6 * 3 + 3 + 5) * 2 * 9 + (3 * 7 * 5 + 5)) * (5 * 4 * 2 * 6) * ((7 + 4 + 7 * 2) * 7 + (5 + 7 * 7 * 3 * 4) * 8)",
    "((3 + 4) * (9 + 5 * 7 + 5 * 7 + 5) * 4) * 6 * 4 + 9",
    "(6 * 9 + 5 + (6 + 8 * 8 * 6 + 6) + 6 + 7) + 7 + 5 * 2 + 2",
    "8 * (7 * 4 + 5) * 8",
    "5 + 8 + 9 + (6 + 8) + (6 + 4 * 4 * 4 * 5 + 5) * 5",
    "4 * 6 * 9 + 8 * (2 * 5 + 2 * (6 * 2 + 4 + 7 * 6 * 2)) * ((7 * 3 + 6) + (2 * 9 * 6 * 8 * 5 + 6) + (8 + 8 * 2 * 9 * 7 + 2) + 8 * 7)",
    "4 + 4 + 7 + 5 + 2",
    "3 + (8 * 2 * 4) * 8 * (2 + (8 * 5 + 3 * 4 + 4)) * (2 * (5 * 6))",
    "7 + 8 + 7 + (2 * 7 * 7 * (9 * 8) * 5)",
    "5 + 7 + 9 + 4 * (8 + 6 + 9 * (3 + 9 * 3 + 5 * 3) * 5 + 4) + (8 * (3 * 8))",
    "4 * 2 * ((8 + 8 + 4) * (9 + 9) * 5 * 4 + 5 * 9) * 2",
    "4 + 3 * 4 * 4 * 6",
    "4 + 8 * (5 + 2 * 3 * 8 * 6 * 9) + 6",
    "2 + 7 * (8 * 4 * 8 * 2 * 8 + 6) + 7 + 2",
    "5 + 3 + ((8 + 3) * 6 + (6 * 2 * 9 * 8 * 8 + 3) + 8 * 2 * 7) + 9 + 3 * ((4 + 9 * 5) * 3 * 4 * 3 * 2 + 9)",
    "7 + (8 + (5 + 6 * 4 * 5 + 4 + 2) + (4 * 2) + 3 * 5 + 9) * 5 + 6 * 6",
    "7 + 3 * (9 + 3 + 5 + 2) * 9",
    "8 * 3 + (9 * (4 * 3 * 4) * 6 + 4 * 7) + 5 * 5 + (4 + 5 * (3 * 4 * 8 + 2 + 3 + 2))",
    "(7 * 2 * (7 + 8 * 4 + 7)) * 9 + 5 + 9",
    "4 * 7 * 5 + 3 * 5",
    "9 * 4 * 4 + 2 * (7 * (7 + 8 * 6 * 9 * 4) + (8 + 2) * 3 + 9 + 5) + 2",
    "8 * 7 * 5 + ((5 + 7 + 8 + 4 + 7) + 7 * (9 * 2 + 2) + 9) + 6",
    "5 + 4 + (4 + 5 * 5 * 2)",
    "6 + 7 * 2",
    "9 + (5 + 6 * (3 + 3 * 4 + 3 * 4) * 4 * (8 * 2 + 6 + 6)) * 3 * 3 + 3 + 4",
    "((3 + 6) * 8 * 5 + 8 * 2 * 9) * (4 * 7 + (5 + 6 + 5 + 9 + 4)) + ((5 + 6 * 3 + 2 * 8 * 3) + 2 + 9 + 7) + ((6 + 5 * 5 * 2 + 7) * 9) * 7",
    "2 + 3 + 6 * (4 + 2 + 8) * ((2 * 8 + 4) + (2 + 3 + 3 + 8))",
    "(6 * 2) + 5 * 9 + 2 + ((5 * 7 + 4) * (7 + 4 + 4) * 2 * 7 + (5 + 7 * 2 + 3 + 5) + (6 + 4 + 2)) * 6",
    "4 + (5 * 3 * (6 * 7 * 8 * 4 + 5) * 3) + 2 * 7",
    "((4 + 7 * 9) * 3) + 9 * (6 + 3 + 8 + 2)",
    "4 * 2 + 2 * (9 + 9 * 3 + (9 + 7 * 3 * 3 + 4 + 7) + (8 + 5) + 8) + 8",
    "5 + (8 * 4 + 9) * 2 + 2 * 2 * 5",
    "(5 * 8 * 3 * 5 + (3 * 2 + 9 * 5) * 6) + 5 + 8 + 3 * 2",
    "8 * (2 * (4 + 6 * 3 * 6 * 2 * 2) * (7 + 4 * 9 + 3) + 9)",
    "6 * ((4 * 3) * 6 * (5 * 5 * 6 + 6) + (3 * 2) * 5 * 3)",
    "8 + (3 * 6 + 4 + 9) * 2",
    "2 * 8 * 4 * ((3 * 3 + 9 * 7 * 5) * 7) + 2",
    "(6 + 9 * 2 + 4) * 4 * 4 * 3 + 6 * 8",
    "2 * 7 * 7 + (3 * 8 + 5) + 5 + 7",
    "3 + (9 * 2 + 6 + (4 + 8 + 9 * 5) * 4) * 8",
    "9 * (7 + 8)",
    "8 * (7 + 8 + (5 + 5 * 8) + 7 + 3) + 6",
    "(4 + 2 + 7 * 5) * 9 * 3 + 8 + 7 + ((9 * 4 * 4) + 4 * (6 * 3 * 3 * 6))",
    "(2 * 8 + (8 + 4 * 9 * 6 * 2) * 7) * 9 * 6 * 5",
    "9 + 7 + (4 + 5 + 6 + 3) + 3 * 6",
    "(8 * 2) + 4 + 3 + 7 * 2",
    "(4 * 4 + 4) + 8",
    "6 + 4 + 8",
    "3 * (5 + (2 * 6 * 5 + 7 * 6) + 7 * 2 * 8 * 8) + 8 * 4",
    "8 * 7 * (3 * 4 + 6 + 2 * 9 + 9) * 3 + (3 + 5)"
);

constexpr static auto result = sum_of_expressions<2, 1>(input);

int main(){
    std::printf("result = %lu\n", result);
    return 0;
}
