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

  // populate preflop equity table
  void populatePreFlopTable();
  
  // clear memoized equities table
  void clearMemoizedEquities();

  // todo: use opponent hand distribution
  /* evaluates opponents hand using pbots_calc */
  double evaluate(const std::vector<std::string> &holeCards,
				   const std::vector<std::string> &boardCards,
				   const std::string &myDiscard);

  // simple version of evaluate function for evaulating discard pairs
  double evaluate_discard_pairs(const std::string &holeCards,
				const std::string &boardCards,
				const std::string &myDiscard);

  /* maps a 3-card hand to index into one of 1655 equivalent classes*/
  int threeCards2index(const std::string &holeCard1, const std::string &holeCard2,
		       const std::string &holeCard3);

  int cardNum2int(char cardNum);
};

#endif  // __EVALUATOR_HPP__
