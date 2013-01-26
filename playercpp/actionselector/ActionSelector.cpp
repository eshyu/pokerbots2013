#include "ActionSelector.hpp"

#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <algorithm>

#include <boost/algorithm/string.hpp>

#include <boost/lexical_cast.hpp>

ActionSelector::ActionSelector(){}

ActionSelector::ActionSelector(Evaluator *_evaluator, OpponentModeler *_opponentModeler){
  evaluator = _evaluator;
  opponentModeler = _opponentModeler;
}

ActionSelector::ActionInfo ActionSelector::getAction(const std::string &getaction_str, std::vector<std::string> &holeCards, std::string &myDiscard, bool myButton, int stackSize){
 
  LegalAction legalAction;
  ActionInfo actionInfo, opponentAction;

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
  opponentAction = actionword2struct(lastActions[numLastActions-1]);

  float mybEq=0;
  switch(legalAction.actionType){
  case DISCARD_ONE:
    //discardUniform(holeCards, myDiscard, actionInfo);
    discardGreedy(holeCards, boardCards, myDiscard, actionInfo);
    break;
  case CHECK_BET:
  case FOLD_CALL_RAISE:
    mybEq=selectActionForRound(potSize, myButton, boardCards, holeCards, myDiscard, legalAction, actionInfo);
     break;
   }
    
  opponentModeler->updateActionStatistics((int)actionInfo.action, 
					  actionInfo.betAmount, 
					  (int)opponentAction.action, 
					  opponentAction.betAmount, 
					  myButton,
					  potSize, 
					  stackSize, 
					  mybEq,
					  boardCards, holeCards);
  
  return actionInfo;
}

/* Different strategies for different rounds*/
float ActionSelector::selectActionForRound(int potSize, bool myButton, 
					  const std::vector<std::string> &boardCards, 
					  const std::vector<std::string> &holeCards, 
					  const std::string &myDiscard,
					  const ActionSelector::LegalAction &legalAction,
					  ActionSelector::ActionInfo &actionInfo)
{
  int numBoardCards = boardCards.size();
  switch (numBoardCards){
  case 0:
    return preflopSelector(potSize, myButton, boardCards, holeCards, myDiscard, legalAction, actionInfo);
  case 3:
  case 4:
  case 5:
    return evalMagic(potSize, myButton, boardCards, holeCards, myDiscard, legalAction, actionInfo);
  }
}

 /* this will be replaced with something more sophisticated later */
float ActionSelector::preflopSelector(int potSize, bool myButton, 
				const std::vector<std::string> &boardCards, 
				const std::vector<std::string> &holeCards, 
				const std::string &myDiscard,
				const ActionSelector::LegalAction &legalAction,
				ActionSelector::ActionInfo &actionInfo)
{
  // open with the top 75% of hands on the button, and with top 50% otherwise
  int raise=0, adjustRaise, noise, toCall;
  toCall = legalAction.callMin;
  
  float premium = myButton ? 0.05 : 0;
  float equity = evaluator->evaluate(holeCards, boardCards, myDiscard);
  float potOdds = (float)toCall/(toCall+potSize);
  
  // generate betting noise
  noise = (rand() % 3 - 1);
  
  if (legalAction.raiseMax > 0 && equity+premium > 0.5){
    int bucket = (int)(20*equity)-9;
    raise = std::max(7, 20 + (bucket + noise)*8);
    adjustRaise = std::max(std::min(raise,legalAction.raiseMax), legalAction.raiseMin);
    std::cout << "ActionSelector:L108 (equity, bucket, noise, raise, adjRaise): " << equity << " " <<bucket << " " << noise << " " << raise << " " << adjustRaise << std::endl;
  } 
  
  // for now no re-raising pre-flop more than our normal raise
  if (raise && (raise == adjustRaise)){
    actionInfo.action= (legalAction.actionType == CHECK_BET) ? BET : RAISE;
    actionInfo.betAmount=raise;
    std::cout << "ActionSelector.cpp:99 Raising " << raise << std::endl;
  } else {    
    std::cout << "ActionSelector L117: myPotOdds: " << toCall << "/" << (toCall+potSize) << ":" << potOdds << " vs. equity: " << equity << std::endl;
    if (legalAction.callMin > 0){
      if (equity > potOdds+0.05){
	std::cout << "ActionSelector.cpp:L00 Calling" << std::endl;
	actionInfo.action=CALL;
      } else {
	actionInfo.action=FOLD;
      }
    } else {      
      actionInfo.action=CHECK;
    }    
  }   
  return equity;
}

 /* this will be replaced with something more sophisticated later */
 float ActionSelector::evalMagic(int potSize, bool myButton, 
				const std::vector<std::string> &boardCards, 
				const std::vector<std::string> &holeCards, 
				const std::string &myDiscard,
				const ActionSelector::LegalAction &legalAction,
				ActionSelector::ActionInfo &actionInfo){
   // TODO: .....
   //  case CHECKFOLD_BET:   case FOLD_CALL_RAISE:  
   int coin = rand() % 4; //LOL
   int callMin = legalAction.callMin;
   
   //std::cout << "coin is " << coin;

   std::cout << "evaluating with # board cards: " << boardCards.size() << std::endl;
   float equity = evaluator->evaluate(holeCards, boardCards, myDiscard);

   if (coin == 1 && equity > 0.8){
     if (legalAction.raiseMax > 0){
       std::cout << "ALL IN"<< std::endl;
       actionInfo.action= (legalAction.actionType == CHECK_BET) ? BET : RAISE;
       actionInfo.betAmount=legalAction.raiseMax;
     } else { 
       // opponent already put us all in
       std::cout << "ActionSelector.cpp:L78 Calling All-in" << std::endl;
       actionInfo.action = CALL;
     }
     //std::cout << "raising to " << actionInfo.betAmount << std::endl;
   } 
   
   else {
     
     // compute pot odds and either call or fold    
     float potOdds = (float)callMin/(callMin+potSize);
     // TODO: lol

     //     if (myButton) equity = equity*1.2;
     
     if (equity>0.62){
       std::cout << "myPotOdds: " << callMin << "/" << (callMin+potSize) << ":" << potOdds << " vs. equity: " << equity << std::endl;
       if (legalAction.raiseMax > 0){
        float oppEquity=1-equity;
        int newPotSize=callMin+potSize;
	//	int raise=1+(int)(newPotSize/oppEquity-newPotSize);
	
	int raise=std::max((int)((newPotSize*oppEquity/equity)),
			   (int)((newPotSize*0.161/(1-0.161)))
			   ) + callMin;

	int numBoardCards = boardCards.size();

	if (numBoardCards >= 3){
	  raise = std::max(raise, 100); // so we actually can make money
	}

	if (numBoardCards >= 4 && equity > 0.7){
	  raise = std::max(raise, 200);
	}

	if (numBoardCards >= 5 && equity > 0.85){
	  raise = std::max(raise, 300);
	}

        int betAmt= std::max(std::min(raise,legalAction.raiseMax), legalAction.raiseMin);

	std::cout << "betAmt: " << betAmt << " vs. raise: " << raise << "(raiseMin, max) " << legalAction.raiseMin << ", " << legalAction.raiseMax << std::endl;
	if (betAmt){
	  actionInfo.action= (legalAction.actionType == CHECK_BET) ? BET : RAISE;
	  actionInfo.betAmount=betAmt;
	} else {
	  actionInfo.action = (legalAction.actionType == CHECK_BET) ? CHECK : FOLD;
	}//end if(betAmt)
       } else {
	 
	 if (boardCards.size()==5 && (equity - 0.2) < potOdds){
	   actionInfo.action = FOLD;
	 } else {
	   std::cout << "ActionSelector.cpp:L103 Calling All-in" << std::endl;
	   actionInfo.action = CALL;
	 }	 	 
       } //end if(raisMax > 0)
     }else{
       std::cout << "myPotOdds: " << callMin << "/" << (callMin+potSize) << ":" << potOdds << " vs. equity: " << equity << std::endl;

       float roundDiscount=0;
       int numBoardCards = boardCards.size();

       // Discount our equity, so we are less willing to call with low equity on later streets (especially the river)
	if (numBoardCards == 5){
	  roundDiscount=0.2;
	}       

	std::cout << "Numboard cards: " << numBoardCards << " and discounting by: " << roundDiscount << std::endl;

	std::cout << "equity: " << equity << ", w/ round discount: " << equity-roundDiscount << std::endl;
       //TODO: lol
       if (legalAction.callMin > 0){
	 if (numBoardCards>=4 && equity < 0.5){ // fold if our equity is bad by the turn	   
	   actionInfo.action=FOLD;
	   std::cout << "ActionSelector.cpp:L235 Folding" << std::endl;
	 }else if (equity-roundDiscount>(potOdds)+0.04){ //less likely to call if we are down
	   std::cout << "ActionSelector.cpp:L237 Calling" << std::endl;
	   actionInfo.action=CALL;
	 } else {
	   std::cout << "ActionSelector.cpp:L240 Folding" << std::endl;
	   actionInfo.action=FOLD;
	 }
       } else {      
	 std::cout << "ActionSelector.cpp:L244 Checkingg" << std::endl;
	 actionInfo.action=CHECK;
       }
     }
   }      
   return equity;
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

  float eval0=evaluator->memoized_evaluate_pairs(discard0, boardString, holeCards[0], 0);
  float eval1=evaluator->memoized_evaluate_pairs(discard1, boardString, holeCards[1], 1);
  float eval2=evaluator->memoized_evaluate_pairs(discard2, boardString, holeCards[2], 2);

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

/* convert last action word into actioninfo*/
ActionSelector::ActionInfo ActionSelector::actionword2struct(const std::string &actionword){
  ActionInfo oppLastAction;
  std::vector<std::string> tokens;
  boost::split(tokens, actionword, boost::is_any_of(":"));
  if (!tokens[0].compare("BET")){
    oppLastAction.action=BET;
    oppLastAction.betAmount=boost::lexical_cast<int>(tokens[1]);
  } else if (!tokens[0].compare("CALL")){
    oppLastAction.action=CALL;
  } else if (!tokens[0].compare("CHECK")){
    oppLastAction.action=CHECK;
  } else if (!tokens[0].compare("FOLD")){
    oppLastAction.action=FOLD;
  } else if (!tokens[0].compare("RAISE")){
    oppLastAction.action=RAISE;
    oppLastAction.betAmount=boost::lexical_cast<int>(tokens[1]);
  } else {
    oppLastAction.action=NONE;
  }

  return oppLastAction;
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
