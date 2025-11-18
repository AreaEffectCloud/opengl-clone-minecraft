#include <iostream>
#include <cstdlib>
#include <chrono>
#include "world/world.hpp"

int main(int argc, char** argv) {
    uint32_t seed = 0;
    if (argc >= 2) {
        seed = static_cast<uint32_t>(std::strtoul(argv[1], nullptr, 10));
    } else {
        seed = static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());
    }

    ocm::World world;
    world.init(seed);

    // generate a few neighboring chunks optionally
    // For initial stage we generate only (0,0)
    world.dump_stats();

    // sample some blocks at the center column for verification
    int cx = ocm::CHUNK_SIZE_X / 2;
    int cz = ocm::CHUNK_SIZE_Z / 2;
    int world_x = cx;
    int world_z = cz;

    int h = world.sample_height(world_x, world_z);
    std::cout << "[Main] Sampled height at (" << world_x << ", " << world_z << ") = " << h << std::endl;

    // print topmost block ID at column center
    ocm::BlockID id = world.get_block(world_x, h, world_z);
    std::cout << "[Main] Top block ID at column = " << static_cast<int>(id) << std::endl;

    world.destroy();
    return 0;
}