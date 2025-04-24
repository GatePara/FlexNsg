#include <efanna2e/index_nsg.h>
#include <efanna2e/util.h>
#include "omp.h"
int main(int argc, char **argv)
{
  if (argc <= 11)
  {
    std::cout << argv[0]
              << " <fvecs/fbin> data_file query_file nsg_path search_K threads_num result_path <ivecs/diskann> gt_path <search_L_lists>"
              << std::endl;
    exit(-1);
  }

  unsigned K = (unsigned)atoi(argv[5]);

  float *data_load = NULL;
  unsigned points_num, dim;
  float *query_load = NULL;
  unsigned query_num, query_dim;

  if (strcmp(argv[1], "fvecs") == 0)
  {
    efanna2e::load_fvecs(argv[2], data_load, points_num, dim);
    efanna2e::load_fvecs(argv[3], query_load, query_num, query_dim);
  }
  else if (strcmp(argv[1], "fbin") == 0)
  {
    efanna2e::load_fbin(argv[2], data_load, points_num, dim);
    efanna2e::load_fbin(argv[3], query_load, query_num, query_dim);
  }
  else
  {
    std::cout << "Unsupport data type: " << argv[1] << std::endl;
  }
  std::cout << "base_points_num: " << points_num << " dim: " << dim << std::endl;
  std::cout << "query_points_num: " << query_num << " dim: " << dim << std::endl;
  assert(dim == query_dim);

  std::cout << "gt type: " << argv[8] << std::endl;
  std::cout << "gt path: " << argv[9] << std::endl;
  std::vector<std::vector<uint32_t>> gt;
  if (strcmp(argv[8], "diskann") == 0)
  {
    efanna2e::load_diskann_gt(argv[9], gt);
  }
  else if (strcmp(argv[8], "ivecs") == 0)
  {
    efanna2e::load_ivecs_gt(argv[9], gt);
  }
  else
  {
    std::cout << "Unsupport gt type: " << argv[8] << std::endl;
    exit(-1);
  }
  std::cout << "gt size: " << gt.size() << std::endl;
  std::cout << "gt[0] size: " << gt[0].size() << std::endl;

  int L_size = argc - 10;
  std::vector<unsigned> Ls(L_size);
  for (int i = 0; i < L_size; i++)
  {
    auto L = atoi(argv[10 + i]);
    if (L <= 0)
    {
      std::cout << "L must be positive" << std::endl;
      exit(-1);
    }
    if (L < (int)K)
    {
      std::cout << "L must be larger than K" << std::endl;
      exit(-1);
    }
    Ls[i] = (unsigned)L;
  }
  std::sort(Ls.begin(), Ls.end());

  int thread_num = atoi(argv[6]);
  if (thread_num <= 0)
  {
    std::cout << "thread_num must be positive" << std::endl;
    exit(-1);
  }
  omp_set_num_threads(thread_num);
  std::cout << "thread_num: " << thread_num << std::endl;
  std::cout << "K: " << K << std::endl;
  // print L
  std::cout << "L: ";
  for (auto L : Ls)
  {
    std::cout << L << " ";
  }
  std::cout << std::endl;
  // check gt
  assert(gt.size() == query_num);
  assert(gt[0].size() >= K);

  // data_load = efanna2e::data_align(data_load, points_num, dim);//one must
  // align the data before build query_load = efanna2e::data_align(query_load,
  // query_num, query_dim);
  efanna2e::IndexNSG index(dim, points_num, efanna2e::L2, nullptr);
  index.Load(argv[4]);

  // save cout to file
  std::ofstream out(argv[7], std::ios::out);
  if (!out.is_open())
  {
    std::cerr << "Error: Unable to open file " << argv[7] << " for writing" << std::endl;
    exit(-1);
  }
  out << "K,L,Time,Query_num,QPS,Recall,Dco,Hops" << std::endl;
  efanna2e::Parameters paras;
  unsigned repeat_times = 3;
  std::vector<std::vector<unsigned>> res(query_num, std::vector<unsigned>(K, 0));
  for (auto L : Ls)
  {
    paras.Set<unsigned>("L_search", L);
    paras.Set<unsigned>("P_search", L);
    index.ResetCount();
    auto s = std::chrono::high_resolution_clock::now();
    for (unsigned repeat = 0; repeat < repeat_times; repeat++)
    {
#pragma omp parallel for
      for (unsigned i = 0; i < query_num; i++)
      {
        index.Search(query_load + i * dim, data_load, K, paras, res[i].data());
      }
    }
    auto e = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = e - s;
    double query_time = diff.count() / (double)repeat_times;
    double qps = (double)query_num / query_time;
    double recall = efanna2e::compute_recall(res, gt);
    auto counts = index.GetCount();

    auto dco = counts.first / repeat_times / query_num;
    auto hops = counts.second / repeat_times / query_num;

    out << K << ","
        << L << ","
        << query_time << ","
        << query_num << ","
        << qps << ","
        << recall << ","
        << dco << ","
        << hops
        << std::endl;
  }
  out.close();

  delete[] data_load;
  delete[] query_load;

  return 0;
}
