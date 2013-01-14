#include <iostream>
#include "player.hpp"

#include <string>

#include <boost/lexical_cast.hpp>

Player::Player() {
}

/* current hand state vriables */
bool myButton;
float myTimeBank;
int myHandId, myBankroll, oppBankroll;
int potSize, numBoardCards;
std::string holeCard1, holeCard2, holeCard3;

/* Pokerbot execution loop */
void Player::run(tcp::iostream &stream) 
{
  std::string line, packet_type, tmp, button_str;
  ActionSelector::ActionInfo nextAction;

  while (std::getline(stream, line)) {
    // print packet for debugging   
    std::cout << line << "\n";

    std::stringstream ss(line); 
    ss >> packet_type;

    if (!packet_type.compare("GETACTION")) {
      // get next action from action selector, write to stream

      nextAction = actionSelector.getAction(line, holeCard1, holeCard2, holeCard3, 
					    myButton, STACK_SIZE);      
      std::string action = action2str(nextAction);
      std::cout << "action: " << action;
      stream << action;

    } else if (!packet_type.compare("NEWHAND")){
      // update info for new hand
      ss >> myHandId >> button_str >> holeCard1 >> holeCard2 >> holeCard3 >> myBankroll >> oppBankroll >> myTimeBank;
      myButton = !button_str.compare("true");
      
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

} // end void Player::run(...)

std::string Player::action2str(const ActionSelector::ActionInfo &info){
  std::cout << "hello thar\n";
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
    return std::string("DISCARD:" + std::string(1, info.cardNum) + std::string(1, info.cardSuit) + "\n");
    break;
  case ActionSelector::FOLD:
    return std::string("FOLD\n");
    break;
  case ActionSelector::RAISE:
    return std::string("RAISE:" + boost::lexical_cast<std::string>(info.betAmount) + "\n");
    break;
  }
}
  
