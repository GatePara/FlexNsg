#include <efanna2e/index_nsg_alpha.h>
#include <efanna2e/util.h>

int main(int argc, char **argv)
{
  if (argc != 9)
  {
    std::cout << argv[0] << " <fvecs/fbin> data_file nn_graph_path L R C alpha save_graph_file"
              << std::endl;
    exit(-1);
  }

  std::string nn_graph_path(argv[3]);
  unsigned L = (unsigned)atoi(argv[4]);
  unsigned R = (unsigned)atoi(argv[5]);
  unsigned C = (unsigned)atoi(argv[6]);
  float Alpha = atof(argv[7]);
  std::cout << "===== Input Parameters =====" << std::endl;
  std::cout << "Data Type      : " << argv[1] << std::endl;
  std::cout << "Data File      : " << argv[2] << std::endl;
  std::cout << "NN Graph Path  : " << argv[3] << std::endl;
  std::cout << "L              : " << L << std::endl;
  std::cout << "R              : " << R << std::endl;
  std::cout << "C              : " << C << std::endl;
  std::cout << "Alpha          : " << Alpha << std::endl;
  std::cout << "Output Index   : " << argv[8] << std::endl;
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

  // data_load = efanna2e::data_align(data_load, points_num, dim);//one must
  // align the data before build
  efanna2e::IndexNSGAlpha index(dim, points_num, efanna2e::L2, nullptr);

  auto s = std::chrono::high_resolution_clock::now();
  efanna2e::Parameters paras;
  paras.Set<unsigned>("L", L);
  paras.Set<unsigned>("R", R);
  paras.Set<unsigned>("C", C);
  paras.Set<float>("ALPHA", Alpha);
  paras.Set<std::string>("nn_graph_path", nn_graph_path);

  std::cout << "start building" << std::endl;
  index.Build(points_num, data_load, paras);
  auto e = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = e - s;

  std::cout << "indexing time: " << diff.count() << "\n";
  index.Save(argv[8]);
  delete[] data_load;

  return 0;
}
