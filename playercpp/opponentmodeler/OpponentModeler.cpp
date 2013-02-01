#include "OpponentModeler.hpp"

#include <iostream>
#include <fstream>

#define ME 0
#define OPP 1

#define NN_IN_COUNT 51
#define NN_OUT_COUNT 4

//NOTE: comment this out to turn of using NN
//#define USE_NN

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
  trained=0;
  predicted=0;
  averageMse=0;
  sumMse=0;
  numMse=0;
}

OpponentModeler::~OpponentModeler(){}

/* Functions to update statistics */

void OpponentModeler::updatebEq(float myBeq, ROUND round)
{
  currentHand.mybEq[round] = myBeq;
}

/* Update opponent's statistics and NN output */
void OpponentModeler::updateOpponentActionStatistics(const std::vector<OpponentModeler::OppActionInfo> &oppActions,
					     bool myButton,
					     int potSize,
					     int stackSize,
					     const std::vector<std::string> &boardCards,
					     const std::vector<std::string> &holeCards)
{
  bool predict=false;
  bool doUpdateNN=true, finishedOldAction=false;
  std::cout << "OPMODEL:L51: NUMBER OF OPPONENT ACTIONS IS " << oppActions.size() << std::endl;
  // TODO(eshyu): LOL indexing is such that the most recent actions are at the front=
  int myBetAmount = currentHand.myPreviousBetAmount;
  float betRatio=0;
  int oppBetAmnt=0;
  ROUND round;
  ACTION oppAction;

  int betAmnts[oppActions.size()];
  for(int i=oppActions.size()-1; i>=0; i--){
    betAmnts[i]=oppActions[i].betAmount;
  }
  for (int i=oppActions.size()-1;i>=0;i--){
    betRatio=0;
    oppBetAmnt=oppActions[i].betAmount;

    int modPotSize=potSize;
    for(int j=0; j<i+1; j++){
      modPotSize-=betAmnts[j];
    }

    std::cout<<"&&& Modified Pot Size: "<<modPotSize<<std::endl;
    round = oppActions[i].round;
    oppAction = oppActions[i].action;

    std::cout << "OPPONENTMODELER:L52: action: " << oppAction << ", round: " << round << ", bet amount: " << oppBetAmnt << std::endl;

#ifdef USE_NN
    if (finishedOldAction){ //we have not already tried to predict his action as data oint
      std::cout << "Creating NN Feature for old action, oppomdeller:L70" << std::endl;
      createActionNNFeatures(round, myButton, currentHand.mybEq[round], myBetAmount, stackSize, modPotSize, holeCards, boardCards, oppActionCount);
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
      currentHand.hasCall[OPP][round]=true;
      std::cout << "OP call" << std::endl;
      break;
    case FOLD:
      playerStats[OPP]->numFold[round]+=1;
      currentHand.hasFold[OPP][round]=true;
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
}

/* Update our own statistics and NN inputs */
void OpponentModeler::updateOurActionStatistics(int myAction, int myBetAmount,
					     bool myButton,
					     int potSize,
					     int stackSize,
					     const std::vector<std::string> &boardCards,
					     const std::vector<std::string> &holeCards)
{
  bool predict=false;
  bool doCreateFeature=true;
  currentHand.myPreviousBetAmount=myBetAmount;
  ROUND round = (ROUND)numBoardCards2round(boardCards.size());
  
  switch(myAction){
  case BET:
    playerStats[ME]->numBet[round]+=1;
    currentHand.totalBets[round]+=myBetAmount;
    currentHand.hasBet[ME][round]=true;
    predict=true;
    break;
  case CHECK:
    playerStats[ME]->numCheck[round]+=1;
    currentHand.hasCheck[ME][round]=true;
    predict=true;
    break;
  case CALL:
    playerStats[ME]->numCall[round]+=1;
    currentHand.hasCall[ME][round]=true;
    break;
  case FOLD:
    playerStats[ME]->numFold[round]+=1;
    currentHand.hasCheck[ME][round]=true;
    break;
  case RAISE:
    playerStats[ME]->numRaise[round]+=1;
    currentHand.totalBets[round]+=myBetAmount;
    currentHand.hasRaise[ME][round]=true;
    predict=true;
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

  if (trained && predict){
    output = nn->get_output(NNFeatures[oppActionCount]);
    predicted=1;
    std::cout << "Neural Net prediction: " << output[0] << " " << output[1] << " " << output[2] << " "<< " " << output[3] << std::endl;//" " << output[4] << std::endl;
  }
#endif
}

/* remind us who is the better player */
void OpponentModeler::updateWinner(int player)
{
  playerStats[player]->numWonShowdowns++;
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
    currentHand.hasCall[playerNumber][round] |= true;
    break;
  case FOLD:
    currentHand.hasFold[playerNumber][round] |= true;
    break;
  case DISCARD:
  case NONE:
    break;
  }
}

// update stats when there is a show, what were real equities and stuff
void OpponentModeler::updateShow(const std::vector<std::string> &ourCards,
				 const std::vector<std::string> &oppCards,
				 std::vector<std::string> boardCards,
				 const std::string &myDiscard)
{
  // update the counts for actions/hand types
  for (int round=3; round>0;round--){
    std::vector<CardHeuristics::HAND_TYPE> handTypes;
    CardHeuristics::getHandType(oppCards, boardCards, handTypes);   
    for (int i=0;i<handTypes.size();i++){
      int handType = handTypes[i]; //ENUM
      if (currentHand.hasCheck[OPP][round]) HandDistribution[CHECK][handType]+=1;
      if (currentHand.hasBet[OPP][round]) HandDistribution[BET][handType]+=1;
      if (currentHand.hasRaise[OPP][round]) HandDistribution[RAISE][handType]+=1;
      if (currentHand.hasCall[OPP][round]) HandDistribution[CALL][handType]+=1;
    }
    
    boardCards.erase(boardCards.end());
 }  
}

void OpponentModeler::getHandDistribution(int actionNum, std::vector<float> &probs)
{
  ACTION action = (ACTION)actionNum;
  float p;
  for (int i=0;i<NUM_HAND_TYPES;i++){
    p = (float)HandDistribution[action][i]/
      (HandDistribution[BET][i] + 
       HandDistribution[CHECK][i] + 
       HandDistribution[RAISE][i]);
    std::cout << "Probability " << p << "for action " << action << std::endl;
    probs.push_back(p);
  }
}
 
std::string OpponentModeler::getHandRangeString(const std::vector<std::string> &myHand,
						const std::vector<std::string> &board,
						std::vector<float> &probs){
  return CardHeuristics::getEquityString(probs, myHand, board);  
}

void OpponentModeler::newHand(){
  handCount++;
  currentHand.round=PREFLOP;
  currentHand.myPreviousBetAmount=0;
  for (int round=0;round<4;round++){
    currentHand.hasCheck[ME][round]=currentHand.hasCheck[OPP][round]=false;
    currentHand.hasBet[ME][round]=currentHand.hasBet[OPP][round]=false;
    currentHand.hasRaise[ME][round]=currentHand.hasRaise[OPP][round]=false;
    currentHand.mybEq[round]=0;
    currentHand.oppBets[round]=0;
    currentHand.totalBets[round]=0;
    currentHand.mybEq[round]=0;
  }

#ifdef USE_NN
  // re-train neural net after every 50 hands
  if (( handCount % 501 == 0)){
    nn->train_net(totalActions, NN_IN_COUNT, NNFeatures, NN_OUT_COUNT, NNOut);
    trained=1;
  }
#endif
}

void OpponentModeler::printStats(){
  std::cout << "Check \tBet \tRaise \tCall \tFold" << std::endl;
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
    int oppTotal= (playerStats[OPP]->numCheck[round] + playerStats[OPP]->numBet[round] + playerStats[OPP]->numRaise[round] + playerStats[OPP]->numCall[round] + playerStats[OPP]->numFold[round]);

    int totalShowdowns = playerStats[0]->numWonShowdowns + playerStats[1]->numWonShowdowns;
									  
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
	      << std::endl 
	      << "We win " << ((float)playerStats[0]->numWonShowdowns/totalShowdowns) << "% of " << totalShowdowns << " of showdowns" << std::endl
	      << std::endl;
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
  for (int i=0;i<NN_OUT_COUNT;i++){
    NNOut[oppActionCount][i]=0;
  }
  std::string realAction;
  switch(oppAction){
  case BET:
  case RAISE:
    NNOut[oppActionCount][0]=1;
    //    NNOut[oppActionCount][4]=normalizedOppBet;
    realAction="betraise";
    break;
  case CHECK:
    NNOut[oppActionCount][1]=1;
    realAction="check";
    break;
  case CALL:
    NNOut[oppActionCount][2]=1;
    realAction="call";
    break;
  case FOLD:
    NNOut[oppActionCount][3]=1;
    realAction="fold";
    break;
  default:
    break;
  }
  //std::cout<<"trainedbool: "<<trained<<std::endl;
  if(trained && predicted){
    mse=calc_MSE(output, NNOut[oppActionCount], NN_OUT_COUNT);
    numMse++;
    sumMse+=mse;

    std::cout << "realaction: " << realAction << " vs. " << "predicted action:(bet,check,call,fold,raiseamnt" 
	      << output[0] 
	      << " " << output[1] 
      	      << " " << output[2] 
	      << " " << output[3] 
	      << " " << output[4]*400      
	      << std::endl;

    std::cout<<"calcMSE: "<<mse<<std::endl;
    std::cout<<"averageMse: "<<sumMse/numMse<<std::endl;
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

float OpponentModeler::calc_MSE(float * nnoutput, float * trueoutput, int num_output){
  float sum=0;
  for(int i=0; i<num_output; i++){
    std::cout<<"l49: "<<nnoutput[i]<<std::endl;
    std::cout<<"l51: "<<trueoutput[i]<<std::endl;
    float diff=nnoutput[i]-trueoutput[i];
    sum+=diff*diff;
  }
  return sum/num_output;
}
