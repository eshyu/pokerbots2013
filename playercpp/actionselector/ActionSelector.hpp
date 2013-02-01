#ifndef __ACTIONSELECTOR_HPP__
#define __ACTIONSELECTOR_HPP__

#include <string>
#include <vector>

#include "../evaluator/Evaluator.hpp"
#include "../opponentmodeler/OpponentModeler.hpp"

class ActionSelector {
public:
  ActionSelector();
  ActionSelector(Evaluator *_evaluator, OpponentModeler *_opponentModeler);
  
  //TODO:  
  // OpponentModeler oppModel;    
  // HandEvaluator handEval;
  // DiscardSelector discardSelector;  
  // BettingSelector bettingSelector;

  enum ACTION {BET, CALL, CHECK, DISCARD, FOLD, RAISE, NONE};

  // todo: THIS really should be like
  // check-fold
  // check-raise
  // check-call
  // bet-fold
  // bet-raise
  // ...
  enum ACTION_TYPE {CHECK_BET, DISCARD_ONE, FOLD_CALL_RAISE};

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

  // hand evaluator 
  Evaluator *evaluator;

  // opponent modeler
  OpponentModeler *opponentModeler;
  
  // Game parameters
  std::string OPP_NAME, MY_NAME;
  int STACK_SIZE, BIG_BLIND, NUM_HANDS;

  /* blackbox for choosing action for current game*/
  ActionInfo getAction(const std::string &getaction_str, std::vector<std::string> &holeCards, std::string &myDiscard, bool myButton);

  /* set the inital game parameters */
  void setGameParams(const std::string &myName, 
		     const std::string &oppName,
		     const int stackSize,
		     const int bb,
		     const int numHands);

  /* update stats and things */
  void updateHandover(const std::string &line, bool myButton,
		      const std::vector<std::string> &holeCards,
		      const std::string &myDiscard);
 
private:
  void Bet(ActionInfo &action, int bet);
  void Check(ActionInfo &action);
  void Call(ActionInfo &action);
  void Raise(ActionInfo &action, int bet);
  void Fold(ActionInfo &action);

  // evaluation mish mash
  float informedEvaluate(const std::vector<std::string> &holeCards,
			 const std::vector<std::string> &boardCards,
			 const std::string &myDiscard);
			 
  void selectActionForRound(int potSize, bool myButton, 
			     const std::vector<std::string> &boardCards,
			     const std::vector<std::string> &holeCards,
			     const std::string &myDiscard,
			     const LegalAction &legalAction, 
			     ActionInfo &actionInfo,
			     float mybEq);

  /* Different rounds */
  void preflopSelector(int potSize, bool myButton, 
		       const std::vector<std::string> &boardCards,
		       const std::vector<std::string> &holeCards,
		       const std::string &myDiscard,
		       const LegalAction &legalAction, 
			ActionInfo &actionInfo,
			float mybEq);
  
  
  /* TODO lol this should disappear in favor of BettingSelector, DiscardSelector*/
  void evalMagic(int potSize, bool myButton, 
		 const std::vector<std::string> &boardCards,
		 const std::vector<std::string> &holeCards,
		 const std::string &myDiscard,
		 const LegalAction &legalAction, 
		  ActionInfo &actionInfo,
		  float mybEq);

  // decide if we go all-in
  bool decideJam(ActionInfo &action);

  /* TODO: this should be in a DiscardSelector */
  void discardUniform(std::vector<std::string> &holeCards, std::string &myDiscard, ActionInfo &actionInfo);

  void discardGreedy(std::vector<std::string> &holeCards, std::vector<std::string> &boardCards, std::string &myDiscard, ActionInfo &actionInfo);
  
  /* convert list of items from packet to vector of strings */
  void packetlist2vector(std::stringstream &ss, std::vector<std::string> &packetlist, int length);

  /* converts actionlist to legal action */
  LegalAction actionlist2struct(std::stringstream &ss, int length, std::string lastAction, bool myButton);

  /* convert last action word into action info */
  OpponentModeler::OppActionInfo actionword2struct(const std::string &actionword, OpponentModeler::ROUND round);

  /* get opopnent actions from action list */
  void actionlist2actioninfos(const std::vector<std::string> &lastActions, 
			      std::vector<OpponentModeler::OppActionInfo> & oppActions,
			      OpponentModeler::ROUND round);


  float informedEvaluate(const std::vector<std::string> &holeCards,
			 const std::vector<std::string> &boardCards,
			 const std::string &myDiscard,
			 int lastOpponentAction);

  /* gets last action of the player from action list */
  //TODO: get rid of this
  //  std::string getLastAction(const std::vector<std::string> &lastActions, const std::string &playerName);

};


#endif  // __ACTIONSELECTOR_HPP__
