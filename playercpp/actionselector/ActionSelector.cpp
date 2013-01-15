#include "ActionSelector.hpp"

#include <vector>
#include <string>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <boost/algorithm/string.hpp>

ActionSelector::ActionSelector(){}

ActionSelector::ActionSelector(Evaluator &_evaluator){
  evaluator = _evaluator;
}

ActionSelector::ActionInfo ActionSelector::getAction(const std::string &getaction_str, std::vector<std::string> &holeCards, std::string &myDiscard, bool myButton, int stackSize){
 
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
  
  // OpponentModeler.update(lastAction);
  
  //  float myEval = handEval.eval(holeCards, boardCards);  
  switch(legalAction.actionType){
  case DISCARD_ONE:
    // DiscardSelector.discard();
    discardUniform(holeCards, myDiscard, actionInfo);
    break;
  case CHECK_BET:
    // Be 
    evalMagic(potSize, myButton, boardCards, holeCards, myDiscard, legalAction, actionInfo);
    break;
  case FOLD_CALL_RAISE:
     // use myEval, potOdds, numBoar
    evalMagic(potSize, myButton, boardCards, holeCards, myDiscard, legalAction, actionInfo);
     break;
   }

   return actionInfo;
 }

 /* this will be replaced with something more sophisticated later */
 void ActionSelector::evalMagic(int potSize, bool myButton, 
				const std::vector<std::string> &boardCards, 
				const std::vector<std::string> &holeCards, 
				const std::string &myDiscard,
				const ActionSelector::LegalAction &legalAction,
				ActionSelector::ActionInfo &actionInfo){
   // TODO: .....
   //dumbass bot allin 25% of time, never raises except allin  
   //  case CHECKFOLD_BET:   case FOLD_CALL_RAISE:              
   int coin = rand() % 4; //LOL
   int callMin = legalAction.callMin;
   
   //std::cout << "coin is " << coin;
   
   if (coin == 1){
     if (legalAction.raiseMax > 0){
       actionInfo.action= (legalAction.actionType == CHECK_BET) ? BET : RAISE;
       actionInfo.betAmount=legalAction.raiseMax;
     } else { 
       // opponent already put us all in
       std::cout << "ActionSelector.cpp:L78 Calling All-in" << std::cout;
       actionInfo.action = CALL;
     }
     //std::cout << "raising to " << actionInfo.betAmount << std::endl;
   } else {
     
     // compute pot odds and either call or fold    
     double potOdds = (double)potSize/(callMin+potSize);
     // TODO: lol
     double equity = evaluator.evaluate(holeCards, boardCards, myDiscard);
    
     if (equity>0.5 and coin!=2){
        double oppEquity=1-equity;
        int newPotSize=callMin+potSize;
        int raise=1+(int)((double)newPotSize/oppEquity-newPotSize);
        int betAmt=std::min(raise,legalAction.raiseMax);
	actionInfo.action= (legalAction.actionType == CHECK_BET) ? BET : RAISE;
        actionInfo.betAmount=betAmt;
     }else{
     
       std::cout << "myPotOdds: " << potOdds << " vs. equity: " << equity << std::endl;

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



/* Discard functions */

void ActionSelector::discardUniform(std::vector<std::string> &holeCards, std::string &myDiscard, ActionInfo &actionInfo){  
  // discard a card at random lol
  //todo lol replace this with discard selector instead of this 

  int remove = rand() % 3;  
  myDiscard = std::string(holeCards[remove]);
  actionInfo.cardNum=myDiscard[0]; actionInfo.cardSuit=myDiscard[1];    
  actionInfo.action = DISCARD;
  std::cout << "holecards b4 discard: " << holeCards.size() << std::endl;
  holeCards.erase(holeCards.begin() + remove);
  std::cout << "holecards post discard: " << holeCards.size() << std::endl;
}

void ActionSelector::discardGreedy(std::vector<std::string> &holeCards, std::string &myDiscard, ActionInfo &actionInfo){  

  // Try all two card hands and discard the 
  int remove = rand() % 3;  
  myDiscard = std::string(holeCards[remove]);
  actionInfo.cardNum=myDiscard[0]; actionInfo.cardSuit=myDiscard[1];    
  actionInfo.action = DISCARD;
  std::cout << "holecards b4 discard: " << holeCards.size() << std::endl;
  holeCards.erase(holeCards.begin() + remove);
  std::cout << "holecards post discard: " << holeCards.size() << std::endl;
}

/* Helper methods*/

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
  legalAction.actionType = CHECK_BET;

  // get call amount from last action
  boost::split(tokens, lastAction, boost::is_any_of(":"));
  
  // std::cout << "last action is: " << tokens[0] << std::endl;

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
      // std::cout << "ACTION IS fold_call_raise" << std::endl;
    } else if (!tokens[0].compare("BET")){
      raiseMin = atoi(tokens[1].c_str());
      raiseMax = atoi(tokens[2].c_str());
      legalAction.actionType  = CHECK_BET;   
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
