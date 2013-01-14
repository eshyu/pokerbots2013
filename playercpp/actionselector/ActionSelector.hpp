#ifndef __ACTIONSELECTOR_HPP__
#define __ACTIONSELECTOR_HPP__

#include <string>
#include <vector>

class ActionSelector {
public:
  ActionSelector();
  
  //TODO:  
  // OpponentModeler oppModel;    
  // HandEvaluator handEval;
  // DiscardSelector discardSelector;  
  // BettingSelector bettingSelector;

  enum ACTION {BET, CALL, CHECK, DISCARD, FOLD, RAISE};

  // todo: THIS really should be like
  // check-fold
  // check-raise
  // check-call
  // bet-fold
  // bet-raise
  // ...
  enum ACTION_TYPE {CHECKFOLD_BET, DISCARD_ONE, FOLD_CALL_RAISE};

  /* return to player the action it will take*/
  struct ActionInfo
  {
    ACTION action;
    int betAmount; //only for BET, RAISE
    char cardNum, cardSuit; // only for DISCARD
  };

  /* parse action list to get the legal actions*/
  struct LegalAction
  {
    ACTION_TYPE actionType;
    int callMin;
    int raiseMin;
    int raiseMax;
  };
  
  /* blackbox for choosing action for current game*/
  ActionInfo getAction(const std::string &getaction_str, const std::string &holeCard1, const std::string &holeCard2, const std::string &holeCard3, bool myButton, int stackSize);


private:
  /* TODO lol this should disappear in favor of BettingSelector, DiscardSelector*/
  void evalMagic(int potSize, bool myButton,
		 std::string holeCard1, std::string holeCard2, std::string holeCard3,
		 const LegalAction &legalAction, ActionInfo &actionInfo);

  /* convert list of items from packet to vector of strings */
  void packetlist2vector(std::stringstream &ss, std::vector<std::string> &packetlist, int length);

  /* converts actionlist to legal action */
  LegalAction actionlist2struct(std::stringstream &ss, int length, std::string lastAction);
  
};


#endif  // __ACTIONSELECTOR_HPP__
