#include <cstdio>
#include <experimental/array>
#include <algorithm>
#include <limits>

template<typename T, std::size_t size>
constexpr auto max(std::array<T, size> const& array) noexcept{
    auto m = std::numeric_limits<T>::min();
    for(auto const& elem : array){
        m = std::max(m, elem);
    }
    return m;
}

template<typename T>
constexpr void swap(T& a, T& b) noexcept{
    auto tmp = a;
    a = b;
    b = tmp;
}

template<typename T, std::size_t size>
constexpr auto sort_impl(std::array<T, size>& array, std::size_t low, std::size_t high) noexcept{
    auto pivot = array[high];
    std::size_t i = low;
    for(std::size_t j = low; j <= high; ++j){
        if(array[j] < pivot){
            swap(array[i], array[j]);
            ++i;
        }
    }
    swap(array[i], array[high]);
    return i;
}

template<typename T, std::size_t size>
constexpr void sort(std::array<T, size>& array, std::size_t low=0, std::size_t high=size-1) noexcept{
    if(low < high){
        auto pivot = sort_impl(array, low, high);
        sort(array, low, pivot - 1);
        sort(array, pivot + 1, high);
    }
}

template<typename T, std::size_t size>
constexpr auto find_differences(std::array<T, size> array) noexcept{
    sort(array);
    auto differences = std::array<std::size_t, 3>{};
    T start = 0;
    for(std::size_t i = 0; i < size; ++i){
        auto left = (i == 0 ? start : array[i - 1]);
        auto right = array[i];
        auto difference = right - left;
        differences[difference - 1] += 1;
    }
    differences[2] += 1;
    return differences;
}


template<auto max, typename T, std::size_t size>
constexpr auto find_arrangements(std::array<T, size> array) noexcept{
    sort(array);
    auto arrangements = std::array<std::size_t, max + 4>{};
    arrangements[max + 3] = 1;
    for(std::size_t i = size - 1; i >= 0; --i){
        auto elem = array[i];
        for(T j = 1; j <= 3; ++j){
            arrangements[elem] += arrangements[elem + j];
        }
        if(i == 0){
            break;
        }
    }
    auto elem = 0;
    for(T j = 1; j <= 3; ++j){
        arrangements[elem] += arrangements[elem + j];
    }
    return arrangements[0];
}

constexpr static auto test_input = std::experimental::make_array(
    16,
    10,
    15,
    5,
    1,
    11,
    7,
    19,
    6,
    12,
    4
);

constexpr static auto test2_input = std::experimental::make_array(
    28,
    33,
    18,
    42,
    31,
    14,
    46,
    20,
    48,
    47,
    24,
    23,
    49,
    45,
    19,
    38,
    39,
    11,
    1,
    32,
    25,
    35,
    8,
    17,
    7,
    9,
    4,
    2,
    34,
    10,
    3
);

constexpr static auto input = std::experimental::make_array(
    54,
    91,
    137,
    156,
    31,
    70,
    143,
    51,
    50,
    18,
    1,
    149,
    129,
    151,
    95,
    148,
    41,
    144,
    7,
    125,
    155,
    14,
    114,
    108,
    57,
    118,
    147,
    24,
    25,
    73,
    26,
    8,
    115,
    44,
    12,
    47,
    106,
    120,
    132,
    121,
    35,
    105,
    60,
    9,
    6,
    65,
    111,
    133,
    38,
    138,
    101,
    126,
    39,
    78,
    92,
    53,
    119,
    136,
    154,
    140,
    52,
    15,
    90,
    30,
    40,
    64,
    67,
    139,
    76,
    32,
    98,
    113,
    80,
    13,
    104,
    86,
    27,
    61,
    157,
    79,
    122,
    59,
    150,
    89,
    158,
    107,
    77,
    112,
    5,
    83,
    58,
    21,
    2,
    66
);

constexpr static auto differences = find_differences(input);
constexpr static auto arrangements = find_arrangements<max(input)>(input);
constexpr static auto answer = std::get<0>(differences) * std::get<2>(differences);

int main(){
    std::printf("1-volt difference = %lu\n", std::get<0>(differences));
    std::printf("2-volt difference = %lu\n", std::get<1>(differences));
    std::printf("3-volt difference = %lu\n", std::get<2>(differences));
    std::printf("1-volt * 3-volt   = %lu\n", answer);
    std::printf("arrangements      = %lu\n", arrangements);
    return 0;
}
