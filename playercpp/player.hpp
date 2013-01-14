#ifndef __PLAYER_HPP__
#define __PLAYER_HPP__

#include <boost/asio.hpp>

#include "actionselector/ActionSelector.hpp"

using boost::asio::ip::tcp;

class Player {
public:
  Player();
  void run(tcp::iostream &stream);
  
private:
  ActionSelector actionSelector;
  
  /* match parameters */
  int STACK_SIZE, BIG_BLIND, NUM_HANDS;
  float TIME_BANK;
  std::string MY_NAME, OPP_NAME;

  // converts actioninfo struct to string to write to tcp
  std::string action2str(const ActionSelector::ActionInfo &info);

};

#endif  // __PLAYER_HPP__
