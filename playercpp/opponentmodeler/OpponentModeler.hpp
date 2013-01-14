#ifndef __OPPONENTMODELER_HPP__
#define __OPPONENTMODELER_HPP__

#include<string>

#define NUM_ROUNDS 4
#define POSITIONS 2

class OpponentModeler {
public:
  OpponentModeler();
  ~OpponentModeler();

  double flopHandDistr[1755];
  double postFlopHandDistr[169];
  
  /* **Aggregate** Statistics for each betting round aggregate over all hands 
     Positions 0 (no button), 1 (button)
     Round: 0 (pre-flop), 1 (flop), 2 (turn), 3(river) */
  struct PlayerStatistics {
    // to compute total hands played/raised preflop
    int numHands;
    int numPlayedHands;
    int numRaisedHands;	
    
    // min / max raises 
    // TODO: for now treat raises and bets the same
    int maxRaiseVsPot[POSITIONS][NUM_ROUNDS]; // in multiples of pot size
    int minRaiseVsPot[POSITIONS][NUM_ROUNDS]; // in multiples of pot size   
    int maxRaiseVsBB[POSITIONS][NUM_ROUNDS]; // in multiples of BB
    int minRaiseVsBB[POSITIONS][NUM_ROUNDS]; // in multiples of BB   
    
    // total bet/raise (to compute average)
    int totalRaiseVsPot[POSITIONS][NUM_ROUNDS]; // total of raises (with "pots" as units)    
    int totalRaiseVsBB[POSITIONS][NUM_ROUNDS]; // total of raises (with "BB" as units)
    
    // probably useful against maniac players like us
    int numAllin[POSITIONS][NUM_ROUNDS]; //number of occurences of all-in
    int numCallFold[POSITIONS][NUM_ROUNDS]; // number of time opponent call and then folds to our raise
    
    // action counts
    int numBet[POSITIONS][NUM_ROUNDS];
    int numCall[POSITIONS][NUM_ROUNDS]; 
    int numCheck[POSITIONS][NUM_ROUNDS]; 
    int numFold[POSITIONS][NUM_ROUNDS]; 
    int numRaise[POSITIONS][NUM_ROUNDS]; 
  };	
  
  // stats for one hand to train neural nets or other learning algorithms
  struct HandStatistics {
    bool myButton, show;
    int numRounds;

    // this is just for our reference later, learning algorithm doesn't use this
    std::string myHand;
    std::string oppHand;

    int totalRaiseVsPot[NUM_ROUNDS]; // total of raises (with "pots" as units)    
    int totalRaiseVsBB[NUM_ROUNDS]; // total of raises (with "BB" as units)

    int numBet[NUM_ROUNDS];
    int numCall[NUM_ROUNDS]; 
    int numCheck[NUM_ROUNDS]; 
    int numFold[NUM_ROUNDS]; 
    int numRaise[NUM_ROUNDS]; 

    double myEquity[NUM_ROUNDS];

    // these are only non-zero if hand was a show
    double opponentImpliedEquity[NUM_ROUNDS]; //using pbots_calc with opphand:XXX
    double opponentTrueEquity[NUM_ROUNDS]; // using opphand:ourhand
  };
  
  // nope will re-use some stuff for this
  void updateStatistics(const std::string &actionLine);

  // print summary of opponent statistics
  void printSummary();
};

#endif  // __OPPONENTMODELER_HPP__
