# FlexNsg: Navigating Spread-out Graph for Approximate Nearest Neighbor Search with Enhanced Flexibility

Table of Contents
=================
<!--ts-->
* [Introduction](#introduction)
* [Building Instruction](#building-instruction)
     * [Prerequisites](#prerequisites)
     * [Compile On Ubuntu/Debian](#compile-on-ubuntudebian)
* [Usage](#usage)
     * [Building NSG Index](#building-nsg-index)
     * [Searching via NSG Index](#searching-via-nsg-index)
* [Parameters Setting](#parameters-setting)
     * [NSG Building](#nsg-building)
* [Reference](#reference)
* [TODO](#todo)
* [License](#license)
<!--te-->

## Introduction

**FlexNsg** is an efficient open-source library for Approximate Nearest Neighbor Search (ANNS) that operates seamlessly across a variety of computing environments without any reliance on third-party runtime libraries, including but not limited to x86 and ARM architectures.

**FlexNsg** can build KNN graphs from datasets and transform them into NSG indices, facilitating efficient ANNS across different platforms. This comprehensive service design allows developers to quickly deploy and utilize NSGFlex, even on MacBook computers with M-series chips. This capability surpasses the original implementations of efanna and nsg, which depend on libraries such as Boost and BLAS and cannot be compiled on non-x86 platforms. 

FlexNsg implements the algorithm described in the PVLDB paper - [Fast Approximate Nearest Neighbor Search With The Navigating Spread-out Graphs](http://www.vldb.org/pvldb/vol12/p461-fu.pdf). The core code of NSGFlex originates from the [efanna_graph](https://github.com/ZJULearning/efanna_graph) and [nsg](https://github.com/ZJULearning/nsg) projects, and we extend our gratitude to these developers for their diligent work. Building on this foundation, NSGFlex has been optimized for cross-platform adaptation and vectorization acceleration to enhance performance.


### Highlights of NSGFlex:
- **Cross-Platform Efficiency**: Optimized for both x86 and ARM architectures, NSGFlex delivers high-speed performance and precision in nearest neighbor calculations across different hardware setups.
- **No External Dependencies**: NSGFlex does not rely on any external libraries or frameworks, simplifying its integration into any project.


## Building Instruction

### Prerequisites

+ GCC 11+
+ CMake 3.22+


### Compile On Ubuntu/Debian

1. Install Dependencies:

```shell
sudo apt-get install g++ cmake
```

2. Compile FlexNsg:

First, you should clone the code to your computer.
```shell
git clone https://github.com/GatePara/NSGFlex.git
cd NSGFlex/
```
Then you can use auto-build script to compile NSGFlex:
```shell
bash build.sh
```

Or you can do it manually.
```shell
mkdir build/ && cd build/
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j
```

## Usage

The main interfaces and classes have its respective test codes under directory `tests/`

### Building NSG Index

To use NSG for ANNS, an NSG index must be built first. Here are the instructions for building NSG.

#### Step 1. Build kNN Graph

Firstly, we need to prepare a kNN graph.

* kNN graph building with nndescent:    
```shell
cd build/tests/
./test_nndescent data_type data_path save_path K L iter S R
```
+ `data_type` is the type of the origin data, must be `fvecs` or `fbin`.
+ `data_path` is the path of the origin data.
+ `save_path` is the path of the kNN graph to be saved.
+ `K` is the 'K' of kNN graph.
+ `L` is the parameter controlling the graph quality, larger is more accurate but slower, no smaller than K.
+ `iter` is the parameter controlling the iteration times, iter usually < 30.
+ `S` is the parameter contollling the graph quality, larger is more accurate but slower.
+ `R` is the parameter controlling the graph quality, larger is more accurate but slower.

#### Step 2. Convert kNN Graph to NSG

Secondly, we will convert the kNN graph to our NSG index.

You can use our demo code to achieve this converstion as follows:
```shell
cd build/tests/
./test_nsg_index data_type data_path knng_path L R C nsg_path
```
+ `data_type` is the type of the origin data, must be `fvecs` or `fbin`.
+ `data_path` is the path of the base data in `fvecs` format.
+ `knng_path` is the path of the pre-built kNN graph in *Step 1.*.
+ `L` controls the quality of the NSG, the larger the better.
+ `R` controls the index size of the graph, the best R is related to the intrinsic dimension of the dataset.
+ `C` controls the maximum candidate pool size during NSG contruction.
+ `nsg_path` is the path of the generated NSG index.

### Searching via NSG Index

Here are the instructions of how to use NSG index for searching.

You can use our demo code to perform kNN searching as follows:
```shell
cd build/tests/
./test_nsg_search data_type data_path query_path nsg_path search_l search_k result_path
```

+ `data_type` is the type of the origin data, must be `fvecs` or `fbin`.
+ `data_path` is the path of the base data in `fvecs` format.
+ `query_path` is the path of the query data in `fvecs` format.
+ `nsg_path` is the path of the pre-built NSG index in previous section.
+ `search_l` controls the quality of the search results, the larger the better but slower. The `search_l` cannot be samller than the `search_k`
+ `search_k` controls the number of result neighbors we want to query.
+ `result_path` is the path of search results in `ivecs` format.

The `test_nsg_optimized_search` method decomposes the Euclidean distance formula and precomputes the norm of the original vectors. This approach optimizes the distance calculation by reducing it to a simple dot product operation, which can potentially speed up the computation in certain scenarios. However, this method comes with trade-offs, including additional memory usage and increased runtime overhead. Therefore, we generally do not recommend using this approach.


```shell
cd build/tests/
./test_nsg_optimized_search data_path query_path nsg_path search_l search_k result_path
```

***NOTE:** Only data-type int32 and float32 are supported for now.*

> HINT: The `data_align()` function we provided is essential for the correctness of our procedure, because we use SIMD instructions for acceleration of numerical computing such as AVX and SSE2.
You should use it to ensure your data elements (feature) is aligned with 8 or 16 int or float.
For example, if your features are of dimension 70, then it should be extend to dimension 72. And the last 2 dimension should be filled with 0 to ensure the correctness of the distance computing. And this is what `data_align()` does.

> HINT: Please refer [here](http://yael.gforge.inria.fr/file_format.html) for the desciption of `fvecs/ivecs` format.

## Parameters Setting

### NSG Building

We use efanna_graph to build the kNN graph， it is also included in NSGFlex.

#### Step 1. Build kNN Graph

+ Parameters:

| Dataset |  K  |  L  | iter |  S |  R  |
|:-------:|:---:|:---:|:----:|:--:|:---:|
|  SIFT1M | 200 | 200 |  10  | 10 | 100 |
|  GIST1M | 400 | 400 |  12  | 15 | 100 |

+ Commands:
```shell
NSGFlex/build/tests/test_nndescent sift.fvecs sift_200nn.graph 200 200 10 10 100    # SIFT1M
NSGFlex/build/tests/test_nndescent gist.fvecs gist_400nn.graph 400 400 12 15 100    # GIST1M
```

#### Step 2. Convert kNN Graph to NSG

+ Parameters:

| Dataset |  L |  R |  C  |
|:-------:|:--:|:--:|:---:|
|  SIFT1M | 40 | 50 | 500 |
|  GIST1M | 60 | 70 | 500 |

+ Commands:
```shell
NSGFlex/build/tests/test_nsg_index sift.fvecs sift_200nn.graph 40 50 500 sift.nsg        # SIFT1M
NSGFlex/build/tests/test_nsg_index gist.fvecs gist_400nn.graph 60 70 500 gist.nsg        # GIST1M
```

## Reference

```
@article{FuNSG17,
  author    = {Cong Fu and Chao Xiang and Changxu Wang and Deng Cai},
  title     = {Fast Approximate Nearest Neighbor Search With The Navigating Spreading-out Graphs},
  journal   = {{PVLDB}},
  volume    = {12},
  number    = {5},
  pages     = {461 - 474},
  year      = {2019},
  url       = {http://www.vldb.org/pvldb/vol12/p461-fu.pdf},
  doi       = {10.14778/3303753.3303754}
}
```

## TODO

- [*] Add neon fp32 simd speedup
- [ ] Support apple m-series chip

## License

NSGFlex is MIT-licensed.
