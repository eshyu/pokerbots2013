#include "ActionSelector.hpp"

#include <vector>
#include <string>
#include <cstdlib>
#include <sstream>

#include <boost/algorithm/string.hpp>

//#include "../evaluator/..pbots_calc.h"

ActionSelector::ActionSelector(){}

ActionSelector::ActionInfo ActionSelector::getAction(const std::string &getaction_str, const std::string &holeCard1, const std::string &holeCard2, const std::string &holeCard3, bool myButton, int stackSize){
 
  LegalAction legalAction;
  ActionInfo actionInfo;

  int potSize, numBoardCards, numLastActions, numLegalActions;

  std::vector<std::string> boardCards;
  std::vector<std::string> lastActions;
  std::vector<std::string> legalActions;
  
  std::stringstream ss(getaction_str);
  std::string tmp;
  
  ss >> tmp >> potSize >> numBoardCards;

  packetlist2vector(ss, boardCards, numBoardCards);  
  ss >> numLastActions;
  packetlist2vector(ss, lastActions, numLastActions);      
  ss >> numLegalActions;

  legalAction = actionlist2struct(ss, numLegalActions, lastActions[numLastActions-1]);

  // TODO: hand evaluator, opponent modeler, betting selector
  evalMagic(potSize, myButton, holeCard1, holeCard2, holeCard3, legalAction, actionInfo);

  return actionInfo;
}

/* this will be replaced with something more sophisticated later */
void ActionSelector::evalMagic(int potSize, bool myButton,
	       std::string holeCard1, std::string holeCard2, std::string holeCard3,
	       const ActionSelector::LegalAction &legalAction, ActionSelector::ActionInfo &actionInfo){
  // TODO: .....
  //dumbass bot allin 25% of time, never raises except allin
  if (legalAction.actionType == DISCARD_ONE){
    actionInfo.action = DISCARD;
    
    //todo lol replace this with discard selector instead of this 
    std::string discard;
    int tricoin = rand() % 3;
    if (tricoin == 0) discard=holeCard1;
    if (tricoin == 1) discard=holeCard2;
    if (tricoin == 2) discard=holeCard3;
    actionInfo.cardNum=discard[0]; actionInfo.cardSuit=discard[1];
  } else {
    //  case CHECKFOLD_BET:   case FOLD_CALL_RAISE:              
    int coin = rand() % 4; //LOL
    int callMin = legalAction.callMin;
    
    std::cout << "coin is " << coin;

    if (coin == 1){
      actionInfo.action= (legalAction.actionType == CHECKFOLD_BET) ? BET : RAISE;
      actionInfo.betAmount=legalAction.raiseMax;
      std::cout << "raising to " << actionInfo.betAmount << std::endl;
    } else {

      // compute pot odds and either call or fold    
      double potOdds = (double)potSize/(callMin+potSize);
      std::cout << "myPotOdds: " << potOdds << std::endl;
      /* TODO: lol
	 std::string calcString = holeCard1+holeCard2+holeCard3+":"+"xxx"
	 double equity = calc(calc
      */
    double equity = 0.2; //TODO LOL
    
    //TODO: lol
    if (legalAction.callMin > 0){
      if (potOdds > equity){
	actionInfo.action=CALL;
      } else {
	actionInfo.action=FOLD;
      }
    } else {      
      actionInfo.action=CHECK;
    }
    
    }    
  }
}

void ActionSelector::packetlist2vector(std::stringstream &ss, std::vector<std::string> &packetlist, int length){
  std::string item;
  for (int i=0;i<length;i++){
    ss >> item;
    packetlist.push_back(item);
  }
}

ActionSelector::LegalAction ActionSelector::actionlist2struct(std::stringstream &ss, int length, std::string lastAction){

  LegalAction legalAction;
  std::vector<std::string> tokens;
  legalAction.actionType = CHECKFOLD_BET;    

  // get call amount from last action
  boost::split(tokens, lastAction, boost::is_any_of(":"));
  
  std::cout << "last action is: " << tokens[0] << std::endl;

  int callMin=0, raiseMin=0, raiseMax=0;
  if ((!tokens[0].compare("BET") || !tokens[0].compare("RAISE") ||
	!tokens[0].compare("POST"))){
    // cuz the call amount doesnt appear in the action..
    callMin = atoi(tokens[1].c_str());
    legalAction.actionType = FOLD_CALL_RAISE;
  } 

  /* TODO: lol this looks at all the actions to decide what to do*/
  std::string tmp;
  for (int i=0;i<length;i++){
    ss >> tmp;
    boost::split(tokens, tmp, boost::is_any_of(":"));
    if (!tokens[0].compare("RAISE")){
      raiseMin = atoi(tokens[1].c_str());
      raiseMax = atoi(tokens[2].c_str());
      legalAction.actionType  = FOLD_CALL_RAISE;    
      //      std::cout << "ACTION IS fold_call_raise" << std::endl;
    } else if (!tokens[0].compare("BET")){
      raiseMin = atoi(tokens[1].c_str());
      raiseMax = atoi(tokens[2].c_str());
      legalAction.actionType  = CHECKFOLD_BET;   
    } else if (!tokens[0].compare("DISCARD")){
      legalAction.actionType = DISCARD_ONE;
      // std::cout << "ACTION IS discard" << std::endl;
    }    
  } 

  legalAction.callMin = callMin;
  legalAction.raiseMin = raiseMin;
  legalAction.raiseMax = raiseMax;

  return legalAction;
}
