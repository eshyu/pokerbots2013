#include "nn.hpp"
/*
 *
 *
 */

#include "../../analysis/fann/src/include/floatfann.h"
#include "../../analysis/fann/src/include/fann_cpp.h"

#include <ios>
#include <iostream>
#include <fstream>
#include <iomanip>
using std::cout;
using std::cerr;
using std::endl;
using std::setw;
using std::left;
using std::right;
using std::showpos;
using std::noshowpos;
using std::ofstream;
using std::ifstream;


float * get_output(FANN::neural_net &net, float * input){
  return net.run(input);
}

// Train the net with all the data
FANN::neural_net FANN::neural_net train_net( int num_data, int num_input, float** input,int num_output, float ** output)
//void train_net(FANN::neural_net &net,unsigned int num_data, unsigned int num_input, fann_type **input,unsigned int num_output,fann_type **output)
{
    FANN::neural_net net;
    const float learning_rate = 0.07f;
    const unsigned int num_layers=3;
    int num_hidden=num_input;
    unsigned int layers[3]={num_input,num_hidden,num_output};
    net.create_standard_array(num_layers,layers); 
    net.set_learning_rate(learning_rate);
  
    net.set_activation_steepness_hidden(.1);
    net.set_activation_steepness_output(.1);
  
    net.set_activation_function_hidden(FANN::SIGMOID_SYMMETRIC_STEPWISE);
    net.set_activation_function_output(FANN::SIGMOID_SYMMETRIC_STEPWISE);

    //net.set_training_algorithm(FANN::TRAIN_INCREMENTAL);
    // Set additional properties such as the training algorithm
    //net.set_training_algorithm(FANN::TRAIN_QUICKPROP);

    const float desired_error = 0.001f;
    const unsigned int max_iterations = 1000;
    const unsigned int iterations_between_reports = 1000;

    FANN::training_data data;

    data.set_train_data(num_data, num_input, input, num_output, output);

    // Initialize and train the network with the data
    net.init_weights(data);

    net.train_on_data(data, max_iterations,0, desired_error);
    return net;

}
/*
//after we have a neural net, we can find the input that would maximize our EV
int max_ev(FANN::neural_net &net,vector<int> hand, vector<int> board,vector<float> input, float callMin, float betMax, float tEq, int round){
  //for EV calculations
  
  float best_move=0;
  float best_ev=-800;
  for(int n=1; n<=num_moves;n++){
    int move=next_move[n];
    float *i=get_input(next_move,input,num_inputs, hand,board);
    float *move_distribution=net.run(i);
    float ev=calc_ev(move_distribution, equity);
    if(ev>best_ev){
      best_ev=ev;
      best_move=n;
    }
  }
  return best_move;
  
  
}

float* get_next_moves(callMin,betMax){
  //fold is -1, check is 0, call is 1, bet/raise is  amount
  //betMin, 2xbetMin,..,betMax
  int num_bets;
  int num_options=2;//fold and check or call
  if (callMin>0){
    //no check
    int num_bets=(int)log(betMax/callMin)/log(2)+1;
  }
  else{
    int num_bets=(int)log(betMax/2)/log(2)+1
  }
  num_options=num_options+num_bets;
  float *next_moves[num_options+1];
  next_moves[0]=num_options;
  next_moves[1]=-1;
  if (callMin>0){
    next_moves[2]=1;
  }
  else{
    next_moves[2]=0;
  }
  for(int i=0; i<num_bets; i++){
    next_moves[i+3]=callMin*2^i;
  }
  return next_moves;   
}


//assume num of inputs is 19
float* get_input(int next_move, float * input){
  if
}
/*
move distribution:
float bet; [0,1]
float check; [0,1]
float call; [0,1]
float fold; [0,1]
double betSize (in terms of pot odds)
double oppTrueEquity
*/
/*
float calc_ev(float * dist, float equity, float callAmt){
  //move_distribution=[bet,call,check,fold,betAmt]
  float ev=0;
  float total=dist[0]+dist[1]+dist[2]+dist[3];
  ev+=(dist[0]*dist[4]*equity+dist[1]*callAmt*equity)/total;
  return ev;
}

*/
/* Startup function. Syncronizes C and C++ output, calls the test function
   and reports any exceptions */
int main(int argc, char **argv)
{
    try
    {
        std::ios::sync_with_stdio(); // Syncronize cout and printf output

    }
    catch (...)
    {
        cerr << endl << "Abnormal exception." << endl;
    }
    return 0;
}





/******************************************************************************/
