#include <cstdio>
#include <cstdint>
#include <cmath>
#include <vector>

struct Element{
    std::size_t value = 0;
    Element* prev = nullptr;
    Element* next = nullptr;
};

template<typename Memory>
struct LinkedRing{
    Memory& memory;
    Element* head;
    Element* last;
};

constexpr auto get_digit(std::size_t value, std::size_t x) noexcept{
    std::size_t i = std::log10(value) + 1;
    for(std::size_t j = x + 1; j < i; ++j){
        value /= 10;
    }
    return value % 10;
}

/*
 * Given memory, setup a (double) linked ring within in
 * `prev` and `next` tell us the previous and next elements, respectively
 * `memory` maps values onto their indicies for O(1) lookup
 */
template<std::size_t ring_size, typename Memory>
inline auto parse_input(Memory& memory, std::size_t value) noexcept{
    auto ring = LinkedRing<Memory>{memory, nullptr, nullptr};
    std::size_t i = std::log10(value) + 1;
    Element* prev = nullptr;
    Element* next = nullptr;
    for(std::size_t j = 0; j < i; ++j){
        auto v = get_digit(value, j);
        memory[v] = Element{v, prev, next};
        prev = &(memory[v]);
    }
    ++i;
    for(; i <= ring_size; ++i){
        auto v = i;
        memory[v] = Element{v, prev, next};
        prev = &(memory[v]);
    }
    auto* elem = prev;
    while(elem->prev != nullptr){
        auto* p = elem->prev;
        p->next = elem;
        elem = p;
    }
    ring.head = elem;
    ring.last = prev;
    ring.head->prev = ring.last;
    ring.last->next = ring.head;
    return ring;
}

template<std::size_t ring_size>
constexpr auto wrap(std::size_t i) noexcept{
    return (i == 1 ? ring_size : i - 1);
}

template<std::size_t simulations, std::size_t ring_size, typename F>
constexpr decltype(auto) simulate(std::size_t value, F&& f) noexcept{
    auto memory = std::vector<Element>{};   // Initialise memory here so that we
    memory.reserve(ring_size + 1);          // don't have to handle it ourselves
    auto ring = parse_input<ring_size>(memory, value);
    auto* start = ring.head;
    for(std::size_t s = 0; s < simulations; ++s){
        // Find the correct v value first
        auto v = wrap<ring_size>(start->value);
        for(std::size_t x = 0; x < 3; ++x){
            auto match = false;
            auto* elem = start;
            for(std::size_t y = 0; y < 3; ++y){
                elem = elem->next;
                match |= (elem->value == v);
            }
            if(match){
                v = wrap<ring_size>(v);
            } else{
                break;
            }
        }
        // Move a sublist of elements to somewhere else in the list
        // O(1) as this is a (double) linked ring
        auto* head = start->next;
        auto* last = &(memory[v]); // Use of O(1) lookup
        auto* prev_of_head = head->prev;
        auto* next_of_last = last->next;
        auto* before_middle = start->next->next->next;
        auto* after_middle = before_middle->next;
        head->prev = last;
        last->next = head;
        prev_of_head->next = after_middle;
        after_middle->prev = prev_of_head;
        next_of_last->prev = before_middle;
        before_middle->next = next_of_last;

        // Go to the next iteration
        start = start->next;
    }
    // Return value based on function working on where 1 is
    return f(&(memory[1]));
}

constexpr auto simulate_100_moves(std::size_t value) noexcept{
    return simulate<100, 9>(value, [](auto* elem){
        std::size_t v = 0;
        for(std::size_t i = 0; i < 8; ++i){
            elem = elem->next;
            v = v * 10 + elem->value;
        }
        return v;
    });
}

constexpr auto product_of_two_stars(std::size_t value) noexcept{
    return simulate<10000000, 1000000>(value, [](auto* elem){
        return elem->next->value * elem->next->next->value;
    });
}

constexpr static std::size_t test_input = 389125467;
constexpr static std::size_t input = 583976241;

int main(){
    auto result = simulate_100_moves(input);
    auto product = product_of_two_stars(input);
    std::printf("result  = %lu\n", result);
    std::printf("product = %lu\n", product);
    return 0;
}
