## Makefile Usage

This Makefile supports two workflows:

1. **Preprocessing an existing graph** from the HammerBench graph dataset.
2. **Generating a new random graph** and producing a configuration file for it.

Both workflows also generate a configuration file used by the HammerBench build system.

---

## Variables

The following variables can be overridden from the command line when running `make`.

### Graph / System Parameters

- **`GRAPH_NAME`**  
  Name of the graph (without the `.mtx` extension when preprocessing).  
  Default: `no-name-given`

- **`ROOT`**  
  Root vertex used by the graph algorithm.  
  Default: `0`

- **`NUMPODS`**  
  Number of pods used by the configuration.  
  Default: `1`

- **`TILE_GROUP_DIM_X`**  
  Tile group dimension in the X direction.  
  Default: `4`

- **`TILE_GROUP_DIM_Y`**  
  Tile group dimension in the Y direction.  
  Default: `4`

---

### Graph Generation Parameters

Used only by the `generate` target.

- **`NUM_VERTICES`**  
  Number of vertices in the generated graph.  
  Default: `16`

- **`NUM_EDGES`**  
  Number of edges in the generated graph.  
  Default: `32`

- **`WEIGHT_LOW`**  
  Minimum edge weight.  
  Default: `1`

- **`WEIGHT_HIGH`**  
  Maximum edge weight.  
  Default: `10`

- **`ALLOW_SELF`**  
  Whether self edges are allowed (`True` or `False`).  
  Default: `False`

---

## Targets

### `preprocess`

Used when a graph already exists in the HammerBench graph dataset.

The Makefile expects the graph file to exist at: `apps/graph_data/<GRAPH_NAME>.mtx`

See the `README.md` in that folder for steps in downloading datasets

#### Example

```bash
make generate preprocess GRAPH_NAME=wiki-Vote ROOT=0
```

### `generate`

Generates a new random graph and creates a matching configuration file.

#### Example
```bash
make generate GRAPH_NAME=test_graph NUM_VERTICES=64 NUM_EDGES=128
```

Example with full parameter control:

```bash
make generate \
GRAPH_NAME=test_graph \
NUM_VERTICES=64 \
NUM_EDGES=128 \
WEIGHT_LOW=1 \
WEIGHT_HIGH=20 \
ALLOW_SELF=False \
ROOT=0 \
NUMPODS=1
```

### clean

Removes all `.mtx` files in `/inputs` and all config file in `bellmanford/`
