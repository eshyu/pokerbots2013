#include "CardHeuristics.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

//return: pair, oneTriple, oneFour, myFirstKicker, mySecondKicker,myFirstPair, mySecondPair, myTriple
void CardHeuristics::getPairs(std::vector<int> hand, std::vector<int> board, std::vector<int> &pairs){
  std::sort (hand.begin(), hand.end()); 
  std::sort (board.begin(), board.end()); 
  int pair=0;
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
    if(i+4<board.size() && board[i]==board[i+3]){
      four=1;
      i+=3;
    } else if(i+3<board.size() && board[i]==board[i+2]){
      triple=1;
      for(int k=0; k<hand.size(); k++){
	if(hand[k]==board[i]){
	  myTriple=board[i];
	}
      }
      i+=2;
    }else if(i+2<board.size() && board[i]==board[i+1]){
      pair++;
      for(int k=0; k<hand.size(); k++){
	if(hand[k]==board[i]){
	  if(firstPair){
	    myFirstPair=board[i];
	    firstPair=0;
	  }else{
	    mySecondPair=board[i];
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

  pairs.push_back(pair);
  pairs.push_back(triple);
  pairs.push_back(four);
  pairs.push_back(myFirstKicker);
  pairs.push_back(mySecondKicker);
  pairs.push_back(myFirstPair);
  pairs.push_back(mySecondPair);
  pairs.push_back(myTriple);

}

//TODO: change to rank instead of raw number
void CardHeuristics::getStraight(std::vector<int> hand, 
					     std::vector<int> board, std::vector<int> &straight){
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
    if(i+4<board.size() && board[i]+5>board[i+4]){
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
    } else if(i+3<board.size() && board[i]+5>board[i+3]){
      fourStraight=1;
      threeStraight=1;
      std::vector<int> v(5);
      std::vector<int> v2(hand.size());   
      int boardCards[]={board[i],board[i+1],board[i+2],board[i+3]};
      int necCards[]={board[i],board[i]+1,board[i]+2,board[i]+3,board[i]+4};

      std::vector<int>::iterator it=set_difference(necCards, necCards+5,boardCards, boardCards+4,v.begin());

      std::vector<int>::iterator it2=set_intersection(v.begin(), it, hand.begin(),hand.end(),v2.begin());
      int numStraightCards=int(it2-v2.begin());
      if(numStraightCards>0){
	firstStraightCard=v2[0];
	if(numStraightCards>1){
	  secondStraightCard=v2[1];
	}
      }
    } else if(i+2<board.size() && board[i]+5>board[i+2]){
      threeStraight=1;
      std::vector<int> v(5);
      std::vector<int> v2(hand.size());   
      int boardCards[]={board[i],board[i+1],board[i+2]};
      int necCards[]={board[i],board[i]+1,board[i]+2,board[i]+3,board[i]+4};

      std::vector<int>::iterator it=set_difference(necCards, necCards+5,boardCards, boardCards+3,v.begin());
      std::vector<int>::iterator it2=set_intersection(v.begin(), it, hand.begin(),hand.end(),v2.begin());
      int numStraightCards=int(it2-v2.begin());
      if(numStraightCards>0){
	firstStraightCard=v2[0];
	if(numStraightCards>1){
	  secondStraightCard=v2[1];
	}
      }

    }	
  }
  straight.push_back(threeStraight);
  straight.push_back(fourStraight);
  straight.push_back(fiveStraight);
  straight.push_back(firstStraightCard);
  straight.push_back(secondStraightCard);

}

//return: threeSuit,fourSuit,fiveSuit,firstFlushCard,secondFlushCard
void CardHeuristics::getFlush(std::vector<int> handSuits, std::vector<int> handNum, std::vector<int> boardSuits, std::vector<int> boardNum, std::vector<int> &flush){
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
    flush.push_back(0);
    flush.push_back(0);
    flush.push_back(0);
    flush.push_back(0);
    flush.push_back(0);
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
    flush.push_back(1);
    flush.push_back(0);
    flush.push_back(0);
    flush.push_back(firstFlushCard);
    flush.push_back(secondFlushCard);
  } else if(suits[flushSuit]==4){
    flush.push_back(1);
    flush.push_back(1);
    flush.push_back(0);
    flush.push_back(firstFlushCard);
    flush.push_back(secondFlushCard);
  }else{
    flush.push_back(1);
    flush.push_back(1);
    flush.push_back(1);
    flush.push_back(firstFlushCard);
    flush.push_back(secondFlushCard);
  }
}

void CardHeuristics::createBoardTextureInput(const std::vector<std::string> &hand, 
					     const std::vector<std::string> &board,
					     float *textureInputs){
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
  std::vector<int> flush;
  getFlush(handSuits,handNums,boardSuits,boardNums, flush);	
  //cout<<"pairs"<<endl;
  std::vector<int> pairs;
  getPairs(handNums,boardNums, pairs);
  //cout<<"straight"<<endl;
  std::vector<int> straight;
  getStraight(handNums,boardNums, straight);
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


void CardHeuristics::createTextureFeatures(std::vector<int> flush, std::vector<int> pairs, std::vector<int> straight, float *out)
{
  *(out+0)=flush[0];
  *(out+1)=flush[1];
  *(out+2)=flush[2];
  *(out+3)=(float)flush[3]/14;
  *(out+4)=(float)flush[4]/14;
  *(out+5)=pairs[0]/2;
  *(out+6)=pairs[1];
  *(out+7)=pairs[2];
  *(out+8)=(float)pairs[3]/14;
  *(out+9)=(float)pairs[4]/14;
  *(out+10)=(float)pairs[5]/14;
  *(out+11)=(float)pairs[6]/14;
  *(out+12)=(float)pairs[7]/14;
  *(out+13)=straight[0];
  *(out+14)=straight[1];
  *(out+15)=straight[2];
  *(out+16)=(float)straight[3]/14;
  *(out+17)=(float)straight[4]/14;

}

/*
std::string CardHeuristics::getTextureString(std::vector<int> flush, std::vector<int> pairs, std::vector<int> straight){
  std::string textureString="";
  textureString=boost::lexical_cast<string>(flush[0])+" "+boost::lexical_cast<string>(flush[1])+" "+boost::lexical_cast<string>(flush[2])+" "+boost::lexical_cast<string>((float)flush[3]/14)+" "+boost::lexical_cast<string>((float)flush[4]/14)+" ";
  textureString+=boost::lexical_cast<string>(pairs[0])+" "+boost::lexical_cast<string>(pairs[1])+" "+boost::lexical_cast<string>(pairs[2])+" "+boost::lexical_cast<string>(pairs[3])+" "+boost::lexical_cast<string>((float)pairs[4]/14)+" "+boost::lexical_cast<string>((float)pairs[5]/14)+" "+boost::lexical_cast<string>((float)pairs[6]/14)+" "+boost::lexical_cast<string>((float)pairs[7]/14)+" "+boost::lexical_cast<string>((float)pairs[8]/14)+" ";
  textureString+=boost::lexical_cast<string>(straight[0])+" "+boost::lexical_cast<string>(straight[1])+" "+boost::lexical_cast<string>(straight[2])+" "+boost::lexical_cast<string>((float)straight[3]/14)+" "+boost::lexical_cast<string>((float)straight[4]/14)+" ";
  return textureString;
}
*/
