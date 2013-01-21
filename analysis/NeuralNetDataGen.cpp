#include "../playercpp/opponentmodeler/OpponentModeler.hpp"
#include "../playercpp/evaluator/Evaluator.hpp"

#include <iostream>
#include <fstream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

#define SIMS_SLOW 5000
#define SIMS_FAST 10000

using namespace std;

enum ROUND {PREFLOP, FLOP, TURN, RIVER};

// data for neural net features
bool check[2][4];
bool bet[2][4];
bool hasRaise[2][4];
int numCheck[2][4];
int numBet[2][4];
int numRaise[2][4];
int numCall[2][4];
int numFold[2][4];  

void printData(double myEquity, bool myButton, double myBet, ROUND round, ofstream &fout){ 
  
  fout << myButton << " "
       << check[0][round] << " "
       << check[1][round] << " "
       << bet[0][round] << " "
       << bet[1][round] << " "
       << hasRaise[0][round] << " "
       << hasRaise[1][round] << " "
       << ((double)numCheck[0][round]/(numCheck[0][round] + numBet[0][round] + numRaise[0][round] + numCall[0][round] + numFold[0][round]))  << " "
       << ((double)numCheck[1][round]/(numCheck[1][round] + numBet[1][round] + numRaise[1][round] + numCall[1][round] + numFold[1][round]))  << " "

       << ((double)numBet[0][round]/(numCheck[0][round] + numBet[0][round] + numRaise[0][round] + numCall[0][round] + numFold[0][round]))  << " "
       << ((double)numBet[1][round]/(numCheck[1][round] + numBet[1][round] + numRaise[1][round] + numCall[1][round] + numFold[1][round]))  << " "

       << ((double)numRaise[0][round]/(numCheck[0][round] + numBet[0][round] + numRaise[0][round] + numCall[0][round] + numFold[0][round]))  << " "
       << ((double)numRaise[1][round]/(numCheck[1][round] + numBet[1][round] + numRaise[1][round] + numCall[1][round] + numFold[1][round]))  << " "

       << ((double)numCall[0][round]/(numCheck[0][round] + numBet[0][round] + numRaise[0][round] + numCall[0][round] + numFold[0][round]))  << " "
       << ((double)numCall[1][round]/(numCheck[1][round] + numBet[1][round] + numRaise[1][round] + numCall[1][round] + numFold[1][round]))  << " "

       << ((double)numFold[0][round]/(numCheck[0][round] + numBet[0][round] + numRaise[0][round] + numCall[0][round] + numFold[0][round]))  << " "
       << ((double)numFold[1][round]/(numCheck[1][round] + numBet[1][round] + numRaise[1][round] + numCall[1][round] + numFold[1][round]))  << " "
  
       << myBet << " " 
       << myEquity << " "

       << endl;
}

OpponentModeler::PlayerStatistics stats[2];
Evaluator evaluator;
string myName, oppName;
int stackSize, bb;

 int main(int argc, char *argv[])
 {
   std::string day, p1Name, p2Name;

   int me;

   boost::program_options::options_description desc("Allowed options");
   desc.add_options()
     ("help", "produce help message")
     ("day", boost::program_options::value<string>(&day), "day")
     ("p1", boost::program_options::value<string>(&p1Name), "p1")
     ("p2", boost::program_options::value<string>(&p2Name), "p2")    
     ("me", boost::program_options::value<int>(&me), "me")    
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

   if (me==1){
     myName = p1Name; oppName = p2Name; 
   } else {
     myName = p2Name; oppName = p1Name;
   }

   std::string suffix = "Casino_Day-" + day + "_" + p1Name + "_vs_" + p2Name + ".txt";
   std::string inPathName = "output/[ANNOTATED]" + suffix;
   std::string outPathName = "neural/" + suffix;
   ifstream fin(inPathName.c_str());
   ofstream fout(outPathName.c_str());

   // match statistics
   int numHands;
   int numShowdown;

   // player statistics variables
   int numPlayedHands[2]; //preflop
   int numRaisedHands[2]; //preflop
   int bankRoll[2];

   // hand state vars
   int handNum, potSize, toCall, prevRaise, button;
   bool preflopRaiseOnce[2];
   ROUND round;
   vector<string> hands[2];
   vector<string> boardCards;
   string discard[2];
   double myBet;
   double myBeq, oppTeq;

   // init stuff
   //  evaluator.populatePreFlopTable();
   for (int i=0;i<2;i++){
     for (int j=0;j<4;j++){
       numCheck[i][j]=0;
       numBet[i][j]=0;
       numRaise[i][j]=0;
       numCall[i][j]=0;
       numFold[i][j]=0;
     }
   }

   string line, first; 
   vector<string> tokens;
   while (!fin.eof()){    
     // OpponentModeler::HandStatistics myHandStats, oppHandStats;

     getline(fin, line);

     if (!line.size()){
       continue; //empty lines    
     }
     boost::split(tokens, line, boost::is_any_of("#[]()=,* "), boost::token_compress_on);

     first = tokens[0].compare("") ? tokens[0] : tokens[1]; // the first word might be ""

     if (!first.compare("6.S912")){      
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
       prevRaise=0;
       preflopRaiseOnce[0]=preflopRaiseOnce[1]=false;
       myBet=0;

       if (handNum % 100) cout << handNum << endl; // progress bar..

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

       if (player == 0)
	 myBeq = boost::lexical_cast<double>(tokens[7]);

     } else if (!first.compare("tEq:")){       
       oppTeq = boost::lexical_cast<double>(tokens[5]);

     } else if (!first.compare(myName) || !first.compare(oppName)){
       int player = !first.compare(myName) ? 0 : 1;
       if (!tokens[1].compare("posts")){
	 if (!player){
	   button = (tokens[5].compare("1")) ? 0 : 1;
	 }

       } else if (!tokens[1].compare("calls")){
	 potSize+=toCall;
	 double potOdds = (double)toCall/potSize;

	 toCall=0;

	 numCall[player][round]+=1;

	 if (player == 1 && round==PREFLOP){
	   printData(myBeq, button==0, myBet, round, fout);
	   fout << "0 0 1 0 0 " << " "<< oppTeq << endl;
	 }
	 
       } else if (!tokens[1].compare("checks")){
	 numCheck[player][round]+=1;
	 check[player][round]=true;
	 if (player == 1 && round==PREFLOP){
	   printData(myBeq, button==0, myBet, round, fout);
	   fout << "0 1 0 0 0 " << " " << oppTeq << endl;
	 }

       } else if (!tokens[1].compare("bets")){      
	 toCall=boost::lexical_cast<int>(tokens[2]);	
	 prevRaise=toCall;
	 potSize+=toCall;
	 double potOdds = (double)toCall/(potSize+toCall);

	numBet[player][round]+=1;
	bet[player][round]=true;

	if (player == 0){
	  myBet = potOdds;
	}
	
	if (player == 1 && round==PREFLOP){
	  printData(myBeq, button==0, myBet, round, fout);
	  fout << "1 0 0 0 " << potOdds << " " << oppTeq  << endl;
	}
	
       } else if (!tokens[1].compare("raises")){
	 int raise=boost::lexical_cast<int>(tokens[3]);
	
	// adjust tocall and potsize for blinds
	if (round == PREFLOP){
	  if (!preflopRaiseOnce[player]){
	    preflopRaiseOnce[player]=true;
	    int blind = (player == button) ? bb : bb/2;
	    potSize-=blind;
	    toCall=2;
	  }
	}

	toCall = raise-prevRaise;
	prevRaise=raise;
	potSize+=raise;

	double potOdds = (double)toCall/(potSize+toCall);	

	numRaise[player][round]+=1;
	hasRaise[player][round]=true;

	if (player == 0){
	  myBet = potOdds;
	}
	
	if (player == 1 && round==PREFLOP){
	  printData(myBeq, button==0, myBet, round, fout);
	  fout << "1 0 0 0 " << potOdds << " " <<  oppTeq  << endl;
	}	

      } else if (!tokens[1].compare("discards")){
	discard[player] = tokens[2];
	// remove discarded card from players' hands
	for (vector<string>::iterator it=hands[player].begin(); it !=hands[player].end();++it){
	  if (!tokens[2].compare(*it)){
	    hands[player].erase(it);
	    break;
	  }
	}
      } else if (!tokens[1].compare("folds")){
	int numSims = (boardCards.size() > 3) ? SIMS_FAST : SIMS_SLOW;
	double believedEquity = evaluator.evaluate(hands[player], boardCards, discard[player], numSims, false);

	numFold[player][round]+=1;

      } else if (!tokens[1].compare("shows")){
	 numShowdown++; //we showdown
      } else if (!tokens[1].compare("wins")){
	int player = !tokens[0].compare(myName) ? 0 : 1;
	//bankRoll[player]+=boost::lexical_cast<int>(tokens[4]);
	//bankRoll[1-player]-=boost::lexical_cast<int>(tokens[4]);
	potSize=0;
      }
      
    } else if (!first.compare("FLOP")){
      round = FLOP;
      boardCards.push_back(tokens[3]);
      boardCards.push_back(tokens[4]);
      boardCards.push_back(tokens[5]);                 

       myBet=0;
    } else if (!first.compare("TURN")){
      round = TURN;
      boardCards.push_back(tokens[6]);                 

      myBet=0;

    } else if (!first.compare("RIVER")){
      round = RIVER;
      boardCards.push_back(tokens[7]);           

      myBet=0;

    }
  }
  
  return 0;
}
