//
// Created by 付聪 on 2017/6/21.
//

#include <efanna2e/index_nsg.h>
#include <efanna2e/util.h>

int main(int argc, char **argv)
{
  if (argc != 8)
  {
    std::cout << argv[0] << " <fvecs/fbin> data_file nn_graph_path L R C save_graph_file"
              << std::endl;
    exit(-1);
  }
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

  std::string nn_graph_path(argv[3]);
  unsigned L = (unsigned)atoi(argv[4]);
  unsigned R = (unsigned)atoi(argv[5]);
  unsigned C = (unsigned)atoi(argv[6]);

  // data_load = efanna2e::data_align(data_load, points_num, dim);//one must
  // align the data before build
  efanna2e::IndexNSG index(dim, points_num, efanna2e::L2, nullptr);

  auto s = std::chrono::high_resolution_clock::now();
  efanna2e::Parameters paras;
  paras.Set<unsigned>("L", L);
  paras.Set<unsigned>("R", R);
  paras.Set<unsigned>("C", C);
  paras.Set<std::string>("nn_graph_path", nn_graph_path);

  std::cout << "start building" << std::endl;
  index.Build(points_num, data_load, paras);
  auto e = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = e - s;

  std::cout << "indexing time: " << diff.count() << "\n";
  index.Save(argv[6]);
  delete[] data_load;

  return 0;
}
