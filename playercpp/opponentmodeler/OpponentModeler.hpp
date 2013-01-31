#ifndef __OPPONENTMODELER_HPP__
#define __OPPONENTMODELER_HPP__

#include "../evaluator/CardHeuristics.hpp"
#include "nn.hpp"

#include <string>
#include <iostream>

#define NUM_ROUNDS 4
#define NUM_PLAYERS 2
#define POSITIONS 2

#define MAX_ACTIONS 3000

class OpponentModeler {
public:
  OpponentModeler();
  ~OpponentModeler();
  
  /* **Aggregate** Statistics for each betting round aggregate over all hands 
     Positions 0 (no button), 1 (button)
     Round: 0 (pre-flop), 1 (flop), 2 (turn), 3(river) */
  struct PlayerStatistics {    
    int numHandsPlayed;
    
    // min / max raises 
    // TODO: for now treat raises and bets the same
    
    // pre-flop button

    // pre-flop no-button   

    // total bet/raise (to compute average)

    // probably useful against maniac players like us

    int numCheck[NUM_ROUNDS];
    int numBet[NUM_ROUNDS];
    int numRaise[NUM_ROUNDS];
    int numCall[NUM_ROUNDS];    
    int numFold[NUM_ROUNDS];

    /*    int numAllin[POSITIONS][NUM_ROUNDS]; //number of occurences of all-in

    float foldEquity[POSITIONS][NUM_ROUNDS]; // their equity when they fold 
    float raiseEquity[POSITIONS][NUM_ROUNDS]; // their equity when they raise
    float betEquity[POSITIONS][NUM_ROUNDS]; // their equity when they bet
    // action counts
    int numBet[POSITIONS][NUM_ROUNDS];
    int numCall[POSITIONS][NUM_ROUNDS]; 
    int numCheck[POSITIONS][NUM_ROUNDS]; 
    int numFold[POSITIONS][NUM_ROUNDS]; 
    int numRaise[POSITIONS][NUM_ROUNDS];    

    int numCallFold[POSITIONS][NUM_ROUNDS]; // number of time opponent call and then folds to our raise

    */
    PlayerStatistics(){
      numHandsPlayed=0;
      for (int j=0;j<NUM_ROUNDS;++j){
	numCheck[j]=0;
	numBet[j]=0;
	numRaise[j]=0;
	numCall[j]=0;
	numFold[j]=0;
      }
    }
  };	


  enum ROUND {PREFLOP, FLOP, TURN, RIVER};
  enum ACTION {BET, CALL, CHECK, DISCARD, FOLD, RAISE, NONE};  

  struct OppActionInfo {
    ACTION action;
    int betAmount; //only for BET, RAISE
    ROUND round; //round nubmer for this actioS
    double mybEq;
  };

  
  // stats for one hand to train neural nets or other learning algorithms
  struct HandStatistics {
    //bool myButton, show, iFolded;
    int round;
    // this is just for our reference later, learning algorithm doesn't use this
    //std::string myHand;
    //std::string oppHand;

    //int totalRaiseVsPot[NUM_ROUNDS]; // total of raises (with "pots" as units)    
    // int totalRaiseVsBB[NUM_ROUNDS]; // total of raises (with "BB" as units)
    
    bool hasCheck[NUM_PLAYERS][NUM_ROUNDS];
    bool hasBet[NUM_PLAYERS][NUM_ROUNDS];
    bool hasRaise[NUM_PLAYERS][NUM_ROUNDS];

    float mybEq[NUM_ROUNDS];

    float oppBets[NUM_ROUNDS];
    float totalBets[NUM_ROUNDS];

    /*
    // these are only non-zero if hand was a show
    float opponentImpliedEquity[NUM_ROUNDS]; //using pbots_calc with opphand:XXX
    float opponentTrueEquity[NUM_ROUNDS]; // using opphand:ourhand
    */
  };

  NeuralNet *nn;

  float flopHandDistr[1755];
  float postFlopHandDistr[169];

  // me = 0, opponent = 1
  PlayerStatistics *playerStats[NUM_PLAYERS];
  HandStatistics currentHand;

  // CIRCULAR buffer features and outputs
  float **NNFeatures, **NNOut;
  
  void updatebEq(float mybEq, ROUND round);
	float calc_MSE(float * nnoutput, float * trueoutput, int num_output);
  void updateActionStatistics(int myAction, int betAmnt,
			      const std::vector<OppActionInfo> &oppActions,
			      bool myButton,
			      int potSize,
			      int stackSize,
			      const std::vector<std::string> &boardCards,
			      const std::vector<std::string> &holeCards);

  // print summary of opponent statistics
  void printSummary();

  // mess to create NN input
  void createActionNNFeatures(ROUND round, bool myButton, float mybEq, int prevBet, int stackSize, int potSize,
			      const std::vector<std::string> &myHand, 
			      const std::vector<std::string> &boardCards,
			      int oppActionNumber);
  void updateNNOut(ACTION oppAction, float normalizedOppBet);
    
  void updateHandStats(int playerNumber, ACTION action, ROUND round);
  void newHand();

  void printStats();

  static ROUND numBoardCards2round(int numBoardCards);  
private:
  
  int oppActionCount, totalActions, handCount, trained; 
	float *output;
	float mse;
};

#endif  // __OPPONENTMODELER_HPP__
