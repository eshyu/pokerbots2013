#ifndef __OPPONENTMODELER_HPP__
#define __OPPONENTMODELER_HPP__

#include "../evaluator/CardHeuristics.hpp"

#include <string>

#define NUM_ROUNDS 4
#define NUM_PLAYERS 2
#define POSITIONS 2

class OpponentModeler {
public:
  OpponentModeler();
  ~OpponentModeler();

  CardHeuristics *cardHeuristics;
  
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

    /*
o    int numAllin[POSITIONS][NUM_ROUNDS]; //number of occurences of all-in

    double foldEquity[POSITIONS][NUM_ROUNDS]; // their equity when they fold 
    double raiseEquity[POSITIONS][NUM_ROUNDS]; // their equity when they raise
    double betEquity[POSITIONS][NUM_ROUNDS]; // their equity when they bet
    
    // action counts
    int numBet[POSITIONS][NUM_ROUNDS];
    int numCall[POSITIONS][NUM_ROUNDS]; 
    int numCheck[POSITIONS][NUM_ROUNDS]; 
    int numFold[POSITIONS][NUM_ROUNDS]; 
    int numRaise[POSITIONS][NUM_ROUNDS];    

    int numCallFold[POSITIONS][NUM_ROUNDS]; // number of time opponent call and then folds to our raise

    */
  };	
  
  // stats for one hand to train neural nets or other learning algorithms
  struct HandStatistics {
    bool myButton, show, iFolded;
    int round, maxRound;

    // this is just for our reference later, learning algorithm doesn't use this
    std::string myHand;
    std::string oppHand;

    //int totalRaiseVsPot[NUM_ROUNDS]; // total of raises (with "pots" as units)    
    // int totalRaiseVsBB[NUM_ROUNDS]; // total of raises (with "BB" as units)
    
    bool hasCheck[NUM_PLAYERS][NUM_ROUNDS];
    bool hasBet[NUM_PLAYERS][NUM_ROUNDS];
    bool hasRaise[NUM_PLAYERS][NUM_ROUNDS];

    double mybEq[NUM_ROUNDS];

    int potSize[NUM_ROUNDS];

    /*
    // these are only non-zero if hand was a show
    double opponentImpliedEquity[NUM_ROUNDS]; //using pbots_calc with opphand:XXX
    double opponentTrueEquity[NUM_ROUNDS]; // using opphand:ourhand
    */
  };

  
  enum ROUND {PREFLOP, FLOP, TURN, RIVER};
  enum ACTION {BET, CALL, CHECK, DISCARD, FOLD, RAISE, NONE};


  double flopHandDistr[1755];
  double postFlopHandDistr[169];

  // me = 0, opponent = 1
  PlayerStatistics playerStats[NUM_PLAYERS];
  HandStatistics currentHand;
  
  // for neural net ..
  struct ActionNNFeatures {
    double features[27];
    double out[5];
  };
  typedef struct ActionNNFeatures ActionNNFeatures;

  std::vector<ActionNNFeatures> neuralnetData;
  
  
  void updateActionStatistics(int myAction, int betAmnt,
			      int oppAction, int oppBetAmnt,
			      bool myButton,
			      int potSize,
			      int stackSize,
			      const std::vector<std::string> &boardCards,
			      const std::vector<std::string> &holeCards);

  // print summary of opponent statistics
  void printSummary();

  // mess to create NN input
  void createActionNNFeatures(ROUND round, bool myButton, double mybEq, int prevBet, int stackSize, int potSize,
			      const std::vector<std::string> &myHand, 
			      const std::vector<std::string> &boardCards,
			      ActionNNFeatures &features);

private:
    ROUND numBoardCards2round(int numBoardCards);
};

#endif  // __OPPONENTMODELER_HPP__
