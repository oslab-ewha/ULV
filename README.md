# ULV(Ultra Light-weight Virtualization)

- This project aims to present early prototyping of ultra light-weight virtualization for shared computing platform.
- Too premature stage based on LKL(Linux Kernel Library) + solo5 tender

## Build

### Build LKL
- `# cd lkl/tools/lkl && make` 
- If build is ok, `lkl/tools/lkl/liblkl.a` is generated.

### Build solo5 lkl tender
- `# cd solo5 && make`
- After build, `solo5/tenders/lkl/solo5-lkl` executable will be created.

### Build a simple syscall test
- `# cd micro_benchmarks && make`
- `micro_benchmarks/syscall_test.spt` will be generated

## Running
- `# solo5/tenders/lkl/solo5-lkl micro_benchmarks/syscall_test.spt`

<hr>
<sub>This project was supported by Basic Science Research Program through the National Research Foundation of Korea(NRF) funded by the Ministry of Education(2020R1I1A1A01066121).</sub>
