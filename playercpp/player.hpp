#ifndef __PLAYER_HPP__
#define __PLAYER_HPP__

#include <boost/asio.hpp>

#include "actionselector/ActionSelector.hpp"
#include "evaluator/Evaluator.hpp"
#include "opponentmodeler/OpponentModeler.hpp"

using boost::asio::ip::tcp;

class Player {
public:
  Player();

  void run(tcp::iostream &stream);
  
private:
  Evaluator *evaluator;
  ActionSelector *actionSelector;
  OpponentModeler *opponentModeler;

  /* match parameters */
  int STACK_SIZE, BIG_BLIND, NUM_HANDS;
  float TIME_BANK;
  std::string MY_NAME, OPP_NAME;

  // cleanup hand state paramters after new hand
  void newHand(const std::string &holeCard1, const std::string& holeCard2, const std::string &holeCard3, const std::string &button_str);

  // converts actioninfo struct to string to write to tcp
  std::string action2str(const ActionSelector::ActionInfo &info);

};

#endif  // __PLAYER_HPP__
