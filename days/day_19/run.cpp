#include <cstdio>
#include <experimental/array>
#include <variant>
#include <map>
#include <regex>

/*
 * Day 19 has arguably been a terrible day for me; there are later days which
 * have caused me fewer problems.
 *
 * My gut choice when seeing this puzzle was that it was a grammar, and
 * therefore can be handled by a parser.
 *
 * However, I found out that after tirelessly debugging and fixing my LR(1) parser,
 * that it didn't work - there are shift/reduce conflicts and reduce/reduce
 * conflicts in the input! This was unfortunately something that I couldn't
 * easily anticipate, as the test input (in part 1) has no such conflicts.
 *
 * You can check yourself to see the input has conflicts here:
 *
 *      http://jsmachines.sourceforge.net/machines/lr1.html
 *
 * (assuming this link still exists)
 *
 * As a result, and the fact I don't wish to try and find a way around the
 * conflicts, I have bitten the bullet and resorted to regular expressions.
 */

// Variant helpers
template<typename... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

constexpr std::uint8_t str_to_uint8(char const* str, char const** endptr) noexcept{
    std::uint8_t i = 0;
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

struct RuleA{
    std::uint8_t id;
    char value;
};

struct RuleB{
    std::uint8_t id;
    std::vector<std::vector<std::uint8_t>> rules{};
};

using Rule = std::variant<RuleA, RuleB>;

inline Rule parse_rule(char const* str) noexcept{
    auto* line = str;
    auto rule_id = str_to_uint8(line, &line);
    ++line;
    skip_whitespace(line, &line);
    if(*line == '\"'){
        // RuleA
        ++line;
        return RuleA{rule_id, *line};
    } else{
        auto rule = RuleB{rule_id};
        using SubRule = std::vector<std::uint8_t>;
        auto s = SubRule{};
        while(*line != '\0'){
            skip_whitespace(line, &line);
            if(*line == '|'){
                rule.rules.push_back(s);
                s = SubRule{};
                ++line;
            } else{
                s.push_back(str_to_uint8(line, &line));
            }
        }
        rule.rules.push_back(s);
        return rule;
    }
}

/*
 * Some people, when confronted with a problem, think
 *
 *      "I know, I'll use regular expressions."
 *
 * Now they have two problems.
 */
template<bool replace_specific_rules, typename Table>
inline std::string build_pattern(Table& table, Rule const& rule) noexcept{
    return std::visit(overloaded{
        [&](RuleA const& r){
            std::string str = "(";
            str += std::string{r.value};
            str += ")";
            return str;
        },
        [&](RuleB const& r){
            if constexpr(replace_specific_rules){
                if(r.id == 8){
                    std::string str = "(";
                    str += build_pattern<replace_specific_rules>(table, table[42]);
                    str += "+)";
                    return str;
                } else if(r.id == 11){
                    std::string str = "(";

                    // Well you ask a stupid question, you get a stupid answer
                    std::string sub = "";
                    for(std::size_t i = 0; i < 5; ++i){
                        if(i > 0){
                            str += "|";
                        }
                        sub = "("
                            + build_pattern<replace_specific_rules>(table, table[42])
                            + sub
                            + build_pattern<replace_specific_rules>(table, table[31])
                            + ")";
                        str += sub;
                    }

                    str += ")";
                    return str;
                }
            }
            std::string str = "(";
            std::size_t i = 0;
            for(auto const& production : r.rules){
                if(i > 0){
                    str += "|";
                }
                str += "(";
                for(auto const& id : production){
                    str += build_pattern<replace_specific_rules>(table, table[id]);
                }
                str += ")";
                ++i;
            }
            str += ")";
            return str;
        }
    }, rule);
}

template<bool replace_specific_rules, typename Table>
inline auto build_regex(Table& table, Rule const& rule) noexcept{
    std::string pattern = "^";
    pattern += build_pattern<replace_specific_rules>(table, rule);
    pattern += "$";
    return std::regex{pattern};
}

template<bool replace_specific_rules, typename Array>
constexpr std::size_t sum_for_rule(Array const& array) noexcept{
    auto table = std::unordered_map<std::uint8_t, Rule>{};
    std::size_t i = 0;
    while(true){
        auto const* str = array[i++];
        if(*str == '\0'){
            break;
        } else{
            auto rule = parse_rule(str);
            std::visit([&](auto const& r){ table[r.id] = r; }, rule);
        }
    }

    auto regex = build_regex<replace_specific_rules>(table, table[0]);

    std::size_t sum = 0;
    for(; i < array.size(); ++i){
        auto const* str = array[i];
        sum += std::size_t(std::regex_match(str, regex));
    }
    return sum;
}

constexpr static auto test_input = std::experimental::make_array(
    "42: 9 14 | 10 1",
    "9: 14 27 | 1 26",
    "10: 23 14 | 28 1",
    "1: \"a\"",
    "11: 42 31",
    "5: 1 14 | 15 1",
    "19: 14 1 | 14 14",
    "12: 24 14 | 19 1",
    "16: 15 1 | 14 14",
    "31: 14 17 | 1 13",
    "6: 14 14 | 1 14",
    "2: 1 24 | 14 4",
    "0: 8 11",
    "13: 14 3 | 1 12",
    "15: 1 | 14",
    "17: 14 2 | 1 7",
    "23: 25 1 | 22 14",
    "28: 16 1",
    "4: 1 1",
    "20: 14 14 | 1 15",
    "3: 5 14 | 16 1",
    "27: 1 6 | 14 18",
    "14: \"b\"",
    "21: 14 1 | 1 14",
    "25: 1 1 | 1 14",
    "22: 14 14",
    "8: 42",
    "26: 14 22 | 1 20",
    "18: 15 15",
    "7: 14 5 | 1 21",
    "24: 14 1",
    "",
    "abbbbbabbbaaaababbaabbbbabababbbabbbbbbabaaaa",
    "bbabbbbaabaabba",
    "babbbbaabbbbbabbbbbbaabaaabaaa",
    "aaabbbbbbaaaabaababaabababbabaaabbababababaaa",
    "bbbbbbbaaaabbbbaaabbabaaa",
    "bbbababbbbaaaaaaaabbababaaababaabab",
    "ababaaaaaabaaab",
    "ababaaaaabbbaba",
    "baabbaaaabbaaaababbaababb",
    "abbbbabbbbaaaababbbbbbaaaababb",
    "aaaaabbaabaaaaababaa",
    "aaaabbaaaabbaaa",
    "aaaabbaabbaaaaaaabbbabbbaaabbaabaaa",
    "babaaabbbaaabaababbaabababaaab",
    "aabbbbbaabbbaaaaaabbbbbababaaaaabbaaabba"
);

constexpr static auto input = std::experimental::make_array(
    "123: 39 86 | 127 32",
    "131: 29 32 | 93 86",
    "69: 76 86 | 79 32",
    "39: 32 105 | 86 120",
    "59: 86 117 | 32 112",
    "81: 134 32 | 22 86",
    "17: 86 55 | 32 50",
    "3: 86 60 | 32 87",
    "78: 132 86 | 41 32",
    "133: 86 6 | 32 132",
    "115: 36 86 | 40 32",
    "87: 38 86 | 49 32",
    "11: 42 31",
    "129: 32 94 | 86 61",
    "126: 137 86 | 67 32",
    "64: 124 32 | 119 86",
    "127: 86 83 | 32 47",
    "84: 2 86 | 5 32",
    "57: 96 32 | 131 86",
    "16: 28 32 | 6 86",
    "21: 86 91 | 32 41",
    "44: 32 88 | 86 92",
    "99: 86 112",
    "4: 135 32 | 59 86",
    "6: 86 86 | 86 32",
    "60: 32 110 | 86 66",
    "20: 70 32 | 29 86",
    "35: 32 23 | 86 136",
    "28: 32 86",
    "51: 9 86",
    "121: 98 32 | 97 86",
    "47: 41 86 | 117 32",
    "27: 86 34 | 32 52",
    "107: 128 32 | 57 86",
    "95: 9 32 | 6 86",
    "1: 26 86 | 114 32",
    "56: 93 86 | 28 32",
    "24: 32 117 | 86 6",
    "134: 86 70 | 32 6",
    "109: 9 32 | 9 86",
    "91: 86 32 | 32 86",
    "128: 33 32 | 46 86",
    "7: 99 32 | 24 86",
    "31: 53 86 | 102 32",
    "72: 86 120 | 32 133",
    "75: 28 32 | 9 86",
    "108: 124 32 | 130 86",
    "53: 89 86 | 12 32",
    "76: 91 86 | 130 32",
    "12: 101 86 | 111 32",
    "111: 32 1 | 86 85",
    "41: 86 86 | 113 32",
    "136: 91 32 | 70 86",
    "93: 32 86 | 113 32",
    "43: 115 86 | 58 32",
    "2: 86 109 | 32 54",
    "118: 86 13 | 32 134",
    "0: 8 11",
    "49: 86 81 | 32 17",
    "46: 117 32",
    "112: 86 32 | 32 32",
    "114: 86 70 | 32 68",
    "106: 32 100 | 86 131",
    "83: 32 112 | 86 130",
    "89: 123 86 | 107 32",
    "25: 86 119 | 32 70",
    "132: 86 86 | 32 86",
    "74: 86 45 | 32 56",
    "119: 32 86 | 86 113",
    "97: 32 63 | 86 106",
    "10: 86 6 | 32 130",
    "45: 124 32 | 6 86",
    "102: 43 86 | 103 32",
    "61: 117 32 | 124 86",
    "96: 32 117 | 86 68",
    "82: 130 86 | 117 32",
    "117: 86 32",
    "116: 86 41 | 32 93",
    "62: 32 75 | 86 10",
    "29: 113 113",
    "37: 86 28 | 32 91",
    "94: 41 32 | 6 86",
    "18: 86 9 | 32 70",
    "15: 86 37 | 32 21",
    "122: 6 32 | 124 86",
    "40: 104 86 | 61 32",
    "85: 80 32 | 25 86",
    "30: 54 86 | 108 32",
    "52: 122 86 | 116 32",
    "50: 32 91 | 86 119",
    "113: 86 | 32",
    "58: 35 86 | 74 32",
    "67: 86 82 | 32 77",
    "68: 86 86 | 32 113",
    "104: 112 32 | 9 86",
    "90: 32 121 | 86 73",
    "130: 32 86 | 32 32",
    "14: 78 32 | 92 86",
    "73: 126 32 | 27 86",
    "32: \"a\"",
    "38: 129 86 | 4 32",
    "70: 86 86 | 32 32",
    "19: 32 62 | 86 14",
    "100: 9 32 | 41 86",
    "86: \"b\"",
    "63: 125 86 | 95 32",
    "135: 28 32 | 29 86",
    "9: 32 32",
    "55: 130 32 | 9 86",
    "77: 6 86 | 70 32",
    "34: 32 20 | 86 16",
    "137: 71 86 | 64 32",
    "124: 86 86",
    "120: 32 93 | 86 124",
    "79: 86 124 | 32 124",
    "125: 86 70 | 32 130",
    "22: 93 32 | 41 86",
    "105: 41 32 | 112 86",
    "80: 32 68 | 86 41",
    "110: 72 32 | 7 86",
    "65: 41 86 | 91 32",
    "103: 32 19 | 86 84",
    "5: 65 32 | 10 86",
    "26: 28 86 | 6 32",
    "33: 113 41",
    "23: 113 130",
    "48: 32 134 | 86 51",
    "54: 86 130 | 32 93",
    "42: 90 32 | 3 86",
    "92: 86 119 | 32 112",
    "98: 15 32 | 30 86",
    "66: 48 32 | 118 86",
    "8: 42",
    "101: 86 44 | 32 69",
    "71: 124 32 | 29 86",
    "36: 18 86 | 65 32",
    "13: 6 32 | 93 86",
    "88: 132 32 | 117 86",
    "",
    "aaabbaabbaaababaaaaabbbb",
    "babaaaabaabbbababbbabaaa",
    "abbaababababababbbbbabaaaaaabbbabbbbabaababbaabbbaaaabbb",
    "ababaabbabaabbabababbbabbaaababaababaaaabbbbbaabbbababbbbabaabaabaaabaabbabbaaabbbbbabbbaaabbaba",
    "babbbabaabaaabababbbabbaabaaababbaaababb",
    "bbaababbabbbbabababaaababbaabbbaabbaaaabbbaaabbaabbbaaabbbabaaaa",
    "abbaabaabaaabababaaababb",
    "bbbabbaabbaababbabaaabaa",
    "bbaabaababbbaababbbbaaab",
    "babbabbbbbbbbbbaabbaabaaaaabbaabbbbabbbbaabbbbaaabababbbaaababbbbbaaabbb",
    "bbbbaabbbaababbbbbbbabbbbbbabaabaabaabaa",
    "baaaababaababbbbabbbabab",
    "babbbabaababababaabbabaaaabbabaaabababbb",
    "bbabbbabaabaabbabbbbabaabaababbbaabaabbbaabbbaababbbabbb",
    "abaaaabbbaaabbbbbaababaaabbaaaabbaabbbaaababaabaabaaaaabbaaaabbaababaaaabbbabbaaabbabbbbaabbbbaa",
    "abbbbbbbbbaabbbbabbbabbabbababab",
    "bbaabbbaaaaabaabbabababb",
    "abbbabbaaabababbbbabaaabbaabbbbb",
    "babbbbabbabbaaaaaabbaaaabaaabbaa",
    "abbababbaabbabbbbbaabaaababaaabaabbbababaabbbaaa",
    "abbbbbabaababaabaababaabababaabbbbabaaaa",
    "babaabbbaabababbbbaabaaaabbbbaaabbabbaabaaaababbaaaabaaaaaaaaaabbaabaaaaabbbabaababbabba",
    "abababbaabbaabaaababbbbb",
    "aaaabaabaaaaaaababaababb",
    "bbbaabaaaabababbbabaaababbabaabbbabaabbaaabbbbabbbbaaaba",
    "abbbabbaabbbaabaaaabaaab",
    "babaaababbbbaaaabbbabbbabaabbaaaababbbab",
    "bbaabbbbaabbbbbbaaababaabbbaabba",
    "ababbbaaaababaabbaaaabbb",
    "aabaaaabaabbabaababaabaaaababbbabbababbbabbaaaab",
    "abbbbaabaaaababbbbbbaaaa",
    "aabbaaaaabbababbaabbabaaababaaba",
    "bbbababababaabababaaaaab",
    "bbaabaaabbbbabbbababbbaabbabbbbbaabbbaab",
    "aabababaabbababbaabbbaaa",
    "abbaababbabbbbabbbbabbab",
    "baaabbababbbbbabaabbbabb",
    "aabaabbbbaabaababaababaa",
    "bbaabaaabbbbbbaaabababaa",
    "bbaabbabaaaaababbbbbabab",
    "aabbbbbaababbbbabbbbbbaabbbaabaaaaabbbbb",
    "abbabbababaabbabbaaabbba",
    "babaaabbabbaaaabbaabaabb",
    "babbbbaaabbbabbaaaabbabb",
    "aaaaabbaaabbaaaabbbaaaaaaaababbb",
    "aaababaababbbabaabaaabbb",
    "abbababbbbababbabbbbbbbb",
    "abbabbabbaaabbabbbaabbaa",
    "aaaaaabababbbbbbbaabaabaaaaaaabaabaababa",
    "babaabbabbbaaaaabbbbbbaaaabbabaaaaabbbaababbbaab",
    "aaabababbaaaaaaaababbbbb",
    "babaabbaaaaaaabaabaabaab",
    "baabbaabbaababbbaaaaaabb",
    "bbababaaabbbaabbabaabbaabbbbaabbbbbaabbaaaaabbabbaabbbbb",
    "babaabaaaabbbbaabaabbaaaaaabbbba",
    "abababababbabbabbbaababbbbabbbaababbbaab",
    "bababbbbbbababbaabbbbbba",
    "abbbaababbbaabaaabbaaaaa",
    "abbaabbabaabbbabbabbbbbbabbbabaabaababba",
    "aaaabbaabbaaabbaaaabaabbaaaabbaabababbba",
    "aabaabbaaaabaaaaaabbbaab",
    "bbaabbabaaabbbabbabaabaa",
    "bbaabbbaaaaabaabbbabbabaabbbbaabbaaabbaa",
    "aabaaabbaaabbaabaaabbaabbbaababbbbababbaaaaaabaa",
    "aabbabaaaaaaaabababaabbb",
    "bbbbbaabbabbabaaabbbabbbbabaaaaaabbabbbbbbaaabbbbbabaaaa",
    "babbbababbabbaaaaababbbbabbbabbb",
    "babbbbbabababaaabbbababaaaaababbbbaabbaababbaaab",
    "aabbaaaaabbaabaababababbbabaaabbaabaabaaaabaababbababbaabbbabaab",
    "bbaaaabaaabaaaabababaaba",
    "bbbbabbbaaaaabbabaabaabb",
    "bbbbaabbbbbbbbbaabaabbbb",
    "aaaababbbabaaaabbabbbabaaaaabbbabaabbaaa",
    "abbabababbbbbabbbaaaaaaabaaabbabbbbbaaaa",
    "aaaabaabaaaabaabbaaaaababaabaaab",
    "ababaabbbbabaabbbbbaabaabbbbbaaaababbbab",
    "abaaababbbaabaabaaabbbaabbbababaaababbbabaababab",
    "bbbbabbabbbbabaababbbabaabaababb",
    "aaabbaabaabaaaabbbbbaaba",
    "babbbbabbaabbaabbaaababb",
    "abbaabaabaaababaabbbbbba",
    "baaababababbbabababbabbbbaababba",
    "babbabaabaaaaabaabaabaab",
    "aaabaaaaaaabbbababbababbbbababaabbaabaaaaabbaaab",
    "bbbbabbbabbaababbbaaabbabbabbbbbaaababababbbababaaabbabbababbbbb",
    "aababababbbbabbaabbaabbabbbbbbaabbabaaabaaaaaaababbaaaaa",
    "abaabaaabaaaaababaabbbaabbabaaaa",
    "baaabbabbabbabaaaaaaaaab",
    "baabbbabbaabbaabaabaaaabaabbaaaabababbab",
    "abbabbbaaabbbbbbaaabaaab",
    "abbbbbabaabbabbbbaaaababaababbbbaabababaaababaabaabbbbaabaaaabba",
    "ababbbbabaaaaabaabbaaabb",
    "abbbabbaabaaabbaabbaaaaa",
    "aabaababbbbbbbbabaababaa",
    "bbbaababbbaababbbbbababababaababababbbabbabaabbbabbbbbba",
    "babbbbabaabbbababbabbabb",
    "aababbaaaaabbaaaaabbabaabbabaaaabaabbaaa",
    "bbbabbaaaaaaabaabbabbaab",
    "babbbbbaaabbbbbaababbbbb",
    "bbaaaababbbbabbabaaaaabb",
    "abbaabbbbaabbbabbabababa",
    "baabbbaabbbabbaabbbaababbabbabbbbabaababaaabaaabaaaababaabaabaab",
    "abbbabbabbbaabbbbaababaa",
    "aababaabaaaababbbbbaababababbaabbbbbaaba",
    "aabbaaaabaaaaaaabaababbbbabbbbabaaabbaba",
    "bbaaaabaaaaaabbaabbabbbabababbba",
    "baabaababbaaaababaaaaaab",
    "bbabbbbbbbaabbbabbababbb",
    "bbaabbbaabbbbbaaaaabaabbbbbbbaaabbaaabbbbbaabbab",
    "bbbaabbbbbbbbbaaabaaaabb",
    "bbaabbbbaababbaababbabaabbbbbbabbabbbaab",
    "baaaaaaabbaaabbaabaaaaaa",
    "abbaabababbabaaabbbabaab",
    "babbbabaaabaaabbbbbaababaaababaaaaabaaaababbaabaababaaba",
    "aaabababaaaaabbabbaababa",
    "aabbabaabbabbbbbbaabaabaababbabb",
    "aaabbaaababbaabbbbbbbaab",
    "aabaabbababbbbbaabbbabbb",
    "bbbbabbaabaaaabaababababaabbbbbbbbbbbbbaabbbabbb",
    "abbababbbbbaaabbbbbbaabbbbaaaabaabaaaababbaabbaa",
    "bbbaaaaababbabbaaaabaaaaabaabaaabaabaabbbbaaabaababaabaa",
    "ababbbbaabbaaaabbbbbabab",
    "babbaabababbbababaababaa",
    "aabbbababbabaaabbaaaabaa",
    "bbabbbabbbbbbbaabbababaaaabbbabb",
    "baabbbaabababaaaababbbbaabaaaaaa",
    "baabaabaabbaabaaaabbbbaa",
    "abbaaaabbbaaaabaabaaababbabbabbaabbbabbabaababbaaaaabbbbbbbaaaba",
    "bbaaabbabbaababbbaababbbbbaaabbababbaaab",
    "abbbbababbbabbaaabbbbbaa",
    "bbaaabbaababaaaababbaaaaaaabbaabaabaabbbbbabaaababababbbbaaaabaa",
    "aabbaabbbaabbbaabbaabbbaabbabaababababaa",
    "bbabbbbbbbbabbaabbbbbbabaaaaaaabababbbabbbbabaab",
    "baababbbaaaabbbbaabaabaaaabaaaaa",
    "baaaaabaaaabbaabbabbbaaabbabbaaabbabaaaa",
    "aaababaaaabaababaabbaaba",
    "aabbabbbabbbabbabbabbaaababaaaababbbaaaabaaaabaa",
    "baabbaabbbabbabaabbbbbabaabbaabbbbaabababaaabbbbbbaababa",
    "bbabbababababaaaabbabaaabbabaaaa",
    "abababbababbaababaaaabba",
    "babbaabaaabbbbbbaaababba",
    "bbaabaabbbbaabbaababaaabbabbaababbbbbbbbababaabb",
    "bbbababaabbbbababbaaabbaabbbaabbaaabbabb",
    "abbaabbabaabbbaaabaabaaababbbaaabaabbabb",
    "aababbababaaaaaaaabbbbaa",
    "bbbbaaaabbaaabbbabbaaabbababaabaaaabbabbabaaabbaaaabaaaa",
    "babbabbbbbbabbaaaaabaabbbaaabaaaaababbab",
    "aaaaaaaaabbabababaaaababaaabaaabbaaabbaa",
    "bbbbabbaaabbaaaababbbabaabaaaaaaabaabaab",
    "babbbbbbaaaaaababbabaaaa",
    "bbbbbbbaabbbbbbbabbabaabbaaabbababbbbaaaabaabaabaababbba",
    "abbbaabaabbbbbbbbabaabbb",
    "aabbbbbbaabababbbaaaaabb",
    "babaaaababbbbbbbaabbaaaaabbaababbababbab",
    "babbaabaaaabbbaaaabababbabbaaabababaabababbbbbbaaabaaabaaaabbbba",
    "aaabbaabbabbabbabababaaaaababaabbbbaaaabbbaaaaaa",
    "babbabaaaaaabbbaaaaababa",
    "baabbbaaaaabbaabbbabbabb",
    "aaababaaabbbbbabababaaba",
    "baaababaabbabbbaabbbaabaabaabaab",
    "abbbaabbaaaaabbbaaabbbba",
    "aaaababbbaaaabababbbbbabbbaaaaabbbabbabb",
    "abbaababbaabbbabbbbbaaba",
    "bbbbabbaaababbaabbbaaaab",
    "abbaabbabbabbbababbabbbb",
    "abaabaababaaaabbaaabbabbbbbbaabaaaaaaabb",
    "bbbbabaababaababbbaabaaaabaaabaa",
    "babbabbaabbaababbabaabbaaabababbaaaabbab",
    "babaaabaabbabbbaabaaabbabaababaa",
    "baabbbababbbbbbbbabbbbbaaaaabaabbbaababbaabbabaabaababab",
    "baabaababaaabbababbaabab",
    "bbabbbbbaaaabbbaabbaabaabbaabaabbbbbbbab",
    "aaaaabbaaababaaaabbbabab",
    "aabababbbbaabbbabbaabaabbaabbabbaabbbaab",
    "babbaabaaaaaaababbbbbbbb",
    "aabbabbbaaabababbbabbbbbbbaabbaa",
    "aaabbbaaabbababbabbaaabaaabbaabbbabaabaabbbaabbaaaababbb",
    "abbaabbabbbaabaababaabaa",
    "babaaaabababbaabbababbba",
    "bbaababbaaabbaabbbabaaba",
    "bbbbbaabaababbaabaaaaababbbbabaabbabaabaabbbbbbabaabbbbbaaabaaab",
    "aababababbaabaababababababbbabaabaaaabaa",
    "abaabbaaaaaabaabbaaabbabaaabbaabbbaaaaaa",
    "aabaabbaaabbaabbabaabbba",
    "bbaabaaaabbaababbaabaaab",
    "babbaaaabaababbbabaaabaa",
    "bbabaaababbbbaabaabbaaaabbabbaab",
    "babbaababbbbaabbabaaaabaabbabaaaaaabbaaabbbaabbaaabbbaababaababa",
    "baababbbabaabbaaabaaaabb",
    "baaaaaaaabababababaaabbb",
    "baabbbababbaabbbaababaabbbaababbaaabbbba",
    "baabaabaabaaaabaabbabbbabababbab",
    "bababaabaabaaabbababbaba",
    "bbbbabbbaaaabbaaabbbbaaa",
    "babaaabbbabaabbaaaabababbbbbaabbaaabaaab",
    "aababaaabbbbabbaaababaabbbbaabbaabbbabaa",
    "baaabababaaaaababbbaababaabbabbabaaaaabb",
    "aabbabbbbaaaaaaabbbbbbbb",
    "aababbaaabbbbabababbaabbabaabaaaaabbabab",
    "ababbbaaaabbabbbabbabaaaabababbb",
    "abbbaabbabbaabbaabbababbbabbabbaaaaaabbaabaababb",
    "bbbabababbabaaabaaaabbbaabaaaaaa",
    "bbaabaabbabbaabaabaaabbababbbaab",
    "aaabbaabaaabaabbbbbabbba",
    "bbababaabaabbbabbabbabab",
    "bbbbbabbbbbaaaaabbaabaabbababaabaaaababbaabaabbaabaabbbaaaaababaaabbbaababbbbaaaaabaaaaa",
    "aaaabaabbabbbaaaaaaaaaab",
    "bababaaaaabaaabbaabaabbbbbaaaabaaababaaaaaaaaabbaabbbaaa",
    "bbbbbbbaabbaababbababbbb",
    "baabaaaabbbabaaaababaaaabababababbababba",
    "abaabbaababbaabbabbbbabababbaaaaaabaabbbbaabaaaa",
    "bbbaabbbaaabaabbaaabbaababbbaababaaabaab",
    "aabbbababaabbbababbaaabb",
    "aaabbaaaababbbaaaaaaaabb",
    "abbaabbabbbbabbabbaaaaab",
    "babbbaaaaaaaabababbbbbbbbabbaabababaabbaabaabbaaaaaaabaabaabbabb",
    "ababbbbabbbbaabbbababaaabbbbaabababbbaab",
    "babbabbaaaaabbaaababbabb",
    "babbabbbaababaaabbbbbaba",
    "aaabbaababbbbabababbbbbbaaaaaabbabaaabbbbabaaaaa",
    "bbbabbbbabbbaabbaaaabaab",
    "bbaababbbbaababbaabaabbbbbbbbbbb",
    "aabbabaaabbbaabbbbbbabbbbbbbbbbb",
    "abbaabbbababbbaaabbabaaabbbaabbaaabbbaab",
    "bbabbababbaabaabbbbaaabb",
    "bbbbbbbaababaabbbbaababbbaabbbabbaaaaabb",
    "bbbbaabbabbabaaabbbbbaba",
    "abbbabbbbaabaaabbbbbbabbaaabaaaababbabbabbabbaaabbabbaabaabaaaaa",
    "babaabbabbbaabbbbabbabaabbbbaaba",
    "aaaaabbababaabbabaaabaab",
    "babaababaaaababbabbbaaab",
    "baabbbaaaabaaaababbababbabbaabbbaaaabaaa",
    "bbbbbabbbaabaaabababbababbabbabb",
    "babaaaabaaaaabbbaaababaaaaaaabbbbaabbaaa",
    "abbaabbbbabbaabbababbbaababbbaab",
    "aaaabbbaababababaabaaabbbabaaababbaababbabbbbaba",
    "babbabaaaaaabbbaabbaaabb",
    "bbbaababababbaabbaabbabb",
    "bbaababbbbaaabbbbbbbbbbbbbaabbabbababaaaaabbbbaaaaababba",
    "bbababbaaabbaabbbabbabaababbbaabbbaaabbb",
    "aabbbababbababbabbaabbaa",
    "babbbbababbbabababababbbaabbabbaabbbbabbbbbabbba",
    "babbabaaabbbbbabbbbbabbbaabbbbbabbaabaaabbaaaaaababbababbabaaaaa",
    "baabbaababbaaababbabaaaa",
    "ababababbabbbbaaaaabbabb",
    "bbbbbbbaababbbaabaaaabbb",
    "abbaabaabbaabaabbbbbaaaa",
    "ababbbaabababaabbaabbabb",
    "aaaaabbaabbababaaaabaaaabbbbabbbabbaabaabaabbabb",
    "aaaaabbabbaaaabaaabbabab",
    "aaaabaabbbbbaaababbbababbbaaabaaaabbabab",
    "bababaabaabbbbbbbbaabbbaabbbbbabbbbaaaba",
    "baababaaaabaabbbbabaabbbbabaaaabbbbaaabbbbbbabba",
    "aaaaabbabbbbabaaaaabbaabaababbbbaabbbbaabbbbaaba",
    "babbabbbbbababbaabaabaab",
    "bbbbabbabbaabaaabbbaababbaaaaabbbbbbaaab",
    "abaaababbababaabaaabbaabbabbaababaaaabbb",
    "babaabbabababaabaabbbaaa",
    "baaababaabbabaaababbabbbbbbabbba",
    "abbabbbabbaabbbaabaabaaabbaaaabb",
    "bbbbaabbaababbaaabababbb",
    "aabbbbbbbbbabbbbaabaaaba",
    "babbaabaabbabbabbabaabbb",
    "baabbbabaaabbaaabbaabbbbbaaaabba",
    "babbabbaabbbaabbbabaababbaabaababaababab",
    "aababbbbaababaaaababbbbb",
    "aababaabaaabbbabaaaabaaa",
    "abbbaababbbabababaabbaba",
    "abaabbaaaababaaabaaaaaaa",
    "ababaabbbbaabbbaaabbbbbbababaabbabbaaabbaabaabaabbbbbaaa",
    "bbbbabbaabaabaaaaabbaaab",
    "baababbbaabbbbbabbabaaaa",
    "babbbbaaaaaaababaabbbababbbbbbaababababaabbbaaaaaaaabaaa",
    "aaaabaabbabbbaaabbbbbaba",
    "bbbaabbbabbabaaaaabababaababbaabaaaaababaaababbaaaabaabaaaaabaaaabaaaabb",
    "baaaaaaabaaabbabaababbba",
    "abbaaaababaaababbaabaaaa",
    "babbbababbabbabaaabbabbbabbaabababbaaabaabbbbabb",
    "bbabaaabbbaabbbabaabaababbaaaaaa",
    "aabbabbbabbaabbabbabaaababababbb",
    "aaaabaabbabaaaabbaaaaaaaabbaaaaaaabbbabb",
    "aaaaabbaababaabbbaaaaaaabbabbabaabaaaaaabbaaabaabbbbaaba",
    "aaaabaabbbbaabbbababbbbaaaabaabbbabbbaaaaaabaabaabbabaab",
    "baabbaabbabbbbbbabbabbbb",
    "bbaabbbabaaaaaaaaaaababa",
    "babaaabbbaabbbababbabbbabbbbabab",
    "bbbaababbbabbbaabbaaaaab",
    "bbaabbababaabaaaaaaababbbaababab",
    "aabbbababbaaaabaabaaabbaababbbab",
    "abbbbaabaaaabbbabbbababaaaaabbaabaabbaaaabaaabaa",
    "aabbbbabbaababbaabbbabab",
    "aabbbabaabbbaabaabbaaaaa",
    "babbaabbabaabaaababaabbabbbaaaaaababaabbbabbabab",
    "abbbbbabbbabbaaabbaabaaabbabbbaabaaaabaaababbbabababbbbbbaaaaabb",
    "bbaabbbabbbabbaaaabbbbaa",
    "abbaaabaaabbbbbabbaaaaaa",
    "baaaaaaaaaaaababbaaababb",
    "aaabbbbbbaababaabbbbbaaa",
    "aaaaaaaabbabbbbbabbbbbbbbbbbaabbbabbabaaababbabbbbababbbaabbbbaa",
    "bbaabaaabbbabbaaaaabbaaabaababbbabbaabaababababa",
    "bbabbbabbbababaababbbaaaaabbbbab",
    "bbbabbbabbbabababbbbbabbbbabaaaaabbaaaabbbbbabababbbaabbbabaaaaaabbabbabbababaaa",
    "bbabaabbaaabaabbaaababaabaaabbaa",
    "bbabaaabbbabbbbabaaabaab",
    "bbbbbbbaabbababbaaaaabaa",
    "bbaaabbaaaaabbaabababbba",
    "aababaaaaabaaabbbabbaabaaaabbaaaabbbaabaabbbbbaabbbabaaa",
    "bbabaabbaaaaaababbbbbaaa",
    "abaabbaaabbbabbabbbbabab",
    "aaababaaaabaababaababbba",
    "abababababababababbaabaabbaabaabbbbbbbbbbbabaaaa",
    "bbababbababaaabbaaaaabbaabaabbba",
    "aabaabbaaababbbbaaabbabb",
    "aaababaaaaabbbaabbbabbbbbbaabbbaaabbbaab",
    "baaaaababbabbababaabbbaababbaabbbbabaabbaababaabbbbabaababaaaaab",
    "aabaabbbaaaabbbababaaabbababaabaaaababba",
    "bbabaabbbabbbbbabbaabaaabaabbbbaaabaaaaa",
    "ababbbaabbabbbaaaaabbbaababaabaa",
    "bbbbaabbaaaaaaaabbaabbbbaaabaababaaaabbb",
    "bbaabbaaaaabbabbbaabbbbababaabbbababaaabbbaaabaababbbaaa",
    "abbbbbbbabbbbabaabaaaaaa",
    "bbbbabbbbabbaabbbabaabaa",
    "abbbaabbbabaabbabbbaabaabbbbabaabbababbaaababbbbbbbaabbaaabbbbab",
    "aabaababaabaaabbbbababaabbabaaabaaaaaaabbabbbabb",
    "aaabbbabbbbbabbbaaabbbba",
    "aabaababbbaaabbababbabbbaabbabba",
    "bbabbaaaaababbbbbabaabbb",
    "ababaaabbbaabaabbbabaaaa",
    "abbaabababbaababbabaabaa",
    "bbaabaaaaabbaabbbbabbbab",
    "abbbbbabbbabbbaaaabaaabbabbaababbababbba",
    "ababaaabaababaabbbbababb",
    "abaabbaaaaabbaabbababbbb",
    "aaabbbaabbbabbaababaaaaa",
    "aabababbaababababaabaabaababbbbb",
    "baabbaabaaaaabbaaabaaaabaaaaaabaabbbbabb",
    "baaababaabbabaaabbbabbab",
    "abbabbabababaaababbbabaa",
    "abbbaabaababaaabbbaabaaaababbaabbabbabbabbaaabaa",
    "baababbbbbabbababbbabbbbbbbbabbbabbaababbbaababa",
    "babaabababbbabbaaaabaaab",
    "aabaabbaaaabaabbbbbbaaaa",
    "ababaaabbaaaaaaababbabab",
    "bbaabbabbaabbbabbbbbbbbb",
    "bbbbbabbbabbbbabbbababbb",
    "aaaabaabbbabbaaabbaaaaab",
    "bbababbabaaaababbabbbbbabbbabbaaabababbbaabbbbab",
    "ababaaabaabbabbbaaaaabbbbabbaaab",
    "abbbbabababaaaabbabbaabaaaabbbabbbaaaababababbaaababbbbbbbaaaabb",
    "bbbbabbbbbaabbbbaabaabaa",
    "bbababaaaababbbbbbbaabbbaabbaababaaabbaa",
    "bbaabaabababbbaaababbaaa",
    "abbbbbababaaaabaaaababababbbabbabaabbaabaabaabbbbaaaabba",
    "ababababaababbbbaabaaaba",
    "aaaababbbbbabbbbaabbbbbaabaababa",
    "aaaabbbaaababbbbbaababab",
    "bbaabbbbabbbbbbbaaaaaaaaabbaababbbaaaababbbbbaab",
    "bbaabaabaabbaabbabaababa",
    "abbaabaaaaaababbbbabaaaa",
    "aaaaaaaababbbbbbbbbbbabbaababbaabaaaabaabaaaaaabaabbabab",
    "abaaaaabaabbbbabbaaaaaabaababbaabbbababbbbababba",
    "aaabababbaaabababaabbbaaaabbaaab",
    "baaaababbbaabbabbbbbbbbabababaaaaaaabaaa",
    "bbbabbbbaaaaaababaabbabb",
    "aabaaaabaababbbbabaaaaaa",
    "aaabbbaaaabbbabbaaaaaabbbbabbabb",
    "babbabbabbaabbabaabbbbbbaaaabaaa",
    "aabbbbbbaabaaabbaabaababbababbbbbaaaaaab",
    "aaaaabbbaaababaabbaabaabaaaabaabbbbaaabb",
    "bbbabbaabaababbbbaaabbbb",
    "babaaabaaababaaabaaaaabaaabbbaaa",
    "babbbbaabaaaaabaaaababaaabbbbaaa",
    "ababaaaabaaaaabaaaabbbababbaaaaababaabaa",
    "bbaabbabbbbbbabbaaaabbaabbbaaaba",
    "aaaaabbaabaaaabaabbabbbbababbbababababaaaabaaaba",
    "abaabbaaabaaababaaababba",
    "abbabaaaababaabbbbaabbbbabbabbbaabbbbbaaaabbabaabababababbbababa",
    "baabbbabaaabbbabbbabbbbbbaabbbbabaaaaabb",
    "baaabbbababbbbaaaaabababbabaabaaaaabaaabbaaaabababbaabab",
    "bbbababaabbbaabbbbbabbab",
    "bbabaabbabaabaaaaaaaaaab",
    "baaaaaaaabababbabbbabbba",
    "ababaaaabaabbbabbbbbaabbababaaabababbabb",
    "aaabbaaaaababaaabbaabbbbbbababbb",
    "abbabaaaabbbbbababbbbbbbaabaabbbbbbaaaaaaaaabbbbabbabbaaaaabbbbaabaaaaaa",
    "babaaabaaabbabbbaabbbbab",
    "babbabbabbaabbbaabaaabbaaabbbaaa",
    "aabbaaaaabbababbbabaababababbbbaabbabbaa",
    "abbababaaaaaaaaabbbaabbbbbababababbabbaa",
    "aabaaaabbbaaaabaababbbab",
    "abbbaabbabbabaaaabbabbbababbaaaabaababbbbbbbaaabbbbababbbbbbbbbbbbabbabb",
    "aaaabbaabbbababaaabaaabbbabaabbabbbababb",
    "baaaaaaababbabbababbabab",
    "bbbabbaabbaabbbbbbabaabaaaabbabbaabbabba",
    "bbababbababbbaaabababbaa",
    "aabaabbabbabbabaaabbbbaa",
    "abbaaababbbbabbbaaabbbbb",
    "aabababbaaaaaababaaaabbb",
    "bbababaabbbababaaaaaababababaabbabbaaabb",
    "ababaaaaaababaababaaaaab",
    "abaabaaaaaabbbaaabaaabbb",
    "aabbbabaaababbbbabbbaababaaababb",
    "bbbababaabaaabbabbaaabaa",
    "babbabbbabbabbabaaabaaab",
    "baaaaaaaababbbbabbbbaaab",
    "aababaabbbabbbabbaaabaab",
    "abbababbbbbbbbbaaabbbbaa",
    "bbbabbaaabaaaabaabaaabaa",
    "ababababaabbbbbabbbabbba",
    "aabababbabbababaabbabaaabbaabbbbbbaababa",
    "babbabbabababaabbbabbbaababbbbabbbaabbaabaabaabb",
    "aababaaaaabaabbaaaaaaabb",
    "babaababbaaaaabababbabab",
    "bbbaabaabbbbbabbaabbabaabaabbaabbaaaabaa",
    "aaaaaabababaabbabbabbaaabbbabaaabbababab",
    "ababbbbabaaabbabbabbbbabaaabababbabbbabaaabbaaabababbabbabababbb",
    "baaababaabbaabaaabababaa",
    "abbbbbbbaaaaaaaaaabaaabbbababababbbabbab",
    "bbbaaaaabaaababaaaabaaba",
    "bbababbababbaabbbbabbbaaaabaabbaababababbaabaabaaaabbbba",
    "bbabbbbaabbbbaabababbbaabbabaaaaababaaba",
    "bbbaabaaaababaabbbabbbababbabbbabaabbabaabbabbaa",
    "abbbaabababbaaaaaababbaabbababaabaabaabababababaaabbbabbbbbbaaabbbaaaabb",
    "abbbaabaaabbaabbababbbbaaabbbabb",
    "aaabbbaababbababbabaabaaaaabaaababbabbbabbbbbabb",
    "bbabbbababbabbbaaabaaaaa",
    "ababababbbaabbbaaaabaaab",
    "abbaabaabbabbbababbbbababbbbaaaa",
    "aaabbababbaabbaaabbabbabaabbabaababbaababbbabaaaabbabaaaabbabbabbbabbbaa",
    "bbaaabbaabbabbbababaaabaabababbaaaabbbaabbaabbbbbbbaaabbbaabbaba",
    "aaabaaaababbabbbabbbabbabbbaabaaaabbbaab",
    "aaaaabbbabbabbbabbbabbba",
    "abababbabaaaababbabbabab",
    "bbbbaabbaaabbbababbbbabb",
    "abaabaaabbaabaababaabbaaabbabababaaaaababbabbbabaaabbbba",
    "bbaaabbaaaabbaababaaabaa",
    "babbbbbabbbbbbaaabaabaaaabbabaab",
    "bbbbbaaaaaababbbaaabaabaabbbbabbbbbbaabbabbbbabbbaabaabb",
    "bbaabbbbbabbbbbabbaaabbb",
    "aababaaaabbaababbbbbbbab",
    "bbaaaabaaabbbabaaabbabab",
    "abbbaabbabbabbabbababbab",
    "babbabbaababaaabbabababb",
    "aababbbbaabbaabbabbbbababbbbabaaabbbabaaaabbbabbaabbaaba",
    "aaabbbaaaaabaabbbabaaabaaabbbaabbaabbabb",
    "aaaaabbbaaabaaaabbbbaabbabbbbbaaabaababa",
    "aabbabbbabaaabbababaabbb",
    "bababaabbabaaabbaaaaabaa",
    "aababaaababbabbababaabbaabbbbbbaaabbbbab",
    "aaaabbbabaabbaababbaaaaa",
    "bbaabaabaabbabbaaaaaaabaabaaaabbbbaabaabbabbaabbabbbabaa",
    "aabbbabaabbbaabababbbaab",
    "bbabbbabaaababaaaabbaabbbabbbbabbabbbabbabaababb",
    "bbbbaabbabaabaaabbbbaaba",
    "bbbbabbababaabbaabbaaababbbbaaaa",
    "aaaaaaaaababbbaaaaaaaabb",
    "abbaababbaabbaababbaaaabbabbbaabbbaaaaab",
    "ababababbbabaababbaaaabbbbababbbbbabbbbbaababbaabababababbbbaabbabbbbbbbbabbbbab",
    "aaabaababbabaabbaaababbabbaaababbbabaabbbbaaabaabbababbabbbbabbbbababbbb",
    "aaabbbabbbabbbbbaabaaaba",
    "babbaabbababaaaababbbaaaabbbbabababaabbbaabbababbaaabaabaabbbabbbaaaaabb",
    "abbaabababbaabbbbbbabaab",
    "babaaabbabaaabbabbaaabbb",
    "babbaabbbbabaaabbbbababb",
    "abbaabbaaaabababaababaabbaabbbababaaaaaa",
    "bababaaabbaabaabbabbabbbababababbaabbaaa",
    "aaabbaaaaabbaaaaabbbaababababbbababbabab",
    "bbaabaabbbbabbbbaababbaabbbbaaba",
    "aabbabaaaaabababaabbaaba",
    "abbaaaabbaabbbabababaaabaabbaabbbabbaaababababbb",
    "bbbbbabbbbabbbbbbbbaabbbbababaaabbbababb",
    "abaabbabbaabbbaaabaaabaa",
    "bbaaaabaababbbaabaabbbba",
    "bbbbbbaababbabbaabbbbbababaaabaa",
    "abbbbaabaababaaabbbabbaaaaaabaabbaabbabb",
    "bbbbabbbaabbaaaaabbbbbaa",
    "abbaaaababaabaaaabbabaab",
    "aabaabbaaaabbaaabbbababb",
    "babbbaaaaaabaabbbbabaaba",
    "abbaabaabbbaababbbbaaaba",
    "bbabbbababaabbaabbabbbabaaaabbbabbaaabaabaabbabbbababbba",
    "bbbbbbbaabbbbbbbabbabaab",
    "babbaaaaaaaabaabbabaabbaabaabbabbabbbbbabbbbaabaabaaabbbabbbaaab",
    "bbababaabbababaaabaaaaaa",
    "bbabbababbababbaaabbaaab",
    "aabbbbbabbbabbaabababbbb",
    "abababbababbaabbbabaaaabbbbababbaabbbbaa",
    "bbabaaabbabaabbabbaabbbaaabaababaabbabbbaababbba",
    "aaabbaaabbaabbabaabbabba",
    "aabbbabbabbbabbbabbbbbaa",
    "aabaabbabbababaaaaaaababbaabababbababbaa",
    "aabaabbbabbaaaabaabbabaabaaaabba"
);

int main(){
    auto result = sum_for_rule<true>(input);
    std::printf("result = %lu\n", result);
    return 0;
}
