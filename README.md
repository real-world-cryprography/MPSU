# Multi-party private Set Union (MPSU)

This is the implementation of our paper: **Efficient Multi-Party Private Set Union Without Non-Collusion Assumptions** [[ePrint](https://eprint.iacr.org/2024/1146)]. 

This repository implements two MPSU protocols: 

  * Our SK-MPSU protocol, based on OT and symmetric-key operations in the standard semi-honest model.

  * Our PK-MPSU protocol, based on public-key operations in the standard semi-honest model.

Evaluating on a single server (`32-cores Intel Xeon 2.70GHz CPU and 128GB of RAM`) with a single thread per party (except for the interaction threads, where one thread is used for communicating with each other party), our SK-MPSU protocol requires only `4.4` seconds in online phase, for 3 parties with sets of `2^20` items each in the LAN setting. 

## Installations

### Required libraries

This code is built on top of [`Vole-PSI`](https://github.com/Visa-Research/volepsi.git) and requires additional library dependencies including [`OpenSSL`](https://www.openssl.org) and [`OpenMP`](https://www.openmp.org). Our code has been tested on Ubuntu 22.04, with g++ 11.4.0. 

To install the required libraries and tools, run the following shell commands:

```bash
# Update the package list
sudo apt-get update

# Install essential development tools and libraries
sudo apt-get install build-essential libssl-dev libomp-dev libtool
sudo apt install gcc g++ git make cmake
```

### Building Vole-PSI

1. Clone the repository:

```bash
#in MPSU
git clone https://github.com/Visa-Research/volepsi.git
cd volepsi
```

2. Compile and install Vole-PSI:

```bash
python3 build.py -DVOLE_PSI_ENABLE_BOOST=ON -DVOLE_PSI_ENABLE_GMW=ON -DVOLE_PSI_ENABLE_CPSI=OFF -DVOLE_PSI_ENABLE_OPPRF=OFF
python3 build.py --install=../libvolepsi
cp out/build/linux/volePSI/config.h ../libvolepsi/include/volePSI/
```

#### Fixing Build Issues

When building Vole-PSI, an error with `DOWNLOAD HASH mismatch` may occur due to issues with the Boost URL. To fix this, update line 8 in `volepsi/out/coproto/thirdparty/getBoost.cmake` as follows:

```bash
set(URL "https://archives.boost.io/release/1.86.0/source/boost_1_86_0.tar.bz2")
```

### Building SK-MPSU

```bash
cd ../SKMPSU
#in MPSU/SKMPSU
mkdir build
cd build
mkdir offline
mkdir sc
cmake ..
make
```

### Building PK-MPSU

```bash
cd ../PKMPSU
#in MPSU/PKMPSU
mkdir build
cd build
mkdir offline
cmake ..
make
```

## Running the code

### Test for SK-MPSU

```bash
cd build
#in MPSU/SKMPSU/build
#print help information
./main -h
```

#### Flags:

        -n:                set size (default 1024)
        -nn:               logarithm of set size (default 10)
        -k:                number of parties (default 3)
        -nt:               number of threads for generating share correlation and boolean triples (default 1)
        -r:                index of the party
        -genSC:            generate share correlation in the offline phase
        -preGen:           generate vole, boolean triples and random OT in the offline phase
        -psu:              run the online phase of our SK-MPSU

#### Examples: 

Run SK-MPSU of 3 parties, each with set size `2^12`:

``` bash
./main -genSC -k 3 -nn 12
./main -preGen -k 3 -nn 12 -r 0 & ./main -preGen -k 3 -nn 12 -r 1 & ./main -preGen -k 3 -nn 12 -r 2
./main -psu -k 3 -nn 12 -r 0 & ./main -psu -k 3 -nn 12 -r 1 & ./main -psu -k 3 -nn 12 -r 2
```


### Test for PK-MPSU

```bash
cd build
#in MPSU/PKMPSU/build
#print help information
./main -h
```

#### Flags:

        -n:                set size (default 1024)
        -nn:               logarithm of set size (default 10)
        -k:                number of parties (default 3)
        -nt:               number of threads for generating boolean triples, ciphertext rerandomization and partial decryption (default 1)
        -r:                index of the party
        -preGen:           generate vole, boolean triples and random OT in the offline phase
        -psu:              run the online phase of our PK-MPSU

#### Examples: 

Run PK-MPSU of 3 parties, each with set size `2^12`:

```bash
./main -preGen -k 3 -nn 12 -r 0 & ./main -preGen -k 3 -nn 12 -r 1 & ./main -preGen -k 3 -nn 12 -r 2
./main -psu -k 3 -nn 12 -r 0 & ./main -psu -k 3 -nn 12 -r 1 & ./main -psu -k 3 -nn 12 -r 2
```

## Docker Quick Start

Docker makes it easy to create, deploy, and run applications by using containers. Here are some quick tips to get you started with Docker:

### Prerequisites

- Ensure you have Docker installed on your machine. You can download Docker from the [official website](https://www.docker.com/products/docker-desktop).

### Pulling the docker image

To pull the Docker image, use the following command:

```bash
docker pull kafei2cy/mpsu:latest
```

### Running a Docker Container

To run a Docker container from the image you pulled and access the Container Shell, use the following command:

```sh
docker run -it --name your-container-name kafei2cy/mpsu /bin/bash
```

- `--name your-container-name` gives your container a name for easier reference.




















