#include "OpponentModeler.hpp"

#include <iostream>
#include <fstream>

#define ME 0
#define OPP 1

#define NN_IN_COUNT 51
#define NN_OUT_COUNT 5

//NOTE: comment this out to turn of using NN
#define USE_NN

OpponentModeler::OpponentModeler()
{
  for (int i=0;i<NUM_PLAYERS;i++){
    playerStats[i] = new PlayerStatistics();    
  }

#ifdef USE_NN
  nn = new NeuralNet();
  NNFeatures= new float*[MAX_ACTIONS];
  NNOut= new float*[MAX_ACTIONS];
  for (int i=0;i<MAX_ACTIONS;i++){
    NNFeatures[i] = new float[NN_IN_COUNT];
    NNOut[i] = new float[NN_OUT_COUNT];
  }
#endif 

  totalActions=0;
  handCount=0;
  oppActionCount=0;
}

OpponentModeler::~OpponentModeler(){}

/* Functions to update statistics */

void OpponentModeler::updatebEq(float myBeq, ROUND round)
{
  currentHand.mybEq[round] = myBeq;
}

void OpponentModeler::updateActionStatistics(int myAction, int myBetAmount,
					     const std::vector<OpponentModeler::OppActionInfo> &oppActions,
					     bool myButton,
					     int potSize,
					     int stackSize,
					     const std::vector<std::string> &boardCards,
					     const std::vector<std::string> &holeCards)
{
  bool doUpdateNN=true, finishedOldAction=false;
  std::cout << "OPMODEL:L51: NUMBER OF OPPONENT ACTIONS IS " << oppActions.size() << std::endl;
  // TODO(eshyu): LOL indexing is such that the most recent actions are at the front=  
  float betRatio=0;
  int oppBetAmnt=0;
  ROUND round;
  ACTION oppAction;
  for (int i=oppActions.size()-1;i>=0;i--){    
    betRatio=0;
    oppBetAmnt=oppActions[i].betAmount;
    round = oppActions[i].round;
    oppAction = oppActions[i].action;

    std::cout << "OPPONENTMODELER:L52: action: " << oppAction << ", round: " << round << ", bet amount: " << oppBetAmnt << std::endl;

#ifdef USE_NN
    if (finishedOldAction){ //we have not already tried to predict his action as data oint
      std::cout << "Creating NN Feature for old action, oppomdeller:L70" << std::endl;
      createActionNNFeatures(round, myButton, currentHand.mybEq[round], myBetAmount, stackSize, potSize, holeCards, boardCards, oppActionCount);
    }
#endif
    switch(oppAction){
    case BET:
      playerStats[OPP]->numBet[round]+=1;
      betRatio=(float)oppBetAmnt/stackSize;
      currentHand.oppBets[round]+=oppBetAmnt;
      currentHand.totalBets[round]+=oppBetAmnt;
      currentHand.hasBet[OPP][round]=true;
      std::cout << "OP BET" << std::endl;
      break;
    case CHECK:
      playerStats[OPP]->numCheck[round]+=1;
      currentHand.hasCheck[OPP][round]=true;
      std::cout << "OP check" << std::endl;
      break;
    case CALL:
      playerStats[OPP]->numCall[round]+=1;
      std::cout << "OP call" << std::endl;
      break;
    case FOLD:
      playerStats[OPP]->numFold[round]+=1;
      
      std::cout << "OP fold" << std::endl;
      break;
    case RAISE:
      playerStats[OPP]->numRaise[round]+=1;
      currentHand.oppBets[round]+=oppBetAmnt;
      currentHand.totalBets[round]+=oppBetAmnt;
      currentHand.hasRaise[OPP][round]=true;
      std::cout << "OP raise" << std::endl;
      break;
    default:
      doUpdateNN=false;
      break;
    }
    

#ifdef USE_NN
    if (doUpdateNN && oppActions.size() > 0){
      std::cout << "Updating NN Output:109 for oppAction: " << totalActions << std::endl;
      updateNNOut((ACTION)oppAction, betRatio);
      oppActionCount = (oppActionCount + 1) % MAX_ACTIONS;
      totalActions++;
    }
#endif
    
    doUpdateNN=true;
    finishedOldAction=true;    
  }

  // std::cout <<  "=========================" << handCount << " " << oppActionCount << std::endl;

  bool doCreateFeature=true;
  round = (ROUND)numBoardCards2round(boardCards.size());
  
  switch(myAction){
  case BET:
    playerStats[ME]->numBet[round]+=1;    
    currentHand.totalBets[round]+=myBetAmount;
    currentHand.hasBet[ME][round]=true;
    break;
  case CHECK:
    playerStats[ME]->numCheck[round]+=1;
    currentHand.hasCheck[ME][round]=true;
    break;
  case CALL:
    playerStats[ME]->numCall[round]+=1;
    break;
  case FOLD:
    playerStats[ME]->numFold[round]+=1;
    break;
  case RAISE:
    playerStats[ME]->numRaise[round]+=1;    
    currentHand.totalBets[round]+=myBetAmount;
    currentHand.hasRaise[ME][round]=true;
    break;
  default:    
    doCreateFeature=false;
    break;
  }
  
#ifdef USE_NN
  if (doCreateFeature){
    std::cout << "Creating NN Feature for new action, oppomdeller:L151" << std::endl;
    createActionNNFeatures(round, myButton, currentHand.mybEq[round], myBetAmount, stackSize, potSize, holeCards, boardCards, oppActionCount);
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
  handCount++;
  currentHand.round=PREFLOP;
  for (int round=0;round<4;round++){
    currentHand.hasCheck[ME][round]=currentHand.hasCheck[OPP][round]=false;
    currentHand.hasBet[ME][round]=currentHand.hasBet[OPP][round]=false;
    currentHand.hasRaise[ME][round]=currentHand.hasRaise[OPP][round]=false;
    currentHand.mybEq[round]=0;    
    currentHand.oppBets[round]=0;
    currentHand.totalBets[round]=0;
    currentHand.mybEq[round]=0;
  }

  // re-train neural net after every 50 hands 
  if (( handCount % 50 == 0) && totalActions > 200){
    nn->train_net(200, 43, NNFeatures, 5, NNOut);
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

    int myTotal = (playerStats[ME]->numCheck[round] + playerStats[ME]->numBet[round] + playerStats[ME]->numRaise[round] + playerStats[ME]->numCall[round] + playerStats[ME]->numFold[round]);
    int oppTotal=  (playerStats[OPP]->numCheck[round] + playerStats[OPP]->numBet[round] + playerStats[OPP]->numRaise[round] + playerStats[OPP]->numCall[round] + playerStats[OPP]->numFold[round]);
    
    std::cout.precision(3);
    std::cout << std::fixed 
	      << ((float)playerStats[ME]->numCheck[round]/myTotal) << " "
	      << ((float)playerStats[OPP]->numCheck[round]/oppTotal) << "\t"
      
	      << ((float)playerStats[ME]->numBet[round]/myTotal) << " "
	      << ((float)playerStats[OPP]->numBet[round]/oppTotal) << "\t"
      
	      << ((float)playerStats[ME]->numRaise[round]/myTotal) << " "
	      << ((float)playerStats[OPP]->numRaise[round]/oppTotal) << "\t"
      
	      << ((float)playerStats[ME]->numCall[round]/myTotal) << " "
	      << ((float)playerStats[OPP]->numCall[round]/oppTotal) << "\t"
      
	      << ((float)playerStats[ME]->numFold[round]/myTotal) << " "
	      << ((float)playerStats[OPP]->numFold[round]/oppTotal) << "\t"
      
	      << std::endl << std::endl;    
  }
}
  
OpponentModeler::ROUND OpponentModeler::numBoardCards2round(int numBoardCards){
  switch(numBoardCards){
  case 0:
    std::cout << "ROUND IS PREFLOP" << std::endl;
    return PREFLOP;
  case 3:
    std::cout << "ROUND IS FLOP" << std::endl;
    return FLOP;
  case 4:
    std::cout << "ROUND IS turn" << std::endl;
    return TURN;
  case 5:
    std::cout << "ROUND IS river" << std::endl;
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
  CardHeuristics::createBoardTextureInput(myHand, boardCards, &NNFeatures[actionNum][0]);
  
  // 19-22
  // round number 
  for (int i=19;i<23;i++){
    NNFeatures[actionNum][i] = ((int)round == (i-19)) ? 1 : 0;
  }
  
  // 23
  // myButton
  NNFeatures[actionNum][23] = myButton ? 1 : 0;

  // 24-29
  // round stats 
  // TODO:(why don't we do this using all the rounds)
  NNFeatures[actionNum][24] = currentHand.hasCheck[ME][round] ? 1 : 0;
  NNFeatures[actionNum][25] = currentHand.hasCheck[OPP][round] ? 1 : 0;
  NNFeatures[actionNum][26] = currentHand.hasBet[ME][round] ? 1 : 0;
  NNFeatures[actionNum][27] = currentHand.hasBet[OPP][round] ? 1 : 0;
  NNFeatures[actionNum][28] = currentHand.hasRaise[ME][round]? 1 : 0;
  NNFeatures[actionNum][29] = currentHand.hasRaise[OPP][round] ? 1 : 0;

  // 30 - 39
  // aggregate stats


  int oppTotal = (playerStats[OPP]->numCheck[round] + playerStats[OPP]->numBet[round] + playerStats[OPP]->numRaise[round] + playerStats[OPP]->numCall[round] + playerStats[OPP]->numFold[round]);
  int myTotal = (playerStats[ME]->numCheck[round] + playerStats[ME]->numBet[round] + playerStats[ME]->numRaise[round] + playerStats[ME]->numCall[round] + playerStats[ME]->numFold[round]);
  
  //check
  NNFeatures[actionNum][30] = ((float)playerStats[ME]->numCheck[round]/myTotal);
  NNFeatures[actionNum][31] = ((float)playerStats[OPP]->numCheck[round]/oppTotal);

  //bet
  NNFeatures[actionNum][32] = ((float)playerStats[ME]->numBet[round]/myTotal);
  NNFeatures[actionNum][33] = ((float)playerStats[OPP]->numBet[round]/oppTotal);
			       
  //raise
  NNFeatures[actionNum][34] = ((float)playerStats[ME]->numRaise[round]/myTotal);
  NNFeatures[actionNum][35] = ((float)playerStats[OPP]->numRaise[round]/oppTotal);
  
  // call
  NNFeatures[actionNum][36] = ((float)playerStats[ME]->numCall[round]/myTotal);
  NNFeatures[actionNum][37] = ((float)playerStats[OPP]->numCall[round]/oppTotal);

  // fold
  NNFeatures[actionNum][38] = ((float)playerStats[ME]->numFold[round]/myTotal);
  NNFeatures[actionNum][39] = ((float)playerStats[OPP]->numFold[round]/oppTotal);

  // prevbet
  NNFeatures[actionNum][40] = (float)prevBet/stackSize ;

  // my beq
  NNFeatures[actionNum][41] = mybEq;

  // potSize / (2*stackSize)
  NNFeatures[actionNum][42] = (float)potSize/(2*stackSize);

  // opp bets
  for (int i=43;i<47;i++){
    NNFeatures[actionNum][i] = (float)currentHand.oppBets[i-43]/(2*stackSize);
  }

  // totalBets
  for (int i=47;i<51;i++){
    NNFeatures[actionNum][i] = (float)currentHand.totalBets[i-47]/(2*stackSize);
  }

  
  std::cout << "=======================FEATURE INPUT IS : " << totalActions ;
  std::cout << std::endl << "board heuristic: 0-18= ";
  for (int i =0; i<19;i++){
    std::cout << NNFeatures[actionNum][i] << " ";
  }
  std::cout << std::endl << "round number, button 19-23= ";
  for (int i =19; i<24;i++){
    std::cout << NNFeatures[actionNum][i] << " ";
  }
  std::cout << std::endl << "round actions 24-29 =" ;
  for (int i =24; i<30;i++){
    std::cout << NNFeatures[actionNum][i] << " ";
    }
  std::cout << std::endl << "aggregate stats 30-39= ";
  for (int i =30; i<40;i++){
    std::cout << NNFeatures[actionNum][i] << " ";
  }
  std::cout << std::endl << "prevBet, myBEq, potsize 40-42= ";
  for (int i =40; i<43;i++){
    std::cout << NNFeatures[actionNum][i] << " ";
  }
  std::cout << std::endl << "opp bets 43-46= ";
  for (int i=43;i<47;i++){
    std::cout << NNFeatures[actionNum][i] << " ";
  }
  std::cout << std::endl << "total bets 47-51= ";
  for (int i=47;i<51;i++){
    std::cout << NNFeatures[actionNum][i] << " ";
  }
  std::cout << std::endl;
}

