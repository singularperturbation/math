#include <stan/math.hpp>
#include <stan/math/prim/arr/functor/mpi_command.hpp>
#include <stan/math/prim/arr/functor/mpi_worker.hpp>
#include <stan/math/rev/arr/functor/map_rect_mpi.hpp>
#include <boost/mpi.hpp>
#include <boost/shared_ptr.hpp>

#include <iostream>

struct hard_work {
  template<typename T>
  std::vector<T> operator()(std::vector<T> theta, std::vector<double> x_r, std::vector<int> x_i) const {
    std::vector<T> res(2);
    res[0] = theta[0]*theta[0];
    res[1] = x_r[0]*theta[1]*theta[0];
    return(res);
  }
};

BOOST_CLASS_EXPORT(stan::math::internal::mpi_distribute_map_rect_data);

BOOST_CLASS_EXPORT(stan::math::internal::run_distributed_map_rect<hard_work>);


int main(int argc, const char* argv[]) {
  boost::mpi::environment env;
  boost::mpi::communicator world;
  using std::vector;

  stan::math::internal::distributed_map_rect_data data3(3);
  std::cout << "created data3 with uid = " << data3.uid_ << std::endl;

  stan::math::internal::distributed_map_rect_flyweight data4(4);

  std::cout << "created data4 with uid = " << data4.get().uid_ << std::endl;

  data4 = data3;
  
  std::cout << "data4 after reassignment = " << data4.get().uid_ << std::endl;

  //std::cout << "created data4 with uid = " << data4.get().uid_ << std::endl;
  
  // on non-root processes this makes the workers listen to commands
  // send from the root
  stan::math::mpi_worker worker;

  std::cout << "Root process starts distributing work..." << std::endl;

  // create a task to be distributed
  vector<vector<stan::math::var> > theta;

  const std::size_t N = 10;

  for(std::size_t n = 0; n != N; ++n) {
    vector<stan::math::var> theta_run(2);
    theta_run[0] = n;
    theta_run[1] = n*n;
    theta.push_back(theta_run);
  }

  vector<vector<double> > x_r(N, vector<double>(1,1.0));
  vector<vector<int> > x_i(N, vector<int>(0));

  std::cout << "Distributing the data to the nodes..." << std::endl;
  
  const std::size_t uid = stan::math::internal::distribute_map_rect_data(theta[0].size(), x_r, x_i);

  // TODO: make the actual functor use the distribute thing
  hard_work f;

  vector<stan::math::var> res = stan::math::map_rect_mpi(f, theta, x_r, x_i);
  
  std::cout << "Root process ends." << std::endl;
  
  return(0);
}