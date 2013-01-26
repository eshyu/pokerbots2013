#include "OpponentModeler.hpp"

OpponentModeler::OpponentModeler()
{
  cardHeuristics = new CardHeuristics();
  
  for (int i=0;i<NUM_PLAYERS;i++){
    for (int j=0;j<NUM_ROUNDS;j++){
      playerStats[i].numCheck[j]=0;
      playerStats[i].numBet[j]=0;
      playerStats[i].numRaise[j]=0;
      playerStats[i].numFold[j]=0;
    }
  }

}

OpponentModeler::~OpponentModeler(){}

#define ME 0
#define OPP 1

void OpponentModeler::updateActionStatistics(int myAction, int betAmnt,
					     int oppAction, int oppBetAmnt,
					     bool myButton,
					     int potSize,
					     int stackSize,
					     const std::vector<std::string> &boardCards,
					     const std::vector<std::string> &holeCards)
{
  
  int round = numBoardCards2round(boardCards.size());
  switch(myAction){
  case BET:
    playerStats[ME].numBet[round]+=1;
    break;
  case CHECK:
    playerStats[ME].numCheck[round]+=1;
    break;
  case CALL:
    playerStats[ME].numCall[round]+=1;
    break;
  case FOLD:
    playerStats[ME].numFold[round]+=1;
    break;
  case RAISE:
    playerStats[ME].numRaise[round]+=1;
    break;
  default:
    break;
  }

  switch(oppAction){
  case BET:
    playerStats[OPP].numBet[round]+=1;
    break;
  case CHECK:
    playerStats[OPP].numCheck[round]+=1;
    break;
  case CALL:
    playerStats[OPP].numCall[round]+=1;
    break;
  case FOLD:
    playerStats[OPP].numFold[round]+=1;
    break;
  case RAISE:
    playerStats[OPP].numRaise[round]+=1;
    break;
  default:
    break;
  }

}
  
OpponentModeler::ROUND OpponentModeler::numBoardCards2round(int numBoardCards){
  switch(numBoardCards){
  case 0:
    return PREFLOP;
  case 3:
    return FLOP;
  case 4:
    return TURN;
  case 5:
    return RIVER;
  }
}
			  
void OpponentModeler::createActionNNFeatures(ROUND round, bool myButton, double mybEq, int prevBet, int stackSize, int potSize,
					     const std::vector<std::string> &myHand, 
					     const std::vector<std::string> &boardCards,
					     ActionNNFeatures &features){
  // 0-18
  // 
  cardHeuristics->createBoardTextureInput(myHand, boardCards, &features.features[0]);
  
  // 19-22
  // round number 
  for (int i=19;i<4;i++){
    features.features[i] = (round == (i-19)) ? 1 : 0;
  }
  
  // 23
  // myButton
  features.features[23] = myButton ? 1 : 0;

  // 24-29
  // round stats 
  // TODO:(why don't we do this using all the rounds)
  features.features[24] = currentHand.hasCheck[ME][round];
  features.features[25] = currentHand.hasCheck[OPP][round];
  features.features[26] = currentHand.hasBet[ME][round];
  features.features[27] = currentHand.hasBet[OPP][round];
  features.features[28] = currentHand.hasRaise[ME][round];
  features.features[29] = currentHand.hasRaise[OPP][round];

  // 30 - 39
  // aggregate stats
  features.features[30] = ((double)playerStats[ME].numCheck[round]/(playerStats[ME].numCheck[round] + playerStats[ME].numBet[round] + playerStats[ME].numRaise[round] + playerStats[ME].numCall[round] + playerStats[ME].numFold[round]));
  features.features[31] = ((double)playerStats[ME].numCheck[round]/(playerStats[ME].numCheck[round] + playerStats[ME].numBet[round] + playerStats[ME].numRaise[round] + playerStats[ME].numCall[round] + playerStats[ME].numFold[round]));

  features.features[32] = ((double)playerStats[ME].numBet[round]/(playerStats[ME].numCheck[round] + playerStats[ME].numBet[round] + playerStats[ME].numRaise[round] + playerStats[ME].numCall[round] + playerStats[ME].numFold[round]));
  features.features[33] = ((double)playerStats[OPP].numCheck[round]/(playerStats[OPP].numCheck[round] + playerStats[OPP].numBet[round] + playerStats[OPP].numRaise[round] + playerStats[OPP].numCall[round] + playerStats[OPP].numFold[round]));

  features.features[34] = ((double)playerStats[ME].numRaise[round]/(playerStats[ME].numCheck[round] + playerStats[ME].numBet[round] + playerStats[ME].numRaise[round] + playerStats[ME].numCall[round] + playerStats[ME].numFold[round]));
  features.features[35] = ((double)playerStats[OPP].numRaise[round]/(playerStats[OPP].numCheck[round] + playerStats[OPP].numBet[round] + playerStats[OPP].numRaise[round] + playerStats[OPP].numCall[round] + playerStats[OPP].numFold[round]));

  features.features[36] = ((double)playerStats[ME].numCall[round]/(playerStats[ME].numCheck[round] + playerStats[ME].numBet[round] + playerStats[ME].numRaise[round] + playerStats[ME].numCall[round] + playerStats[ME].numFold[round]));
  features.features[37] = ((double)playerStats[OPP].numCall[round]/(playerStats[OPP].numCheck[round] + playerStats[OPP].numBet[round] + playerStats[OPP].numRaise[round] + playerStats[OPP].numCall[round] + playerStats[OPP].numFold[round]));

  features.features[38] = ((double)playerStats[ME].numFold[round]/(playerStats[ME].numCheck[round] + playerStats[ME].numBet[round] + playerStats[ME].numRaise[round] + playerStats[ME].numCall[round] + playerStats[ME].numFold[round]));
  features.features[39] = ((double)playerStats[OPP].numFold[round]/(playerStats[OPP].numCheck[round] + playerStats[OPP].numBet[round] + playerStats[OPP].numRaise[round] + playerStats[OPP].numCall[round] + playerStats[OPP].numFold[round]));

  // prevbet
  // TODO(eshyu): ... why is this not normalized?
  features.features[40] = prevBet;

  // my beq
  features.features[41] = mybEq;

  // potSize / (2*stackSize)
  features.features[42] = (double)potSize/(2*stackSize);
  //
}
