/*
 *
 *  Fast Artificial Neural Network (fann) C++ Wrapper Sample
 *
 *  C++ wrapper XOR sample with functionality similar to xor_train.c
 *
 *  Copyright (C) 2004-2006 created by freegoldbar (at) yahoo dot com
 *
 *  This wrapper is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This wrapper is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "floatfann.h"
#include "fann_cpp.h"

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



// Callback function that simply prints the information to cout
int print_callback(FANN::neural_net &net, FANN::training_data &train,
    unsigned int max_epochs, unsigned int epochs_between_reports,
    float desired_error, unsigned int epochs, void *user_data)
{
    cout << "Epochs     " << setw(8) << epochs << ". "
         << "Current Error: " << left << net.get_MSE() << right << endl;
    return 0;
}

//Given an input, get output from net
fann_type* run_net(FANN::neural_net &net, fann_type * arr){

    return net.run(arr);	

}

//Creates a neural net with 1 hidden layer
FANN::neural_net create_net(float learning_rate, unsigned int num_layers,unsigned int num_input, unsigned int num_hidden, unsigned int num_output){

    FANN::neural_net net;
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

    return net;
}

// Train the net with all the data
void train_net(FANN::neural_net &net,std::string oppName, std::string day, std::string type, const int num_output)
//void train_net(FANN::neural_net &net,unsigned int num_data, unsigned int num_input, fann_type **input,unsigned int num_output,fann_type **output)
{
  

    const float desired_error = 0.001f;
    const unsigned int max_iterations = 1000;
    const unsigned int iterations_between_reports = 1000;

    // Output network type and parameters
    /*
    cout << endl << "Network Type                         :  ";
    switch (net.get_network_type())
    {
    case FANN::LAYER:
        cout << "LAYER" << endl;
        break;
    case FANN::SHORTCUT:
        cout << "SHORTCUT" << endl;
        break;
    default:
        cout << "UNKNOWN" << endl;
        break;
    }
    */
    //net.print_parameters();
    std::string trainFileName="input/"+type+"_Casino_Day-"+day+"_"+oppName+"_vs_mybotisamazing.txt";
    std::string valFileName="input/"+type+"_Casino_Day-"+day+"_mybotisamazing_vs_"+oppName+".txt";

    cout << endl << "Training network." << endl;

    FANN::training_data data;
    FANN::training_data vData;
    data.read_train_from_file(trainFileName);
    vData.read_train_from_file(valFileName);

    // Initialize and train the network with the data
    net.init_weights(data);

    cout << "Max Epochs " << setw(8) << max_iterations << ". "
        << "Desired Error: " << left << desired_error << right << endl;
    net.set_callback(print_callback, NULL);
    net.train_on_data(data, max_iterations,
        iterations_between_reports, desired_error);

    cout << endl << "Testing network." << endl;
    std::string oFileName="output/"+type+"_"+oppName+"_day_"+day+".txt";
    std::string actionFile="input/action_Casino_Day-"+day+"_mybotisamazing_vs_"+oppName+".txt";
    ofstream outputFile(oFileName.c_str());
    ifstream finAction(actionFile.c_str());
    for (unsigned int i = 0; i < vData.length_train_data(); ++i)
    {
        // Run the network on the test data
      fann_type * input=vData.get_input()[i];
        fann_type *calc_out = net.run(input);
	std::string line;
	getline(finAction, line);
	outputFile<< "***********"<<endl;
	if((int)input[0]==1){
	  outputFile<<"PREFLOP"<<endl;
	}else if((int)input[1]==1){
	  outputFile<<"FLOP"<<endl;
	}else if((int)input[2]==1){
	  outputFile<<"TURN"<<endl;
	}else if((int)input[3]==1){
	  outputFile<<"RIVER"<<endl;
	}
	outputFile<<line<<endl;
	outputFile<<"nn output"<<endl;
	for(int k=0; k<num_output; k++){
	  outputFile<<calc_out[k]<<" ";
	}
	outputFile<<endl;
	for(int j=0; j<num_output;j++){
	  outputFile<<vData.get_output()[i][j]<<" ";
	}
	outputFile<<endl;
	   //<< "difference = " << noshowpos
	   //<< fann_abs(*calc_out - data.get_output()[i][0]) << endl;
	
    }
    outputFile.close();
    
    //cout << endl << "Saving network." << endl;

    // Save the network in floating point and fixed point
    std::string netFile="output/"+type+"_"+oppName+"_day_"+day+".net";
    net.save(netFile);
    //unsigned int decimal_point = net.save_to_fixed("training.net");
    //data.save_train_to_fixed("training_fixed.data", decimal_point);
	
}
/*
//after we have a neural net, we can find the input that would maximize our EV
int max_ev(FANN::neural_net &net,float *input,int num_inputs, float callMin, float betMax){
  //for EV calculations

  float *next_move=get_next_moves(callMin,betMax);
  float num_moves=next_move[0];
  float best_move=0;
  float best_ev=-800;
  for(int n=1; n<=num_moves;n++){
    int move=next_move[n];
    float *i=get_input(next_move,input,num_inputs);
    float *move_distribution=net.run(i);
    float ev=calc_ev(next_move,move_distribution);
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
float calc_ev(int next_move,float * move_distribution){
  //last entry in move distribution is equity
  float oppEquity=move_distribution[5];
  
  
}

*/
/* Startup function. Syncronizes C and C++ output, calls the test function
   and reports any exceptions */
int main(int argc, char **argv)
{
    try
    {
        std::ios::sync_with_stdio(); // Syncronize cout and printf output
	
	std::string oppName="Poseidon";
	std::string day="7";
	const float learning_rate = 0.07f;
	const unsigned int num_layers=3;
	
	const unsigned int num_input_dist = 43;
	const unsigned int num_hidden_dist = num_input_dist/2;
	const unsigned int num_output_dist = 5;

	const unsigned int num_input_eq = 43;
	const unsigned int num_hidden_eq = num_input_eq/2;
	const unsigned int num_output_eq = 1;
	
	FANN::neural_net net_dist=create_net(learning_rate,num_layers,num_input_dist,num_hidden_dist,num_output_dist);
	FANN::neural_net net_eq=create_net(learning_rate,num_layers,num_input_eq,num_hidden_eq,num_output_eq);

	train_net(net_dist,oppName,day,"dist",num_output_dist);
	train_net(net_eq,oppName,day,"equity",num_output_eq);
	//float *output=run_net(net,a);
	//cout<< endl<<  output[0]<<" and "<< output[1]<<endl;

    }
    catch (...)
    {
        cerr << endl << "Abnormal exception." << endl;
    }
    return 0;
}





/******************************************************************************/
