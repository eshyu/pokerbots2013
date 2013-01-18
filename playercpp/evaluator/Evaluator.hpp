#ifndef __EVALUATOR_HPP__
#define __EVALUATOR_HPP__

#include <string>
#include <vector>

class Evaluator {
public:
  Evaluator();
  ~Evaluator();

  // current betting round
  enum BETTING_ROUND {FLOP, TURN, RIVER};  

  // pre-compute pre-flop equities for all starting hands
  double preFlopEquities[1755];

  // memoize the hand equity for the flop, turn, river
  double memoizedEquities[3];
  
  // memoize the equity for discard pairs
  double memoizedHandPairs[3];

  // populate preflop equity table
  void populatePreFlopTable();
  
  // clear memoized equities tables
  void clearMemoizedEquities();

  // todo: use opponent hand distribution
  /* evaluates opponents hand using pbots_calc */
  double evaluate(const std::vector<std::string> &holeCards,
		  const std::vector<std::string> &boardCards,
		  const std::string &myDiscard,
		  int manualNumSimulations=0,
		  bool memoize=true);

  double memoized_evaluate_pairs(const std::string &holeCards,
				 const std::string &boardCards,
				 const std::string &myDiscard,
				 int discardIdx);

  // simple version of evaluate function for evaulating discard pairs
  double evaluate_pairs(const std::string &holeCards,
				const std::string &boardCards,
				const std::string &myDiscard);

  // for evaluation when we know both our and opponent cards
  double evaluate_true(const std::vector<std::string> &myHoleCards,
		       const std::vector<std::string> &oppHoleCards,
		       const std::vector<std::string> &boardCards,
		       const std::string &myDiscard,
		       const std::string &oppDiscard);

  /* maps a 3-card hand to index into one of 1655 equivalent classes*/
  int threeCards2index(const std::string &holeCard1, const std::string &holeCard2,
		       const std::string &holeCard3);

  int cardNum2int(char cardNum);
};

#endif  // __EVALUATOR_HPP__
