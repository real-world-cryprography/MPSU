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
### Automated Testing

We also provide automated scripts to test both SK-MPSU and PK-MPSU, allowing for quick and efficient evaluation across different configurations.

#### SK-MPSU Auto Test

```shell
#in MPSU/autoTestShell
#copy the script to MPSU/SKMPSU/build
cp autoTestSK.sh ../SKMPSU/build/

#in MPSU/SKMPSU/build
chmod +x autoTestSK.sh

#run the shell script
# - number of parties: {3,4,5,7,9,10}, if you want to change it, you can modify the k_values in shell script
# - logarithm of set size: from 10 to 12, step size of 2
./autoTestSK.sh -start 10 -end 12 -step 2
```

### PK-MPSU Auto Test

```shell
#in MPSU/autoTestShell
#copy the script to MPSU/PKMPSU/build
cp autoTestPK.sh ../PKMPSU/build/

#in MPSU/PKMPSU/build
chmod +x autoTestPK.sh

#run the shell
# - number of parties: {3,4,5,7,9,10}, if you want to change it, you can modify the k_values in shell script
# - logarithm of set size: from 10 to 12, step size of 2
./autoTestPK.sh -start 10 -end 12 -step 2
```
## Docker Quick Start

Docker simplifies the setup and execution of our MPSU protocols by providing a pre-configured environment. Follow these steps to get started:

### Prerequisites

- Ensure the Docker is installed on your machine. You can download Docker from the [official website](https://www.docker.com/products/docker-desktop).

### Pulling the Docker image

```bash
docker pull kafei2cy/mpsu:latest
```

### Running a Docker Container

To create and start a container from the downloaded image, use:

```sh
docker run -it --name your-container-name kafei2cy/mpsu /bin/bash
```

- Replace `--name your-container-name` with a preferred container name.

### Runing SK-MPSU in Docker

```shell
cd /home/MPSU/SKMPSU/build/
#in /home/MPSU/SKMPSU/build/
#print help information
./main -h
# The other commands remain the same as described earlier
```

### Runing PK-MPSU in Docker

```shell
cd /home/MPSU/PKMPSU/build/
#in /home/MPSU/PKMPSU/build/
#print help information
./main -h
# The other commands remain the same as described earlier
```


















