#include <cstdio>
#include <cstdint>
#include <experimental/array>
#include <unordered_map>
#include <algorithm>
#include <limits>

struct Tile{
    std::ptrdiff_t q;
    std::ptrdiff_t r;
};

constexpr auto parse_tile(char const* str) noexcept{
    auto tile = Tile{};
    for(auto* line = str; *line != '\0'; ++line){
        switch(*line){
          case 's': {
                ++line;
                switch(*line){
                    case 'w': { tile.q -= 1; } // fallthrough
                    default: { tile.r += 1; break; }
                }
                break;
          }
          case 'n': {
                ++line;
                switch(*line){
                    case 'e': { tile.q += 1; } // fallthrough
                    default: { tile.r -= 1; break; }
                }
                break;
          }
          default: { tile.q += (*line == 'e' ? 1 : -1); break; }
        }
    }
    return tile;
}

template<typename Array>
constexpr auto flipping_process(Array const& array) noexcept{
    auto floor = std::unordered_map<std::ptrdiff_t, std::unordered_map<std::ptrdiff_t, bool>>{};
    for(auto const* str : array){
        auto tile = parse_tile(str);
        auto result = floor[tile.q][tile.r];
        floor[tile.q][tile.r] = !result;
    }
    return floor;
}

template<typename Floor>
constexpr auto count(Floor& floor) noexcept{
    std::size_t sum = 0;
    for(auto const& [q, row] : floor){
        for(auto const& [r, is_black] : row){
            sum += std::size_t(is_black);
        }
    }
  return sum;
}

template<typename F, typename Floor>
constexpr void adjacent(F&& f, Floor& floor, std::ptrdiff_t q, std::ptrdiff_t r) noexcept{
    f(q + 1, r + 0);
    f(q + 0, r + 1);
    f(q - 1, r + 1);
    f(q - 1, r + 0);
    f(q + 0, r - 1);
    f(q + 1, r - 1);
}

template<typename Floor>
constexpr auto colour(Floor& floor, std::ptrdiff_t q, std::ptrdiff_t r) noexcept{
    std::size_t sum = 0;
    adjacent([&](auto q0, auto r0){
        sum += std::size_t(floor[q0][r0]);
    }, floor, q, r);
    return (floor[q][r]
        ? (sum == 1 || sum == 2)
        : (sum == 2)
    );
}

template<typename Floor>
constexpr auto part_of(Floor const& floor, std::ptrdiff_t q, std::ptrdiff_t r) noexcept{
    auto q_it = floor.find(q);
    if(q_it == floor.cend()){
        return false;
    }
    auto const& row = q_it->second;
    auto r_it = row.find(r);
    return r_it != row.cend();
}

template<typename Floor>
constexpr auto flipping_rules(Floor& floor, std::ptrdiff_t min_q, std::ptrdiff_t min_r, std::ptrdiff_t max_q, std::ptrdiff_t max_r) noexcept{
    auto new_floor = Floor{};
    for(auto q = min_q - 1; q <= max_q + 1; ++q){
        for(auto r = min_r - 1; r <= max_r + 1; ++r){
            new_floor[q][r] = colour(floor, q, r);
            adjacent([&](auto q0, auto r0){
                if(!part_of(new_floor, q0, r0)){
                    new_floor[q0][r0] = colour(floor, q0, r0);
                }
            }, floor, q, r);
        }
    }
    return new_floor;
}

template<std::size_t n, typename Floor>
constexpr auto simulate_n_days(Floor& floor) noexcept{
    auto min_q = std::numeric_limits<std::ptrdiff_t>::max();
    auto min_r = std::numeric_limits<std::ptrdiff_t>::max();
    auto max_q = std::numeric_limits<std::ptrdiff_t>::min();
    auto max_r = std::numeric_limits<std::ptrdiff_t>::min();
    for(auto const& [q, row] : floor){
        for(auto const& [r, is_black] : row){
            min_q = std::min(min_q, q);
            min_r = std::min(min_r, r);
            max_q = std::max(max_q, q);
            max_r = std::max(max_r, r);
        }
    }
    for(std::size_t i = 0; i < n; ++i){
        floor = flipping_rules(floor, min_q, min_r, max_q, max_r);
        min_q -= 1;
        min_r -= 1;
        max_q += 1;
        max_r += 1;
    }
    return floor;
}

constexpr static auto test_input = std::experimental::make_array(
    "sesenwnenenewseeswwswswwnenewsewsw",
    "neeenesenwnwwswnenewnwwsewnenwseswesw",
    "seswneswswsenwwnwse",
    "nwnwneseeswswnenewneswwnewseswneseene",
    "swweswneswnenwsewnwneneseenw",
    "eesenwseswswnenwswnwnwsewwnwsene",
    "sewnenenenesenwsewnenwwwse",
    "wenwwweseeeweswwwnwwe",
    "wsweesenenewnwwnwsenewsenwwsesesenwne",
    "neeswseenwwswnwswswnw",
    "nenwswwsewswnenenewsenwsenwnesesenew",
    "enewnwewneswsewnwswenweswnenwsenwsw",
    "sweneswneswneneenwnewenewwneswswnese",
    "swwesenesewenwneswnwwneseswwne",
    "enesenwswwswneneswsenwnewswseenwsese",
    "wnwnesenesenenwwnenwsewesewsesesew",
    "nenewswnwewswnenesenwnesewesw",
    "eneswnwswnwsenenwnwnwwseeswneewsenese",
    "neswnwewnwnwseenwseesewsenwsweewe",
    "wseweeenwnesenwwwswnew"
);

constexpr static auto input = std::experimental::make_array(
    "wenwwsenwwwwnwwnwwwnwsewseewe",
    "nenenwnenenenwnwnenenwneneseseswnenwwne",
    "newewwwwwwswwwww",
    "esenwwwswwnwnwswnwnwnwewnwsenwswwnwe",
    "esesewseeneswnenwneeewnwneswsenwnee",
    "neseswwswwseseseseseseswnwesesesesesese",
    "sewwswsweswwwswswnwenwswswenesww",
    "wenenesenenewnenenenwenweneneenewse",
    "swswsenwneeseswnwswseseswswswneswsesesesw",
    "wswswswsweswseswnwswswnwswswnwseswswswsee",
    "nesweeeneeenwneswneswewneneenw",
    "swseswswswswswswswswswnwswswswsw",
    "eneneenwneeeeeeneeeese",
    "newenwsenewnesenesenwnenenesenwnwnwne",
    "nwswnenwswneneswenenwneswseeeneneneneee",
    "nenenenwsesenwnwwnwnwsesenwseneswnenenw",
    "eeeseseseeeeeewnwsweseneesesene",
    "wesenwswwwwnewwnwenwnwswwswnww",
    "esenweeeeneeswewenwneweseeee",
    "swswnenenenenenenwneneswneswneneneenwe",
    "neswseenwesenwneswnesewneswnewenwne",
    "nwweswseseswewneeenweeseneeesese",
    "wnwnwwnwnwswswnwswwneeewnwnwsenwnwnw",
    "nenesenewnwneeenene",
    "eseeeewneeeseswnwnwwwseswnewese",
    "neswnenwnwneswenwenwswsewnwneenwwnw",
    "sewswnwswsweswswwwweswwswneswwsw",
    "seseseneseseswseswsese",
    "eswswwnwewwnwswnwswneneseswe",
    "nwswnwswsenwsesweseseseseswseseswswswsww",
    "neneenwswnwwesesenese",
    "wswswswwwwnwswswswweseswneswswwsw",
    "eesewswsweseswsesweseswswswwswnwsww",
    "neswnenwswnesenwnwswwenwswnenwenwnwnwne",
    "swswneseneswwneswswswswwwwswnwnesesee",
    "nwnwnenwneneesenwnwwseswwnwnenwnwnwnenw",
    "esewseseseeswswsweesewnwwsesenwsew",
    "nwsewswnwnwnwenwnwnwwnenwswnwnwnwnwnw",
    "senewswnwnwwnwnwnweneseseneneswnwneesw",
    "swseswnwseswnwseswneswseswswswse",
    "nesenwnwnwnesewwseenenwswnesesenenewe",
    "eseseweeseseneseeseeseeewnwse",
    "wsewwneswneneswwswsewswwenwsewsww",
    "ewswwwwwnwwsewnewwwwwwwe",
    "swwweewwwwwewwewwwwswww",
    "nwnenwwnenwnenenwnenwnwnwneswnenwnesenese",
    "eseseseeseweseeeeenweseswseewse",
    "nwswseswswswnesweswnwnwseswsw",
    "seseseseseseseseneseseswesesese",
    "swneswwnesenwwweswsewswsw",
    "nwneneneenwnenenewnwnenene",
    "swnwenwswwswwwswnwsweswswwswwsee",
    "nwnwnwwnwnwnwnwswnwnwnwesenwnwsenwsenwne",
    "nwneneesenweswswswwewswsewwswswse",
    "senwneeneesweneeeewnweeswsenenew",
    "newnwwnewswwwswneswewewsesenww",
    "swnwnwnwnwnewswnwnesenweneweseeswnw",
    "eswwwnwseswwwnewenewwwswwww",
    "nwnwnwwnwneneneswsewneesenwneswnwewenw",
    "neswswswwswswwswswswwseswwswwenwsw",
    "wwwwwwsewnwwnwsenewwnw",
    "nwneswswnweswnweswswswwsewneseswsesesw",
    "wnwsweneswsenwnenwseswwnenewnwneseswse",
    "newewneswswswnwwswwswseswswwswsesw",
    "neneneswnenwnenwneeseseewsweneswnwnw",
    "nwnewneneenwseeswesenewneneneswenwnw",
    "nenwnenwnenenwnenenwwnwsenwsenwnesewnee",
    "nwnenenewenenenwnenenenenesenwswnenwnese",
    "nesewswwwwsenwnenwwnwwsenewwww",
    "enwnwnwnwnwwnwnwnwnw",
    "swseseneseneswenewsewwsewnesesesenwswse",
    "sweneneeneeswneeeeeneenee",
    "sesenenwsesenwseseseseseesenwswswswsene",
    "seswseewswsesweswseswewswnw",
    "swneenwneeeeeenewneeswneneneene",
    "eeeweneneeeneeeneswnesenewwnenw",
    "swnwseenwnenwnwnwnwnenenwnwwnenwnwnwnw",
    "wnwnwneswenwnwenwnwswnwwnwnwnwnwswnw",
    "swswswswnwseseeneseseseseseseswnwseswse",
    "neeweneswesweeeeseenenweeseeenw",
    "wnwnwnwneenwswnwnwnwenenweswnwwnwnwnw",
    "senwwswnwsenewewenenwewsenwwseesw",
    "ewswwswnwwwwnewwwenw",
    "enenwnenwnenwnwsenewnwneeswnenenenenwnw",
    "seseeseeseswsewnwwseseseseeseeeneese",
    "nwneseswnwsenesewnwnewsesweswsw",
    "nwswsweeswswnwswwenwwswnweswswnwswse",
    "neewseeseseeseeseseseenwee",
    "senwnwwwnwewnwwnwnwnwnwnwnwnw",
    "eeewneenwesenwseseweeeeseee",
    "swnesenwsewseseswnwswwnwnesw",
    "wnwwwnwnwnwnwswnwnweenwnwnwwwenwswnw",
    "nwnwsenwnwnesenwnwnwnwnenwnwnenwswwnwnw",
    "nwewswswwwsewwewnwnenwenwnwnwnwwnw",
    "wnesweneseswnwnesesewseswseswwswse",
    "enesewneenenweseswnesenwesewwneene",
    "eneenwnenenenewswsese",
    "wsewnewswnwseneswswwswnwwweswsesw",
    "eeswneeseeeeeeneeeenwnwwenee",
    "eswsenweeeseeseewnwneeseeseeenw",
    "sesewseswewseeseneeseseneesenwsesese",
    "wswnewnwnwswnenwnwswenwnwsenewswenw",
    "newnesenwnwwnwnwwsenenwnwnwsenenwnwnwe",
    "esweweswsenwnenwnweneeeseeeee",
    "nenwseneneneneneeneseenenenwnenene",
    "esesenwnwswesewneseseesenwseseseese",
    "swswwsweswswnesweswneswwsewswswse",
    "enesweeeenewneswswenenenenwnenene",
    "enwswswnwsewnenwnweneeeeeeeese",
    "enwnweneswswnenenwnesweswnwse",
    "seswseseewneweenweesenewseesesw",
    "swswnenwnwwswswseenenwwneneeeneneneswne",
    "sewwnwnwwwwsewwnwnwwnwewewnww",
    "neseseseswneseeswseseseseseswswwswswse",
    "seseswswswseswwseswsesesesesesene",
    "swenweeneeenwneeeeeese",
    "eneseswswswswewwseseswswnwwswnwneswnenw",
    "wswseeeeneswneneenenenwenweeene",
    "wwwwwnewwwnwsewnwww",
    "nwnwswnwnwnwnwwswsenwnwnenwwewnenw",
    "nwswwswnwwnwnwwewwneenwnwnwnwwwwse",
    "nwnenewseswnwnwsenenenenwnenenenenenee",
    "nwswsenwwsewsenenwnenwnwse",
    "nenwnwnenesenwnwnwnwnenww",
    "ewswnwswswswweswwwseswwswwnenwsw",
    "neseswswneswseswswneswswswswwsweswsenwswsw",
    "newnenwneseeneneswnenenenwnenwenwswnesw",
    "swswswseseseseswswswseswne",
    "nwswswswwwwneseeswsewwswswswswseswnew",
    "swnwsewwwsewnwww",
    "swswseseeswswnenwwswswswswneswwseswswswnw",
    "enenwswneeeneneneneeweneneeseenenw",
    "neswneswwnwenwnwnwswswneeseneewenw",
    "neeenewswneeseeeeneeeeneeenesw",
    "swsesewsesweesewnwnenwnwseswse",
    "swswneswsewnewwwneneesww",
    "nenwnesenesesenwnwnenwnenewnenwnewnenwne",
    "wwsenwsenwenwnwnwwwnwswenwsewnew",
    "neseseenwswswwenwneeeneswenwswweswnw",
    "swswneneneseneseswnwnenwnwneenenwneewne",
    "wnwswwwwweewwswwwsw",
    "wneeneswswseswnwswswswsewswswnwswnesesw",
    "nwenwwnwnwsewnwnwnesesenwnwswnwwneww",
    "enenwseeesewweeswnewsesweesee",
    "wwnewwwwwswww",
    "swneseswswwneneeneswsewnenenwswewnese",
    "nwnewnwnwsenwnwnwwwseeseseewswewnw",
    "eeeweeeesenenene",
    "nwnwwnwenwswnwnwenwnwnwnwsenenwswnwnwnw",
    "wnwneewseenwnwnwwnwwweeswwswnw",
    "sesesesenwsesesesesenwnwse",
    "esweenewsweeneneenenewneeeee",
    "swnweeeneeewsweeneeeseeeeseee",
    "newsweswswnwwewnwwswneswsw",
    "senwnwwnenenenenwnene",
    "nenwneneneenenewseeeneneenewnewne",
    "neswnwswsewwnwenenwwwwnwwsewsww",
    "swnwsesesesesenweeeseseswwseneseeesese",
    "nwnwnwnwnwwnwwnwnwwwwnwe",
    "swswwneswswneswswswswswswseseswnenwswe",
    "enenenenwseseseenwnenenwswnenwnenesenene",
    "eneseeneeenwnwneseenwsewnwesenee",
    "sewswswwswwswneewnwswwnwwswnwswsesenw",
    "seseseseseseseseseneswsewe",
    "swwswswswewswswswswswsw",
    "eeeeneenweswseswee",
    "nwsesesewseswseneeseseseseswsesenesesenw",
    "swneneswswswsenenwenwswswseswneeseswswnw",
    "nwwnwnewwswswwwwneseswwswnwswwsese",
    "esenwneenweeneswswweneeeswseee",
    "eeeeeeeeeneswesewseeneewsee",
    "neswwwswwwnweswswwewwnw",
    "nenenwnenenwseneneswnwewnwwneswnw",
    "swseswwswswweneswwswnwwwsewenwne",
    "swseesewswseseseeswsenw",
    "nesewswewnewwnwwsenwnwwswwswsenw",
    "eseneeseesenwswsesesewseeseseseneswe",
    "weseswwnwseseseseesesenwenesesenwsese",
    "ewsesenwneseneswswewswswwnwswnesesw",
    "swswswswseswswswsenesesewsenw",
    "nwnwnenesenenenwnwnwsenwnwnwnenwnwwsesw",
    "enewswnenwnwnenwnenenweewenenwsw",
    "neeswnenenwnenenwneenenwswnenenenenwne",
    "nenenwneswnenwnwnwswenwnwnenenwnwnwswswe",
    "wnwwwnenwnwwnenwwwswwwwnwwsewse",
    "neeeneneneneesenenenenenwnenewwnese",
    "nwenwesenwwnwnwnwnwnwnwnwswnwnwsenenenw",
    "ewwwswwweswewswswswwwwewnww",
    "swwsenwwwsenwwwswnwesenwwswenesww",
    "eweswswswnwnwswwnwswneeseeseswwsww",
    "nwnwnwenenwnwnwnwnwewnwnwnww",
    "wneenwenewesesesee",
    "nwswswswwsweseswnwswswwswswneenwswswe",
    "eseseseseswseseswsenwseswneseseswnw",
    "swseswswseswsweweswswswnwswswseswswswnw",
    "swewswswswseseseseswswswswsesenwseeswnw",
    "nenwnwnenwneneseswnwneswseneneswsw",
    "swnweseswnwneseseenewsese",
    "eweneswsenesewsesenweswseeenwew",
    "wweswsenwnwswwswneneneswwswsewsesene",
    "seeeseeseseseweseseseeswnwe",
    "nwnwnwnesenwenwnwsenwwnwnwnwnwwnwwnw",
    "sesewseeeeseseneeseseweesesesenwe",
    "wnewswswnewswswswswswnwswsweswswww",
    "neweeeeeeneneenweesesesw",
    "enesenewsenwnewwwwnwwnwsenwwwswse",
    "senwseesweseeenweenesweneseeesee",
    "sesweseswseswwneswwswswsweswsenwswswsw",
    "swneneeneswneneneneeenwewneseeew",
    "sweswwnewswsweswswnwwwswswswwwwsew",
    "nwseseseswneesesewseseseseseseesesese",
    "seseswswnwnwwseeseswneseesesesesesewne",
    "neseseswseseseswseneswseswseswwnwswsesesw",
    "nwswwwwwnwwnwweswnwwwsewwnwnewe",
    "eeeneeenewswnene",
    "seseseseswnwesenwseseseesweesesesee",
    "nwswwnewneseswwseneneseswnewwwswwsesw",
    "eeenweeeeeesenenweewsweese",
    "enwswnwswneneswnwnwnwnwnwnenweneneseswnw",
    "senenwwneswseswwswneneneneswsewswnenene",
    "nwnwwsesenewnwswsesewsesewseseeenee",
    "swneseseeeweeesenesewewnwseseesesw",
    "neeenwweeenesewnwwewseesesese",
    "neneswswnenenenenenenenenenenenewneese",
    "swenenenwnwnwseesesenwswsenwsewesweww",
    "wenewswwwnwesewswwwenwnenww",
    "neneneseeneneneneneswneenenwnewwnenene",
    "sewwnwsenenwwwsewwwwne",
    "sewsesenwseseseeeeeseewwesenwnee",
    "seswnwnenwneseeseswnenewwesewnenenw",
    "seeseswseseenenwenwneenewsweenww",
    "wnwnwwwnwswwwwwsewnenw",
    "seswswneswswswwswswneneswswswsw",
    "swnwsewnwsewnwwnwnenwwnwnwnw",
    "nwsewswswnwwswswswsweeswesesesenesw",
    "swswnwseseswswswnwseswseswseseweswsenesese",
    "wnwnwwnwswswneeweswseewwnweswswse",
    "nwwwewnwwnenwnwsewnwwnewnwnwsww",
    "seeswseswseswseswsesesenewswswsenww",
    "swnenwswwneneenenesenwneeeneeenee",
    "weseswsesesewsesesenwseneseseseseese",
    "swneeneesewwseswswenwneeswseenwnw",
    "wnwwnwwwwwnwwwwnwe",
    "newwenwseswnewnwsesweneneesenwsew",
    "seenewneneswnwnenewnenwneneeewsenwsw",
    "nwewnwsenwnwwnwnew",
    "neneneneswnenenwnenenenenenene",
    "newsenewneeewnwwwwswseswwwnee",
    "wswneswewnwswnwwswesewwneswswswww",
    "wseeeneseneesesweeeswnweeseneee",
    "seeswswswwsewswswseeswnesewswsweswse",
    "sesenwnwnwwwswwnwnwswenwewsenwwnew",
    "weseneeeeeeeeee",
    "swwswseeswswsesenwnwseswswseesenwesw",
    "eneneweeneeneeeneene",
    "newwswwnewwwswnewsewwwswwwwse",
    "wsenwnwnwnwenwenwnewsw",
    "wwnwnwwwnwwewwnwnwneenwnweswww",
    "eseeeeeneneeesewswenweewnenene",
    "nenwneneswneneswnenwneneswnenwnenenwene",
    "sesesesesewnewnesenwseseswsesesesesesesese",
    "swneneswneseswneneneeswneeenwnewenwne",
    "nwsenwnwsenwwnwnwsewnwnwsenwnwnwnwnwnenw",
    "wnenwnwwswenwenwwnwwnwseswnwswnwwnw",
    "nwnwnwwsewswnewnwwesenesenwnwnwnwnw",
    "swswenesenwneenwwsenwnwnwnenwnwnwsew",
    "nenenwswnenenwnenenwsenwnesenwnewnwwnenene",
    "swwnewwswwwwwnewwww",
    "eeeswswneneeeneneeeeswnenee",
    "swseseswnesewseseswswseenwswseeswswse",
    "wewwewwswwswwwwwnwswenwnwesww",
    "nwnwwnwnenwsewswwwnewwnwwnwsenwww",
    "eeneneswneesenesweneneenenewneenenw",
    "enenwnwswnwnwseneswswnweeswnwnwsenwnwnwnw",
    "neneneneneswneneneneneneeswnenesenwnenwnw",
    "enweseeeseswseenweeswseeesesee",
    "nwswseseswneswsewseseswneseswseseseew",
    "swsenesewswswswnwwwnwwswswwswwwse",
    "sweswnwswwsewseseseswswseswnwneswsw",
    "neeweneesenenwnwswseeeeeenenewe",
    "wnwnwwwnwweenwswwwww",
    "neseswseeeseeewsenweneseese",
    "neswswswswswswseseswswswswnwsweswswnwswsww",
    "nwwnwewwwwwswwwwswnwnwwnwew",
    "nwnwnenwnwnwnwenwneenwnenwnwnwsewnenwsw",
    "nwnenwnenwnwwnwsenwnwwnwwwwnwsesenwnw",
    "swswswseswsesewsewnenwseswswswseswswsenesw",
    "seweweswswseswwnwseswseseenwenew",
    "eenweeeeeeseesenw",
    "wsewwwwsewnewwwesenwneseswneenew",
    "seweseeeeeeeeeee",
    "swsenwswneswswswsesesesewseseneswseswsew",
    "swswneneneneenenenenenweneneneneneswnwene",
    "seeseeneneswwswsenwenwseseneseswswnw",
    "nenesesewnwnenwnwsenwnenenwnenenwnwnwwse",
    "wswnwwneswnweseseswwe",
    "wswsenwswswswweswnwswswwswswsweswnwe",
    "swsesesenwsesenewsesenwseseswsesesenesese",
    "nwnwwnweswnwsenwnwnwsewswnenwnwneese",
    "swswseswwnwsesewesewneswneneseswnwnw",
    "swsweseswsesenwwsenwseseswswseenwsesw",
    "eeneeswenweeneesweeneeenwene",
    "nwswwwwwneswswswseeneswwswseswswnwsw",
    "nwswneseneswswneswneseseswswnesenw",
    "nwnwwneneswneesweenenenewnwnenenwnw",
    "nwwnwenwnwenwwnwnwnwnwnwnwwww",
    "eeesweeeweeneeeenweeweseee",
    "nwenwnwnwnwnwenwnwwnwewnwnwwnwswnwnw",
    "nwneneenewenewseneswneneneneenwwneswne",
    "wwnwwnesesewwwwnewwsewwnwww",
    "swswswnwswseswswwswsweswneswseswswswe",
    "sewwneseseswneseseseneswseswseseew",
    "nwneneenenesweneneneenenenenweesw",
    "sesesenwneewseseeenwnwneenwwswnwswsw",
    "weseneseeneneneweeswewnesesewsw"
);

int main(){
    auto floor = flipping_process(input);
    std::printf("day 0   = %lu\n", count(floor));
    floor = simulate_n_days<100>(floor);
    std::printf("day 100 = %lu\n", count(floor));
    return 0;
}
