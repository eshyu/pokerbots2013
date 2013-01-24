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

void printData(std::string textureString, int potSize,  double myEquity, bool myButton, double prevBet, ROUND round, ofstream &fout){ 
	fout<<textureString;
   switch (round) {
   case 0:
    fout << "1 0 0 0 ";
    break;
   case 1:
    fout << "0 1 0 0 ";
    break;
   case 2:
    fout << "0 0 1 0 ";
    break;
   case 3:
    fout << "0 0 0 1 ";
    break;
   default:
    fout << "0 0 0 0";
  }
  
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
  
       << prevBet << " " //bet of opp or me
       << myEquity << " "
       << potSize/800.0 << " "
    //<< myBetvsStack << " "
    
       << endl;
}


int getSuit(char suit){
	switch(suit){
		case 's':
			return 0;
		case 'h':
			return 1;
		case 'c':
			return 2;
		case 'd':
			return 3;
		default:
			return 0;
	}

}

int getNum(char card){
	switch(card){
		case 'A':
			return 14;
		case 'K':
			return 13;
		case 'Q':
			return 12;
		case 'J':
			return 11;
		case 'T':
			return 10;
		default:
			return card-'0';
	}
}
std::string getTextureString(vector<int> flush, vector<int> pairs, vector<int> straight){
	std::string textureString="";
	textureString=boost::lexical_cast<string>(flush[0])+" "+boost::lexical_cast<string>(flush[1])+" "+boost::lexical_cast<string>(flush[2])+" "+boost::lexical_cast<string>((double)flush[3]/14)+" "+boost::lexical_cast<string>((double)flush[4]/14)+" ";
	textureString+=boost::lexical_cast<string>(pairs[0])+" "+boost::lexical_cast<string>(pairs[1])+" "+boost::lexical_cast<string>(pairs[2])+" "+boost::lexical_cast<string>(pairs[3])+" "+boost::lexical_cast<string>((double)pairs[4]/14)+" "+boost::lexical_cast<string>((double)pairs[5]/14)+" "+boost::lexical_cast<string>((double)pairs[6]/14)+" "+boost::lexical_cast<string>((double)pairs[7]/14)+" "+boost::lexical_cast<string>((double)pairs[8]/14)+" ";
	textureString+=boost::lexical_cast<string>(straight[0])+" "+boost::lexical_cast<string>(straight[1])+" "+boost::lexical_cast<string>(straight[2])+" "+boost::lexical_cast<string>((double)straight[3]/14)+" "+boost::lexical_cast<string>((double)straight[4]/14)+" ";
	return textureString;
}


vector<int> getPairs(vector<int> hand, vector<int> board){
	//return: onePair, twoPair, oneTriple, oneFour, myFirstKicker, mySecondKicker,myFirstPair, mySecondPair, myTriple
	std::sort (hand.begin(), hand.end()); 
	std::sort (board.begin(), board.end()); 
	int onePair=0;
	int twoPair=0;
	int triple=0;
	int four=0;
	int myFirstKicker=0;
	int mySecondKicker=0;
	int myFirstPair=0;
	int mySecondPair=0;
	int myTriple=0;

	int firstPair=1;
	int firstKicker=1;
	for(int i=0; i<board.size(); i++){
		if(i<board.size()-4 && board[i]==board[i+3]){
			four=1;
			i+=3;
		} else if(i<board.size()-3 && board[i]==board[i+2]){
			triple=1;
			for(int k=0; k<hand.size(); k++){
				if(hand[k]==board[i]){
					myTriple=board[i];
				}
			}
			i+=2;
		}else if(i<board.size()-2 && board[i]==board[i+1]){

			for(int k=0; k<hand.size(); k++){
				if(hand[k]==board[i]){
					if(firstPair){
						myFirstKicker=board[i];
						firstPair=0;
					}else{
						mySecondKicker=board[i];
					}
				}
			}
			i++;
			
		}else{
			for(int k=0; k<hand.size(); k++){
				if(hand[k]==board[i]){
					if(firstKicker){
						myFirstKicker=board[i];
						firstKicker=0;
					}else{
						mySecondKicker=board[i];
					}
				}
			}
		}
	}
	
	int myints[] = {onePair, twoPair, triple, four, myFirstKicker, mySecondKicker,myFirstPair, mySecondPair, myTriple};
	
  	std::vector<int> returnValues (myints, myints + sizeof(myints) / sizeof(int) );

	return returnValues;

}

//TODO: change to rank instead of raw number
vector<int> getStraight(vector<int> hand, vector<int> board){
	//return: threeStraight, fourStraight, fiveStraight, firstStraightCard,secondStraightCard
	std::sort (hand.begin(), hand.end()); 
	hand.erase( unique( hand.begin(), hand.end() ), hand.end() );
	std::sort (board.begin(), board.end()); 
	board.erase( unique( board.begin(), board.end() ), board.end() );
	int threeStraight=0;
	int fourStraight=0;
	int fiveStraight=0;
	int firstStraightCard=0;
	int secondStraightCard=0;

	for(int i=0; i<board.size(); i++){
		if(i<board.size()-4 && board[i]+5>board[i+4]){
			fiveStraight=1;
			fourStraight=1;
			threeStraight=1;
			if((hand[0]==board[i+4]+1 && hand[1]==board[i+4]+2)){
				firstStraightCard=hand[0];
				secondStraightCard=hand[1];
			} else if(hand[0]==board[i+4]+1 || hand[1]==board[i+4]+1){
				firstStraightCard=board[i+4]+1;
			}
			break;
		} else if(i<board.size()-3 && board[i]+5>board[i+3]){
			fourStraight=1;
			threeStraight=1;
			vector<int> v(5);
			vector<int> v2(hand.size());   
			int boardCards[]={board[i],board[i+1],board[i+2],board[i+3]};
			int necCards[]={board[i],board[i]+1,board[i]+2,board[i]+3,board[i]+4};
			//vector<int> boardCards(board.begin()+i,board.begin()+i+3);
			//vector<int> necCards(board.begin()+i,board.begin()+i+3);
			vector<int>::iterator it=set_difference(necCards, necCards+5,boardCards, boardCards+4,v.begin());

			vector<int>::iterator it2=set_intersection(v.begin(), it, hand.begin(),hand.end(),v2.begin());
			int numStraightCards=int(it2-v2.begin());
			if(numStraightCards>0){
				firstStraightCard=v2[0];
				if(numStraightCards>1){
					secondStraightCard=v2[1];
				}
			}
		} else if(i<board.size()-2 && board[i]+5>board[i+2]){
			threeStraight=1;
			vector<int> v(5);
			vector<int> v2(hand.size());   
			int boardCards[]={board[i],board[i+1],board[i+2]};
			int necCards[]={board[i],board[i]+1,board[i]+2,board[i]+3,board[i]+4};
			//vector<int> boardCards(board.begin()+i,board.begin()+i+3);
			//vector<int> necCards(board.begin()+i,board.begin()+i+3);
			vector<int>::iterator it=set_difference(necCards, necCards+5,boardCards, boardCards+3,v.begin());

			vector<int>::iterator it2=set_intersection(v.begin(), it, hand.begin(),hand.end(),v2.begin());
			int numStraightCards=int(it2-v2.begin());
			if(numStraightCards>0){
				firstStraightCard=v2[0];
				if(numStraightCards>1){
					secondStraightCard=v2[1];
				}
			}
		/*
			vector<int> v(5);
			vector<int> v2(hand.size());   
			vector<int> boardCards=(board.begin()+i,board.begin()+i+2);
			vector<int> necCards(board.begin()+i,board.begin()+i+2);
			vector<int>::iterator it=set_difference(necCards.begin(), necCards.begin()+5,boardCards.begin(), boardCards.end(),v);//TODO
			vector<int>::iterator it2=set_intersection(v.begin, it, hand.begin(),hand.end(),v2);
			int numStraightCards=int(it2-v2.begin());
			if(numStraightCards>0){
				firstStraightCard=v2[0];
				if(numStraightCards>1){
					secondStraightCard=v2[1];
				}
			}*/
		}	
	}
	
	int myints[] = {threeStraight,fourStraight,fiveStraight,firstStraightCard,secondStraightCard};
  	std::vector<int> returnValues (myints, myints + sizeof(myints) / sizeof(int) );
	return returnValues;
}

vector<int> getFlush(vector<int> handSuits, vector<int> handNum, vector<int> boardSuits, vector<int> boardNum){
	//return: threeSuit,fourSuit,fiveSuit,firstFlushCard,secondFlushCard
	vector<int> returnValues;
	int suits[4]={0,0,0,0};
	int flushSuit=-1;
	for(int i=0; i<boardSuits.size(); i++){
		suits[boardSuits[i]]++;
	}
	for(int k=0; k<4; k++){
		if(suits[k]>2){//at least 3
			flushSuit=k;
		}
	}
	if(flushSuit<0){
		vector<int> returnValues(5,0);
		return returnValues;
	}
	bool first=1;
	int firstFlushCard=0;
	int secondFlushCard=0;
	for(int i=0; i<handSuits.size(); i++){
		if(handSuits[i]==flushSuit){
			if(first){//doesn't quite work for us with 3 cards, but w/e
				first=0;
				firstFlushCard=handNum[i];
			}else{
				secondFlushCard=handNum[i];
			}
		}
	}
	if(suits[flushSuit]==3){
		int myints[] = {1,0,0,firstFlushCard,secondFlushCard};
  		std::vector<int> returnValues (myints, myints + sizeof(myints) / sizeof(int) );
		return returnValues;
	} else if(suits[flushSuit]==4){
		int myints[] = {1,1,0,firstFlushCard,secondFlushCard};
  		std::vector<int> returnValues (myints, myints + sizeof(myints) / sizeof(int) );
		return returnValues;
	}else{
		int myints[] = {1,1,1,firstFlushCard,secondFlushCard};
  		std::vector<int> returnValues (myints, myints + sizeof(myints) / sizeof(int) );
		return returnValues;
	}
}
std::string boardTexture(vector<string> hand, vector<string> board){
	//cout<<"board texture"<<endl;
	/*
	//straight
	bool threeStraight=0;
	bool fourStraight=0;
	bool fiveStraight=0;
	
	//three cards
	bool threeInRow=0;
	bool two1one=0;
	bool two2one=0;
	bool one1one1one=0;
	//four cards
	bool fourInRow=0;
	bool three1one=0;
	bool three2one=0;
	bool two1two=0;
	bool two2two=0;
	bool two1one1one=0;
	//five cards
	bool fiveInRow=0;
	bool four1one=0;
	bool four2one=0;
	bool three1one=0;
	bool three2one=0;
	bool three1one1one=0;
	bool two1two1one=0;
	
	//suits
	bool threeSuit=0;
	bool fourSuit=0;
	bool fiveSuit=0;
	//pairs, triples
	bool onePair=0;
	bool twoPair=0;
	bool oneTriple=0;
	bool oneFour=0;
	//my cards (currently doesn't handle third card, so not great for pre-discard?)
	float myFirstStraightCard=0;
	float mySecondStraightCard=0;
	float myFirstKicker=0;
	float mySecondKicker=0;
	float myFirstPair=0;
	float mySecondPair=0;
	float myTriple=0;
	float myFirstSuit=0;
	float mySecondSuit=0;
	*/
	//****
	int num_hand=hand.size();
	int num_board=board.size();
	
	vector<int> handSuits;
	vector<int> handNum;
	vector<int> boardSuits;
	vector<int> boardNum;

	for(int i=0; i<num_hand; i++){
		std::string card=hand[i];
		handNum.push_back(getNum(card[0]));
		handSuits.push_back(getSuit(card[1]));
	}

	for(int j=0; j<num_board; j++){
		std::string card=board[j];
		boardNum.push_back(getNum(card[0]));
		boardSuits.push_back(getSuit(card[1]));
	}

	//flush: threeSuit,fourSuit,fiveSuit,firstFlushCard,secondFlushCard
	//cout<<"flush"<<endl;
	vector<int> flush = getFlush(handSuits,handNum,boardSuits,boardNum);	
	//cout<<"pairs"<<endl;
	vector<int> pairs=getPairs(handNum,boardNum);
	//cout<<"straight"<<endl;
	vector<int> straight = getStraight(handNum,boardNum);
	return getTextureString(flush,pairs,straight);

}




OpponentModeler::PlayerStatistics stats[2];
Evaluator evaluator;
string myName, oppName;
int stackSize, bb;

 int main(int argc, char *argv[])
 {
   std::string day, p1Name, p2Name;

   int me;
   int numData=0;
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
   std::string outPathNameEquityTemp = "neural/temp_equity_" + suffix;
   std::string outPathNameDistTemp="neural/temp_dist_"+suffix;
   std::string outPathNameEquity = "fann/examples/input/equity_" + suffix;
   std::string outPathNameDist="fann/examples/input/dist_"+suffix;
   ifstream fin(inPathName.c_str());
   ofstream foutEquity(outPathNameEquityTemp.c_str());
   ofstream foutDist(outPathNameDistTemp.c_str());

   std::string outPathNameAction="fann/examples/input/action_"+suffix;
   ofstream foutAction(outPathNameAction.c_str());
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

     } else if (!first.compare("tEq:")&& me==1){       
       oppTeq = boost::lexical_cast<double>(tokens[5]);

     } else if (!first.compare("tEq:")){
       oppTeq = boost::lexical_cast<double>(tokens[2]);
     } else if (!first.compare("True")&& me==1){       
       oppTeq = boost::lexical_cast<double>(tokens[6]);

     } else if (!first.compare("True")){
       oppTeq = boost::lexical_cast<double>(tokens[3]);
     }else if (!first.compare(myName) || !first.compare(oppName)){
       int player = !first.compare(myName) ? 0 : 1;
       if (!tokens[1].compare("posts")){
	 if (!player){
	   button = (tokens[5].compare("1")) ? 0 : 1;
	 }

       } else if (!tokens[1].compare("calls")){
	if (player==1){
	   printData(boardTexture(hands[0],boardCards),potSize, myBeq, button==0, (double)prevRaise/stackSize, round, foutDist);
	   foutDist<< "0 0 1 0 0"<<endl;
	 }

	 potSize+=toCall;
	 double potOdds = (double)toCall/potSize;

	 //toCall=0;

	 numCall[player][round]+=1;

	 if (player == 1){//removed && round == PREFLOP
	   foutAction << line<<  endl;
		//cout<<line<<endl;
           numData++;
	   printData(boardTexture(hands[0],boardCards),potSize, myBeq, button==0, (double)toCall/stackSize, round, foutEquity);
	   foutEquity << oppTeq <<  endl;
	 }
	 toCall=0;
	 
	 
       } else if (!tokens[1].compare("checks")){

	 if (player == 1){//removed && round == PREFLOP

	   printData(boardTexture(hands[0],boardCards),potSize, myBeq, button==0, 0, round, foutDist);
	   foutDist<< "0 1 0 0 0"<<endl;
	 }

	 numCheck[player][round]+=1;
	 check[player][round]=true;

	 if (player == 1){//removed && round == PREFLOP
	   foutAction << line <<  endl;
	//cout<<line<<endl;
           numData++;
	   printData(boardTexture(hands[0],boardCards),potSize, myBeq, button==0, 0, round, foutEquity);
	   foutEquity << oppTeq <<  endl;

	 }

       } else if (!tokens[1].compare("bets")){  
	 toCall=boost::lexical_cast<int>(tokens[2]);	    
	 if (player == 1 ){//removed && round == PREFLOP
	   printData(boardTexture(hands[0],boardCards),potSize, myBeq, button==0, (double)prevRaise/stackSize, round, foutDist);
	   foutDist<< "1 0 0 0 "<<(double)toCall/stackSize<<endl;
	 }
	 int prevBet = prevRaise;
	 prevRaise=toCall;
	 potSize+=toCall;
	 double potOdds = (double)toCall/(potSize+toCall);

	numBet[player][round]+=1;
	bet[player][round]=true;

	if (player == 0){
	  myBet = potOdds;
	}
	
	 if (player == 1 ){//removed && round == PREFLOP
	   foutAction << line <<  endl;
//cout<<line<<endl;
           numData++;
	   printData(boardTexture(hands[0],boardCards),potSize, myBeq, button==0, (double)prevRaise/stackSize, round, foutEquity);
	   foutEquity << oppTeq <<  endl;

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

	int prevBet = prevRaise;
	toCall = raise-prevRaise;
	if (player == 1 ){//removed && round == PREFLOP
	  printData(boardTexture(hands[0],boardCards),potSize, myBeq, button==0, (double)prevBet/stackSize, round, foutDist);
	  foutDist<< "1 0 0 0 "<<(double)toCall/stackSize<<endl;
	}

	prevRaise=raise;
	potSize+=raise;

	double potOdds = (double)toCall/(potSize+toCall);	

	numRaise[player][round]+=1;
	hasRaise[player][round]=true;

	if (player == 0){
	  myBet = potOdds;
	}
	
	if (player == 1 ){//removed && round == PREFLOP
	   foutAction << line <<  endl;
//cout<<line<<endl;
           numData++;
	  printData(boardTexture(hands[0],boardCards),potSize, myBeq, button==0, (double)prevRaise/stackSize, round, foutEquity);
	  foutEquity << oppTeq <<  endl;

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
	if (player == 1){//removed && round == PREFLOP
	  printData(boardTexture(hands[0],boardCards),potSize, myBeq, button==0, (double)prevRaise/stackSize, round, foutDist);
	  foutDist<< "0 0 0 1 0 "<<endl;
	}
	int numSims = (boardCards.size() > 3) ? SIMS_FAST : SIMS_SLOW;
	double believedEquity = evaluator.evaluate(hands[player], boardCards, discard[player], numSims, false);

	numFold[player][round]+=1;

	if (player == 1){//removed && round == PREFLOP
	   foutAction << line <<  endl;
//cout<<line<<endl;
           numData++;
	  printData(boardTexture(hands[0],boardCards),potSize, myBeq, button==0, 0, round, foutEquity);
	  foutEquity << oppTeq <<  endl;
	}

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
  fin.close();
  foutEquity.close();
  foutDist.close();
  foutAction.close();

  ifstream finEquity(outPathNameEquityTemp.c_str());
  ifstream finDist(outPathNameDistTemp.c_str());
  ofstream foutEquityFinal(outPathNameEquity.c_str());
  ofstream foutDistFinal(outPathNameDist.c_str());

  foutEquityFinal << numData << " " << 43 << " " << 1 << endl;
  foutDistFinal<< numData << " " << 43 << " " << 5 << endl;
  foutEquityFinal<<finEquity.rdbuf();
  foutDistFinal << finDist.rdbuf();
  finEquity.close();
  finDist.close();
  foutEquityFinal.close();
  foutDistFinal.close();
  remove(outPathNameEquityTemp.c_str());
  remove(outPathNameDistTemp.c_str());

  return 0;
}
