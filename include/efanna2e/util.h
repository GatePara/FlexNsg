//
// Created by 付聪 on 2017/6/21.
//

#ifndef EFANNA2E_UTIL_H
#define EFANNA2E_UTIL_H
#include <random>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#ifdef __APPLE__
#else
#include <malloc.h>
#endif
namespace efanna2e
{

  static void GenRandom(std::mt19937 &rng, unsigned *addr, unsigned size, unsigned N)
  {
    for (unsigned i = 0; i < size; ++i)
    {
      addr[i] = rng() % (N - size);
    }
    std::sort(addr, addr + size);
    for (unsigned i = 1; i < size; ++i)
    {
      if (addr[i] <= addr[i - 1])
      {
        addr[i] = addr[i - 1] + 1;
      }
    }
    unsigned off = rng() % N;
    for (unsigned i = 0; i < size; ++i)
    {
      addr[i] = (addr[i] + off) % N;
    }
  }

  inline float *data_align(float *data_ori, unsigned point_num, unsigned &dim)
  {
#ifdef __GNUC__
#ifdef __AVX__
#define DATA_ALIGN_FACTOR 8
#else
#ifdef __SSE2__
#define DATA_ALIGN_FACTOR 4
#else
#define DATA_ALIGN_FACTOR 1
#endif
#endif
#endif

    // std::cout << "align with : "<<DATA_ALIGN_FACTOR << std::endl;
    float *data_new = 0;
    unsigned new_dim = (dim + DATA_ALIGN_FACTOR - 1) / DATA_ALIGN_FACTOR * DATA_ALIGN_FACTOR;
// std::cout << "align to new dim: "<<new_dim << std::endl;
#ifdef __APPLE__
    data_new = new float[new_dim * point_num];
#else
    data_new = (float *)memalign(DATA_ALIGN_FACTOR * 4, point_num * new_dim * sizeof(float));
#endif

    for (unsigned i = 0; i < point_num; i++)
    {
      memcpy(data_new + i * new_dim, data_ori + i * dim, dim * sizeof(float));
      memset(data_new + i * new_dim + dim, 0, (new_dim - dim) * sizeof(float));
    }
    dim = new_dim;
#ifdef __APPLE__
    delete[] data_ori;
#else
    free(data_ori);
#endif
    return data_new;
  }

  inline void load_fvecs(char *filename, float *&data, unsigned &num, unsigned &dim)
  { // load data with sift pattern
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open())
    {
      std::cerr << "Error: Unable to open file " << filename << " for reading" << std::endl;
      exit(-1);
    }
    in.read((char *)&dim, 4);
    in.seekg(0, std::ios::end);
    uint64_t dim_64 = dim;
    std::ios::pos_type ss = in.tellg();
    size_t fsize = (size_t)ss;
    uint64_t num_64 = (size_t)(fsize / (dim_64 + 1) / 4);
    num = num_64;
    data = new float[num_64 * dim_64];

    in.seekg(0, std::ios::beg);
    for (uint64_t i = 0; i < num_64; i++)
    {
      in.seekg(4, std::ios::cur);
      in.read((char *)(data + i * dim_64), dim_64 * 4);
    }
    in.close();
  }

  inline void load_fbin(char *filename, float *&data, unsigned &num, unsigned &dim)
  { // load data with fbin pattern
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open())
    {
      std::cerr << "Error: Unable to open file " << filename << " for reading" << std::endl;
      exit(-1);
    }
    in.read((char *)&num, 4);
    in.read((char *)&dim, 4);

    uint64_t dim_64 = dim;
    uint64_t num_64 = num;

    data = new float[num_64 * dim_64];

    in.seekg(0, std::ios::beg);
    for (uint64_t i = 0; i < num_64; i++)
    {
      in.read((char *)(data + i * dim_64), dim_64 * 4);
    }
    in.close();
  }

  inline void save_result(char *filename, std::vector<std::vector<unsigned>> &results)
  {
    std::ofstream out(filename, std::ios::binary | std::ios::out);
    if (!out.is_open())
    {
      std::cerr << "Error: Unable to open file " << filename << " for writing" << std::endl;
      exit(-1);
    }
    for (unsigned i = 0; i < results.size(); i++)
    {
      unsigned GK = (unsigned)results[i].size();
      out.write((char *)&GK, sizeof(unsigned));
      out.write((char *)results[i].data(), GK * sizeof(unsigned));
    }
    out.close();
  }

  inline void print_csv_header()
  {
    std::cout << std::left;
    std::cout << std::setw(12) << "L" << ","
              << std::setw(12) << "time" << ","
              << std::setw(12) << "querynum" << ","
              << std::setw(12) << "qps" << ","
              << std::setw(12) << "recall"
              << std::endl;
  }

  inline void print_csv_row(unsigned L, double time, unsigned query_num, double qps, double recall)
  {
    std::cout << std::fixed << std::setprecision(6);
    std::cout << std::setw(12) << L << ","
              << std::setw(12) << time << ","
              << std::setw(12) << query_num << ","
              << std::setw(12) << qps << ","
              << std::setw(12) << recall
              << std::endl;
  }

}

#endif // EFANNA2E_UTIL_H
