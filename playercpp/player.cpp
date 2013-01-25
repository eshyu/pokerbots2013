#include <iostream>
#include "player.hpp"

#include <string>

#include <boost/lexical_cast.hpp>

#include <ctime>
#include <cstdlib>

Player::Player() {
}

/* current hand state vriables */
bool myButton;
float myTimeBank;
int myHandId, myBankroll, oppBankroll;
int potSize, numBoardCards;
std::vector<std::string>  holeCards;
std::string myDiscard="";

/* Pokerbot execution loop */
void Player::run(tcp::iostream &stream) 
{
  std::string line, packet_type, tmp, button_str;
  std::string holeCard1, holeCard2, holeCard3;
  
  evaluator = new Evaluator();
  evaluator->populatePreFlopTable(); //initialize preflop equities
  actionSelector = new ActionSelector(evaluator);
  srand(time(NULL));

  ActionSelector::ActionInfo nextAction;  
  while (std::getline(stream, line)) {
    // print packet for debugging   
    std::cout << "(" << line << ")\n";

    std::stringstream ss(line); 
    ss >> packet_type;

    if (!packet_type.compare("GETACTION")) {
      // get next action from action selector, write to stream

      nextAction = actionSelector->getAction(line, holeCards, myDiscard,
					    myButton, STACK_SIZE);      
      std::string action = action2str(nextAction);
      std::cout << "action: " << action;
      stream << action;

    } else if (!packet_type.compare("NEWHAND")){
      // update info for new hand
      ss >> myHandId >> button_str >> holeCard1 >> holeCard2 >> holeCard3 >> myBankroll >> oppBankroll >> myTimeBank;
      newHand(holeCard1, holeCard2, holeCard3, button_str);
    } else if (!packet_type.compare("HANDOVER")){
      // TODO: ...
      
    } else if (!packet_type.compare("REQUESTKEYVALUES")){
      // TODO ...
      stream << "FINISH\n";

    } else if (!packet_type.compare("KEYVALUE")){
      // TODO ...

    } else if (!packet_type.compare("NEWGAME")){
      // set game params
      ss >> MY_NAME >> OPP_NAME >> STACK_SIZE >> BIG_BLIND >> NUM_HANDS >> TIME_BANK;
    } 

  } // end while(std::getline ...)       
  
  std::cout << "Gameover, engine disconnected.\n";

  delete actionSelector;
  delete evaluator;

} // end void Player::run(...)

/* resets hand state variables after new hand*/
void Player::newHand(const std::string &holeCard1, const std::string& holeCard2, const std::string &holeCard3, const std::string &button_str)
{
  holeCards.clear();
  holeCards.push_back(holeCard1); holeCards.push_back(holeCard2); holeCards.push_back(holeCard3);
  myButton = !button_str.compare("true");
  myDiscard = std::string("");
  evaluator->clearMemoizedEquities();
}

/* Converts ActionInfo struct into string */
std::string Player::action2str(const ActionSelector::ActionInfo &info)
{
  switch(info.action){
  case ActionSelector::BET:
    return std::string("BET:" + boost::lexical_cast<std::string>(info.betAmount) + "\n");
    break;
  case ActionSelector::CALL:
    return std::string("CALL\n");
    break;
  case ActionSelector::CHECK:
    return std::string("CHECK\n");
    break;
  case ActionSelector::DISCARD:
    return std::string("DISCARD:" + myDiscard + "\n");
    break;
  case ActionSelector::FOLD:
    return std::string("FOLD\n");
    break;
  case ActionSelector::RAISE:
    return std::string("RAISE:" + boost::lexical_cast<std::string>(info.betAmount) + "\n");
    break;
  }
}
  
