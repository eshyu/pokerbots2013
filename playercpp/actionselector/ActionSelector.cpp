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

void ActionSelector::setGameParams(const std::string &myName, 
				   const std::string &oppName,
				   const int stackSize,
				   const int bb,
				   const int numHands)
{
  MY_NAME=myName;
  OPP_NAME=oppName;
  STACK_SIZE=stackSize;
  BIG_BLIND=bb;
  NUM_HANDS=numHands;
}

ActionSelector::ActionInfo ActionSelector::getAction(const std::string &getaction_str, std::vector<std::string> &holeCards, std::string &myDiscard, bool myButton){
 
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

  // create the legal action for us
  legalAction = actionlist2struct(ss, numLegalActions, lastActions[numLastActions-1], myButton);
  
  OpponentModeler::ROUND round = OpponentModeler::numBoardCards2round(numBoardCards);
  std::cout << "ACTIONsELECTOR.cpp:L59: round is " << round << std::endl;

  // get opponent's last action's information
  std::vector<OpponentModeler::OppActionInfo> oppActions;
  actionlist2actioninfos(lastActions, oppActions, round);  

  OpponentModeler::OppActionInfo lastAction = actionword2struct(lastActions[lastActions.size()-1], round);

  
  int lastaction = (int)lastAction.action;
 
  float mybEq=0;
  switch(legalAction.actionType){
  case DISCARD_ONE:
    //discardUniform(holeCards, myDiscard, actionInfo);
    discardGreedy(holeCards, boardCards, myDiscard, actionInfo);
    break;
  case CHECK_BET:
  case FOLD_CALL_RAISE:

    if (numBoardCards > 0 && 
	(lastaction == 0 || 
	 lastaction == 1 ||
	 lastaction == 2 ||
	 lastaction == 5)){
      mybEq=informedEvaluate(holeCards, boardCards, myDiscard, lastaction);

    } else {
      mybEq=evaluator->evaluate(holeCards, boardCards, myDiscard);
    }   

    opponentModeler->updateOpponentActionStatistics(oppActions,
						    myButton,
						    potSize,
						    STACK_SIZE,
						    boardCards,
						    holeCards);

    opponentModeler->updatebEq(mybEq, round);

    selectActionForRound(potSize, myButton, boardCards, holeCards, myDiscard, legalAction, actionInfo, mybEq);

    break;
  }

  // update our other statistics, from this GETACTION packet
  opponentModeler->updateOurActionStatistics((int)actionInfo.action, 
					     actionInfo.betAmount, 
					     myButton,
					     potSize, 
					     STACK_SIZE, 
					     boardCards, holeCards);
  
  return actionInfo;
}

/* Different strategies for different rounds*/
void ActionSelector::selectActionForRound(int potSize, bool myButton, 
					   const std::vector<std::string> &boardCards, 
					   const std::vector<std::string> &holeCards, 
					   const std::string &myDiscard,
					   const ActionSelector::LegalAction &legalAction,
					   ActionSelector::ActionInfo &actionInfo,
					   float mybEq)
{
  int numBoardCards = boardCards.size();
  switch (numBoardCards){
  case 0:
    preflopSelector(potSize, myButton, boardCards, holeCards, myDiscard, legalAction, actionInfo, mybEq);
    break;
  case 3:
  case 4:
  case 5:
    evalMagic(potSize, myButton, boardCards, holeCards, myDiscard, legalAction, actionInfo, mybEq);
    break;
  }
}

/* this will be replaced with something more sophisticated later */
void ActionSelector::preflopSelector(int potSize, bool myButton, 
				      const std::vector<std::string> &boardCards, 
				      const std::vector<std::string> &holeCards, 
				      const std::string &myDiscard,
				      const ActionSelector::LegalAction &legalAction,
				      ActionSelector::ActionInfo &actionInfo,
				      float equity)
{
  // open with the top 75% of hands on the button, and with top 50% otherwise
  int raise=0, adjustRaise, noise, toCall;
  toCall = legalAction.callMin;
  
  float discount = myButton ? 0.09 : 0;  
  float potOdds = (float)toCall/(toCall+potSize);
  
  // generate betting noise
  noise = (rand() % 3 - 1);

  bool canRaise=legalAction.raiseMax>0, isAllin=legalAction.raiseMax==0, mustCall=legalAction.callMin>0;;  
  if (canRaise > 0 && equity-discount > 0.5){
    int bucket = (int)(20*equity)-9;
    raise = std::max(7, 7 + (bucket + noise)*8);
    adjustRaise = std::max(std::min(raise,legalAction.raiseMax), legalAction.raiseMin);
    std::cout << "ActionSelector:L108 (equity, bucket, noise, raise, adjRaise): " << equity << " " <<bucket << " " << noise << " " << raise << " " << adjustRaise << std::endl;
  }  

  // for now no re-raising pre-flop more than our normal raise
  if (raise && (raise == adjustRaise)){ //can raise/bet without any problem
    if (legalAction.actionType == CHECK_BET)
      Bet(actionInfo,raise);
    else
      Raise(actionInfo,raise);
  } else {    
    if (isAllin) {
      if (equity > 0.59)
	Call(actionInfo);   //only call all-in if have top hands
      else 
	Fold(actionInfo);
    } else if (mustCall){
      if (equity > potOdds+0.05)
	Call(actionInfo);  // call if cheap enough to call
      else 
	Fold(actionInfo);
    } else {      
      Check(actionInfo);
    }    
  }   
}
 

/* this will be replaced with something more sophisticated later */
void ActionSelector::evalMagic(int potSize, bool myButton, 
				const std::vector<std::string> &boardCards, 
				const std::vector<std::string> &holeCards, 
				const std::string &myDiscard,
				const ActionSelector::LegalAction &legalAction,
				ActionSelector::ActionInfo &actionInfo,
				float equity){
  int coin = rand() % 3; //LOL
  int callMin = legalAction.callMin;
   
  if (coin == 1 && equity > 0.8){
    if (legalAction.raiseMax > 0){
      if (legalAction.actionType == CHECK_BET)
	Bet(actionInfo, legalAction.raiseMax);
      else 
	Raise(actionInfo, legalAction.raiseMax);
    } else { 
      std::cout << "ActionSelector.cpp:L78 Calling All-in" << std::endl;
      Call(actionInfo);
    }
    //std::cout << "raising to " << actionInfo.betAmount << std::endl;
    return;
  } 
   
  // compute pot odds and either call or fold    
  float potOdds = (float)callMin/(callMin+potSize);

  bool canRaise=legalAction.raiseMax>0, isAllin=legalAction.raiseMax==0, mustCall=legalAction.callMin>0;
  // raise with the worst and best cards in our range

  if (equity>0.62){
    std::cout << "myPotOdds: " << callMin << "/" << (callMin+potSize) << ":" << potOdds << " vs. equity: " << equity << std::endl;
    if (legalAction.raiseMax > 0){
      float oppEquity=1-equity;
      int newPotSize=callMin+potSize;
      
      int raise=(int)((newPotSize*equity/oppEquity)) + callMin;
      //	int raise=(int)((newPotSize*equity/oppEquity)) + callMin;
		
      int numBoardCards = boardCards.size();
      if (numBoardCards >= 3) raise = 7; //7std::max(raise, 100); // so we actually can make money      
      if (numBoardCards >= 4 && equity > 0.7) raise = std::max(raise, 200);      
      if (numBoardCards >= 5 && equity > 0.85) raise = std::max(raise, 300);      
	  
      int betAmt= std::max(std::min(raise,legalAction.raiseMax), legalAction.raiseMin);

      std::cout << "betAmt: " << betAmt << " vs. raise: " << raise << "(raiseMin, max) " << legalAction.raiseMin << ", " << legalAction.raiseMax << std::endl;
      if (betAmt){
	actionInfo.action= (legalAction.actionType == CHECK_BET) ? BET : RAISE;
	actionInfo.betAmount=betAmt;
      } else {
	actionInfo.action = (legalAction.actionType == CHECK_BET) ? CHECK : FOLD;
	actionInfo.betAmount=0;
      }//end if(betAmt)
    } else {	 
      if ((legalAction.callMin > 100 && equity < 0.65))
	Fold(actionInfo);
      else 
	Call(actionInfo);
    } //end if(raisMax > 0)
  }else{
    std::cout << "myPotOdds: " << callMin << "/" << (callMin+potSize) << ":" << potOdds << " vs. equity: " << equity << std::endl;

    float roundDiscount=0;
    int numBoardCards = boardCards.size();

    // Discount our equity, so we are less willing to call with low equity on later streets (especially the river)
    if (numBoardCards == 5){
      roundDiscount=0.2;
    }       

    if (legalAction.callMin > 0){
      if (numBoardCards>=3 && equity < 0.55){ // fold if our equity is bad by the turn	  
	Fold(actionInfo);
      }else if (equity-roundDiscount>(potOdds)+0.04){ //less likely to call if we are down
	Call(actionInfo);
      } else {
	Fold(actionInfo);
      }
    } else {      
      Check(actionInfo);
    }
  }
}

/* actions */ 
void ActionSelector::Bet(ActionInfo &actionInfo, int bet){
  actionInfo.action=BET;
  actionInfo.betAmount=bet;
}

void ActionSelector::Check(ActionInfo &actionInfo){
  actionInfo.action=CHECK;
  actionInfo.betAmount=0;
}

void ActionSelector::Call(ActionInfo &actionInfo){
  actionInfo.action=CALL;
  actionInfo.betAmount=0;
}

void ActionSelector::Fold(ActionInfo &actionInfo){
  actionInfo.action=FOLD;
  actionInfo.betAmount=0;
}

void ActionSelector::Raise(ActionInfo &actionInfo, int bet){
  actionInfo.action=RAISE;
  actionInfo.betAmount=bet;
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

/* Get last action done by the player*/

  std::string ActionSelector::getLastAction(const std::vector<std::string> &lastActions, const std::string &playerName)
  {
  std::vector<std::string> tokens;
  std::string actionword;
  std::string player;
  for (int i=lastActions.size()-1;i>=0;i--){
  boost::split(tokens, lastActions[i], boost::is_any_of(":"));
  std::string actionword = tokens[0];
  std::string player = tokens[tokens.size()-1];

  if (!player.compare(playerName)){
  if (!actionword.compare("BET") ||
  !actionword.compare("CALL") ||
  !actionword.compare("FOLD") ||
  !actionword.compare("RAISE")){
  return lastActions[i];
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

/* convert last action word into actioninfo*/
OpponentModeler::OppActionInfo ActionSelector::actionword2struct(const std::string &actionword, OpponentModeler::ROUND round){
  OpponentModeler::OppActionInfo oppLastAction;
  std::vector<std::string> tokens;
  boost::split(tokens, actionword, boost::is_any_of(":"));
  if (!tokens[0].compare("BET")){
    oppLastAction.action=OpponentModeler::BET;
    oppLastAction.betAmount=boost::lexical_cast<int>(tokens[1]);
  } else if (!tokens[0].compare("CALL")){
    oppLastAction.action=OpponentModeler::CALL;
    oppLastAction.betAmount=0;
  } else if (!tokens[0].compare("CHECK")){
    oppLastAction.action=OpponentModeler::CHECK;
    oppLastAction.betAmount=0;
  } else if (!tokens[0].compare("FOLD")){
    oppLastAction.action=OpponentModeler::FOLD;
    oppLastAction.betAmount=0;
  } else if (!tokens[0].compare("RAISE")){
    oppLastAction.action=OpponentModeler::RAISE;
    oppLastAction.betAmount=boost::lexical_cast<int>(tokens[1]);
  } else {
    oppLastAction.action=OpponentModeler::NONE;
  }
  oppLastAction.round=round;  

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

/* Update states at end of hand over */
void ActionSelector::updateHandover(const std::string &line, bool myButton, 
				    const std::vector<std::string> &holeCards,
				    const std::string &myDiscard)
{
  float myBankroll, oppBankroll;
  int numBoardCards, numLastActions;
  std::string tmp;
 
  std::stringstream ss(line); 
  ss >> tmp >> myBankroll >> oppBankroll >> numBoardCards;
  std::vector<std::string> boardCards;
  packetlist2vector(ss, boardCards, numBoardCards);  

  // get actions from HANDOVER packet
  ss >> numLastActions;
  std::vector<std::string> lastActions;
  packetlist2vector(ss, lastActions, numLastActions);      
  
  std::vector<std::string> tokens;
  std::string actionword;
  std::string player;
  int pot=3;
  for (int i=lastActions.size()-1;i>=0;i--){
    boost::split(tokens, lastActions[i], boost::is_any_of(":"));
    actionword = tokens[0];
    player = tokens[tokens.size()-1];
    
    if (!actionword.compare("TIE")){
      // update hand modeler or something with hands
      pot=2*boost::lexical_cast<int>(tokens[1]);
    }
    
    if (!actionword.compare("WINS")){
      // update hand modeler or something with hands
      pot=boost::lexical_cast<int>(tokens[1]);
      int player = tokens[2].compare(MY_NAME);
      std::cout << "winnnnnnnnnnnnnnnnnnnnewr is: " << player << std::endl;
      opponentModeler->updateWinner(player);
    }
    
    if (!actionword.compare("SHOW") && !player.compare(OPP_NAME)){
      // update hand modeler or something with hands
      std::vector<std::string> oppCards;
      oppCards.push_back(tokens[1]); oppCards.push_back(tokens[2]);

      opponentModeler->updateShow(holeCards, oppCards, 
				  boardCards, myDiscard);

    }

  }
  
  std::vector<OpponentModeler::OppActionInfo> oppActions;
  actionlist2actioninfos(lastActions, oppActions, 
			 OpponentModeler::numBoardCards2round(numBoardCards));
  
  opponentModeler->updateOpponentActionStatistics(oppActions,
						  myButton,
						  pot,
						  STACK_SIZE,
						  boardCards,
						  holeCards);	 				    
}


void ActionSelector::actionlist2actioninfos(const std::vector<std::string> &lastActions, std::vector<OpponentModeler::OppActionInfo> & oppActions, OpponentModeler::ROUND round)
{
  std::vector<std::string> tokens;
  std::string actionword;
  for (int i=lastActions.size()-1;i>=0;i--){
    boost::split(tokens, lastActions[i], boost::is_any_of(":"));
    actionword = tokens[0];
    if ((!actionword.compare("BET") ||
	 !actionword.compare("CALL") ||
	 !actionword.compare("FOLD") ||
	 !actionword.compare("CHECK") || 
	 !actionword.compare("RAISE")) && 
	!tokens[tokens.size()-1].compare(OPP_NAME)){
      oppActions.push_back(actionword2struct(lastActions[i], round));
    }        
    
    if (!actionword.compare("DEAL")) round=(OpponentModeler::ROUND)((int)round-1);
  }
}

float ActionSelector::informedEvaluate(const std::vector<std::string> &holeCards,
				       const std::vector<std::string> &boardCards,
				       const std::string &myDiscard,
				       int lastOpponentAction)
{
  std::vector<float> weights;

  std::vector<std::string> knowncards = std::vector<std::string>(holeCards);
  knowncards.push_back(myDiscard);

  opponentModeler->getHandDistribution(lastOpponentAction, weights);
  std::string oppRange = opponentModeler->getHandRangeString(holeCards, boardCards, weights);  
  
  return evaluator->evaluate_range(holeCards, boardCards, oppRange, myDiscard);
}
