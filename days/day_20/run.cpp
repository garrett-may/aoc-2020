#include <cstdio>
#include <tuple>
#include <experimental/array>
#include <vector>
#include <map>
#include <unordered_set>
#include <functional>
#include <bitset>

constexpr std::uint16_t str_to_uint16(char const* str, char const** endptr) noexcept{
    std::uint16_t i = 0;
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

template<std::size_t size>
constexpr auto reverse_bits(std::bitset<size> bits) noexcept{
    for(std::size_t i = 0; i < (size / 2); ++i){
        std::size_t j = size - 1 - i;
        bool tmp = bits[i];
        bits[i] = bits[j];
        bits[j] = tmp;
    }
    return bits;
}

// This is required because for some reason operator[] of std::bitset
// returns a reference, and `auto` does not treat it as a value
// Likely because a bitset reference is not the same as a normal reference

template<typename T>
struct value_of{
    using type = typename T::value_type;
};

template<std::size_t size>
struct value_of<std::bitset<size>>{
    using type = bool;
};

template<typename T>
using value_of_t = typename value_of<T>::type;

// Flipping and Rotating

template<typename G>
constexpr void flip_x(G& g) noexcept{
    for(std::size_t i = 0; i < g.size() / 2; ++i){
        auto j = g.size() - 1 - i;
        for(std::size_t k = 0; k < g.size(); ++k){
            using T = value_of_t<typename G::value_type>;
            T tmp = g[i][k];
            g[i][k] = g[j][k];
            g[j][k] = tmp;
        }
    }
}

template<typename G>
constexpr void flip_y(G& g) noexcept{
    for(std::size_t i = 0; i < g.size() / 2; ++i){
        auto j = g.size() - 1 - i;
        for(std::size_t k = 0; k < g.size(); ++k){
            using T = value_of_t<typename G::value_type>;
            T tmp = g[k][i];
            g[k][i] = g[k][j];
            g[k][j] = tmp;
        }
    }
}

template<typename G>
constexpr void rotate_clockwise(G& g) noexcept{
    for(std::size_t i = 0; i < g.size() / 2; ++i){
        std::size_t h = g.size() - 1 - i;
        for(std::size_t j = i; j < g.size() - 1 - i; ++j){
            std::size_t k = g.size() - 1 - j;
            using T = value_of_t<typename G::value_type>;
            T tmp = g[i][j];
            g[i][j] = g[k][i];
            g[k][i] = g[h][k];
            g[h][k] = g[j][h];
            g[j][h] = tmp;
        }
    }
}

struct Tile{
    constexpr auto flipped_x() const noexcept{
        auto tile = *this;
        auto tmp = tile.sides[0];
        tile.sides[0] = tile.sides[2];
        tile.sides[2] = tmp;
        tile.sides[1] = reverse_bits(tile.sides[1]);
        tile.sides[3] = reverse_bits(tile.sides[3]);
        flip_x(tile.grid);
        return tile;
    }
    constexpr auto flipped_y() const noexcept{
        auto tile = *this;
        auto tmp = tile.sides[1];
        tile.sides[1] = tile.sides[3];
        tile.sides[3] = tmp;
        tile.sides[0] = reverse_bits(tile.sides[0]);
        tile.sides[2] = reverse_bits(tile.sides[2]);
        flip_y(tile.grid);
        return tile;
    }
    constexpr auto rotated_clockwise() const noexcept{
        auto tile = *this;
        auto tmp = tile.sides[0];
        tile.sides[0] = reverse_bits(tile.sides[3]);
        tile.sides[3] = tile.sides[2];
        tile.sides[2] = reverse_bits(tile.sides[1]);
        tile.sides[1] = tmp;
        rotate_clockwise(tile.grid);
        return tile;
    }
    friend constexpr bool operator==(Tile const& a, Tile const& b) noexcept{
        return a.id == b.id;
    }
    friend constexpr bool operator!=(Tile const& a, Tile const& b) noexcept{
        return !(a == b);
    }

    std::uint16_t id;
    std::array<std::bitset<10>, 4> sides;
    std::array<std::bitset<10>, 10> grid;
};

namespace std{
    template<>
    struct hash<Tile>{
        constexpr std::size_t operator()(Tile const& tile) const noexcept{
            return tile.id;
        }
    };
}

constexpr auto calc_side_info(std::ptrdiff_t dy, std::ptrdiff_t dx) noexcept{
    auto side = (dx == 0
            ? (dy == 1 ? 2 : 0)
            : (dx == 1 ? 1 : 3)
    );
    auto other_side = (side + 2) % 4;
    return std::experimental::make_array(side, other_side);
}

template<typename Array>
constexpr auto parse_tile(Array const& array, std::size_t k) noexcept{
    auto tile = Tile{};
    auto* str = array[k];
    str += 5;
    tile.id = str_to_uint16(str, &str);
    for(std::size_t i = 0; i < 10; ++i){
        for(std::size_t j = 0; j < 10; ++j){
            auto b = (array[k + 1 + i][j] == '#');
            if(i == 0){
                tile.sides[0][j] = b;
            }
            if(j == 9){
                tile.sides[1][i] = b;
            }
            if(i == 9){
                tile.sides[2][j] = b;
            }
            if(j == 0){
                tile.sides[3][i] = b;
            }
            tile.grid[i][j] = b;
        }
    }
    return tile;
}

struct Graph{
    std::array<std::map<std::uint16_t, std::vector<Tile>>, 4> maps;
};

template<typename F, typename T>
constexpr void iterate_through_variations(F&& f, bool& continue_iterating, T const& value) noexcept{
    auto w = value;
    auto x = w.flipped_x();
    auto y = w.flipped_y();
    auto z = x.flipped_y();
    for(auto const& flipped_value : std::experimental::make_array(w, x, y, z)){
        auto a = flipped_value;
        auto b = a.rotated_clockwise();
        auto c = b.rotated_clockwise();
        auto d = c.rotated_clockwise();
        for(auto const& rotated_value : std::experimental::make_array(a, b, c, d)){
            std::invoke(std::forward<F>(f), rotated_value, continue_iterating);
            if(!continue_iterating){
                return;
            }
        }
    }
}

template<typename F, typename Tiles>
constexpr void tile_iterator(F&& f, Tiles const& tiles) noexcept{
    auto continue_iterating = true;
    for(auto const& tile : tiles){
        iterate_through_variations(std::forward<F>(f), continue_iterating, tile);
        if(!continue_iterating){
            return;
        }
    }
}

template<typename Tiles>
constexpr auto build_graph(Tiles const& tiles) noexcept{
    auto graph = Graph{};
    tile_iterator([&](auto const& rotated_tile, auto const&){
        for(std::size_t i = 0; i < 4; ++i){
            graph.maps[i][rotated_tile.sides[i].to_ulong()].push_back(rotated_tile);
        }
    }, tiles);
    return graph;
}

template<std::size_t grid_size>
struct Grid{
    constexpr auto flipped_x() const noexcept{
        auto grid = *this;
        for(auto& row : grid.grid){
            for(auto& tile : row){
                tile = tile.flipped_x();
            }
        }
        flip_x(grid.grid);
        return grid;
    }
    constexpr auto flipped_y() const noexcept{
        auto grid = *this;
        for(auto& row : grid.grid){
            for(auto& tile : row){
                tile = tile.flipped_y();
            }
        }
        flip_y(grid.grid);
        return grid;
    }
    constexpr auto rotated_clockwise() const noexcept{
        auto grid = *this;
        for(auto& row : grid.grid){
            for(auto& tile : row){
                tile = tile.rotated_clockwise();
            }
        }
        rotate_clockwise(grid.grid);
        return grid;
    }
    constexpr auto& stitched_borderless(std::size_t j, std::size_t i) noexcept{
        return grid[j / 8][i / 8].grid[j % 8 + 1][i % 8 + 1];
    }
    constexpr auto stitched_borderless(std::size_t j, std::size_t i) const noexcept{
        return grid[j / 8][i / 8].grid[j % 8 + 1][i % 8 + 1];
    }
    std::unordered_set<Tile> set{};
    std::array<std::array<Tile, grid_size>, grid_size> grid{};
};

template<typename F, std::size_t size>
constexpr void grid_iterator(F&& f, Grid<size> const& grid) noexcept{
    auto continue_iterating = true;
    iterate_through_variations(std::forward<F>(f), continue_iterating, grid);
}

template<std::size_t grid_size>
constexpr auto check_valid(Grid<grid_size> const& grid, Tile const& tile, std::size_t j, std::size_t i) noexcept{
    for(auto [dy, dx] : std::experimental::make_array(
        std::experimental::make_array(-1, 0),
        std::experimental::make_array(0, 1),
        std::experimental::make_array(1, 0),
        std::experimental::make_array(0, -1)
    )){
        auto b = j + dy;
        auto a = i + dx;
        if(0 <= a && a < grid_size && 0 <= b && b < grid_size){
            auto const& other_tile = grid.grid[b][a];
            if(other_tile.id == 0){
                continue;
            }
            auto [side, other_side] = calc_side_info(dy, dx);
            if(tile.sides[side] != other_tile.sides[other_side]){
                return false;
            }
        }
    }
    return true;
}

template<std::size_t grid_size>
constexpr auto calc_coords(std::size_t k) noexcept{
    auto b = k / grid_size;
    auto a = k % grid_size;
    if(b % 2 == 1){
        a = grid_size - 1 - a; // This causes the coordinates to snake around the grid
    }
    return std::experimental::make_array(b, a);
}

template<std::size_t grid_size>
constexpr auto find_corners_product_impl(Grid<grid_size>& grid, Graph& graph, std::size_t k) noexcept{
    auto [b, a] = calc_coords<grid_size>(k);
    auto const& tile = grid.grid[b][a];
    auto l = k + 1;
    if(l == grid_size * grid_size){
        return true;
    }
    auto [y, x] = calc_coords<grid_size>(l);
    std::ptrdiff_t dy = y - b;
    std::ptrdiff_t dx = x - a;
    auto [side, other_side] = calc_side_info(dy, dx);
    for(auto const& other_tile : graph.maps[other_side][tile.sides[side].to_ulong()]){
        if(other_tile.id == 0){
            continue;
        }
        if(grid.set.find(other_tile) != grid.set.cend()){
            continue;
        }
        if(check_valid(grid, other_tile, y, x)){
            grid.grid[y][x] = other_tile;
            grid.set.insert(other_tile);
            auto success = find_corners_product_impl(grid, graph, l);
            if(success){
                return true;
            } else{
                grid.grid[y][x] = Tile{};
                grid.set.erase(other_tile);
                return false;
            }
        }
    }
    return false;
}

// part 1

template<std::size_t grid_size, typename Array>
constexpr auto find_corners_product(Array const& array) noexcept{
    auto tiles = std::array<Tile, grid_size * grid_size>{};
    std::size_t i = 0;
    for(std::size_t k = 0; k < array.size(); k += 12){
        tiles[i++] = parse_tile(array, k);
    }
    auto graph = build_graph(tiles);
    auto success = false;
    auto grid = Grid<grid_size>{};
    tile_iterator([&](auto const& rotated_tile, auto& continue_iterating){
        grid.grid[0][0] = rotated_tile;
        grid.set.insert(rotated_tile);
        success = find_corners_product_impl(grid, graph, 0);
        if(!success){
            grid.grid[0][0] = Tile{};
            grid.set.erase(rotated_tile);
        }
        continue_iterating = !success;
    }, tiles);
    if(success){
        std::size_t product = 1;
        for(std::size_t j = 0; j < grid_size; j += grid_size - 1){
            for(std::size_t i = 0; i < grid_size; i += grid_size - 1){
                product *= grid.grid[j][i].id;
            }
        }
        return std::make_tuple(product, grid);
    } else{
        return std::make_tuple(std::size_t{0}, grid);
    }
}

// part 2

template<std::size_t grid_size>
constexpr auto search_for_nessie(Grid<grid_size> const& grid, std::size_t j, std::size_t i) noexcept{
    if(0 <= i - 18 && i + 1 < grid_size * 8 && j + 2 < grid_size * 8){
        for(auto [dy, dx] : std::experimental::make_array(
            std::experimental::make_array(1, -18),                                      //  #
            std::experimental::make_array(2, -17),                                      // #
                                                                                        //
            std::experimental::make_array(2, -14),                                      // #
            std::experimental::make_array(1, -13),                                      //  #
            std::experimental::make_array(1, -12),                                      //  #
            std::experimental::make_array(2, -11),                                      // #
                                                                                        //
            std::experimental::make_array(2, -8),                                       // #
            std::experimental::make_array(1, -7),                                       //  #
            std::experimental::make_array(1, -6),                                       //  #
            std::experimental::make_array(2, -5),                                       // #
                                                                                        //
            std::experimental::make_array(2, -2),                                       // #
            std::experimental::make_array(1, -1),                                       //  #
            std::experimental::make_array(1, 0), std::experimental::make_array(0, 0),   //  ##
            std::experimental::make_array(1, 1)                                         //  #
        )){
            auto b = j + dy;
            auto a = i + dx;
            if(!grid.stitched_borderless(b, a)){
                return false;
            }
        }
        return true;
    } else{
        return false;
    }
}

template<std::size_t grid_size>
constexpr auto search_for_nessies(Grid<grid_size> const& grid) noexcept{
    std::size_t sum = 0;
    for(std::size_t j = 0; j < grid_size * 8; ++j){
        for(std::size_t i = 0; i < grid_size * 8; ++i){
            sum += std::size_t{search_for_nessie(grid, j, i)};
        }
    }
    return sum;
}

template<std::size_t grid_size>
constexpr auto determine_how_rough(Grid<grid_size> const& grid, std::size_t number_of_nessies) noexcept{
    std::size_t sum = 0;
    for(std::size_t j = 0; j < grid_size * 8; ++j){
        for(std::size_t i = 0; i < grid_size * 8; ++i){
            sum += std::size_t{grid.stitched_borderless(j, i)};
        }
    }
    return sum - (number_of_nessies * 15);
}

template<std::size_t grid_size>
constexpr auto find_nessies(Grid<grid_size> const& grid) noexcept{
    std::size_t sum = 0;
    std::size_t i = 0;
    grid_iterator([&](auto const& new_grid, auto& continue_iterating){
        auto number_of_nessies = search_for_nessies(new_grid);
        if(number_of_nessies > 0){
            sum = determine_how_rough(grid, number_of_nessies);
            continue_iterating = false;
        }
    }, grid);
    return sum;
}

constexpr static auto test_input = std::experimental::make_array(
    "Tile 2311:",
    "..##.#..#.",
    "##..#.....",
    "#...##..#.",
    "####.#...#",
    "##.##.###.",
    "##...#.###",
    ".#.#.#..##",
    "..#....#..",
    "###...#.#.",
    "..###..###",
    "",
    "Tile 1951:",
    "#.##...##.",
    "#.####...#",
    ".....#..##",
    "#...######",
    ".##.#....#",
    ".###.#####",
    "###.##.##.",
    ".###....#.",
    "..#.#..#.#",
    "#...##.#..",
    "",
    "Tile 1171:",
    "####...##.",
    "#..##.#..#",
    "##.#..#.#.",
    ".###.####.",
    "..###.####",
    ".##....##.",
    ".#...####.",
    "#.##.####.",
    "####..#...",
    ".....##...",
    "",
    "Tile 1427:",
    "###.##.#..",
    ".#..#.##..",
    ".#.##.#..#",
    "#.#.#.##.#",
    "....#...##",
    "...##..##.",
    "...#.#####",
    ".#.####.#.",
    "..#..###.#",
    "..##.#..#.",
    "",
    "Tile 1489:",
    "##.#.#....",
    "..##...#..",
    ".##..##...",
    "..#...#...",
    "#####...#.",
    "#..#.#.#.#",
    "...#.#.#..",
    "##.#...##.",
    "..##.##.##",
    "###.##.#..",
    "",
    "Tile 2473:",
    "#....####.",
    "#..#.##...",
    "#.##..#...",
    "######.#.#",
    ".#...#.#.#",
    ".#########",
    ".###.#..#.",
    "########.#",
    "##...##.#.",
    "..###.#.#.",
    "",
    "Tile 2971:",
    "..#.#....#",
    "#...###...",
    "#.#.###...",
    "##.##..#..",
    ".#####..##",
    ".#..####.#",
    "#..#.#..#.",
    "..####.###",
    "..#.#.###.",
    "...#.#.#.#",
    "",
    "Tile 2729:",
    "...#.#.#.#",
    "####.#....",
    "..#.#.....",
    "....#..#.#",
    ".##..##.#.",
    ".#.####...",
    "####.#.#..",
    "##.####...",
    "##..#.##..",
    "#.##...##.",
    "",
    "Tile 3079:",
    "#.#.#####.",
    ".#..######",
    "..#.......",
    "######....",
    "####.#..#.",
    ".#...#.##.",
    "#.#####.##",
    "..#.###...",
    "..#.......",
    "..#.###...",
    ""
);

int main(){
    auto input = std::vector<char const*>{{
        "Tile 1753:",
        "..##.#.#.#",
        "#...#.....",
        "#......#..",
        "#..##..#.#",
        "#..##....#",
        "#.........",
        "#.#...##..",
        "#....#..##",
        "##.......#",
        "#...######",
        "",
        "Tile 1699:",
        ".###.....#",
        "#.#......#",
        ".....#....",
        "#.....#..#",
        "..........",
        "#.#......#",
        "...#......",
        "#.........",
        ".###......",
        ".#....#.#.",
        "",
        "Tile 2297:",
        "####....##",
        "....#...#.",
        "###...#...",
        "...##.....",
        "#...#..#.#",
        "..........",
        "#......#.#",
        ".........#",
        "..........",
        "#..##.##..",
        "",
        "Tile 2017:",
        "#....#..##",
        ".#.#..#..#",
        "....##....",
        "#...#....#",
        "#.#.......",
        "..........",
        "#.#......#",
        "....##...#",
        "...#....##",
        "..###.##.#",
        "",
        "Tile 1283:",
        ".#....##..",
        "###...##..",
        "#.........",
        "#........#",
        "...####..#",
        ".#........",
        "....#.....",
        "........#.",
        "#....#..#.",
        "..#.####.#",
        "",
        "Tile 1171:",
        ".#..#.#.##",
        ".........#",
        "#...##..##",
        "#...#....#",
        ".......#..",
        "####.#...#",
        "..#....#..",
        "#.........",
        "#...#....#",
        "#..#.#.###",
        "",
        "Tile 2251:",
        ".##.##..#.",
        ".###.##..#",
        "#..##.#.#.",
        "#....#..#.",
        ".#........",
        "......#..#",
        ".....#...#",
        ".###.#....",
        "##...##..#",
        ".####.#.##",
        "",
        "Tile 2357:",
        "###...##.#",
        "......#.##",
        "#........#",
        "..#....#.#",
        "..#...##..",
        "#....##..#",
        "........#.",
        "#........#",
        "#.##...#..",
        "###.##.#..",
        "",
        "Tile 2503:",
        "...#...###",
        "..#......#",
        "#......##.",
        "#...#.....",
        ".....#..#.",
        "##...#..##",
        ".......#.#",
        ".#...#.#..",
        "#.......#.",
        "##..#..###",
        "",
        "Tile 1741:",
        ".#....##.#",
        "#.....#...",
        "...#...#..",
        "##....#...",
        ".##....###",
        "#.###.#..#",
        "#..#....##",
        "#.##.....#",
        "#.#.......",
        "###.##...#",
        "",
        "Tile 3433:",
        "#.......#.",
        "#.#.......",
        ".....#..#.",
        "#........#",
        "###.#..#..",
        "#....#....",
        ".....#...#",
        "..#......#",
        "....#.#.##",
        "##.##.....",
        "",
        "Tile 3947:",
        ".##.##.###",
        "#.#...#.##",
        "...#.##..#",
        "#.......##",
        "..#.......",
        ".#...#...#",
        ".....###.#",
        "####..##.#",
        ".##..#.#..",
        "..###..#.#",
        "",
        "Tile 1627:",
        "##....##.#",
        "#....#....",
        "....##...#",
        "##..#.....",
        "#..#....#.",
        ".#..#.##.#",
        ".#.......#",
        "#.........",
        ".#...##..#",
        "#.#....#..",
        "",
        "Tile 1667:",
        "###.#...##",
        "#..#.##..#",
        "##.....#.#",
        ".#........",
        "..#....#.#",
        ".##.......",
        "......#...",
        "....###.#.",
        "##.#..#.#.",
        ".#..##...#",
        "",
        "Tile 2287:",
        "##..####.#",
        "#.........",
        "#.##...#..",
        "#..#....#.",
        "#........#",
        "......##..",
        "#.##..#..#",
        "...#.....#",
        ".........#",
        "#####..###",
        "",
        "Tile 1787:",
        ".##.#.#.#.",
        "#.##.....#",
        ".........#",
        "...#......",
        "..#..#...#",
        "##....#..#",
        "##.......#",
        "..#.......",
        "...#....#.",
        ".##.####..",
        "",
        "Tile 1873:",
        "..#.###.##",
        ".......#.#",
        "#.#.......",
        "#...#.##.#",
        "......##..",
        ".#.....#.#",
        "...##...#.",
        "........#.",
        ".......#.#",
        "..######.#",
        "",
        "Tile 3709:",
        "..##..##.#",
        "##.......#",
        "#.........",
        "....#.....",
        "#..#...#.#",
        ".#....#..#",
        "...##....#",
        "#..#.....#",
        "....##....",
        ".##.###...",
        "",
        "Tile 3137:",
        "###..#.#.#",
        ".........#",
        "#....#....",
        "#.#......#",
        "...#.#.#.#",
        "..#...#...",
        ".........#",
        ".........#",
        "#.........",
        "##..###..#",
        "",
        "Tile 1423:",
        "###.##..##",
        "..........",
        "...#.....#",
        ".......#.#",
        "..........",
        "##.......#",
        "##........",
        "#.#....#..",
        "#.#.....##",
        ".##...#.##",
        "",
        "Tile 1399:",
        "#.#.#....#",
        ".........#",
        "#..#..#..#",
        "..#.......",
        "...#..##.#",
        "#..##...##",
        "#..#.....#",
        "#.##.#.#.#",
        ".....#...#",
        "####.....#",
        "",
        "Tile 2411:",
        ".##.#.#..#",
        "##..##...#",
        "#..#......",
        "##..#.##.#",
        ".......#.#",
        "##........",
        "#...#...#.",
        ".....#.#..",
        "........##",
        "#####.#..#",
        "",
        "Tile 3323:",
        "##.#...###",
        ".........#",
        ".....#....",
        "#...#.#..#",
        "#........#",
        "....#...##",
        "#........#",
        "#.....###.",
        "...#.#...#",
        "#.#...#...",
        "",
        "Tile 1213:",
        "..##.##...",
        ".....##..#",
        "#.....#..#",
        "#........#",
        "...#...#.#",
        "#.....#...",
        ".#.#.##...",
        ".....#...#",
        "#...#.#...",
        ".###.#####",
        "",
        "Tile 1619:",
        "#..###..##",
        "#.#.#.....",
        "#........#",
        ".........#",
        "###...#.##",
        ".#........",
        "..........",
        "..........",
        "#........#",
        "##.###.#.#",
        "",
        "Tile 1997:",
        "#....#....",
        "..#......#",
        "#.#...#..#",
        "#..#.....#",
        "#.....#..#",
        "#.#......#",
        "#.........",
        ".........#",
        ".....#...#",
        "#####..##.",
        "",
        "Tile 3533:",
        "#....#.#.#",
        "......##.#",
        "......##..",
        "#.....#..#",
        "....#..#.#",
        ".....#...#",
        ".#.#...#.#",
        "#...#..###",
        "#...#.#...",
        ".#..###.##",
        "",
        "Tile 3389:",
        "#..###....",
        ".#..#....#",
        "#.#...#.#.",
        ".........#",
        "#....#...#",
        "#.#....#.#",
        "......#..#",
        "##.......#",
        "##........",
        "...#####..",
        "",
        "Tile 3413:",
        "..##.##..#",
        "#........#",
        "#....#.#..",
        "#....#....",
        ".....##..#",
        ".........#",
        "....#.#..#",
        "#.#.#.#...",
        "#....#...#",
        ".####..#.#",
        "",
        "Tile 3833:",
        "##.......#",
        ".........#",
        "#.###...#.",
        ".....#.#..",
        "#.........",
        "#.....#...",
        "##.#......",
        "####.##..#",
        "....##...#",
        ".#.#...#..",
        "",
        "Tile 2273:",
        ".##.#.#.##",
        "#........#",
        "#.#....#..",
        "##..#....#",
        "....#.....",
        "#.......#.",
        ".#........",
        ".#...#....",
        "#........#",
        ".###...##.",
        "",
        "Tile 1987:",
        "...#...###",
        "#..#......",
        "#...#....#",
        "...##.....",
        "#........#",
        ".....##...",
        "#....##..#",
        ".#..##...#",
        "....#..#.#",
        "##.##..##.",
        "",
        "Tile 3877:",
        ".##...#.##",
        "..........",
        "..#.#.....",
        "..........",
        "#..##...#.",
        "#.#.....#.",
        ".#.......#",
        ".#..#.#...",
        "...#...#..",
        "#.##..#..#",
        "",
        "Tile 3109:",
        "##....##..",
        "##...#.#..",
        "..........",
        "...#.....#",
        ".####..###",
        ".#.##..#.#",
        "#.#...#...",
        "..........",
        "#.......#.",
        "##......#.",
        "",
        "Tile 3907:",
        "#####.##.#",
        "##.....#.#",
        "..##..#...",
        ".##....#.#",
        "..#.....##",
        "#.........",
        ".........#",
        "#.........",
        "........##",
        ".#...#.#.#",
        "",
        "Tile 1823:",
        "##...#.#.#",
        "#.........",
        "..........",
        "....#.....",
        "...#.##...",
        "##...#...#",
        "#.........",
        "##....#.##",
        "#...##..#.",
        "#####.#.#.",
        "",
        "Tile 1747:",
        ".#...##.#.",
        "...#.#....",
        "..#.##....",
        "#...##...#",
        "...#......",
        "#....##..#",
        "...#.....#",
        "...##..#.#",
        "#.###....#",
        "#......###",
        "",
        "Tile 3671:",
        ".####..###",
        "#.#......#",
        "..........",
        "#.....#...",
        ".#..#.#...",
        "..........",
        "#.........",
        "#.........",
        "..........",
        "#..#..##.#",
        "",
        "Tile 3541:",
        "###..#####",
        "##....#.#.",
        ".......#..",
        "....#.....",
        "#.#.....##",
        "#.###..###",
        "...#.#....",
        ".#.......#",
        ".....#....",
        "..#####..#",
        "",
        "Tile 3457:",
        "#..###....",
        "#....#..##",
        ".##......#",
        "#....#..#.",
        "..........",
        "#........#",
        "..........",
        "...#...#..",
        "#.....##.#",
        ".##..#.#..",
        "",
        "Tile 1721:",
        "..##.##.##",
        ".........#",
        "..#.......",
        "##.......#",
        "...#.....#",
        "...#...#..",
        "#..##.....",
        "........##",
        "#.#......#",
        "..#.#..###",
        "",
        "Tile 1597:",
        "#.#...#..#",
        "..#.......",
        "#.....#..#",
        "..#......#",
        "..#..#...#",
        "#..####...",
        "...#......",
        "#.#.#.#..#",
        "....#...#.",
        "#.##...#.#",
        "",
        "Tile 2087:",
        "####.#.###",
        "#...#....#",
        "##.......#",
        "##......##",
        "#.#......#",
        "..........",
        "#....##...",
        "....#..#.#",
        "####....#.",
        "######.##.",
        "",
        "Tile 2309:",
        "...###...#",
        ".##.......",
        ".....#...#",
        "#......#.#",
        "##.......#",
        "####.#....",
        "#.........",
        "##.#......",
        "..#..#..#.",
        "####.#.#.#",
        "",
        "Tile 3797:",
        "##..######",
        ".....#....",
        "#.###....#",
        ".#.......#",
        "..........",
        "#......#.#",
        "#..#......",
        "...#.#....",
        "#.........",
        "#.###...##",
        "",
        "Tile 2663:",
        "#....#.##.",
        "..........",
        ".#.......#",
        "##.....#..",
        ".#....#...",
        "#......#..",
        "#..#....##",
        ".#..#....#",
        "........##",
        "....######",
        "",
        "Tile 3779:",
        ".##....#.#",
        "##.#..#..#",
        "#.........",
        "#.........",
        "..#..#.#.#",
        ".......##.",
        "#....#...#",
        "#........#",
        "###.......",
        "#..##.###.",
        "",
        "Tile 1163:",
        "#####.#.##",
        "#......###",
        "..........",
        "#..#....#.",
        "#..##...##",
        "#...#.....",
        "#..##.#..#",
        "#....#.###",
        "#....#...#",
        "##.#.####.",
        "",
        "Tile 2683:",
        "....#.##.#",
        "#.....#.##",
        ".#....#...",
        ".#.......#",
        "......##..",
        ".#.#.....#",
        ".........#",
        ".......#..",
        "..........",
        "######.#.#",
        "",
        "Tile 1783:",
        "#.###.##.#",
        ".......#.#",
        "#..#....##",
        ".#..#.#.#.",
        "#........#",
        "#.......##",
        "#....###.#",
        "..##.#.#.#",
        ".....#..##",
        "..######.#",
        "",
        "Tile 1097:",
        "#.##..#...",
        "....#...##",
        "#...#.....",
        ".....#....",
        ".....#..##",
        "...##..#..",
        "..#...#...",
        "#.........",
        "#..##..#.#",
        "#.####...#",
        "",
        "Tile 2731:",
        ".#.##.#.#.",
        "......#...",
        ".....##..#",
        "...#..#..#",
        "###......#",
        "#...#.#..#",
        "..###....#",
        ".....#...#",
        "#...#....#",
        ".##.#.#...",
        "",
        "Tile 1433:",
        "###.###.##",
        "...#.#....",
        "..........",
        "..#......#",
        "..#.......",
        "#..#......",
        "..##.....#",
        "..........",
        "#.#..##..#",
        "##..##.#.#",
        "",
        "Tile 3853:",
        "..##..#..#",
        "#.#.......",
        ".....#...#",
        "....#..#..",
        ".......#.#",
        "##...##...",
        "##......##",
        ".........#",
        "##..#...##",
        "#.#..#....",
        "",
        "Tile 3169:",
        ".###....#.",
        "##.#....##",
        "#.........",
        "#.........",
        "..#.......",
        "#.......#.",
        ".#.##....#",
        ".#....#...",
        "##..##.#..",
        "#######..#",
        "",
        "Tile 3299:",
        "##..##...#",
        "..##......",
        "###.##....",
        "...#..##..",
        "#...#.##..",
        ".........#",
        "##....#...",
        ".........#",
        ".#...#.#.#",
        "##.#.....#",
        "",
        "Tile 2999:",
        "##..#.....",
        "#..#.....#",
        "#......#.#",
        ".........#",
        "#.........",
        ".........#",
        ".#...#....",
        "........##",
        "#.......#.",
        ".###..#...",
        "",
        "Tile 1499:",
        ".######...",
        "....##..#.",
        ".#.....###",
        "#...#.##.#",
        "......#.#.",
        "..#....#.#",
        ".#.......#",
        "......#.##",
        ".......#..",
        "###.####.#",
        "",
        "Tile 2237:",
        "#.#.###.#.",
        "..#.......",
        "#.#..#...#",
        "#......##.",
        "#####....#",
        ".#......#.",
        "##.#..###.",
        "..#...#...",
        "##........",
        "....####.#",
        "",
        "Tile 3049:",
        "......#..#",
        "...#....##",
        "...##.....",
        ".#.#..#...",
        ".#......##",
        ".#.#..#..#",
        ".........#",
        ".#.#....#.",
        "..#.......",
        "#.#..#..#.",
        "",
        "Tile 1583:",
        "#.#####...",
        "#........#",
        "........##",
        "..#.......",
        ".........#",
        "#.#..#..#.",
        "...#..##.#",
        "....##..##",
        "##.#.....#",
        "####...###",
        "",
        "Tile 2791:",
        ".....###..",
        "#...##....",
        "....##...#",
        ".........#",
        "#.........",
        ".#...##..#",
        "#.........",
        "#.#.#....#",
        "##..#..#..",
        "#.#..##..#",
        "",
        "Tile 1879:",
        ".#.###.#..",
        "#......#.#",
        "....#....#",
        "......####",
        "#......#.#",
        "#....#.#..",
        "......#...",
        "#........#",
        "#........#",
        "#..###.###",
        "",
        "Tile 2633:",
        "##.#.....#",
        "#.#......#",
        "........#.",
        "..#......#",
        "#.....#..#",
        "#.......##",
        "#.....#...",
        "...#.....#",
        "#..#....#.",
        "#..#.###.#",
        "",
        "Tile 3701:",
        "..##.##.#.",
        ".........#",
        ".#...##.##",
        "..#.......",
        "##.......#",
        ".....#....",
        "......##.#",
        ".#.#...#.#",
        "....#...##",
        "#..#..#.##",
        "",
        "Tile 2267:",
        ".....##..#",
        "...#.#.###",
        ".......###",
        ".#......##",
        ".........#",
        "#.....#..#",
        ".........#",
        "#....##..#",
        "#.#....##.",
        "...#####..",
        "",
        "Tile 2687:",
        "#..##.####",
        ".....#...#",
        "#....##..#",
        "..........",
        ".#.....#..",
        "#........#",
        "#...#.#...",
        "#........#",
        "#..#...##.",
        "#..##..#.#",
        "",
        "Tile 2113:",
        "##.#.##..#",
        ".#........",
        ".......#.#",
        "#.##.....#",
        ".........#",
        "...#......",
        "#.#...#...",
        ".##.......",
        "..........",
        ".#.#.#...#",
        "",
        "Tile 3319:",
        "...##..###",
        "..........",
        ".......#..",
        "#....#..#.",
        "..###....#",
        ".#....#..#",
        "#.#.#..#.#",
        "..##...#..",
        ".#.......#",
        "#..#.####.",
        "",
        "Tile 1303:",
        "#..##....#",
        "#....#....",
        "#.......#.",
        "#........#",
        "...#...#..",
        "..##.##.##",
        "........#.",
        "..........",
        "....#..#.#",
        "##..#.#.##",
        "",
        "Tile 1709:",
        "###.##.#.#",
        "##.......#",
        "#...#..#.#",
        "..........",
        ".........#",
        ".........#",
        "#.##....##",
        "....#..###",
        ".......#..",
        "#.#......#",
        "",
        "Tile 3547:",
        "#....####.",
        "#..##...##",
        "##....#..#",
        "#.........",
        "..#.#....#",
        "#....#...#",
        "...#....##",
        "..#.....##",
        "........#.",
        ".###.....#",
        "",
        "Tile 2383:",
        "#...#.#..#",
        "...#..##..",
        "##......##",
        "#..#......",
        "##......#.",
        "#........#",
        "........##",
        "..........",
        "#....#....",
        "####..###.",
        "",
        "Tile 3989:",
        "..##.#..#.",
        "#.......##",
        "#.#......#",
        "##.#.....#",
        "#..#......",
        ".........#",
        "#.##......",
        "...#.....#",
        ".#........",
        "#.##.#....",
        "",
        "Tile 3499:",
        "..##..#...",
        "..#.#..#..",
        ".##....###",
        "##.......#",
        "#..#.#..##",
        "#....#...#",
        ".........#",
        "##.#.#.#..",
        ".........#",
        "#..###.##.",
        "",
        "Tile 1049:",
        "###....##.",
        "........##",
        "..........",
        "..........",
        "..........",
        "....#...#.",
        ".........#",
        "#.........",
        ".........#",
        "##.####.#.",
        "",
        "Tile 2467:",
        ".....##.##",
        "#.##......",
        "..###...##",
        "#..#.....#",
        "..#....###",
        "...#.....#",
        "...#....##",
        ".##...#...",
        "...##...##",
        "#.##...#.#",
        "",
        "Tile 3343:",
        "###...####",
        "..........",
        "#####.....",
        ".#..#..###",
        "#....#..#.",
        "#........#",
        ".....#...#",
        ".......#..",
        "#........#",
        "##.#.#.###",
        "",
        "Tile 2129:",
        "##.####..#",
        "#..#..#...",
        ".#.....#.#",
        "##.......#",
        "##.#...#..",
        "..#.#.#...",
        "#....#...#",
        "#.....##..",
        ".#.....#..",
        "..#######.",
        "",
        "Tile 1483:",
        ".##.#.##.#",
        "##.#......",
        "#.#....#..",
        "#.#...#..#",
        "#........#",
        ".......#.#",
        "##...#..##",
        "#......#.#",
        "..........",
        "###.###.##",
        "",
        "Tile 2393:",
        "#.###.##.#",
        "#......#..",
        "..###..##.",
        "##......##",
        ".......#..",
        "........#.",
        ".......#..",
        "#........#",
        "..##.....#",
        "#####..##.",
        "",
        "Tile 2749:",
        "..#.#.#.#.",
        ".#.#......",
        "#.#......#",
        "....#.....",
        ".##......#",
        ".###......",
        "##..#.....",
        ".#..##..##",
        "..###.#...",
        "####.##.#.",
        "",
        "Tile 1453:",
        "#.##.#.##.",
        "....#...##",
        "#.#.......",
        "...#......",
        "....###..#",
        "...#...#..",
        "#...#.....",
        ".#..#....#",
        "##.......#",
        "#..###.##.",
        "",
        "Tile 2213:",
        "#.#.##.##.",
        "#.......#.",
        "#......#..",
        "#....#..#.",
        "......#...",
        "#..#.#....",
        "....#...##",
        "..#...#...",
        "###.#....#",
        "#..#.##.#.",
        "",
        "Tile 2833:",
        ".#.....#.#",
        "..#...#..#",
        ".##.....#.",
        "..##....#.",
        ".....#.###",
        "........##",
        ".#........",
        ".##......#",
        "......##..",
        "#....##...",
        "",
        "Tile 2671:",
        ".##.#.#.#.",
        ".#.##.#...",
        "##...#...#",
        "###...#...",
        "..........",
        "........##",
        "..#......#",
        ".#.......#",
        ".#.#.#.#.#",
        "#...##.#..",
        "",
        "Tile 1801:",
        ".#..#.####",
        "#..##....#",
        ".....#....",
        ".#.......#",
        "#.#.##...#",
        "#.........",
        ".........#",
        "......#..#",
        "..........",
        ".#.....###",
        "",
        "Tile 3329:",
        "##.#..#...",
        "..#.#.#..#",
        ".........#",
        "..........",
        "......#...",
        "#......#..",
        "#.#...#...",
        ".....###..",
        ".....##...",
        "#..#...#.#",
        "",
        "Tile 2179:",
        "....#.#.##",
        ".......###",
        "..#....###",
        "#........#",
        "...##....#",
        "..###....#",
        ".##.#....#",
        "#..#......",
        ".#........",
        "#.#..###..",
        "",
        "Tile 2003:",
        "##.##.####",
        ".....#....",
        "#...#.#...",
        "#..#.....#",
        ".....#....",
        ".......#..",
        "#..#......",
        "..#.....#.",
        "#..#.....#",
        "....#####.",
        "",
        "Tile 2707:",
        "..#..#.##.",
        ".##.......",
        "##.#......",
        ".#.#..#...",
        ".###..#.#.",
        "#.........",
        "#...#....#",
        "#.##.##..#",
        "##.......#",
        "..#..#####",
        "",
        "Tile 3847:",
        "..##..#..#",
        ".........#",
        "..........",
        "#.......#.",
        "........#.",
        ".##......#",
        ".#.......#",
        ".....#....",
        "#..#......",
        "#..###...#",
        "",
        "Tile 1289:",
        "#...##.#.#",
        "#.....##..",
        "....#.#...",
        ".........#",
        "....#....#",
        "#..#.#....",
        "#....#.#.#",
        ".#...#.##.",
        "#.....###.",
        "#.##..####",
        "",
        "Tile 1867:",
        "####..##.#",
        "#.##.....#",
        "..#..##...",
        "#...#....#",
        ".###..#..#",
        "....##....",
        "..........",
        "..#......#",
        "##.......#",
        "###.##....",
        "",
        "Tile 1021:",
        "####.##.#.",
        "#...#.....",
        "##.......#",
        "#.#.#..#.#",
        "##........",
        "##....#...",
        "###...#...",
        "##.##.....",
        ".##..#.#..",
        "...#..##..",
        "",
        "Tile 1427:",
        "#..####...",
        ".....#....",
        "#.........",
        "##........",
        "#.#.......",
        "........#.",
        ".........#",
        "#.#......#",
        "..#......#",
        ".####.#.#.",
        "",
        "Tile 1307:",
        "#.#.#...##",
        "....#.....",
        "#....#....",
        "##....##..",
        ".#.#.....#",
        "#....#..##",
        ".....#.#..",
        "#........#",
        "#........#",
        "#..#####..",
        "",
        "Tile 2099:",
        "#..###.###",
        "..........",
        "#.#..#..##",
        "..#.......",
        ".........#",
        "#.......##",
        ".#.#.....#",
        "..#..#..##",
        ".##...##..",
        "##....#.#.",
        "",
        "Tile 2389:",
        "#.###.#.##",
        "##....#.##",
        "#...#..###",
        ".#.#...#.#",
        "#.....#...",
        "#......#..",
        "...#...#..",
        "#..#.##...",
        ".......#.#",
        "..#.#...##",
        "",
        "Tile 2011:",
        "#.##.#.#.#",
        "#........#",
        "#......###",
        ".#..#.#..#",
        "##..###..#",
        ".#..#.##.#",
        "#.#.#...#.",
        "#.....#...",
        "....#...##",
        ".###.#....",
        "",
        "Tile 2539:",
        "###.#.#.##",
        "......#.##",
        "....##.#..",
        "#.#...#..#",
        "#..#......",
        "##...#.#..",
        ".........#",
        "#.#....#.#",
        ".##......#",
        ".###..#.##",
        "",
        "Tile 3761:",
        "...#.#..#.",
        "#.....#..#",
        "#..#......",
        "#..#.....#",
        "##........",
        "#..#.#....",
        "#......#..",
        "..#.##.#..",
        ".#.#..#...",
        ".....#.##.",
        "",
        "Tile 2549:",
        ".##.####..",
        "#.#.......",
        "#.##....#.",
        ".#........",
        "......#...",
        "....#.##..",
        "..#.....#.",
        "...#..#..#",
        "....#..###",
        "##...##...",
        "",
        "Tile 2039:",
        "#.#.####.#",
        "#.#....##.",
        "..........",
        "#.#.......",
        "##.....#..",
        "##....#.##",
        "#...#.....",
        "#...#...##",
        ".#.#..##.#",
        ".###......",
        "",
        "Tile 2063:",
        ".##.##..#.",
        "..#.##....",
        "#..#.#....",
        ".#.......#",
        "##........",
        "#.#..#.#..",
        "..........",
        "...###....",
        "#......#.#",
        "#.##.....#",
        "",
        "Tile 3181:",
        "#####.####",
        "#.....#..#",
        "#.#......#",
        ".........#",
        "#....#....",
        ".#.......#",
        "......#..#",
        "#......#..",
        "#..#....##",
        "#.....#..#",
        "",
        "Tile 2459:",
        ".##..#.#..",
        "#...#..#.#",
        "#.......##",
        "#........#",
        ".#.#.....#",
        "#...#..##.",
        ".......###",
        "....#..###",
        "........#.",
        "...#.##..#",
        "",
        "Tile 2953:",
        "########.#",
        "..#.......",
        "#...##...#",
        "........#.",
        "#..#....#.",
        "...#.#....",
        "..........",
        "#...#....#",
        "........#.",
        "##...##...",
        "",
        "Tile 1487:",
        "#.####.#..",
        ".....#...#",
        "#....#...#",
        "..#......#",
        ".....#...#",
        "#...##...#",
        "........#.",
        ".#.....#..",
        "#...#.....",
        ".#.###.#..",
        "",
        "Tile 1201:",
        "..#...#..#",
        "#.........",
        "#.........",
        "...#.....#",
        "..#......#",
        "..........",
        "..........",
        "#.#.......",
        "#....#....",
        "#.###.##..",
        "",
        "Tile 1609:",
        "##.##.###.",
        "#........#",
        "..........",
        "#.........",
        "........#.",
        ".#.##..#.#",
        "#..#....##",
        "#.#....#..",
        ".#...#....",
        "##.#..#...",
        "",
        "Tile 2593:",
        "####.#.#..",
        "#.#......#",
        "..........",
        "#..##.....",
        "...#...#..",
        "#..#.....#",
        "..#.......",
        "#........#",
        "..#.......",
        "..#..#..##",
        "",
        "Tile 3889:",
        "######..#.",
        "....#.#.#.",
        "##...#..##",
        "....#....#",
        "#...#....#",
        "..##.##...",
        "#.###....#",
        ".#......##",
        "#....#....",
        "#....#...#",
        "",
        "Tile 3041:",
        "###.......",
        "##..#...#.",
        "##..#..#.#",
        "......##.#",
        "###....##.",
        "..#...#...",
        "#...#.....",
        "##...#....",
        "#......#.#",
        "#...#.#.##",
        "",
        "Tile 1367:",
        "###.##.##.",
        "..##.#...#",
        "....#.....",
        "#........#",
        "###....#..",
        "#.####....",
        "####....#.",
        ".###.#.##.",
        "...#.....#",
        "..#...####",
        "",
        "Tile 2609:",
        ".###...#.#",
        "..........",
        "#........#",
        "...#.#.#.#",
        "......#..#",
        "#.....##..",
        "#..#..#..#",
        ".####..##.",
        "##......##",
        "#...##..##",
        "",
        "Tile 3167:",
        "########.#",
        ".#..#....#",
        "..#.......",
        "..#..#.#..",
        ".#.##.#..#",
        ".##......#",
        "##.....#.#",
        "..#.#....#",
        "#........#",
        ".#...#.###",
        "",
        "Tile 2753:",
        "#...#....#",
        "#....#...#",
        "....#....#",
        "#........#",
        "..........",
        "..........",
        "#.........",
        "#........#",
        "#.........",
        ".#....####",
        "",
        "Tile 3527:",
        ".####...##",
        "........##",
        "#......#.#",
        "#...#..#..",
        ".#......#.",
        ".#.....#.#",
        "#........#",
        "#..#....#.",
        "..#....##.",
        "#.#..#####",
        "",
        "Tile 1279:",
        ".###...#.#",
        "..#......#",
        "#.......##",
        "..........",
        "#......#.#",
        ".#...#....",
        ".#........",
        "#....##..#",
        "#.##...#..",
        "####..#..#",
        "",
        "Tile 1291:",
        "#......###",
        "##.#.....#",
        "..........",
        "#.#..##...",
        "......##.#",
        ".#.....#..",
        "..........",
        "#.#......#",
        "#.........",
        "#..#...###",
        "",
        "Tile 1319:",
        "##..##.###",
        "#.......#.",
        "..........",
        "###......#",
        "..........",
        "...##..#..",
        "#.#..#....",
        "#...#.....",
        ".#...#...#",
        "#..#.#..##",
        "",
        "Tile 1009:",
        "....##...#",
        "#.....##.#",
        "#....##...",
        ".#.....#..",
        "#...#.....",
        "#.........",
        "##........",
        "#....#..##",
        "...#...#.#",
        "#######...",
        "",
        "Tile 2659:",
        "....####.#",
        "#.##......",
        "..#.....##",
        ".#.......#",
        ".#........",
        "..##.....#",
        "...#......",
        "......#..#",
        "#.#..#...#",
        "#..#.##..#",
        "",
        "Tile 1543:",
        "#######.#.",
        "#..#.#...#",
        ".........#",
        "##.#.....#",
        "#.#......#",
        "#.......##",
        "..##......",
        "..#.###..#",
        "...#.....#",
        "##.###..#.",
        "",
        "Tile 1019:",
        "........#.",
        "#....#.#.#",
        "...#......",
        "##.#.#..##",
        "....#.#.#.",
        "#.###...##",
        "####.#..#.",
        "#....#....",
        "#........#",
        ".#...##.#.",
        "",
        "Tile 2239:",
        "##..###...",
        "......#..#",
        "##...#....",
        "..........",
        "....#...##",
        "...#...#.#",
        "..#.......",
        ".........#",
        "#..###...#",
        "##...#...#",
        "",
        "Tile 2797:",
        "#.#..#....",
        "#.......#.",
        "#.#..#....",
        "#......#..",
        "#....##..#",
        "#...#...##",
        "#.#.......",
        ".#.....#.#",
        "##.......#",
        "#...#.#.##",
        "",
        "Tile 1663:",
        "..##.#...#",
        "#....##..#",
        "....#....#",
        "..#......#",
        "#........#",
        "..........",
        "#..#.....#",
        "#...#....#",
        "#..#....#.",
        ".#..#.#..#",
        "",
        "Tile 1069:",
        "#########.",
        ".#........",
        "###.......",
        "...#....##",
        "#....#....",
        "#........#",
        "#...#....#",
        ".....#....",
        "#.#......#",
        ".#....##.#",
        "",
        "Tile 1559:",
        "##.###.#..",
        "##...#....",
        "..#......#",
        "...#.#..#.",
        "...##.#...",
        ".#..#.#.##",
        "#..#....#.",
        "....#.#..#",
        "..#.......",
        "####.#....",
        "",
        "Tile 3203:",
        "##.##.#...",
        "##........",
        "..#..#...#",
        "....#....#",
        "..##.....#",
        "#.....##..",
        "..........",
        "..........",
        "#......#..",
        "...#.#.##.",
        "",
        "Tile 2819:",
        ".####...##",
        "...#.##...",
        "..#....##.",
        "........#.",
        "...#...###",
        "..#....#..",
        "#.#.....##",
        "#.#..#.#..",
        "#...#.....",
        "###.......",
        "",
        "Tile 2027:",
        "..#..##.##",
        "#....#.#..",
        "......#...",
        "#.#....#..",
        ".........#",
        "........##",
        "#.#.......",
        "......#...",
        ".......#.#",
        ".#.#.##.#.",
        "",
        "Tile 1091:",
        "..#..###.#",
        "...#.....#",
        ".........#",
        "#........#",
        "..#.#..#..",
        "#....#...#",
        "....#.#..#",
        ".......###",
        "#.........",
        "#..##.##.#",
        "",
        "Tile 2381:",
        "...###...#",
        "#.#.#...#.",
        "....#....#",
        "#.....##..",
        "#........#",
        "..##....##",
        "#.#......#",
        "##.......#",
        ".....#.###",
        ".#.#######",
        "",
        "Tile 1381:",
        "######.##.",
        "#.#....#..",
        "#....###.#",
        "#.#.....#.",
        "#.#.#....#",
        "#....#...#",
        "#.#.#.#...",
        ".#.#.#....",
        "....#...#.",
        "####.#####",
        "",
        "Tile 3719:",
        "...#.#...#",
        "...#.....#",
        "...#..#.##",
        "#..#.....#",
        "#.......##",
        "##..#..#.#",
        ".......#..",
        ".##.......",
        ".......##.",
        ".#.#.###.#",
        "",
        "Tile 3593:",
        "##...##.#.",
        "#.........",
        "......#.#.",
        "..........",
        "..#......#",
        "#........#",
        ".....##..#",
        ".#....#.##",
        ".#.....###",
        "#.#..#...#",
        "",
        "Tile 2971:",
        "..##.....#",
        "#.....#.##",
        "#.#......#",
        "#....#....",
        ".........#",
        "#.........",
        "#......#..",
        ".......#.#",
        "##....#..#",
        "#.##.#..##",
        "",
        "Tile 1637:",
        "#.###.#..#",
        ".#..#.#..#",
        ".........#",
        "......#.##",
        "#..###...#",
        "....#.#...",
        "#.#.....##",
        "#........#",
        "......#..#",
        "##..#.....",
        "",
        "Tile 2711:",
        "..##.##...",
        "###..#...#",
        "......#...",
        "#.......#.",
        "#.#.......",
        "#.....##..",
        "...#.##..#",
        "..........",
        ".....#...#",
        "##.....###",
        "",
        "Tile 3881:",
        "..#####.##",
        ".#....#..#",
        ".........#",
        ".##..#....",
        "..###.#..#",
        "...#......",
        "...#...#.#",
        "..#.......",
        "#.....#...",
        ".#.##.#...",
        "",
        "Tile 1907:",
        "..####..##",
        ".......#..",
        ".......#..",
        ".........#",
        "....#.##.#",
        "..#..##...",
        "....#....#",
        "#...#....#",
        "#........#",
        ".#.##..#.#",
        ""
    }};
    auto [product, grid] = find_corners_product<12>(input);
    //auto [product, grid] = find_corners_product<3>(test_input);
    std::printf("product   = %lu\n", product);
    auto sum = find_nessies(grid);
    std::printf("roughness = %lu\n", sum);
    return 0;
}
