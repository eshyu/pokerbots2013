#include "ActionSelector.hpp"

#include <vector>
#include <string>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <ctime>

ActionSelector::ActionSelector(){}

ActionSelector::ActionSelector(Evaluator *_evaluator){
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

  legalAction = actionlist2struct(ss, numLegalActions, lastActions[numLastActions-1], myButton);
  
  // OpponentModeler.update(lastAction);
  
  //  float myEval = handEval.eval(holeCards, boardCards);  
  switch(legalAction.actionType){
  case DISCARD_ONE:
    // DiscardSelector.discard();
    //discardUniform(holeCards, myDiscard, actionInfo);
    discardGreedy(holeCards, boardCards, myDiscard, actionInfo);
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

   //josep: seeding the randomness!!!!!
   srand(time(NULL));

   int coin = rand() % 4; //LOL
   int callMin = legalAction.callMin;
   
   //std::cout << "coin is " << coin;

   std::cout << "evaluating with # board cards: " << boardCards.size() << std::endl;
   double equity = evaluator->evaluate(holeCards, boardCards, myDiscard);

   if (coin == 1 && equity > 0.8){
     if (legalAction.raiseMax > 0){
       std::cout << "ALL IN"<< std::endl;
       actionInfo.action= (legalAction.actionType == CHECK_BET) ? BET : RAISE;
       actionInfo.betAmount=legalAction.raiseMax;
     } else { 
       // opponent already put us all in
       std::cout << "ActionSelector.cpp:L78 Calling All-in" << std::cout;
       actionInfo.action = CALL;
     }
     //std::cout << "raising to " << actionInfo.betAmount << std::endl;
   } 
   
   else {
     
     // compute pot odds and either call or fold    
     double potOdds = (double)callMin/(callMin+potSize);
     // TODO: lol

     //     if (myButton) equity = equity*1.2;
     
     if (equity>0.62){
       std::cout << "myPotOdds: " << callMin << "/" << (callMin+potSize) << ":" << potOdds << " vs. equity: " << equity << std::endl;
       if (legalAction.raiseMax > 0){
        double oppEquity=1-equity;
        int newPotSize=callMin+potSize;
	//	int raise=1+(int)(newPotSize/oppEquity-newPotSize);
	
	int raise=std::max((int)((newPotSize*oppEquity/equity)),
			   (int)((newPotSize*0.161/(1-0.161)))
			   ) + callMin;

	int numBoardCards = boardCards.size();
	if (numBoardCards == 0){
	  raise = 20;
	} else if (numBoardCards >= 3){
	  raise = std::max(raise, 100); // so we actually can make money
	}

	if (numBoardCards >= 4 && equity > 0.7){
	  raise = std::max(raise, 200);
	}

	if (numBoardCards == 5 && equity > 0.85){
	  raise = std::max(raise, 300);
	}

        int betAmt= std::max(std::min(raise,legalAction.raiseMax), legalAction.raiseMin);

	std::cout << "betAmt: " << betAmt << " vs. raise: " << raise << "(raiseMin, max) " << legalAction.raiseMin << ", " << legalAction.raiseMax << std::endl;
	if (betAmt){
	  actionInfo.action= (legalAction.actionType == CHECK_BET) ? BET : RAISE;
	  actionInfo.betAmount=betAmt;
	} else {
	  actionInfo.action = (legalAction.actionType == CHECK_BET) ? CHECK : FOLD;
	}
       } else {
	 std::cout << "ActionSelector.cpp:L103 Calling All-in" << std::cout;
	 actionInfo.action = CALL;
       }
     }else{
       std::cout << "myPotOdds: " << callMin << "/" << (callMin+potSize) << ":" << potOdds << " vs. equity: " << equity << std::endl;

       int roundDiscount=0;
       int numBoardCards = boardCards.size();

       // Discount our equity, so we are less willing to call with low equity on later streets (especially the river)
	if (numBoardCards == 5){
	  roundDiscount=0.2;
	}       

       //TODO: lol
       if (legalAction.callMin > 0){
	 if (equity-roundDiscount>(potOdds)+0.04){ //less likely to call if we are down
	   //std::cout << "equity: " << equity << ", w/ round discount: " << equity-roundDiscout << std:endl;
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

void ActionSelector::discardGreedy(std::vector<std::string> &holeCards, std::vector<std::string> &boardCards, std::string &myDiscard, ActionInfo &actionInfo){  

  // Try all two card hands and discard the lowest equity one
  std::string discard0 = holeCards[1] + holeCards[2];
  std::string discard1 = holeCards[0] + holeCards[2];
  std::string discard2 = holeCards[0] + holeCards[1];
  std::string boardString;
  for (int i=0;i<boardCards.size();i++) boardString = boardString + boardCards[i];

  double eval0=evaluator->memoized_evaluate_pairs(discard0, boardString, holeCards[0], 0);
  double eval1=evaluator->memoized_evaluate_pairs(discard1, boardString, holeCards[1], 1);
  double eval2=evaluator->memoized_evaluate_pairs(discard2, boardString, holeCards[2], 2);

  // higest
  int discard;
  if (eval0 >= eval1 && eval0 >= eval2){
    discard=0;
  } else if (eval1 >= eval0 && eval1 >= eval2) {
    discard=1;
  } else if (eval2 >= eval0 && eval2 >= eval1){
    discard=2;
  }
  
  std::cout << "Equities: " << discard0 << ", " << eval0 << " | "
	    << discard1 << ", " << eval1 << " | " 
	    << discard2 << ", " << eval2 << " | "
	    << " discarding: " << holeCards[discard] << std::endl;
  
  myDiscard = std::string(holeCards[discard]);
    
  actionInfo.cardNum=myDiscard[0]; actionInfo.cardSuit=myDiscard[1];    
  actionInfo.action = DISCARD;
  holeCards.erase(holeCards.begin() + discard);

}

/* Helper methods*/

void ActionSelector::packetlist2vector(std::stringstream &ss, std::vector<std::string> &packetlist, int length){
  std::string item;
  for (int i=0;i<length;i++){
    ss >> item;
    packetlist.push_back(item);
  }
}

ActionSelector::LegalAction ActionSelector::actionlist2struct(std::stringstream &ss, int length, std::string lastAction, bool myButton){

  LegalAction legalAction;
  std::vector<std::string> tokens;
  legalAction.actionType = CHECK_BET;

  // get call amount from last action
  boost::split(tokens, lastAction, boost::is_any_of(":"));
  
  // std::cout << "last action is: " << tokens[0] << std::endl;

  int callMin=0, raiseMin=0, raiseMax=0;
  if ((!tokens[0].compare("BET") || !tokens[0].compare("RAISE"))){
      // cuz the call amount doesnt appear in the action..
    callMin = atoi(tokens[1].c_str());    
    legalAction.actionType = FOLD_CALL_RAISE;
  }
    
  if (!tokens[0].compare("POST")){
    int blind = atoi(tokens[1].c_str());
    if (!myButton){
      callMin = blind;
    } else {
      callMin = blind/2;
    }
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
