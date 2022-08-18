# The HammerBlade Benchmark Suite

This repository is a collection of applications ported for HammerBlade. 
These benchmarks are meant to be optimized for execution on HB using HammerBlade's CUDA-style runtime.

## How to Use This Repository

### Install
This repository is meant to be cloned into [bsg_replicant](https://github.com/bespoke-silicon-group/bsg_replicant) which is meant to be cloned into [bsg_bladerunner](https://github.com/bespoke-silicon-group/bsg_bladerunner).

For initial setup do the following:
1. Clone [bsg_bladerunner](https://github.com/bespoke-silicon-group/bsg_bladerunner) and follow the setup instructions.
2. From bsg_bladerunner, `cd bsg_replicant/examples; git clone git@github.com:bespoke-silicon-group/hb_hammerbench`

### Running Individual Benchmarks

Each application is meant to be standalone and will have a README explaining how to run it.
All applications live in the apps/ directory.

### Running All Benchmarks

Currently, go into each benchmark and follow the instructions to run it until you have run them all.

### Creating a Benchmark with Parameter Sweeps

If you want an easy way to setup a parameterizable testbench, a helper script exists in `mk/make_testbench.mk`.

Here's how to use it:
1. `cd apps/`
2. `make -f ../mk/make_testbench.mk TESTBENCH="benchmark-name" PARAMETERS="first-parameter [second-parameter etc...]"`

This will generate a template testbench from which multiple program executions can be run.
You will need to do the following:

1. `cd apps/[benchmark-name]`
2. Edit `template.mk` to use the test parameters (these are included in the generated  `parameters.mk`)
3. Edit main.c and kernel.cpp to implement your benchmark.

## Notes

2022-07-26: 11 apps present, none working.
