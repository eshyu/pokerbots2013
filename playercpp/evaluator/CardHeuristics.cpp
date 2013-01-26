#include "CardHeuristics.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

//return: onePair, twoPair, oneTriple, oneFour, myFirstKicker, mySecondKicker,myFirstPair, mySecondPair, myTriple
std::vector<int> CardHeuristics::getPairs(std::vector<int> hand, std::vector<int> board){
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
std::vector<int> CardHeuristics::getStraight(std::vector<int> hand, 
					     std::vector<int> board){
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
      std::vector<int> v(5);
      std::vector<int> v2(hand.size());   
      int boardCards[]={board[i],board[i+1],board[i+2],board[i+3]};
      int necCards[]={board[i],board[i]+1,board[i]+2,board[i]+3,board[i]+4};
      //vector<int> boardCards(board.begin()+i,board.begin()+i+3);
      //vector<int> necCards(board.begin()+i,board.begin()+i+3);
      std::vector<int>::iterator it=set_difference(necCards, necCards+5,boardCards, boardCards+4,v.begin());

      std::vector<int>::iterator it2=set_intersection(v.begin(), it, hand.begin(),hand.end(),v2.begin());
      int numStraightCards=int(it2-v2.begin());
      if(numStraightCards>0){
	firstStraightCard=v2[0];
	if(numStraightCards>1){
	  secondStraightCard=v2[1];
	}
      }
    } else if(i<board.size()-2 && board[i]+5>board[i+2]){
      threeStraight=1;
      std::vector<int> v(5);
      std::vector<int> v2(hand.size());   
      int boardCards[]={board[i],board[i+1],board[i+2]};
      int necCards[]={board[i],board[i]+1,board[i]+2,board[i]+3,board[i]+4};
      //vector<int> boardCards(board.begin()+i,board.begin()+i+3);
      //vector<int> necCards(board.begin()+i,board.begin()+i+3);
      std::vector<int>::iterator it=set_difference(necCards, necCards+5,boardCards, boardCards+3,v.begin());
      std::vector<int>::iterator it2=set_intersection(v.begin(), it, hand.begin(),hand.end(),v2.begin());
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

//return: threeSuit,fourSuit,fiveSuit,firstFlushCard,secondFlushCard
std::vector<int> CardHeuristics::getFlush(std::vector<int> handSuits, std::vector<int> handNum, std::vector<int> boardSuits, std::vector<int> boardNum){
  std::vector<int> returnValues;
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
    std::vector<int> returnValues(5,0);
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

void CardHeuristics::createBoardTextureInput(const std::vector<std::string> &hand, 
					     const std::vector<std::string> &board,
					     double *textureInputs){
  int num_hand=hand.size();
  int num_board=board.size();
  
  std::vector<int> handSuits;
  std::vector<int> handNums;
  std::vector<int> boardSuits;
  std::vector<int> boardNums;
	
  std::string card;
  for(int i=0; i<num_hand; i++){
    card=hand[i];
    handNums.push_back(getNum(card[0]));
    handSuits.push_back(getSuit(card[1]));
  }
  
  for(int j=0; j<num_board; j++){
    card=board[j];
    boardNums.push_back(getNum(card[0]));
    boardSuits.push_back(getSuit(card[1]));
  }
  
  //flush: threeSuit,fourSuit,fiveSuit,firstFlushCard,secondFlushCard
  //cout<<"flush"<<endl;
  std::vector<int> flush = getFlush(handSuits,handNums,boardSuits,boardNums);	
  //cout<<"pairs"<<endl;
  std::vector<int> pairs=getPairs(handNums,boardNums);
  //cout<<"straight"<<endl;
  std::vector<int> straight = getStraight(handNums,boardNums);
  //return getTextureString(flush,pairs,straight);  
  createTextureFeatures(flush,pairs,straight, textureInputs);  
}


int CardHeuristics::getSuit(char suit){
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

int CardHeuristics::getNum(char card){
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


void CardHeuristics::createTextureFeatures(std::vector<int> flush, std::vector<int> pairs, std::vector<int> straight, double *out)
{
  *(out+0)=flush[0];
  *(out+1)=flush[1];
  *(out+2)=flush[2];
  *(out+3)=(double)flush[3]/14;
  *(out+4)=(double)flush[4]/14;
  *(out+5)=pairs[0];
  *(out+6)=pairs[1];
  *(out+7)=pairs[2];
  *(out+8)=pairs[3];
  *(out+9)=(double)pairs[4]/14;
  *(out+10)=(double)pairs[5]/14;
  *(out+11)=(double)pairs[6]/14;
  *(out+12)=(double)pairs[7]/14;
  *(out+13)=(double)pairs[8]/14;
  *(out+14)=straight[0];
  *(out+15)=straight[1];
  *(out+16)=straight[2];
  *(out+17)=(double)straight[3]/14;
  *(out+18)=(double)straight[4]/14;

}

/*
std::string CardHeuristics::getTextureString(std::vector<int> flush, std::vector<int> pairs, std::vector<int> straight){
  std::string textureString="";
  textureString=boost::lexical_cast<string>(flush[0])+" "+boost::lexical_cast<string>(flush[1])+" "+boost::lexical_cast<string>(flush[2])+" "+boost::lexical_cast<string>((double)flush[3]/14)+" "+boost::lexical_cast<string>((double)flush[4]/14)+" ";
  textureString+=boost::lexical_cast<string>(pairs[0])+" "+boost::lexical_cast<string>(pairs[1])+" "+boost::lexical_cast<string>(pairs[2])+" "+boost::lexical_cast<string>(pairs[3])+" "+boost::lexical_cast<string>((double)pairs[4]/14)+" "+boost::lexical_cast<string>((double)pairs[5]/14)+" "+boost::lexical_cast<string>((double)pairs[6]/14)+" "+boost::lexical_cast<string>((double)pairs[7]/14)+" "+boost::lexical_cast<string>((double)pairs[8]/14)+" ";
  textureString+=boost::lexical_cast<string>(straight[0])+" "+boost::lexical_cast<string>(straight[1])+" "+boost::lexical_cast<string>(straight[2])+" "+boost::lexical_cast<string>((double)straight[3]/14)+" "+boost::lexical_cast<string>((double)straight[4]/14)+" ";
  return textureString;
}
*/
