#include <cstdio>
#include <cstdint>
#include <tuple>
#include <experimental/array>
#include <unordered_map>

/*
 * Van Eck sequence
 */
 template<std::size_t iteration, typename Array>
 constexpr auto find_spoken(Array const& array) noexcept{
     auto history = std::unordered_map<std::uint32_t, std::uint32_t>{};
     for(std::uint32_t i = 0; i < array.size() - 1; ++i){
         history[array[i]] = i + 1;
     }
     std::uint32_t last_elem = array[array.size() - 1];
     for(std::uint32_t i = array.size(); i < iteration; ++i){
         auto last_turn = history[last_elem];
         auto diff = (last_turn > 0 ? i - last_turn : 0);
         history[last_elem] = i;
         last_elem = diff;
     }
     return last_elem;
 }

constexpr static auto input = std::experimental::make_array(
    0, 6, 1, 7, 2, 19, 20
);

int main(){
    auto answer = find_spoken<30000000>(input);
    std::printf("result = %d\n", answer);
    return 0;
}
