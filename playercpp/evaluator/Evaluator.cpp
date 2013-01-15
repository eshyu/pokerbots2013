#include "Evaluator.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>

extern "C" {
  //#include "pbots_calc/export/linux2/include/pbots_calc.h"
  #include "../export/linux2/include/pbots_calc.h"
}

// number of simulations
int NUM_SIMULATIONS_FAST = 100000;
int NUM_SIMULATIONS_SLOW = 100000;

int numHoleCards;

Evaluator::Evaluator(){}

Evaluator::~Evaluator(){}

/* evaluate current hand given board cards and discard  */
//  TODO: use opponent hand distribution or implied equity 
double Evaluator::evaluate(const std::vector<std::string> &holeCards,
			   const std::vector<std::string> &boardCards,
			   const std::string &myDiscard)
{   
  BETTING_ROUND round;
  int NUM_SIMULATIONS;
  // preflop equity table
  switch (boardCards.size()){
  case 0:
    // these are already precomputed
    return preFlopEquities[threeCards2index(holeCards[0], holeCards[1], holeCards[2])];
  case 3:
    round = FLOP;
    break;
  case 4:
    round = TURN;
    break;
  case 5:
    round = RIVER;
    break;
  }

  // do not recompute if already computed for this round
  if (memoizedEquities[round]){
    return memoizedEquities[round];
  }

  std::string calcString="";
  numHoleCards = holeCards.size();
  if (numHoleCards == 2){
    calcString = holeCards[0] + holeCards[1] + ":xx";
    NUM_SIMULATIONS = NUM_SIMULATIONS_FAST;
  } else if (numHoleCards == 3){
    calcString = holeCards[0] + holeCards[1] + holeCards[2] + ":xxx";
    NUM_SIMULATIONS = NUM_SIMULATIONS_SLOW;
  } else {
    std::cout << "Evaluator.cpp:L24: ERROR: number of hole cards is not 2 or 3!!" << std::endl;
  }
  
  // create board c_string
  std::string boardString;
  char board_c_str[boardString.size()+1];
  for (int i=0;i<boardCards.size();i++) boardString = boardString + boardCards[i];  
  for (int i=0;i<boardString.size();++i) board_c_str[i] = boardString[i];
  board_c_str[boardString.size()] = '\0';

  // create discard c_string
  char discard_c_str[myDiscard.size()+1];
  for (int i=0;i<myDiscard.size(); i++) discard_c_str[i] = myDiscard[i];
  discard_c_str[myDiscard.size()] = '\0';
  
  std::cout << "calcstring: " << calcString << " | " << "boardString: " << boardString << "myDicsard: " << myDiscard << std::endl;

  Results *result = alloc_results();

  calc(calcString.c_str(), board_c_str, discard_c_str, NUM_SIMULATIONS, result);

  double myEv = result->ev[0]; // opponent equity from this calculator is 1-ours

  // make sure to free result
  free_results(result);

  // memoize
  memoizedEquities[round] = myEv;

  return myEv;

 }

double Evaluator::evaluate_discard_pairs(const std::string &holeCards,
					 const std::string &boardCards,
					 const std::string &myDiscard)
{   
  int NUM_SIMULATIONS = 10000;

  std::string calcString=holeCards+":xx";

  char board_c_str[boardCards.size()+1];
  for (int i=0;i<boardCards.size();++i) board_c_str[i] = boardCards[i];
  board_c_str[boardCards.size()] = '\0';

  // create discard c_string
  char discard_c_str[myDiscard.size()+1];
  for (int i=0;i<myDiscard.size(); i++) discard_c_str[i] = myDiscard[i];
  discard_c_str[myDiscard.size()] = '\0';
  
  std::cout << "calcstring: " << calcString << " | " << "boardCards: " << boardCards << "myDicsard: " << myDiscard << std::endl;

  Results *result = alloc_results();

  calc(calcString.c_str(), board_c_str, discard_c_str, NUM_SIMULATIONS, result);

  double myEv = result->ev[0]; // opponent equity from this calculator is 1-ours

  // make sure to free result
  free_results(result);

  return myEv;

 }

/* This should be called by main() during set-up to initialize pre-flop equities*/
void Evaluator::populatePreFlopTable()
{
  std::ifstream fin("data/preflop.in");
  int index;
  std::string hand;
  double ev;

  std::cout << "Initializing pre-flop equity table..." << std::endl;
  
  for (int i=0;i<1755;i++){
    fin >> index >> hand >> ev;
    preFlopEquities[i] = ev;    
  }

  std::cout << "Done." << std::endl;

}

void Evaluator::clearMemoizedEquities(){
  for (int i=0;i<3;i++) memoizedEquities[i]=0;
}

/* Enumeration goes like this:
   0-285: 1-suit hands (286) 
     A23s, A24s, ... A34s, A35s, ... JQKs
   
   286-1299: 2-suit hands (1014)
     AoA2s, AoA3s, ... AoQKs, 1oA2s, ... KoQKs

   1300-1754: 3-suit hands (455)
     AAAo, AA2o, ... AQKo, AKKo, 222o, 223o, ... KKKo

   A=1, 2=2, ... T=10, J=11, Q=12, K=13
*/
int Evaluator::threeCards2index(const std::string &holeCard1, 
				const std::string &holeCard2,
				const std::string &holeCard3)
{
  char suit1=holeCard1[1], suit2=holeCard2[1], suit3=holeCard3[1];
  int tmp = (int)(suit1!=suit2) + (int)(suit1!=suit3) + (int)(suit2!=suit3);
  int numSuits = tmp == 0 ? 1 : tmp;
  
  int card1=cardNum2int(holeCard1[0]), 
      card2=cardNum2int(holeCard2[0]),
      card3=cardNum2int(holeCard3[0]);
  int a = std::min(std::min(card1, card2), card3);
  int c = std::max(std::max(card1, card2), card3);
  int b = (card1+card2+card3)-(a+c);

  int l, k;
  switch(numSuits){
  case 1:
    l=14-a;
    k=13-a;    
    return (b-a-1)*(2*l-b+a-2)/2+(c-b)+13*12*11/6-l*(l-1)*(l-2)/6-1;
  case 2:
    return 286+a*13*6-0.5*(13-b)*(13-b+1) + (c-b) - 1;
  case 3:
    return 286+1014+15*14*13/6 -(16-a)*(15-a)*(14-a)/6 + (13-a+1)*(13-a+2)/2-((13-a+1)-(b-a))*((13-a+1)-(b-a)+1)/2 + (c-b+1) - 1;
  }

  std::cout << "Evaluator.cpp:L101 INVALID NUMSUITS??\n";

  return -1;
}

int Evaluator::cardNum2int(char cardNum){
  switch (cardNum){
  case 'A':
  case 'a':
    return 1;
  case '2':
    return 2;
  case '3':
   return 3;
  case '4':
    return 4;
  case '5':
    return 5;
  case '6':
    return 6;
  case '7':
    return 7;
  case '8':
   return 8;
  case '9':
    return 9;
  case 'T':
  case 't':
    return 10;
  case 'J':
  case 'j':
    return 11;
  case 'Q':
  case 'q':
    return 12;
  case 'K':
  case 'k':
    return 13;
  default:
    std::cout << "Evalulator.cpp:L131 INVALID CARD NUMBER\n";
    return -1;
  }   
  
}
