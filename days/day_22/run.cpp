#include <cstdio>
#include <tuple>
#include <experimental/array>
#include <vector>
#include <deque>

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

template<typename Array>
inline auto parse_input(Array const& array) noexcept{
    auto players = std::experimental::make_array(
        std::deque<std::size_t>{},
        std::deque<std::size_t>{}
    );
    std::size_t i = 0;
    for(auto const* str : array){
        if(*str == 'P'){
            continue;
        } else if(*str == '\0'){
            ++i;
        } else{
            players[i].push_back(str_to_size_t(str, &str));
        }
    }
    return players;
}

template<typename Deque>
constexpr auto scoring(bool is_player_1_winner, Deque const& winning_player) noexcept{
    std::size_t j = winning_player.size();
    std::size_t sum = 0;
    for(auto const& card : winning_player){
        sum += (card * j--);
    }
    return std::make_tuple(is_player_1_winner, sum);
}

template<bool is_recursive, typename Deque>
constexpr std::tuple<bool, std::size_t> combat(Deque player_1, Deque player_2) noexcept{
    auto player_1_history = std::vector<Deque>{};
    auto player_2_history = std::vector<Deque>{};
    while(!player_1.empty() && !player_2.empty()){
        if constexpr(is_recursive){
            for(std::size_t i = 0; i < player_1_history.size(); ++i){
                if(player_1_history[i] == player_1 && player_2_history[i] == player_2){
                    return scoring(true, player_1);
                }
            }
            player_1_history.push_back(player_1);
            player_2_history.push_back(player_2);
        }
        auto player_1_card = player_1.front();
        player_1.pop_front();
        auto player_2_card = player_2.front();
        player_2.pop_front();
        auto is_player_1_winner = (player_1_card > player_2_card);
        if constexpr(is_recursive){
            if(player_1.size() >= player_1_card && player_2.size() >= player_2_card){
                auto new_player_1 = Deque{};
                new_player_1.insert(new_player_1.end(), player_1.begin(), player_1.begin() + player_1_card);
                auto new_player_2 = Deque{};
                new_player_2.insert(new_player_2.end(), player_2.begin(), player_2.begin() + player_2_card);
                std::tie(is_player_1_winner, std::ignore) = combat<is_recursive>(new_player_1, new_player_2);
            }
        }
        if(is_player_1_winner){
            player_1.push_back(player_1_card);
            player_1.push_back(player_2_card);
        } else{
            player_2.push_back(player_2_card);
            player_2.push_back(player_1_card);
        }
    }
    auto is_player_1_winner = player_2.empty();
    return scoring(is_player_1_winner, (is_player_1_winner ? player_1 : player_2));
}

template<bool is_recursive, typename Array>
constexpr auto winning_players_score(Array const& array) noexcept{
    auto [player_1, player_2] = parse_input(array);
    auto [is_player_1_winner, score] = combat<is_recursive>(player_1, player_2);
    return score;
}

constexpr static auto test_input = std::experimental::make_array(
    "Player 1:",
    "9",
    "2",
    "6",
    "3",
    "1",
    "",
    "Player 2:",
    "5",
    "8",
    "4",
    "7",
    "10"
);

constexpr static auto input = std::experimental::make_array(
    "Player 1:",
    "29",
    "30",
    "44",
    "35",
    "27",
    "2",
    "4",
    "38",
    "45",
    "33",
    "50",
    "21",
    "17",
    "11",
    "25",
    "40",
    "5",
    "43",
    "41",
    "24",
    "12",
    "19",
    "23",
    "8",
    "42",
    "",
    "Player 2:",
    "32",
    "13",
    "22",
    "7",
    "31",
    "16",
    "37",
    "6",
    "10",
    "20",
    "47",
    "46",
    "34",
    "39",
    "1",
    "26",
    "49",
    "9",
    "48",
    "36",
    "14",
    "15",
    "3",
    "18",
    "28"
);

int main(){
    auto score = winning_players_score<true>(input);
    std::printf("score = %lu\n", score);
    return 0;
}
