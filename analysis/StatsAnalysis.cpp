#include "../playercpp/evaluator/Evaluator.hpp"
#include "../playercpp/opponentmodeler/OpponentModeler.hpp"

#include <iostream>
#include <fstream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#define SIMS_SLOW 5000
#define SIMS_FAST 10000

using namespace std;

OpponentModeler::PlayerStatistics stats[2];

Evaluator evaluator;

string myName, oppName;
int stackSize, bb;

enum ROUND {PREFLOP, FLOP, TURN, RIVER};


// data for neural net features
// inputs 
bool check[2][4];
bool bet[2][4];
bool hasRaise[2][4];

int numCheck[2][4];
int numBet[2][4];
int numRaise[2][4];
int numCall[2][4];
int numFold[2][4];  

int numBetFold[2][4];
int numCheckFold[2][4];

// betting stats
int minBet[2][4];
int maxBet[2][4];
int minRaise[2][4];
int maxRaise[2][4];
int totalBet[2][4];
int totalRaise[2][4];
double minBetbEq[2][4];
double maxBetbEq[2][4];
double totalBetbEq[2][4];
double minRaisebEq[2][4];
double maxRaisebEq[2][4];
double totalRaisebEq[2][4];
double minBettEq[2][4];
double maxBettEq[2][4];
double totalBettEq[2][4];
double minRaisetEq[2][4];
double maxRaisetEq[2][4];
double totalRaisetEq[2][4];

//equity stuff
double equityError[2][4];

// folding stats
double minFoldtEq[2][4];
double maxFoldtEq[2][4];
double totalFoldtEq[2][4];
double minFoldbEq[2][4];
double maxFoldbEq[2][4];
double totalFoldbEq[2][4];
double minFoldEqDiff[2][4];
double maxFoldEqDiff[2][4];
double totalFoldEqDiff[2][4];

// percent all hands all-in
int allin[2][4];

// all in stats
int numWon[2];
int numAllinWon[2];
int numAllinWonBreak[2][4];

void printData(ROUND round, ofstream &fout){   
  switch (round){
  case PREFLOP:
    cout << "Round: PREFLOP" << endl;
    break;
  case FLOP:
    cout << "Round: FLOP" << endl;
    break;
  case TURN:
    cout << "Round: TURN" << endl;
    break;
  case RIVER:
    cout << "Round: RIVER" << endl;
    break;
  }
  cout << "Check      \tBet     \tRaise      \tCall          \tFold" << endl;

  cout.precision(3);
  cout << fixed << ((double)numCheck[0][round]/(numCheck[0][round] + numBet[0][round] + numRaise[0][round] + numCall[0][round] + numFold[0][round]))  << " "
       << ((double)numCheck[1][round]/(numCheck[1][round] + numBet[1][round] + numRaise[1][round] + numCall[1][round] + numFold[1][round]))  << "\t"

       << ((double)numBet[0][round]/(numCheck[0][round] + numBet[0][round] + numRaise[0][round] + numCall[0][round] + numFold[0][round]))  << " "
       << ((double)numBet[1][round]/(numCheck[1][round] + numBet[1][round] + numRaise[1][round] + numCall[1][round] + numFold[1][round]))  << "\t"

       << ((double)numRaise[0][round]/(numCheck[0][round] + numBet[0][round] + numRaise[0][round] + numCall[0][round] + numFold[0][round]))  << " "
       << ((double)numRaise[1][round]/(numCheck[1][round] + numBet[1][round] + numRaise[1][round] + numCall[1][round] + numFold[1][round]))  << "\t"

       << ((double)numCall[0][round]/(numCheck[0][round] + numBet[0][round] + numRaise[0][round] + numCall[0][round] + numFold[0][round]))  << " "
       << ((double)numCall[1][round]/(numCheck[1][round] + numBet[1][round] + numRaise[1][round] + numCall[1][round] + numFold[1][round]))  << "\t"

       << ((double)numFold[0][round]/(numCheck[0][round] + numBet[0][round] + numRaise[0][round] + numCall[0][round] + numFold[0][round]))  << " "
       << ((double)numFold[1][round]/(numCheck[1][round] + numBet[1][round] + numRaise[1][round] + numCall[1][round] + numFold[1][round]))  << " "

    /*
       << endl << "Bet Fold: " 
       << ((double)numBetFold[0][round]/numBet[0][round]) << " "
       << ((double)numBetFold[1][round]/numBet[1][round]) 

       << endl << "Check Fold: " 
       << ((double)numCheckFold[0][round]/numCheck[0][round]) << " "
       << ((double)numCheckFold[1][round]/numCheck[1][round]) 
    */

       << endl << "===Betting Stats==="

       << endl << "(Min, Max, Avg) Bet: (" << minBet[0][round] << " "
       << maxBet[0][round] << " " << ((double)totalBet[0][round]/numBet[0][round]) 
       << ")" << "\t" 
       << "(" << minBet[1][round] << " "
       << maxBet[1][round] << " " << ((double)totalBet[1][round]/numBet[1][round]) 
       << ")" << "\t"

       << endl << "(Min, Max, Avg) Bet bEq: (" << minBetbEq[0][round] << " "
       << maxBetbEq[0][round] << " " << ((double)totalBetbEq[0][round]/numBet[0][round])
       << ")" << "\t"
       << "(" << minBetbEq[1][round] << " "
       << maxBetbEq[1][round] << " " << ((double)totalBetbEq[1][round]/numBet[1][round])
       << ")" << "\t"

       << endl << "(Min, Max, Avg) Bet tEq: (" << minBettEq[0][round] << " "
       << maxBettEq[0][round] << " " << ((double)totalBettEq[0][round]/numBet[0][round])
       << ")" << "\t"
       << "(" << minBettEq[1][round] << " "
       << maxBettEq[1][round] << " " << ((double)totalBettEq[1][round]/numBet[1][round])
       << ")" << "\t"

       << endl << "===Raising Stats==="

       << endl << "(Min, Max, Avg) Raise: (" << minRaise[0][round] << " "
       << maxRaise[0][round] << " " << ((double)totalRaise[0][round]/numRaise[0][round]) 
       << ")" << "\t" 
       << "(" << minRaise[1][round] << " "
       << maxRaise[1][round] << " " << ((double)totalRaise[1][round]/numRaise[1][round]) 
       << ")" << "\t" 


       << endl << "(Min, Max, Avg) Raise bEq: (" << minRaisebEq[0][round] << " "
       << maxRaisebEq[0][round] << " " << ((double)totalRaisebEq[0][round]/numRaise[0][round])
       << ")" << "\t"
       << "(" << minRaisebEq[1][round] << " "
       << maxRaisebEq[1][round] << " " << ((double)totalRaisebEq[1][round]/numRaise[1][round])
       << ")" << "\t"

       << endl << "(Min, Max, Avg) Raise tEq: (" << minRaisetEq[0][round] << " "
       << maxRaisetEq[0][round] << " " << ((double)totalRaisetEq[0][round]/numRaise[0][round])
       << ")" << "\t"
       << "(" << minRaisetEq[1][round] << " "
       << maxRaisetEq[1][round] << " " << ((double)totalRaisetEq[1][round]/numRaise[1][round])
       << ")" << "\t"

       << endl << "===Folding Stats==="

       << endl << "(Min, Max, Avg) Fold bEq: (" << minFoldbEq[0][round] << " "
       << maxFoldbEq[0][round] << " " << ((double)totalFoldbEq[0][round]/numFold[0][round]) 
       << ")" << "\t" 
       << "(" << minFoldbEq[1][round] << " "
       << maxFoldbEq[1][round] << " " << ((double)totalFoldbEq[1][round]/numFold[1][round]) 
       << ")" << "\t" 

       << endl << "(Min, Max, Avg) Fold tEq: (" << minFoldtEq[0][round] << " "
       << maxFoldtEq[0][round] << " " << ((double)totalFoldtEq[0][round]/numFold[0][round]) 
       << ")" << "\t" 
       << "(" << minFoldtEq[1][round] << " "
       << maxFoldtEq[1][round] << " " << ((double)totalFoldtEq[1][round]/numFold[1][round]) 
       << ")" << "\t" 

       << endl << "Avg Fold (bEq-tEq): " <<  ((double)totalFoldEqDiff[0][round]/numFold[0][round]) 
       << "\t"     
       << ((double)totalFoldEqDiff[1][round]/numFold[1][round]) 
       <<  endl
   
       << "% Allin won by us: " << (double)numAllinWonBreak[0][round]/(numAllinWonBreak[0][round]+numAllinWonBreak[1][round]) 
       << " out of " << (numAllinWonBreak[0][round] + numAllinWonBreak[1][round])
       << endl << endl;
}

 int main(int argc, char *argv[])
 {
   std::string day, p1Name, p2Name, type;

   boost::program_options::options_description desc("Allowed options");
   desc.add_options()
     ("help", "produce help message")
     ("day", boost::program_options::value<string>(&day), "day")
     ("p1", boost::program_options::value<string>(&p1Name), "p1")
     ("p2", boost::program_options::value<string>(&p2Name), "p2")    

     ("t", boost::program_options::value<string>(&type), "t")         
     ;

   boost::program_options::variables_map vm;
   boost::program_options::store(parse_command_line(argc, argv, desc), vm);
   boost::program_options::notify(vm);

   if (vm.count("help"))
     {
       cout << desc << "\n";
       return 1;
     }

   if (vm.count("day") == 0)
     {
      cout << "Must specify day" << endl;
       return 1;
     }

   if (vm.count("p1") == 0)
     {
       cout << "Must specify player 1" << endl;
       return 1;
     }

   if (vm.count("p2") == 0)
     {
       cout << "Must specify player 2" << endl;
       return 1;
     }


   bool minit = vm.count("t") == 0? false : true;
   std::string suffix;
   if (!type.compare("mt")){
     suffix = "MiniTournament_Mini-Tournament-Round-1_" + p1Name + "_vs_" + p2Name + ".txt";
   } else {
     suffix = "Casino_Day-" + day + "_" + p1Name + "_vs_" + p2Name + ".txt";
   }
  
   std::string inPathName = "output/[ANNOTATED]" + suffix;
   //   std::string inPathName = "handhistories/" + suffix;
   std::string outPathName = "reports/" + suffix;

   ifstream fin(inPathName.c_str());
   ofstream fout(outPathName.c_str());

   string line, first; 
   vector<string> tokens;

   // match statistics
   int numHands=0;
   int numShowdown=0;
   
   // player statistics variables
   int numPlayedHands[2]; //preflop
   int numRaisedHands[2]; //preflop
   int bankRoll[2];

   double hiddenEquity[2][4];
   double trueEquity[2][4];

   // outputs

   // hand state vars
   int handNum, potSize, toCall, raiseDiff, button;
   bool preflopRaiseOnce[2];
   ROUND round;
   vector<string> hands[2];
   vector<string> boardCards;
   string discard[2];
   double myBet;

   int allinround;

   // init stuff
   //  evaluator.populatePreFlopTable();
   for (int i=0;i<2;i++){
     for (int j=0;j<4;j++){
       numCheck[i][j]=0;
       numBet[i][j]=0;
       numRaise[i][j]=0;
       numCall[i][j]=0;
       numFold[i][j]=0;
       numBetFold[i][j]=0;

       numCheckFold[i][j]=0;
       minRaise[i][j]=400;
       maxRaise[i][j]=0;
       totalRaise[i][j]=0;
       minBet[i][j]=400;
       maxBet[i][j]=0;
       totalBet[i][j]=0;

       minFoldbEq[i][j]=1;
       maxFoldbEq[i][j]=0;
       totalFoldbEq[i][j]=0;
       minFoldtEq[i][j]=1;
       maxFoldtEq[i][j]=0;
       totalFoldtEq[i][j]=0;

       minBetbEq[i][j]=1;
       maxBetbEq[i][j]=0;
       totalBetbEq[i][j]=0;
       minBettEq[i][j]=1;
       maxBettEq[i][j]=0;
       totalBettEq[i][j]=0;

       minRaisebEq[i][j]=1;
       maxRaisebEq[i][j]=0;
       totalRaisebEq[i][j]=0;
       minRaisetEq[i][j]=1;
       maxRaisetEq[i][j]=0;
       totalRaisetEq[i][j]=0;

       minFoldEqDiff[i][j]=1;
       maxFoldEqDiff[i][j]=-1;
       totalFoldEqDiff[i][j]=0;       
       
       numAllinWonBreak[i][j]=0;
     }
   }
   
   numWon[0]=numWon[1]=0;
   numAllinWon[0]=numAllinWon[1]=0;
   int i=0;
   while (!fin.eof()){    
     // OpponentModeler::HandStatistics myHandStats, oppHandStats;

     getline(fin, line);

     if (!line.size()){
       i++;
       continue; //empty lines    
     }

     boost::split(tokens, line, boost::is_any_of("#[]()=,* "), boost::token_compress_on);

     first = tokens[0].compare("") ? tokens[0] : tokens[1]; // the first word might be ""

     if (!first.compare("6.S912")){      
       // //cout << line << endl;
       myName = tokens[4]; oppName= tokens[6];
       stackSize=boost::lexical_cast<int>(tokens[8]); 
       bb=boost::lexical_cast<int>(tokens[10]);

     } else if (!first.compare("Hand")){
       round = PREFLOP;

       handNum = boost::lexical_cast<int>(tokens[1]);
       boardCards.clear(); // clear board cards
       hands[0].clear(); hands[1].clear();
       discard[0]=discard[1]="";
       potSize= (3 * bb)/2;
       toCall=1;
       preflopRaiseOnce[0]=preflopRaiseOnce[1]=false;
       myBet=0;
       allinround=-1;

       for (int i=0;i<2;i++){
	 for (int j=0;j<4;j++){
	   check[i][j]=false;
	   bet[i][j]=false;
	   hasRaise[i][j]=false;
	 }
       }

     } else if (!first.compare("Dealt")){
       int player = !tokens[2].compare(myName) ? 0 : 1;
       hands[player].push_back(tokens[3]);
       hands[player].push_back(tokens[4]);
       hands[player].push_back(tokens[5]);      

     } else if (!first.compare(myName) || !first.compare(oppName)){
       int player = !first.compare(myName) ? 0 : 1;
       if (!tokens[1].compare("posts")){
	 if (!player){
	   button = (tokens[5].compare("1")) ? 0 : 1;
	 }

       } else if (!tokens[1].compare("calls")){
	 numCall[player][round]+=1;
       } else if (!tokens[1].compare("checks")){
	 numCheck[player][round]+=1;
	 check[player][round]=true;
       } else if (!tokens[1].compare("bets")){      
	 toCall=boost::lexical_cast<int>(tokens[2]);	
	 
	 double bEq = boost::lexical_cast<double>(tokens[4]);
	 double tEq = boost::lexical_cast<double>(tokens[5]);

	numBet[player][round]+=1;
	bet[player][round]=true;
	
	minBet[player][round] = min(minBet[player][round], toCall);
	maxBet[player][round] = max(maxBet[player][round], toCall);		
	totalBet[player][round] += toCall;

	minBetbEq[player][round] = min(minBetbEq[player][round], bEq);
	maxBetbEq[player][round] = max(maxBetbEq[player][round], bEq);
	totalBetbEq[player][round] += bEq;
	minBettEq[player][round] = min(minBettEq[player][round], tEq);
	maxBettEq[player][round] = max(maxBettEq[player][round], tEq);
	totalBettEq[player][round] += tEq;

	if (toCall == stackSize) allin[player][round]+=1;

       } else if (!tokens[1].compare("raises")){
	 int raise=boost::lexical_cast<int>(tokens[3]);
	 double bEq = boost::lexical_cast<double>(tokens[5]);
	 double tEq = boost::lexical_cast<double>(tokens[6]);

	numRaise[player][round]+=1;
	hasRaise[player][round]=true;

	minRaise[player][round] = min(minRaise[player][round], raise);
	maxRaise[player][round] = max(maxRaise[player][round], raise);
	totalRaise[player][round] += raise;
	
	minRaisebEq[player][round] = min(minRaisebEq[player][round], bEq);
	maxRaisebEq[player][round] = max(maxRaisebEq[player][round], bEq);
	totalRaisebEq[player][round] += bEq;

	minRaisetEq[player][round] = min(minRaisetEq[player][round], tEq);
	maxRaisetEq[player][round] = max(maxRaisetEq[player][round], tEq);
	totalRaisetEq[player][round] += tEq;

	if (toCall == stackSize) allin[player][round]+=1;

      } else if (!tokens[1].compare("discards")){
	discard[player] = tokens[2];
	for (vector<string>::iterator it=hands[player].begin(); it !=hands[player].end();++it){
	  if (!tokens[2].compare(*it)){
	    hands[player].erase(it);
	    break;
	  }
	}

      } else if (!tokens[1].compare("folds")){

	 double bEq = boost::lexical_cast<double>(tokens[3]);
	 double tEq = boost::lexical_cast<double>(tokens[4]);

	minFoldbEq[player][round] = min(minFoldbEq[player][round], bEq);
	maxFoldbEq[player][round] = max(maxFoldbEq[player][round], bEq);
	totalFoldbEq[player][round] += bEq;

	minFoldtEq[player][round] = min(minFoldtEq[player][round], tEq);
	maxFoldtEq[player][round] = max(maxFoldtEq[player][round], tEq);
	totalFoldtEq[player][round] += tEq;

	minFoldEqDiff[player][round] = min(minFoldEqDiff[player][round], bEq-tEq);
	maxFoldEqDiff[player][round] = max(maxFoldEqDiff[player][round], bEq-tEq);
	totalFoldEqDiff[player][round] += bEq-tEq;


	 numFold[player][round]+=1;
	if (bet[player][round]) numBetFold[player][round]+=1;

	if (check[player][round]) numCheckFold[player][round]+=1;

      } else if (!tokens[1].compare("shows")){
	numShowdown++;
      } else if (!tokens[1].compare("wins")){
	int player = !tokens[0].compare(myName) ? 0 : 1;
	
	numWon[player]+=1;
	if (boost::lexical_cast<int>(tokens[4]) == 800){
	    numAllinWon[player]+=1;
	    if(allinround==-1) allinround=RIVER;
	    numAllinWonBreak[player][allinround]+=1;
	}
      }
      
    } else if (!first.compare("FLOP")){
      round = FLOP;
      boardCards.push_back(tokens[3]);
      boardCards.push_back(tokens[4]);
      boardCards.push_back(tokens[5]);                 

      int thepot = boost::lexical_cast<int>(tokens[2]);
      if (thepot == 800 && allinround==-1) allinround=PREFLOP;

    } else if (!first.compare("TURN")){
      round = TURN;
      boardCards.push_back(tokens[6]);                 
      
      int thepot = boost::lexical_cast<int>(tokens[2]);
      if (thepot == 800 && allinround==-1) allinround=FLOP;

    } else if (!first.compare("RIVER")){
      round = RIVER;
      boardCards.push_back(tokens[7]);           

      int thepot = boost::lexical_cast<int>(tokens[2]);
      if (thepot == 800 && allinround==-1) allinround=TURN;
    }

    i++;
    
  }

   
   cout << "% Hands won by us: " << (double)numWon[0]/(numWon[0]+numWon[1]) << " out of " << (numWon[0]+numWon[1]) << endl;
   cout << "% Allin won by us: " << (double)numAllinWon[0]/(numAllinWon[0]+numAllinWon[1]) << " out of " << (numAllinWon[0] + numAllinWon[1]) << endl;
   cout << "Num showdowns: " << (numShowdown/2) << endl;
   printData(PREFLOP, fout);
   printData(FLOP, fout);
   printData(TURN, fout);
   printData(RIVER, fout);

  return 0;
}
