#include <string>
#include <cstring>

namespace BFS {
    class CL {
    public:
        CL() {}
        int parse(int argc, char *argv[]) {
            _exec_path      = argv[0];
            _binary_path    = argv[1];
            _kernel_name    = argv[2];
            _input_graph_path = argv[3];
            _graph_type     = argv[4];
            _graph_vertices = atoi(argv[5]);
            _graph_edges    = atoi(argv[6]);
            _bfs_root       = atoi(argv[7]);
            _bfs_iteration  = atoi(argv[8]);
            _groups         = atoi(argv[9]);
            _tgx            = atoi(argv[10]);
            _tgy            = atoi(argv[11]);
            _pod            = atoi(argv[12]);
            return 0;
        }

        std::string _binary_path;
        std::string _exec_path;
        std::string _kernel_name;
        std::string _input_graph_path;
        std::string _graph_type;
        int         _graph_vertices;
        int         _graph_edges;
        int         _bfs_root;
        int         _bfs_iteration;
        int         _groups;        
        int         _tgx;
        int         _tgy;
        int         _pod;
        
        std::string binary_path() const { return _binary_path; }
        std::string exec_path() const { return _exec_path; }
        std::string kernel_name() const { return _kernel_name; }
        std::string input_graph_path() const { return _input_graph_path; }        
        std::string graph_type() const { return _graph_type; }
        int graph_vertices() const { return _graph_vertices; }
        int graph_edges() const { return _graph_edges; }
        int bfs_root() const { return _bfs_root; }
        int bfs_iteration() const { return _bfs_iteration; }
        int groups() const { return _groups; }
        int tgx() const { return _tgx; }
        int tgy() const { return _tgy; }
        int pod_id() const { return _pod; }             
    };
}
