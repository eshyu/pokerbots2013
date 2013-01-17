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

int main(int argc, char *argv[])
{
  std::string day, p1Name, p2Name;

  boost::program_options::options_description desc("Allowed options");
  desc.add_options()
    ("help", "produce help message")
    ("daynumber", boost::program_options::value<string>(&day), "day number")
    ("player1", boost::program_options::value<string>(&p1Name), "player 1 name")
    ("player2", boost::program_options::value<string>(&p2Name), "player 2 name")    
    ;

  boost::program_options::variables_map vm;
  boost::program_options::store(parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);

  if (vm.count("help"))
    {
      //cout << desc << "\n";
      return 1;
    }

  if (vm.count("daynumber") == 0)
    {
      //cout << "Must specify day" << endl;
      return 1;
    }

  if (vm.count("player1") == 0)
    {
      //cout << "Must specify player 1" << endl;
      return 1;
    }

  if (vm.count("player2") == 0)
    {
      //cout << "Must specify player 2" << endl;
      return 1;
    }

  std::string suffix = "Casino_Day-" + day + "_" + p1Name + "_vs_" + p2Name + ".txt";
  std::string inPathName = "handhistories/" + suffix;
  std::string outPathName = "output/[ANNOTATED]" + suffix;
  
  ifstream fin(inPathName.c_str());
  ofstream fout(outPathName.c_str());

  string line, first; 
  vector<string> tokens;

  // match statistics
  int numHands;
  int numShowdown;
  
  // player statistics variables
  int numPlayedHands[2]; //preflop
  int numRaisedHands[2]; //preflop
  int bankRoll[2];

  double hiddenEquity[2][4];
  double trueEquity[2][4];

  // hand state vars
  int handNum, potSize, toCall, raiseDiff, button;
  bool preflopRaiseOnce[2];
  ROUND round;
  vector<string> hands[2];
  vector<string> boardCards;
  string discard[2];
  
  // init stuff
  evaluator.populatePreFlopTable();

  int i=0;
  while (!fin.eof()){    
    // OpponentModeler::HandStatistics myHandStats, oppHandStats;

    getline(fin, line);
    
    if (!line.size()){
      //cout << endl;
      fout << endl;
      i++;
      continue; //empty lines    
    }
    boost::split(tokens, line, boost::is_any_of("#[]()=,* "), boost::token_compress_on);
    
    first = tokens[0].compare("") ? tokens[0] : tokens[1]; // the first word might be ""
    //cout << line;
    fout << line;
    
    if (!first.compare("6.S912")){      
      // //cout << line << endl;
      myName = tokens[4]; oppName= tokens[6];
      stackSize=boost::lexical_cast<int>(tokens[8]); 
      bb=boost::lexical_cast<int>(tokens[10]);

      //      bankRoll[0]=bankRoll[1]=stackSize;
      ////cout << myName << " " << oppName << " " << stackSize << " " << bb << endl;
      
    } else if (!first.compare("Hand")){
      round = PREFLOP;
      
      handNum = boost::lexical_cast<int>(tokens[1]);
      boardCards.clear(); // clear board cards
      hands[0].clear(); hands[1].clear();
      discard[0]=discard[1]="";
      potSize= (3 * bb)/2;
      toCall=1;
      preflopRaiseOnce[0]=preflopRaiseOnce[1]=false;
      cout << handNum << endl;

    } else if (!first.compare("Dealt")){
      int player = !tokens[2].compare(myName) ? 0 : 1;
      hands[player].push_back(tokens[3]);
      hands[player].push_back(tokens[4]);
      hands[player].push_back(tokens[5]);
      
      double aHiddenEquity = evaluator.evaluate(hands[player], boardCards, discard[player], false);

      //cout << " (hEq: " << aHiddenEquity << ")";
      fout << " (hEq: " << aHiddenEquity << ")";
	
      if (button==player){
	//cout << endl;
	fout << endl;
	double aRealEquity = evaluator.evaluate_true(hands[0], hands[1], boardCards, discard[0], discard[1]);
	trueEquity[0][round]=aRealEquity;
	trueEquity[1][round]=1-aRealEquity;
	//cout << "tEq: " << hands[0][0] << hands[0][1] << hands[0][2] << " (" << aRealEquity <<") vs. " << hands[1][0] << hands[1][1] << hands[1][2] << " (" << 1-aRealEquity <<")";
	fout << "tEq: " << hands[0][0] << hands[0][1] << hands[0][2] << " (" << aRealEquity <<") vs. " << hands[1][0] << hands[1][1] << hands[1][2] << " (" << 1-aRealEquity <<")";
      }

    } else if (!first.compare(myName) || !first.compare(oppName)){
      int player = !first.compare(myName) ? 0 : 1;
      if (!tokens[1].compare("posts")){
	if (!player){
	  button = (tokens[5].compare("1")) ? 0 : 1;
	  //	  //cout << "myButton: " << (button == 0) << endl;
	}
      } else if (!tokens[1].compare("calls")){
	potSize+=toCall;
	double potOdds = (double)toCall/potSize;
	int numSims = (boardCards.size() > 3) ? SIMS_FAST : SIMS_SLOW;
	double believedEquity = evaluator.evaluate(hands[player], boardCards, discard[player], numSims, false);
	//cout << " (bEq: " << believedEquity << "[" << trueEquity[player][round] << "], getting potOdds: " << toCall << "/" << (potSize) << "=" <<potOdds << ")";
	fout << " (bEq: " << believedEquity << "[" << trueEquity[player][round] << "], getting potOdds: " << toCall << "/" << (potSize) << "=" <<potOdds << ")";
	toCall=0;
      } else if (!tokens[1].compare("checks")){
	////cout << "checks" << endl;
	int numSims = (boardCards.size() > 3) ? SIMS_FAST : SIMS_SLOW;
	double believedEquity = evaluator.evaluate(hands[player], boardCards, discard[player], numSims, false);
	//cout << " (bEq: " << believedEquity << "[" << trueEquity[player][round] << "])";
	fout << " (bEq: " << believedEquity << "[" << trueEquity[player][round] << "])";
      } else if (!tokens[1].compare("bets")){      
	toCall=boost::lexical_cast<int>(tokens[2]);	
	potSize+=toCall;
	double potOdds = (double)toCall/(potSize+toCall);
	int numSims = (boardCards.size() > 3) ? SIMS_FAST : SIMS_SLOW;
	double believedEquity = evaluator.evaluate(hands[player], boardCards, discard[player], numSims, false);
	//cout << " (bEq: " << believedEquity << "[" << trueEquity[player][round] << "], giving potOdds: " << toCall << "/" << (potSize+toCall) << "=" <<potOdds << ")";
	fout << " (bEq: " << believedEquity << "[" << trueEquity[player][round] << "], giving potOdds: " << toCall << "/" << (potSize+toCall) << "=" <<potOdds << ")";
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

	toCall = raise-toCall;
	potSize+=raise;

	double potOdds = (double)toCall/(potSize+toCall);	int numSims = (boardCards.size() > 3) ? SIMS_FAST : SIMS_SLOW;
	double believedEquity = evaluator.evaluate(hands[player], boardCards, discard[player], numSims, false);
	//cout << " (bEq: " << believedEquity << "[" << trueEquity[player][round] << "], giving potOdds: " << toCall << "/" << (potSize+toCall) << "=" <<potOdds << ")";
	fout << " (bEq: " << believedEquity << "[" << trueEquity[player][round] << "], giving potOdds: " << toCall << "/" << (potSize+toCall) << "=" <<potOdds << ")";
      } else if (!tokens[1].compare("discards")){
	discard[player] = tokens[2];
	for (vector<string>::iterator it=hands[player].begin(); it !=hands[player].end();++it){
	  if (!tokens[2].compare(*it)){
	    hands[player].erase(it);
	    break;
	  }
	}
	//	//cout << "discards: " << tokens[2]  << endl;
      } else if (!tokens[1].compare("folds")){
	////cout << "folds" << endl;
	int numSims = (boardCards.size() > 3) ? SIMS_FAST : SIMS_SLOW;
	double believedEquity = evaluator.evaluate(hands[player], boardCards, discard[player], numSims, false);
	double potOdds = (double)toCall/(potSize+toCall);
	//cout << " (bEq: " << believedEquity << "[" << trueEquity[player][round] << "], had potOdds: " << toCall << "/" << (potSize+toCall) << "=" <<potOdds << ")";
	fout << " (bEq: " << believedEquity << "[" << trueEquity[player][round] << "], had potOdds: " << toCall << "/" << (potSize+toCall) << "=" <<potOdds << ")";
      } else if (!tokens[1].compare("shows")){
	numShowdown++;
      } else if (!tokens[1].compare("wins")){
	int player = !tokens[0].compare(myName) ? 0 : 1;
	//bankRoll[player]+=boost::lexical_cast<int>(tokens[4]);
	//bankRoll[1-player]-=boost::lexical_cast<int>(tokens[4]);

	////cout << "myBankroll: " << bankRoll[0] << " oppBankRoll: " << bankRoll[1] << endl;
	potSize=0;
      }
      
    } else if (!first.compare("FLOP")){
      round = FLOP;
      boardCards.push_back(tokens[3]);
      boardCards.push_back(tokens[4]);
      boardCards.push_back(tokens[5]);                 

      double aRealEquity = evaluator.evaluate_true(hands[0], hands[1], boardCards, discard[0], discard[1]);
      trueEquity[0][round]=aRealEquity;
      trueEquity[1][round]=1-aRealEquity;
      
      //cout << endl;
      fout << endl;
      //cout << "True equities: " << hands[0][0] << hands[0][1] << hands[0][2] << " (" << aRealEquity <<") vs. " << hands[1][0] << hands[1][1] << hands[1][2] << " (" << 1-aRealEquity <<")";
      fout << "True equities: " << hands[0][0] << hands[0][1] << hands[0][2] << " (" << aRealEquity <<") vs. " << hands[1][0] << hands[1][1] << hands[1][2] << " (" << 1-aRealEquity <<")";

    } else if (!first.compare("TURN")){
      round = TURN;
      boardCards.push_back(tokens[6]);                 
      ////cout << "TURN: " << tokens[6]  << endl;
      double aRealEquity = evaluator.evaluate_true(hands[0], hands[1], boardCards, discard[0], discard[1]);
      trueEquity[0][round]=aRealEquity;
      trueEquity[1][round]=1-aRealEquity;

      //cout << endl;
      fout << endl;
      //cout << "True equities: " << hands[0][0] << hands[0][1] << " (" << aRealEquity <<") vs. " << hands[1][0] << hands[1][1] << " (" << 1-aRealEquity <<")";
      fout << "True equities: " << hands[0][0] << hands[0][1] << " (" << aRealEquity <<") vs. " << hands[1][0] << hands[1][1] << " (" << 1-aRealEquity <<")";

    } else if (!first.compare("RIVER")){
      round = RIVER;
      boardCards.push_back(tokens[7]);           

      ////cout << "RIVER: " << tokens[7] << endl;

      double aRealEquity = evaluator.evaluate_true(hands[0], hands[1], boardCards, discard[0], discard[1]);
      trueEquity[0][round]=aRealEquity;
      trueEquity[1][round]=1-aRealEquity;

      //cout << endl;
      fout << endl;
      //cout << "True equities: " << hands[0][0] << hands[0][1] << " (" << aRealEquity <<") vs. " << hands[1][0] << hands[1][1] << " (" << 1-aRealEquity <<")";
      fout << "True equities: " << hands[0][0] << hands[0][1] << " (" << aRealEquity <<") vs. " << hands[1][0] << hands[1][1] << " (" << 1-aRealEquity <<")";

    }
    
    //cout << endl;
    fout << endl;

    i++;
    
  }
  
  return 0;
}
