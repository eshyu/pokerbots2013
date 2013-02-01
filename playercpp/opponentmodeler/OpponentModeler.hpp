#ifndef __OPPONENTMODELER_HPP__
#define __OPPONENTMODELER_HPP__

#include "../evaluator/CardHeuristics.hpp"

#include "nn.hpp"

#include <string>
#include <iostream>

#define NUM_ROUNDS 4
#define NUM_PLAYERS 2
#define POSITIONS 2

#define NUM_ACTIONS 7
#define NUM_HAND_TYPES 10 

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

    int numCheck[NUM_ROUNDS];
    int numBet[NUM_ROUNDS];
    int numRaise[NUM_ROUNDS];
    int numCall[NUM_ROUNDS];    
    int numFold[NUM_ROUNDS];

    int numWonShowdowns; //tells us how good the other play is
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
    int round;
    bool hasCheck[NUM_PLAYERS][NUM_ROUNDS];
    bool hasBet[NUM_PLAYERS][NUM_ROUNDS];
    bool hasRaise[NUM_PLAYERS][NUM_ROUNDS];

    float mybEq[NUM_ROUNDS];

    float oppBets[NUM_ROUNDS];
    float totalBets[NUM_ROUNDS];

    int myPreviousBetAmount;
  };

  /// HAND DISTRIBUTION TABLE FOR ACTION
  int HandDistribution[NUM_ACTIONS][NUM_HAND_TYPES];

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

  // updating stats
  void updateActionStatistics(int myAction, int betAmnt,
			      const std::vector<OppActionInfo> &oppActions,
			      bool myButton,
			      int potSize,
			      int stackSize,
			      const std::vector<std::string> &boardCards,
			      const std::vector<std::string> &holeCards);
  
  void updateOpponentActionStatistics(const std::vector<OppActionInfo> &oppActions,
				      bool myButton,
				      int potSize,
				      int stackSize,
				      const std::vector<std::string> &boardCards,
				      const std::vector<std::string> &holeCards);

  void updateOurActionStatistics(int myAction, int betAmnt,
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

  void updateShow(const std::vector<std::string> &ourCards,
		  const std::vector<std::string> &oppCards,
		  std::vector<std::string> boardCards,
		  const std::string &myDiscard);

  void newHand();

  void printStats();

  static ROUND numBoardCards2round(int numBoardCards);  
private:
  
  int oppActionCount, totalActions, handCount, trained,numMse, predicted; 
	float *output;
	float mse, sumMse,averageMse;
};

#endif  // __OPPONENTMODELER_HPP__
