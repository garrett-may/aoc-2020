#include <cstdio>
#include <tuple>
#include <experimental/array>
#include <type_traits>

template<std::size_t size>
using CharConstPtr = char const (&)[size];

template<typename T, std::size_t size>
struct Grid;

template<std::size_t size_0, std::size_t size_1>
struct Grid<CharConstPtr<size_0>, size_1>{
    constexpr static auto no_of_rows = size_1;
    constexpr static auto no_of_cols = size_0 - 1;

    std::array<std::array<char, no_of_cols>, no_of_rows> array;

    constexpr auto& operator[](std::size_t index) noexcept{
        return array[index];
    }
    constexpr auto const& operator[](std::size_t index) const noexcept{
        return array[index];
    }
    constexpr bool operator==(Grid const& that) const noexcept{
        for(std::size_t i = 0; i < no_of_rows; ++i){
            for(std::size_t j = 0; j < no_of_cols; ++j){
                if(this->array[i][j] != that.array[i][j]){
                    return false;
                }
            }
        }
        return true;
    }
    constexpr bool operator!=(Grid const& that) const noexcept{
        return !(*this == that);
    }
};

template<std::size_t size, typename... Ts>
constexpr auto make_grid(CharConstPtr<size> value, Ts&&... values) noexcept{
    return Grid<CharConstPtr<size>, 1 + sizeof...(Ts)>{[](auto&&... elems){
        return std::experimental::make_array([](auto const* elem){
            auto array = std::array<char, size - 1>{};
            for(std::size_t i = 0; i < size - 1; ++i){
                array[i] = elem[i];
            }
            return array;
        }(elems)...);
    }(value, std::forward<Ts>(values)...)};
}

///

template<std::size_t...>
struct Dimension;

template<std::size_t s, std::size_t... sizes>
struct Dimension<s, sizes...>{
    constexpr static std::size_t size = s;
    constexpr static std::size_t dimensionality = 1 + sizeof...(sizes);
    using dim_type = Dimension<sizes...>;
    std::array<dim_type, size> value;

    template<typename F, typename... Is>
    constexpr void for_each(F&& f, Is&&... is) noexcept{
        for(std::size_t i = 0; i < size; ++i){
            value[i].for_each(std::forward<F>(f), std::forward<Is>(is)..., i);
        }
    }
};

template<>
struct Dimension<>{
    constexpr static std::size_t size = 0;
    constexpr static std::size_t dimensionality = 0;
    using dim_type = void;
    char value = '.';

    template<typename F, typename... Is>
    constexpr void for_each(F&& f, Is&&... is) noexcept{
        std::forward<F>(f)(value, std::forward<Is>(is)...);
    }
};

// Checking functions that work regardless of dimension

template<std::size_t index, typename A, typename Indices>
inline std::size_t check_help(A const& old_state, Indices const& indices) noexcept{
    if constexpr(index < std::tuple_size_v<Indices>){
        auto i = indices[index];
        if(0 <= i && i < old_state.size){
            return check_help<index + 1>(old_state.value[i], indices);
        } else{
            return 0;
        }
    } else{
        return old_state.value == '#';
    }
}

template<typename A, typename Indices, typename I, typename... Is>
constexpr std::size_t check_impl(A const& old_state, Indices& indices, I&& i, Is&&... is) noexcept{
    constexpr std::size_t index = old_state.dimensionality - 1 - sizeof...(Is);
    if constexpr(sizeof...(Is) > 0){
        std::size_t sum = 0;
        indices[index] = i - 1;
        sum += check_impl(old_state, indices, std::forward<Is>(is)...);
        indices[index] = i;
        sum += check_impl(old_state, indices, std::forward<Is>(is)...);
        indices[index] = i + 1;
        sum += check_impl(old_state, indices, std::forward<Is>(is)...);
        return sum;
    } else{
        std::size_t sum = 0;
        indices[index] = i - 1;
        sum += check_help<0>(old_state, indices);
        indices[index] = i;
        sum += check_help<0>(old_state, indices);
        indices[index] = i + 1;
        sum += check_help<0>(old_state, indices);
        return sum;
    }
}

template<typename A, typename B>
constexpr void check(A const& old_state, B& new_state) noexcept{
    new_state.for_each([&](auto& value, auto const&... is){
        auto indices = std::array<std::size_t, A::dimensionality>{};
        auto sum = check_impl(old_state, indices, is...) - std::size_t(value == '#');
        value = (value == '#'
              ? (2 <= sum && sum <= 3   ? '#' : '.')
              : (sum == 3               ? '#' : '.')
        );
    });
}

///

template<std::size_t no_of_cycles, typename G, std::size_t I0, std::size_t I1, std::size_t... Is>
constexpr auto make_dimension(G const& grid, std::index_sequence<I0, I1, Is...> const&) noexcept{
    constexpr auto size = 2 * no_of_cycles + grid.no_of_rows;
    return Dimension<size, size, (size * bool(Is + 1))...>{};
}

template<typename D>
constexpr void init_impl(D& dimension, char c) noexcept{
    if constexpr(D::size > 0){
        init_impl(dimension.value[D::size / 2], c);
    } else{
        dimension.value = c;
    }
}

template<typename D, typename G>
constexpr void init(D& dimension, G const& grid) noexcept{
    for(std::size_t i = 0; i < grid.no_of_rows; ++i){
        for(std::size_t j = 0; j < grid.no_of_cols; ++j){
            auto a = D::size / 2 - grid.no_of_rows / 2 + i;
            auto b = D::dim_type::size / 2 - grid.no_of_cols / 2 + j;
            init_impl(dimension.value[a].value[b], grid[i][j]);
        }
    }
}

template<std::size_t dim, std::size_t no_of_cycles, typename G>
constexpr auto cycle(G const& grid) noexcept{
    auto dimension = make_dimension<no_of_cycles>(grid, std::make_index_sequence<dim>{});
    using Dim = std::decay_t<decltype(dimension)>;
    init(dimension, grid);
    for(std::size_t i = 0; i < no_of_cycles; ++i){
        auto new_state = dimension;
        check(dimension, new_state);
        dimension = new_state;
    }
    std::size_t sum = 0;
    dimension.for_each([&](auto const& value, auto const&...){
        sum += std::size_t(value == '#');
    });
    return sum;
}

constexpr static auto test_input = make_grid(
    ".#.",
    "..#",
    "###"
);

constexpr static auto input = make_grid(
    "#.##.##.",
    ".##..#..",
    "....#..#",
    ".##....#",
    "#..##...",
    ".###..#.",
    "..#.#..#",
    ".....#.."
);

int main(){
    auto result = cycle<4, 6>(input);
    std::printf("result = %lu\n", result);
    return 0;
}
