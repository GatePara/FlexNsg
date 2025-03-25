#include <efanna2e/index_graph.h>
#include <efanna2e/index_random.h>
#include <efanna2e/util.h>

int main(int argc, char **argv)
{
  if (argc != 9)
  {
    std::cout << argv[0] << " <fvecs/fbin> data_file save_graph K L iter S R" << std::endl;
    exit(-1);
  }

  char *graph_filename = argv[3];
  unsigned K = (unsigned)atoi(argv[4]);
  unsigned L = (unsigned)atoi(argv[5]);
  unsigned iter = (unsigned)atoi(argv[6]);
  unsigned S = (unsigned)atoi(argv[7]);
  unsigned R = (unsigned)atoi(argv[8]);

  std::cout << "===== Input Parameters =====" << std::endl;
  std::cout << "Data Type     : " << argv[1] << std::endl;
  std::cout << "Data File     : " << argv[2] << std::endl;
  std::cout << "Graph Output  : " << argv[3] << std::endl;
  std::cout << "K             : " << K << std::endl;
  std::cout << "L             : " << L << std::endl;
  std::cout << "Iterations    : " << iter << std::endl;
  std::cout << "S             : " << S << std::endl;
  std::cout << "R             : " << R << std::endl;
  std::cout << "============================" << std::endl;

  float *data_load = NULL;
  unsigned points_num, dim;
  if (strcmp(argv[1], "fvecs") == 0)
  {
    efanna2e::load_fvecs(argv[2], data_load, points_num, dim);
  }
  else if (strcmp(argv[1], "fbin") == 0)
  {
    efanna2e::load_fbin(argv[2], data_load, points_num, dim);
  }
  else
  {
    std::cout << "Unsupport data type: " << argv[1] << std::endl;
  }

  std::cout << "points_num: " << points_num << " dim: " << dim << std::endl;

  // data_load = efanna2e::data_align(data_load, points_num, dim);//one must align the data before build
  efanna2e::IndexRandom init_index(dim, points_num);
  efanna2e::IndexGraph index(dim, points_num, efanna2e::L2, (efanna2e::Index *)(&init_index));

  efanna2e::Parameters paras;
  paras.Set<unsigned>("K", K);
  paras.Set<unsigned>("L", L);
  paras.Set<unsigned>("iter", iter);
  paras.Set<unsigned>("S", S);
  paras.Set<unsigned>("R", R);

  auto s = std::chrono::high_resolution_clock::now();
  index.Build(points_num, data_load, paras);
  auto e = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = e - s;
  std::cout << "Time cost: " << diff.count() << std::endl;

  index.Save(graph_filename);

  delete[] data_load;

  return 0;
}
