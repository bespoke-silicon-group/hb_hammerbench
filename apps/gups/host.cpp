#include <standard/host/program.hpp>
#include <algorithm>
#include <random>

#define SEEDS_PER_THREAD (UPDATES_PER_THREAD > 512 ? 512 : UPDATES_PER_THREAD)

using namespace bsg_global_pointer;

class program : public standard::program {
public:
    int init(int argc, char *argv[]) override {
        standard::program::init(argc, argv);        
        table_ptr = find<hb_mc_eva_t>("g_table");
        seeds_ptr = find<hb_mc_eva_t>("g_seeds");
        return 0;
    }

    int input() override {
        standard::program::input();
        hb_mc_pod_id_t pod_id;
        hb_mc_device_foreach_pod_id(&mc, pod_id) {
            auto pod = pod_id_to_coord(pod_id);
            table_ptr.set_pod_x(pod.x).set_pod_y(pod.y);
            seeds_ptr.set_pod_x(pod.x).set_pod_y(pod.y);

            hb_mc_eva_t eva;
            BSG_CUDA_CALL(hb_mc_device_pod_malloc
                          (&mc, pod_id, TABLE_SIZE * sizeof(uint32_t), &eva));
            *table_ptr = eva;
            BSG_CUDA_CALL(hb_mc_device_pod_malloc
                          (&mc, pod_id, seeds() * sizeof(uint32_t), &eva));
            *seeds_ptr = eva;
        }

        h_seeds.resize(mc.num_pods);
        hb_mc_device_foreach_pod_id(&mc, pod_id) {
            h_seeds[pod_id].resize(seeds());
            for (size_t i = 0; i < h_seeds[pod_id].size(); i++) {
                h_seeds[pod_id][i] = rand() % table_size();
            }
            jobs_in[pod_id].push_back({
                    *seeds_ptr
                    , h_seeds[pod_id].data()
                    , h_seeds[pod_id].size()*sizeof(uint32_t)
                });
        }
        return 0;
    }

    int seeds() const { return bsg_tiles_X*bsg_tiles_Y*SEEDS_PER_THREAD; }
    int table_size() const { return TABLE_SIZE; }
    pointer<hb_mc_eva_t> table_ptr;
    pointer<hb_mc_eva_t> seeds_ptr;
    std::vector<std::vector<int>> h_seeds;
};

standard::program *make_program()
{
    return new program();
}
