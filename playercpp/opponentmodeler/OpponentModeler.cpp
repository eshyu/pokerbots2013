#include "OpponentModeler.hpp"

#include <iostream>
#include <fstream>

#define ME 0
#define OPP 1

//NOTE: comment this out to turn of using NN
//#define USE_NN

OpponentModeler::OpponentModeler()
{
  cardHeuristics = new CardHeuristics();
  nn = new NeuralNet();

  for (int i=0;i<NUM_PLAYERS;i++){
    playerStats[i] = new PlayerStatistics();    
  }

  NNFeatures= new float*[MAX_ACTIONS];
  NNOut= new float*[MAX_ACTIONS];
  for (int i=0;i<MAX_ACTIONS;i++){
    NNFeatures[i] = new float[43];
    NNOut[i] = new float[5];
  }

  oppActionCount=0;
  havePrediction=false;
}

OpponentModeler::~OpponentModeler(){}

void OpponentModeler::updateActionStatistics(int myAction, int betAmnt,
					     int oppAction, int oppBetAmnt,
					     bool myButton,
					     int potSize,
					     int stackSize,
					     float mybEq,
					     const std::vector<std::string> &boardCards,
					     const std::vector<std::string> &holeCards)
{
    
  ROUND round = (ROUND)numBoardCards2round(boardCards.size());

  bool doUpdateNN=true;
  float betRatio=0;
  switch(oppAction){
  case BET:
    playerStats[OPP]->numBet[round]+=1;
    betRatio=(float)oppBetAmnt/stackSize;
    break;
  case CHECK:
    playerStats[OPP]->numCheck[round]+=1;
    break;
  case CALL:
    playerStats[OPP]->numCall[round]+=1;
  case FOLD:
    playerStats[OPP]->numFold[round]+=1;
    break;
  case RAISE:
    playerStats[OPP]->numRaise[round]+=1;
    break;
  default:
    doUpdateNN=false;
    break;
  }
  
#ifdef USE_NN
  if (doUpdateNN){
    updateNNOut((ACTION)oppAction, betRatio);
    oppActionCount++;
  }
#endif

  // std::cout <<  "=========================" << handCount << " " << oppActionCount << std::endl;
  if (handCount > 150 && !havePrediction){
    //nn->train_net(200, 43, NNFeatures, 5, NNOut);
  }
	
  bool doCreateFeature=true;
  switch(myAction){
  case BET:
    playerStats[ME]->numBet[round]+=1;    
    break;
  case CHECK:
    playerStats[ME]->numCheck[round]+=1;
    break;
  case CALL:
    playerStats[ME]->numCall[round]+=1;
    break;
  case FOLD:
    playerStats[ME]->numFold[round]+=1;
    break;
  case RAISE:
    playerStats[ME]->numRaise[round]+=1;    
    break;
  default:    
    doCreateFeature=false;
    break;
  }
  
#ifdef USE_NN
  if (doCreateFeature){
    createActionNNFeatures(round, myButton, mybEq, betAmnt, stackSize, potSize,
			   holeCards, boardCards, oppActionCount);
  }
#endif

  //  if (oppActionCount > 0 && !(handCount % 50) && handCount> 200 ){     
  /*
  if (
    float *output = nn->get_output(NNFeatures[0]);
    std::cout << output[0] << " " << output[1] << " " << output[2] << " "<< " " << output[3] << " " << output[4] << std::endl;
  }
  */
}


void OpponentModeler::updateHandStats(int playerNumber, ACTION action, ROUND round)
{
  switch(action){
  case BET:
    currentHand.hasBet[playerNumber][round] |= true;
    break;
  case CHECK:
    currentHand.hasCheck[playerNumber][round] |= true;
    break;
  case RAISE:
    currentHand.hasRaise[playerNumber][round] |= true;
    break;
  case CALL:
  case FOLD:
  case DISCARD:
  case NONE:
    break;
  }

}

void OpponentModeler::newHand(){
  havePrediction=false;
  handCount++;
  currentHand.round=PREFLOP;
  for (int round=0;round<4;round++){
    currentHand.hasCheck[ME][round]=currentHand.hasCheck[OPP][round]=false;
    currentHand.hasBet[ME][round]=currentHand.hasBet[OPP][round]=false;
    currentHand.hasRaise[ME][round]=currentHand.hasRaise[OPP][round]=false;
    currentHand.mybEq[round]=0;    
  }
}

void OpponentModeler::printStats(){ 
  std::cout << "Check      \tBet     \tRaise      \tCall          \tFold" << std::endl;
  for (int round=0;round<4; round++){
    switch (round){
    case 0:
      std::cout << "Round: PREFLOP" << std::endl;
      break;
    case 1:
      std::cout << "Round: FLOP" << std::endl;
      break;
    case 2:
      std::cout << "Round: TURN" << std::endl;
      break;
    case 3:
      std::cout << "Round: RIVER" << std::endl;
      break;
    }
    std::cout << std::fixed << ((float)playerStats[ME]->numCheck[round]/(playerStats[ME]->numCheck[round] + playerStats[ME]->numBet[round] + playerStats[ME]->numRaise[round] + playerStats[ME]->numCall[round] + playerStats[ME]->numFold[round]))  << " "
              << ((float)playerStats[OPP]->numCheck[round]/(playerStats[OPP]->numCheck[round] + playerStats[OPP]->numBet[round] + playerStats[OPP]->numRaise[round] + playerStats[OPP]->numCall[round] + playerStats[OPP]->numFold[round]))  << "\t"

              << ((float)playerStats[ME]->numBet[round]/(playerStats[ME]->numCheck[round] + playerStats[ME]->numBet[round] + playerStats[ME]->numRaise[round] + playerStats[ME]->numCall[round] + playerStats[ME]->numFold[round]))  << " "
              << ((float)playerStats[OPP]->numBet[round]/(playerStats[OPP]->numCheck[round] + playerStats[OPP]->numBet[round] + playerStats[OPP]->numRaise[round] + playerStats[OPP]->numCall[round] + playerStats[OPP]->numFold[round]))  << "\t"

              << ((float)playerStats[ME]->numRaise[round]/(playerStats[ME]->numCheck[round] + playerStats[ME]->numBet[round] + playerStats[ME]->numRaise[round] + playerStats[ME]->numCall[round] + playerStats[ME]->numFold[round]))  << " "
              << ((float)playerStats[OPP]->numRaise[round]/(playerStats[OPP]->numCheck[round] + playerStats[OPP]->numBet[round] + playerStats[OPP]->numRaise[round] + playerStats[OPP]->numCall[round] + playerStats[OPP]->numFold[round]))  << "\t"

              << ((float)playerStats[ME]->numCall[round]/(playerStats[ME]->numCheck[round] + playerStats[ME]->numBet[round] + playerStats[ME]->numRaise[round] + playerStats[ME]->numCall[round] + playerStats[ME]->numFold[round]))  << " "
              << ((float)playerStats[OPP]->numCall[round]/(playerStats[OPP]->numCheck[round] + playerStats[OPP]->numBet[round] + playerStats[OPP]->numRaise[round] + playerStats[OPP]->numCall[round] + playerStats[OPP]->numFold[round]))  << "\t"

              << ((float)playerStats[ME]->numFold[round]/(playerStats[ME]->numCheck[round] + playerStats[ME]->numBet[round] + playerStats[ME]->numRaise[round] + playerStats[ME]->numCall[round] + playerStats[ME]->numFold[round]))  << " "
              << ((float)playerStats[OPP]->numFold[round]/(playerStats[OPP]->numCheck[round] + playerStats[OPP]->numBet[round] + playerStats[OPP]->numRaise[round] + playerStats[OPP]->numCall[round] + playerStats[OPP]->numFold[round]))  << " "
              << std::endl << std::endl;

    
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

/* NN Functions */
void OpponentModeler::updateNNOut(ACTION oppAction, float normalizedOppBet)
{
  for (int i=0;i<5;i++){
    NNOut[oppActionCount][i]=0;
  }

  switch(oppAction){
  case BET:
  case RAISE:
    NNOut[oppActionCount][0]=1;
    NNOut[oppActionCount][4]=normalizedOppBet;
    break;
  case CHECK:    
    NNOut[oppActionCount][1]=1;
    break;
  case CALL:
    NNOut[oppActionCount][2]=1;
    break;
  case FOLD:
    NNOut[oppActionCount][3]=1;
    break;
  default:
    break;
  }
}
			  
void OpponentModeler::createActionNNFeatures(ROUND round, bool myButton, float mybEq, int prevBet, int stackSize, int potSize,
					     const std::vector<std::string> &myHand, 
					     const std::vector<std::string> &boardCards, int actionNum){
  // 0-18
  // 
  cardHeuristics->createBoardTextureInput(myHand, boardCards, &NNFeatures[actionNum][0]);
  
  // 19-22
  // round number 
  for (int i=19;i<4;i++){
    NNFeatures[actionNum][i] = (round == (i-19)) ? 1 : 0;
  }
  
  // 23
  // myButton
  NNFeatures[actionNum][23] = myButton ? 1 : 0;

  // 24-29
  // round stats 
  // TODO:(why don't we do this using all the rounds)
  NNFeatures[actionNum][24] = currentHand.hasCheck[ME][round];
  NNFeatures[actionNum][25] = currentHand.hasCheck[OPP][round];
  NNFeatures[actionNum][26] = currentHand.hasBet[ME][round];
  NNFeatures[actionNum][27] = currentHand.hasBet[OPP][round];
  NNFeatures[actionNum][28] = currentHand.hasRaise[ME][round];
  NNFeatures[actionNum][29] = currentHand.hasRaise[OPP][round];

  // 30 - 39
  // aggregate stats
  NNFeatures[actionNum][30] = ((float)playerStats[ME]->numCheck[round]/(playerStats[ME]->numCheck[round] + playerStats[ME]->numBet[round] + playerStats[ME]->numRaise[round] + playerStats[ME]->numCall[round] + playerStats[ME]->numFold[round]));
  NNFeatures[actionNum][31] = ((float)playerStats[ME]->numCheck[round]/(playerStats[ME]->numCheck[round] + playerStats[ME]->numBet[round] + playerStats[ME]->numRaise[round] + playerStats[ME]->numCall[round] + playerStats[ME]->numFold[round]));

  NNFeatures[actionNum][32] = ((float)playerStats[ME]->numBet[round]/(playerStats[ME]->numCheck[round] + playerStats[ME]->numBet[round] + playerStats[ME]->numRaise[round] + playerStats[ME]->numCall[round] + playerStats[ME]->numFold[round]));
  NNFeatures[actionNum][33] = ((float)playerStats[OPP]->numCheck[round]/(playerStats[OPP]->numCheck[round] + playerStats[OPP]->numBet[round] + playerStats[OPP]->numRaise[round] + playerStats[OPP]->numCall[round] + playerStats[OPP]->numFold[round]));

  NNFeatures[actionNum][34] = ((float)playerStats[ME]->numRaise[round]/(playerStats[ME]->numCheck[round] + playerStats[ME]->numBet[round] + playerStats[ME]->numRaise[round] + playerStats[ME]->numCall[round] + playerStats[ME]->numFold[round]));
  NNFeatures[actionNum][35] = ((float)playerStats[OPP]->numRaise[round]/(playerStats[OPP]->numCheck[round] + playerStats[OPP]->numBet[round] + playerStats[OPP]->numRaise[round] + playerStats[OPP]->numCall[round] + playerStats[OPP]->numFold[round]));

  NNFeatures[actionNum][36] = ((float)playerStats[ME]->numCall[round]/(playerStats[ME]->numCheck[round] + playerStats[ME]->numBet[round] + playerStats[ME]->numRaise[round] + playerStats[ME]->numCall[round] + playerStats[ME]->numFold[round]));
  NNFeatures[actionNum][37] = ((float)playerStats[OPP]->numCall[round]/(playerStats[OPP]->numCheck[round] + playerStats[OPP]->numBet[round] + playerStats[OPP]->numRaise[round] + playerStats[OPP]->numCall[round] + playerStats[OPP]->numFold[round]));

  NNFeatures[actionNum][38] = ((float)playerStats[ME]->numFold[round]/(playerStats[ME]->numCheck[round] + playerStats[ME]->numBet[round] + playerStats[ME]->numRaise[round] + playerStats[ME]->numCall[round] + playerStats[ME]->numFold[round]));
  NNFeatures[actionNum][39] = ((float)playerStats[OPP]->numFold[round]/(playerStats[OPP]->numCheck[round] + playerStats[OPP]->numBet[round] + playerStats[OPP]->numRaise[round] + playerStats[OPP]->numCall[round] + playerStats[OPP]->numFold[round]));

  // prevbet
  // TODO(eshyu): ... why is this not normalized?
  NNFeatures[actionNum][40] = prevBet;

  // my beq
  NNFeatures[actionNum][41] = mybEq;

  // potSize / (2*stackSize)
  NNFeatures[actionNum][42] = (float)potSize/(2*stackSize);
  //

  if (actionNum % 5 == 0){
    std::cout << "=======================FEATURE INPUT IS : ";
    for (int i =0; i<43;i++){
      std::cout << NNFeatures[actionNum][i] << " ";
    }
    std::cout << std::endl;
  }
}
