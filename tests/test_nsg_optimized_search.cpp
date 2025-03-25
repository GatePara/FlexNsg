#include <efanna2e/index_nsg.h>
#include <efanna2e/util.h>
#include <chrono>
#include <string>

int main(int argc, char **argv)
{
  if (argc != 8)
  {
    std::cout << argv[0]
              << " <fvecs/fbin> data_file query_file nsg_path search_L search_K result_path"
              << std::endl;
    exit(-1);
  }
  unsigned L = (unsigned)atoi(argv[5]);
  unsigned K = (unsigned)atoi(argv[6]);

  if (L < K)
  {
    std::cout << "search_L cannot be smaller than search_K!" << std::endl;
    exit(-1);
  }

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

  // data_load = efanna2e::data_align(data_load, points_num, dim);//one must
  // align the data before build query_load = efanna2e::data_align(query_load,
  // query_num, query_dim);
  efanna2e::IndexNSG index(dim, points_num, efanna2e::FAST_L2, nullptr);
  index.Load(argv[4]);
  index.OptimizeGraph(data_load);

  efanna2e::Parameters paras;
  paras.Set<unsigned>("L_search", L);
  paras.Set<unsigned>("P_search", L);

  std::vector<std::vector<unsigned>> res(query_num);
  for (unsigned i = 0; i < query_num; i++)
    res[i].resize(K);

  auto s = std::chrono::high_resolution_clock::now();
  for (unsigned i = 0; i < query_num; i++)
  {
    index.SearchWithOptGraph(query_load + i * dim, K, paras, res[i].data());
  }
  auto e = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = e - s;
  efanna2e::print_csv_header();
  efanna2e::print_csv_row(L, diff.count(), query_num, query_num / diff.count(), 0);

  efanna2e::save_result(argv[7], res);

  delete[] data_load;
  delete[] query_load;

  return 0;
}
