#ifndef __NN_HPP__
#define __NN_HPP__

float * get_output(FANN::neural_net &net, float * input);

// Train the net with all the data
FANN::neural_net FANN::neural_net train_net( int num_data, int num_input, float** input,int num_output, float ** output);

#endif  // __NN_HPP__
