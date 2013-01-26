#ifndef __NN_HPP__
#define __NN_HPP__


class NeuralNet {

float * get_output(float * input);

// Train the net with all the data
FANN::neural_net train_net( int num_data, int num_input, float** input,int num_output, float ** output);
private:
  FANN::neural_net *net;
};
#endif  // __NN_HPP__
