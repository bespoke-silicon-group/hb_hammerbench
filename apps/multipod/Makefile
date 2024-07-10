BENCHMARKS = 
BENCHMARKS += jacobi
BENCHMARKS += fft128
BENCHMARKS += fft256
BENCHMARKS += sgemm
BENCHMARKS += barnes_hut
BENCHMARKS += bfs
BENCHMARKS += pagerank
BENCHMARKS += spgemm

# clean target;
clean.jacobi:
	make -C jacobi/ clean
clean.fft128:
	make -C fft/128/ clean
clean.fft256:
	make -C fft/256/ clean
clean.sgemm:
	make -C sgemm/ clean
clean.barnes_hut:
	make -C barnes_hut/ clean
clean.bfs:
	make -C bfs/ clean
clean.pagerank:
	make -C pagerank/ clean
clean.spgemm:
	make -C spgemm/ clean


# run target;
run.jacobi:
	make -C jacobi/ profile
run.fft128:
	make -C fft/128/ profile
run.fft256:
	make -C fft/256/ profile
run.sgemm:
	make -C sgemm/ profile
run.barnes_hut:
	make -C barnes_hut/ profile -j3
run.bfs:
	make -C bfs/ profile -j6
run.pagerank:
	make -C pagerank/ profile -j7
run.spgemm:
	make -C spgemm/ profile -j4


run_all: $(addprefix run., $(BENCHMARKS))
clean_all: $(addprefix clean., $(BENCHMARKS))
