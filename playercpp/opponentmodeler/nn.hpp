#ifndef __NN_HPP__
#define __NN_HPP__

#include "../../analysis/fann/src/include/floatfann.h"
#include "../../analysis/fann/src/include/fann_cpp.h"

#include "../../analysis/fann/src/include/fann.h"

class NeuralNet {

public:
  NeuralNet();
  ~NeuralNet();

  float * get_output(float * input);
  
  // Train the net with all the data
  void train_net( int num_data, int num_input, float** input,int num_output, float ** output);
private:
  FANN::neural_net *net;
};
#endif  // __NN_HPP__
