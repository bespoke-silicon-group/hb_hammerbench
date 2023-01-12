#include <hb_intrinsics.h>
#include <infra_hb/host/arg_parser.hpp>
#include <bsg_manycore_regression.h>
#include <string.h>
#include <stdio.h>
#include <fstream>

using hammerblade::Device;
using hammerblade::Vector;
using hammerblade::GraphHB;
using hammerblade::GlobalScalar;

#define X 16
#define Y 8

GraphHB edges;
Vector<int> old_rank_dev;
Vector<int> new_rank_dev;
Vector<int32_t> out_degree_dev;
Vector<int> contrib_dev;
Vector<int> contrib_new_dev;
Vector<int> error_dev;

GlobalScalar<int> damp_dev;
GlobalScalar<int> beta_score_dev;

Graph edges_cpu;
//int* __restrict new_rank_hb;
int* __restrict contrib_hb;
//int  * __restrict inneighbor_hb;
int  * __restrict c2sr_index_hb;
int* __restrict old_rank_hb;
int  * __restrict out_degree_hb;
int* __restrict old_rank_cpu;
int* __restrict new_rank_cpu;
int  * __restrict out_degree_cpu;
int* __restrict contrib_cpu;
int* __restrict error_cpu;
int  * __restrict generated_tmp_vector_cpu;
int damp_cpu;
int beta_score_cpu;

template <typename APPLY_FUNC > void edgeset_apply_pull_parallel(Graph & g , APPLY_FUNC apply_func)
{
        int64_t numVertices = g.num_nodes(), numEdges = g.num_edges();
        parallel_for ( NodeID d=0; d < g.num_nodes(); d++) {
                for(NodeID s : g.in_neigh(d)){
                        //      std::cout << "in_neigh on cpu: " << s << std::endl;
                        apply_func ( s , d );
                } //end of loop on in neighbors
        } //end of outer for loop
} //end of edgeset apply function
struct error_generated_vector_op_apply_func_5
{
        void operator() (NodeID v)
        {
                error_cpu[v] = ((int) 0) ;
        };
};
struct contrib_generated_vector_op_apply_func_4
{
        void operator() (NodeID v)
        {
                contrib_cpu[v] = ((int) 0) ;
        };
};
struct generated_vector_op_apply_func_3
{
        void operator() (NodeID v)
        {
                out_degree_cpu[v] = generated_tmp_vector_cpu[v];
        };
};
struct new_rank_generated_vector_op_apply_func_1
{
        void operator() (NodeID v)
        {
                new_rank_cpu[v] = ((int) 0) ;
        };
};
struct old_rank_generated_vector_op_apply_func_0
{
        void operator() (NodeID v)
        {
                old_rank_cpu[v] = (((int) 1)  / builtin_getVertices(edges_cpu) );
        };
};
struct computeContrib
{
        void operator() (NodeID v)
        {
                contrib_cpu[v] = (old_rank_cpu[v] / out_degree_cpu[v]);
        };
};
struct updateEdge
{
        void operator() (NodeID src, NodeID dst)
        {
                //    std::cout << new_rank_cpu[dst] << ", " << contrib_cpu[src] << std::endl;
                new_rank_cpu[dst] += contrib_cpu[src];
        };
};

struct updateVertex
{
        void operator() (NodeID v)
        {
                double old_score = old_rank_cpu[v];
                new_rank_cpu[v] = (beta_score_cpu + (damp_cpu * new_rank_cpu[v]));
                error_cpu[v] = fabs((new_rank_cpu[v] - old_rank_cpu[v])) ;
                old_rank_cpu[v] = new_rank_cpu[v];
                new_rank_cpu[v] = ((int) 0) ;
        };
};
struct printRank
{
        void operator() (NodeID v)
        {
                std::cout << old_rank_cpu[v]<< std::endl;
        };
};
struct reset
{
        void operator() (NodeID v)
        {
                old_rank_cpu[v] = (((int) 1)  / builtin_getVertices(edges_cpu) );
                new_rank_cpu[v] = ((int) 0) ;
        };
};

int launch(int argc, char * argv[]){

        // Load graph from file
        edges_cpu = builtin_loadEdgesFromFile ( argv_safe((3) , argv, argc)) ;
        // inneighbor_hb = new int [ edges_cpu.num_edges() ];
        c2sr_index_hb = new int [ 2 * builtin_getVertices(edges_cpu) + 1 ];
        contrib_hb = new int [ builtin_getVertices(edges_cpu) ];
        // new_rank_hb = new int [ builtin_getVertices(edges_cpu) ];
        old_rank_hb = new int [ builtin_getVertices(edges_cpu) ];
        out_degree_hb = new int [ builtin_getVertices(edges_cpu) ];
        old_rank_cpu = new int [ builtin_getVertices(edges_cpu) ];
        new_rank_cpu = new int [ builtin_getVertices(edges_cpu) ];
        out_degree_cpu = new int [ builtin_getVertices(edges_cpu) ];
        contrib_cpu = new int [ builtin_getVertices(edges_cpu) ];
        error_cpu = new int [ builtin_getVertices(edges_cpu) ];

        // Set parameters
        damp_cpu = ((int) 1);
        beta_score_cpu = ((((int) 1)  - damp_cpu) / builtin_getVertices(edges_cpu) );

        // Reset CPU vectors?
        parallel_for (int vertexsetapply_iter = 0; vertexsetapply_iter < builtin_getVertices(edges_cpu) ; vertexsetapply_iter++) {
                old_rank_generated_vector_op_apply_func_0()(vertexsetapply_iter);
        };

        parallel_for (int vertexsetapply_iter = 0; vertexsetapply_iter < builtin_getVertices(edges_cpu) ; vertexsetapply_iter++) {
                new_rank_generated_vector_op_apply_func_1()(vertexsetapply_iter);
        };

        generated_tmp_vector_cpu = builtin_getOutDegrees(edges_cpu) ;

        parallel_for (int vertexsetapply_iter = 0; vertexsetapply_iter < builtin_getVertices(edges_cpu) ; vertexsetapply_iter++) {
                generated_vector_op_apply_func_3()(vertexsetapply_iter);
        };

        parallel_for (int vertexsetapply_iter = 0; vertexsetapply_iter < builtin_getVertices(edges_cpu) ; vertexsetapply_iter++) {
                contrib_generated_vector_op_apply_func_4()(vertexsetapply_iter);
        };

        parallel_for (int vertexsetapply_iter = 0; vertexsetapply_iter < builtin_getVertices(edges_cpu) ; vertexsetapply_iter++) {
                error_generated_vector_op_apply_func_5()(vertexsetapply_iter);
        };

        parallel_for (int vertexsetapply_iter = 0; vertexsetapply_iter < builtin_getVertices(edges_cpu) ; vertexsetapply_iter++) {
                reset()(vertexsetapply_iter);
        };


        // Execute CPU Pagerank
        for ( int j = 0; j < 1 ; j++ ) {
                parallel_for (int vertexsetapply_iter = 0; vertexsetapply_iter < builtin_getVertices(edges_cpu) ; vertexsetapply_iter++) {
                        computeContrib()(vertexsetapply_iter);
                };
                //    for(int i=0; i < builtin_getVertices(edges_cpu); i++) {
                //      std::cout << "contrib_cpu[" << i << "] is " << contrib_cpu[i] << std::endl;
                //    }
                edgeset_apply_pull_parallel(edges_cpu, updateEdge());
                //    for(int i=0; i < builtin_getVertices(edges_cpu); i++) {
                //      std::cout << "new_rank_cpu[" << i << "] is " << new_rank_cpu[i] << std::endl;
                //    }
                parallel_for (int vertexsetapply_iter = 0; vertexsetapply_iter < builtin_getVertices(edges_cpu) ; vertexsetapply_iter++) {
                        updateVertex()(vertexsetapply_iter);
                };
        }

        parallel_for (int vertexsetapply_iter = 0; vertexsetapply_iter < builtin_getVertices(edges_cpu) ; vertexsetapply_iter++) {
                computeContrib()(vertexsetapply_iter);
        };

        // Start HB Code
        InputParser input(argc, argv);
        if(!input.cmdOptionExists("-g")){

                std::cerr << "no input args\n";
                return 0;
        }
        std::string ucode_path = input.getRISCVFile();

        int version = 0;

        hammerblade::builtin_loadMicroCodeFromFile(ucode_path);
        std::cerr << "load graph" << std::endl;

        std::string graph_f = input.getCmdOption("-g");
        edges = hammerblade::builtin_loadEdgesFromFileToHB (graph_f.c_str());
        std::cerr << "Finish initialize graph" << std::endl;
        int tmp_rank_val = (((int) 1)  / builtin_getVertices(edges_cpu) );
        std::vector<int> zerosf(edges.num_nodes(), 0.0);
        std::vector<int32_t> zeros(edges.num_nodes(), 0);
        std::vector<int> rank(edges.num_nodes(), tmp_rank_val);

        //  int* rank_test = rank.data();
        //  for(int i = 0; i < edges.num_nodes(); i++) {
        //    std::cout << rank_test[i] << std::endl;
        //  }
        old_rank_dev = Vector<int>(edges.num_nodes());
        new_rank_dev = Vector<int>(edges.num_nodes());
        out_degree_dev = Vector<int32_t>(edges.num_nodes());
        contrib_dev = Vector<int>(edges.num_nodes());
        contrib_new_dev = Vector<int>(edges.num_nodes());
        error_dev = Vector<int>(edges.num_nodes());

        damp_dev = GlobalScalar<int>("damp");
        beta_score_dev = GlobalScalar<int>("beta_score");

        Device::Ptr device = Device::GetInstance();

        int damp = ((int) 1);
        int beta_score = ((((int) 1) - damp) / edges.num_nodes());
        damp_dev.set(damp);
        beta_score_dev.set(beta_score);

        std::vector<int32_t> generated_tmp = edges.get_out_degrees();
        out_degree_dev.copyToDevice(generated_tmp.data(), edges.num_nodes());
        old_rank_dev.copyToDevice(rank.data(), rank.size());
        new_rank_dev.copyToDevice(zerosf.data(), zerosf.size());
        contrib_dev.copyToDevice(zerosf.data(), zerosf.size());
        contrib_new_dev.copyToDevice(zerosf.data(), zerosf.size());
        error_dev.copyToDevice(zerosf.data(), zerosf.size());

        std::cerr << "doing batch dma write" << std::endl;
        device->write_dma();


        //  edges.getInIndices().copyToHost(index_hb, edges.num_nodes()+1);
        //  edges.getInNeighbors().copyToHost(inneighbor_hb, edges.num_edges());
        //  old_rank_dev.copyToHost(old_rank_hb, edges.num_nodes());
        //  out_degree_dev.copyToHost(out_degree_hb, edges.num_nodes());
        //  device->read_dma();
        //  for(int i=0; i < 129; i++) {
        //    std::cout << "index_hb[" << i << "] is " << index_hb[i] << std::endl;
        //    std::cout << "out_degree_hb[" << i << "] is " << out_degree_hb[i] << " and out_degree_cpu[" << i << "] is " << out_degree_cpu[i] << std::endl;
        //  }
        //  for(int i=0; i< edges.num_edges(); i++) {
        //    std::cout << "inneighbor_hb[" << i << "] is " << inneighbor_hb[i] << std::endl;
        //  }
        //
        int64_t nodes = edges.num_nodes();

        // Rows per pod, is the number of rows in pods 0 to NUM_PODS-2. NUM_PODS-1 is a special case (handled by rows_in_pod).
        int64_t rows_per_pod = (nodes % NUM_PODS) == 0 ? (nodes / NUM_PODS) : ((nodes / NUM_PODS) + 1);
        int pod_row_start = CURRENT_POD * rows_per_pod;
        // Handle edge case
        int64_t rows_in_pod = CURRENT_POD == (NUM_PODS - 1) ? nodes - (pod_row_start) : rows_per_pod;
        int* __restrict out_degree_blocked_hb = new int[rows_per_pod];

        std::cout << "Current Pod: " << CURRENT_POD << std::endl;
        std::cout << "Num Pods: " << NUM_PODS << std::endl;
        std::cout << "Technique: Pull" << std::endl;
        std::cout << "Graph: " << graph_f << std::endl;
        std::cout << "Total Nodes: " << nodes << std::endl;
        std::cout << "Local Nodes: " << rows_in_pod << std::endl;
#ifdef HB_CYCLIC
        std::cout << "Distribution: Cyclic" << std::endl;
#else
        std::cout << "Distribution: Blocking" << std::endl;
#endif
        string kernel_function = KERNEL_FUNCTION;
        std::cout << "Function: " << kernel_function << std::endl;

        switch(version) {
        case 0:
                old_rank_dev.copyToHost(old_rank_hb, edges.num_nodes());
                out_degree_dev.copyToHost(out_degree_hb, edges.num_nodes());
                contrib_dev.copyToHost(contrib_hb, edges.num_nodes());
                device->read_dma();
                for(int i = 0; i < edges.num_nodes(); i++) {
                        contrib_hb[i] = old_rank_hb[i] / out_degree_hb[i];
                }
                contrib_dev.copyToDevice(contrib_hb, edges.num_nodes());
                for(int i = 0; i < rows_in_pod; i++) {
#ifdef HB_CYCLIC
                        out_degree_blocked_hb[i] = out_degree_hb[CURRENT_POD+i*NUM_PODS];
#else
                        out_degree_blocked_hb[i] = out_degree_hb[pod_row_start + i];
#endif

                }
                out_degree_dev.copyToDevice(out_degree_blocked_hb, rows_in_pod);
                device->write_dma();

                device->enqueueJob(kernel_function.c_str(), hb_mc_dimension(X,Y),{edges.getInIndicesAddr(), edges.getInNeighborsAddr(), out_degree_dev.getAddr(), old_rank_dev.getAddr(), new_rank_dev.getAddr(), contrib_dev.getAddr(), contrib_new_dev.getAddr(), rows_in_pod});
                uint64_t start_cycle = device->getCycle();
                hb_mc_manycore_trace_enable(device->getDevice()->mc);
                device->runJobs();
                hb_mc_manycore_trace_disable(device->getDevice()->mc);
                uint64_t end_cycle = device->getCycle();
                std::cerr << "Finished. Execution Cycles: " << (end_cycle - start_cycle) << std::endl;
        }

        old_rank_dev.copyToHost(old_rank_hb, rows_in_pod);
        device->read_dma();

        double rmse = 0.0;
        bool fail = false;

        for(int64_t i= 0; i < rows_in_pod; i++) {
#ifdef HB_CYCLIC
                int64_t cyclic_i = CURRENT_POD+i*NUM_PODS;
                rmse += (old_rank_hb[i] - old_rank_cpu[cyclic_i]) * (old_rank_hb[i] - old_rank_cpu[cyclic_i]);
                std::cout << "old_rank_hb[" << i << "] is " << old_rank_hb[i] << " and old_rank_cpu[" << i << "] is " << old_rank_cpu[cyclic_i] << std::endl;
                if (std::fabs(old_rank_hb[i] - old_rank_cpu[cyclic_i]) > 0.0000001) {
                        std::cerr << "Result is not equal at index " << i << ". FAIL!" << std::endl;
                        fail = true;
                }
#else
                int64_t block_i = pod_row_start+i;
                rmse += (old_rank_hb[i] - old_rank_cpu[block_i]) * (old_rank_hb[i] - old_rank_cpu[block_i]);
                std::cout << "old_rank_hb[" << i << "] is " << old_rank_hb[i] << " and old_rank_cpu[" << i << "] is " << old_rank_cpu[block_i] << std::endl;
                if (std::fabs(old_rank_hb[i] - old_rank_cpu[block_i]) > 0.0000001) {
                        std::cerr << "Result is not equal at index " << i << ". FAIL!" << std::endl;
                        fail = true;
                }
#endif
        }
        std::cerr << "RMSE: " << rmse << std::endl;

        device->close();
        return fail == true;
}

declare_program_main("GraphIt PageRank", launch);
