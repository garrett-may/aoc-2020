#include <cstdio>
#include <limits>

constexpr auto find_encryption_key(std::size_t card_public_key, std::size_t door_public_key) noexcept{
    std::size_t value = 1;
    std::size_t encryption_key = 1;
    for(std::size_t i = 0; i < std::numeric_limits<std::size_t>::max(); ++i){
        value *= 7;
        value %= 20201227;
        encryption_key *= door_public_key;
        encryption_key %= 20201227;
        if(value == card_public_key){
            return encryption_key;
        }
    }
    return std::size_t{0};
}

constexpr static auto test_card_public_key = 5764801;
constexpr static auto test_door_public_key = 17807724;

constexpr static auto card_public_key = 11239946;
constexpr static auto door_public_key = 10464955;

constexpr static auto encryption_key = find_encryption_key(card_public_key, door_public_key);

int main(){
    std::printf("encryption_key = %lu\n", encryption_key);
    return 0;
}
