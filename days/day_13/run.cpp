#include <cstdio>
#include <experimental/array>

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

constexpr auto get_timetable(char const* str) noexcept{
    auto bus_ids = std::array<std::size_t, 200>{};
    std::size_t i = 0;
    while(true){
        if(*str == 'x'){
            bus_ids[i++] = 0;
            ++str;
        } else{
            auto bus_id = str_to_size_t(str, &str);
            bus_ids[i++] = bus_id;
        }
        if(*str == '\0'){
            break;
        }
        ++str;
    }
    return bus_ids;
}

// part 1

template<typename Array>
constexpr auto find_bus_mul_min(Array const& array) noexcept{
    auto earliest_timestamp = str_to_size_t(array[0], nullptr);
    auto bus_ids = get_timetable(array[1]);
    auto timestamp = earliest_timestamp;
    while(true){
        for(auto bus_id : bus_ids){
            if(bus_id > 0 && timestamp % bus_id == 0){
                return bus_id * (timestamp - earliest_timestamp);
            }
        }
        ++timestamp;
    }
    return std::size_t{0};
}

// part 2

/*
 * Find the inverse i.e.:
 *
 * `yz (mod m) == 1 (mod m)`
 */
constexpr auto find_inverse(std::size_t z, std::size_t m) noexcept{
    std::size_t y = 1;
    while((z * y) % m != 1){
        ++y;
    }
    return y;
}

/*
 * For each bus we have:
 *
 * `t + dm (mod id) == 0 (mod id)`
 *
 * which resolves to:
 *
 * `t (mod id) == -dm (mod id)`
 *
 * Use the Chinese remainder theorem
 * to solve multiple equations for t
 */
template<typename Array>
constexpr auto find_earliest_timestamp(Array const& array) noexcept{
    auto bus_ids = get_timetable(array[1]);
    std::size_t m = 1;
    for(auto bus_id : bus_ids){
        if(bus_id > 0){
            m *= bus_id;
        }
    }
    std::size_t t = 0;
    std::size_t i = 0;
    for(auto bus_id : bus_ids){
        if(bus_id > 0){
            auto z = m / bus_id;
            auto y = find_inverse(z, bus_id);
            auto w = (y * z) % m;
            t += (bus_id - i) * w;
        }
        ++i;
    }
    t %= m;
    return t;
}

constexpr static auto test_input = std::experimental::make_array(
    "939",
    "7,13,x,x,59,x,31,19"
);

constexpr static auto input = std::experimental::make_array(
    "1000677",
    "29,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,x,41,x,x,x,x,x,x,x,x,x,661,x,x,x,x,x,x,x,x,x,x,x,x,13,17,x,x,x,x,x,x,x,x,23,x,x,x,x,x,x,x,521,x,x,x,x,x,37,x,x,x,x,x,x,x,x,x,x,x,x,19"
);

constexpr static auto bus_mul_min = find_bus_mul_min(input);
constexpr static auto earliest_timestamp = find_earliest_timestamp(input);

int main(){
    std::printf("bus ID * mins      = %lu\n", bus_mul_min);
    std::printf("earliest_timestamp = %lu\n", earliest_timestamp);
    return 0;
}
