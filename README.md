
# ULV(Ultra Light-weight Virtualization)

- This project aims to present early prototyping of ultra light-weight virtualization for shared computing platform.
- Currently, ULV is based on LKL(Linux Kernel Library) + solo5 tender
- Newly developed solo5 lkl tender is derrived from spt tender.

## Build

### Prerequistes

- GNU make
- musl-C library
- libgdbm
 
### Build ULV
- `# make` 
- If build is ok, following files will be created
  - `solo5/tenders/lkl/solo5-lkl`
  - `micro_benchmarks/syscall_test.spt`
  - `macro_benchmarks/{gdbm_test.spt,wget_test.spt}`


## Running
- `# solo5/tenders/lkl/solo5-lkl micro_benchmarks/syscall_test.spt`
  - execute system call tests(uname,gettimeofday,open/close) on ULV

- `# solo5/tenders/lkl/solo5-lkl macro_benchmarks/gdbm_test.spt`
  - execute gdbm application on ULV

- `# solo5/tenders/lkl/solo5-lkl macro_benchmarks/wget_test.spt`
  - execute wget application on ULV

<hr>
<sub>This project was supported by Basic Science Research Program through the National Research Foundation of Korea(NRF) funded by the Ministry of Education(2020R1I1A1A01066121).</sub>
