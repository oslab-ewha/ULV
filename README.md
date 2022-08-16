
# ULV(Ultra Light-weight Virtualization)

- This project aims to present early prototyping of ultra light-weight virtualization for micro service and shared computing platform.
- Old ULV was based on LKL(Linux Kernel Library) + solo5 spt tender.
- The newly developed ULV is based on pure userland unikernel with no hypvervisor.

## Build
- Build process is tested on ubuntu 20.04

### Prerequistes

- gcc, g++, make, cmake
- musl-tools
 
### Build ULV
- `$ make prep`
  - builds lwip and musl
- `$ make`
  - builds libulv, libulfs and micro benchmarks

## Running (micro benchmark)
- `$ microbenchmarks/mb_thread`
  - pthread test under ULV

- `$ microbenchmarks/mb_syscall`
  - simple system call test

- `$ microbenchmarks/mb_network <connect IP:port> [<bind IP>]`
  - bind IP is an optional argument which is a local binding IP.
  - connect IP and port should be specified like as `192.168.12.100:30000`.
  - before test, a tap device should be setup as follows.
    ```
    # ip tuntap add tap0 mode tap
    # ip link set tap0 up
    # ip addr add 192.168.11.175/24 dev tap0
    ```
  - And, 4 environment variables should be set
    - PRECONFIGURED_TAPIF=tap0
    - IPADDR=192.168.11.170 , IP address on unikernel
    - GWADDR=192.168.11.175 , IP address on host tap device as a gateway
    - NETMASK=255.255.255.0
  - echo server using ncat
    - `ncat -l 20000 -k -c 'xargs -n1 echo'`
- `$ microbenchmarks/mb_file <file path>`
  - environment variable required. ULV_BLOCK=<path>
  - unikernel file system can be generated as follows:
    - `tools/ulfs/ulfs mkfs /tmp/testfile.img 10m`

<hr>
<sub>This project was supported by Basic Science Research Program through the National Research Foundation of Korea(NRF) funded by the Ministry of Education(2020R1I1A1A01066121).</sub>
