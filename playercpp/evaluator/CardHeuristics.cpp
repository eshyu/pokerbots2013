#include "CardHeuristics.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

bool CardHeuristics::havePair(const std::vector<std::string> &hand){
  int num_hand=hand.size();
  
  std::vector<int> handNum;
 
  for(int i=0; i<num_hand; i++){
    std::string card=hand[i];
    handNum.push_back(getNum(card[0]));
  }
  

  bool pair=false;
  std::sort(handNum.begin(),handNum.end());
  for(int i=0; i+1<handNum.size();i++){
    if(handNum[i]==handNum[i+1]  && handNum[i]>7){
      pair=true;
    }
  }
  return pair;
}


void CardHeuristics::getCards(const std::vector<std::string> &hand, 
					     const std::vector<std::string> &board, std::vector<int> handSuits, std::vector<int> handNums,std::vector<int> boardSuits,std::vector<int> boardNums){
  int num_hand=hand.size();
	int num_board=board.size();
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
}
std::string CardHeuristics::getSuitStrings(const std::vector<int> &hand, const std::vector<int> &board, const std::vector<int> &handNums, const std::vector<int> &boardNums, const std::vector<float> &weights){
	std::string numString="";
	std::vector<std::string> flush;
	std::vector<std::string> flushDraw;
	int combSuits[4]={0,0,0,0};
	int cardSuits[56];
	for(int i=0; i<56; i++){

		cardSuits[i]=0;

	}
	for(int i=0; i<board.size(); i++){
		combSuits[board[i]]++;
		cardSuits[board[i]*14+boardNums[i]-1]++;
	}
	for(int i=0; i<hand.size(); i++){
		combSuits[hand[i]]++;
		cardSuits[hand[i]*14+handNums[i]-1]++;
	}
	for(int i=0; i<4; i++){
		for(int j=0; j<4; j++){
			combSuits[i]++;
			combSuits[j]++;
			if(combSuits[i]<=13 && combSuits[j]<=13){
				std::vector<int> allCards(board);
				allCards.insert(allCards.begin(), i);
				allCards.insert(allCards.begin(), j);
				std::sort(allCards.begin(), allCards.end());
				std::vector<int> have;
				haveFlush(allCards, have);
				
				getStringFromSuits(i,j,cardSuits, have[0], have[1], flushDraw,flush);

			}
		}
	}
	float weight=weights[8];
	for(int i=(int)((1-weight)*flushDraw.size()); i<flushDraw.size(); i++){
		numString+=flushDraw[i];
		numString+=",";
	}
	weight=weights[9];
	for(int i=(int)((1-weight)*flush.size()); i<flush.size(); i++){
		numString+=flush[i];
		numString+=",";
	}
	return numString;
}
void CardHeuristics::getStringFromSuits(int i,int j, int * cardSuits, int have0, int have1, std::vector<std::string> &flushDraw, std::vector<std::string> &flush){
std::string suitI=convertSuit2String(i);
std::string suitJ=convertSuit2String(j);

for(int a=0; a<14; a++){
	for(int b=0; b<14; b++){
		if(!cardSuits[i*14+a] && !cardSuits[j*14+b] && (i!=j || a!=b)){
			std::string cardString=boost::lexical_cast<std::string>(a)+suitI+boost::lexical_cast<std::string>(b)+suitJ;
			if(have0){
				flushDraw.push_back(cardString);
			}else if(have1){
				flush.push_back(cardString);
			}
		}
	}
}

	
}

std::string CardHeuristics::convertSuit2String(int suit){
  switch(suit){
  case 0:
    return "s";
  case 1:
    return "h";
  case 2:
    return "c";
  case 3:
    return "d";
  default:
    return "s";
  }
}
std::string CardHeuristics::getNumStrings(const std::vector<int> &hand, const std::vector<int> &board, const std::vector<float> &weights){
	std::vector<std::string> lowPairs;
	std::vector<std::string> midPairs;
	std::vector<std::string> highPairs;
	std::vector<std::string> twoPairs;
	std::vector<std::string> triples;
	std::vector<std::string> fours;
	std::vector<std::string> straight;
	std::vector<std::string> straightDraw;
	int high=0;
	int low=0;
	std::string numString="";
	int combCards[14]={0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int boardCards[14]={0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	for(int i=0; i<board.size(); i++){
		combCards[board[i]-1]++;
		boardCards[board[i]-1]++;
	}
	for(int i=0; i<hand.size(); i++){
		combCards[hand[i]-1]++;
	}
	for(int i=0; i<14; i++){
		if(boardCards[i]>0){
			high=boardCards[i]+1;
		}
		if(boardCards[14-1-i]>0){
			low=boardCards[i]+1;
		}
	}
	for(int i=0; i<14; i++){
		for(int j=0; j<14; j++){
			combCards[i]++;
			combCards[j]++;
			if(combCards[i]<=4 && combCards[j]<=4){
				std::vector<int> allCards(board);
				allCards.insert(allCards.begin(), i);
				allCards.insert(allCards.begin(), j);
				std::sort(allCards.begin(), allCards.end());
				std::vector<int> have;
				std::vector<int> have2;
				haveMulti(allCards, have, high, low);
				haveStraight(allCards, have2);
				std::string cardString=""+boost::lexical_cast<std::string>(i)+boost::lexical_cast<std::string>(j);
				if(have[3]>=2){
					twoPairs.push_back(cardString);
				}
				else if(have[2]){
					highPairs.push_back(cardString);
				}
				else if(have[0]){
					lowPairs.push_back(cardString);
				}
				else if(have[1]){
					midPairs.push_back(cardString);
				}

				if(have[4]){
					triples.push_back(cardString);
				}
				if(have[5]){
					fours.push_back(cardString);
				}
				if(have2[0]){
					straightDraw.push_back(cardString);
				}
				if(have2[1]){
					straight.push_back(cardString);
				}				
			}
			combCards[i]--;
			combCards[j]--;
		}
	}
	float weight=weights[0];
	for(int i=(int)((1-weight)*lowPairs.size()); i<lowPairs.size(); i++){
		numString+=lowPairs[i];
		numString+=",";
	}
	weight=weights[1];
	for(int i=(int)((1-weight)*midPairs.size()); i<midPairs.size(); i++){
		numString+=midPairs[i];
		numString+=",";
	}
	weight=weights[2];
	for(int i=(int)((1-weight)*highPairs.size()); i<highPairs.size(); i++){
		numString+=highPairs[i];
		numString+=",";
	}
	weight=weights[3];
	for(int i=(int)((1-weight)*twoPairs.size()); i<twoPairs.size(); i++){
		numString+=twoPairs[i];
		numString+=",";
	}
	weight=weights[4];
	for(int i=(int)((1-weight)*triples.size()); i<triples.size(); i++){
		numString+=triples[i];
		numString+=",";
	}
	weight=weights[5];
	for(int i=(int)((1-weight)*fours.size()); i<fours.size(); i++){
		numString+=fours[i];
		numString+=",";
	}
	weight=weights[6];
	for(int i=(int)((1-weight)*straight.size()); i<straight.size(); i++){
		numString+=straight[i];
		numString+=",";
	}
	weight=weights[7];
	for(int i=(int)((1-weight)*straightDraw.size()); i<straightDraw.size(); i++){
		numString+=straightDraw[i];
		numString+=",";
	}
	return numString;
	
}

//cardSuits
void CardHeuristics::haveFlush(const std::vector<int> &cards, std::vector<int> &have){
	int flush=0;
	int flushDraw=0;

	for(int i=0; i<cards.size(); i++){
		if(i+4<cards.size() && cards[i]==cards[i+4]){
			flush=1;
		}else if( i+3<cards.size() && cards[i]==cards[i+3]){
			flushDraw=1;
		}
	}
	if(flush)flushDraw=0;
	have.push_back(flushDraw);
	have.push_back(flush);	
}

//cardNums
void CardHeuristics::haveStraight(const std::vector<int> &cards, std::vector<int> &have){
	int straight=0;
	int straightDraw=0;
	std::vector<int> copy(cards);
  copy.erase( unique( copy.begin(), copy.end() ), copy.end() );	
	for(int i=0; i<copy.size(); i++){
		if(i+4<copy.size() && copy[i]+5>copy[i+4]){
			straight=1;
		}else if( i+3<copy.size() && copy[i]+5>copy[i+3]){
			straightDraw=1;
		}
	}
	if(straight)straightDraw=0;
	have.push_back(straightDraw);
	have.push_back(straight);

}
//cardNums
void CardHeuristics::haveMulti(const std::vector<int> &cards, std::vector<int> &have, int hi, int low){

	int hasLow=0;
	int hasHigh=0;
	int hasMid=0;
	int pairs=0;
	int triples=0;
	int fours=0;

	for(int i=0; i<cards.size(); i++){
		if(i+3<cards.size() && cards[i]==cards[i+3]){
			fours++;
			i+=3;
		}else if(i+2<cards.size() && cards[i]==cards[i+2]){
			triples++;
			i+=2;
		}else if(i+1<cards.size() && cards[i]==cards[i+1]){
			if(cards[i]>=hi){
				hasHigh=1;
			}else if(cards[i]<=low){
				hasLow=1;
			} else{
				hasMid=1;
			}
			pairs++;
			i++;
		}
	}
	have.push_back(hasLow);
	have.push_back(hasMid);
	have.push_back(hasHigh);
	have.push_back(pairs);
	have.push_back(triples);
	have.push_back(fours);
}




std::string CardHeuristics::getEquityString(const std::vector<float> &weights,const std::vector<std::string> &hand, const std::vector<std::string> &board){
	
  std::vector<int> handSuits;
  std::vector<int> handNums;
  std::vector<int> boardSuits;
  std::vector<int> boardNums;

	getCards(hand, board, handSuits, handNums, boardSuits, boardNums);

  std::sort (boardNums.begin(), boardNums.end()); 
  std::sort (handNums.begin(), handNums.end()); 		
	
	std::string returnString =getNumStrings(handNums, boardNums, weights);
	returnString+=getSuitStrings(handSuits, boardSuits, handNums, boardNums, weights);
	return returnString;
	
}

void CardHeuristics::getHandType(const std::vector<std::string> &cards, const std::vector<std::string> &board, std::vector<CardHeuristics::HAND_TYPE> &handType){
  std::vector<int> handSuits;
  std::vector<int> handNums;
  std::vector<int> boardSuits;
  std::vector<int> boardNums;


	getCards(cards, board, handSuits, handNums, boardSuits, boardNums);

	int hi=boardNums[boardNums.size()-1];
	int low=boardNums[0];

	std::vector<int> multi;
	std::vector<int> straight;
	std::vector<int> flush;

	std::vector<int> numCards;
	std::vector<int> suitCards;

	numCards.reserve( handNums.size() + boardNums.size() ); // preallocate memory
	numCards.insert( numCards.end(), handNums.begin(), handNums.end() );
	numCards.insert( numCards.end(), boardNums.begin(), boardNums.end() );
	suitCards.reserve( handSuits.size() + boardSuits.size() ); // preallocate memory
	suitCards.insert( suitCards.end(), handSuits.begin(), handSuits.end() );
	suitCards.insert( suitCards.end(), boardSuits.begin(), boardSuits.end() );
	std::sort(numCards.begin(), numCards.end());
	std::sort(suitCards.begin(), suitCards.end());
	haveMulti(numCards, multi, hi, low);
	haveStraight(numCards,straight);
	haveFlush(suitCards, flush);
	if(multi[3]>=2){
		handType.push_back(TWO_PAIR);
	}
	else if(multi[2]){
		handType.push_back(HI_PAIR);
	}
	else if(multi[0]){
		handType.push_back(LOW_PAIR);
	}
	else if(multi[1]){
		handType.push_back(MID_PAIR);
	}
	if(multi[4]){
		handType.push_back(TRIPLE);
	}
	if(multi[5]){
		handType.push_back(FOUR);
	}
	if(straight[0]){
		handType.push_back(STRAIGHT_DRAW);
	}
	if(straight[1]){
		handType.push_back(STRAIGHT);
	}
	if(flush[0]){
		handType.push_back(FLUSH_DRAW);
	}
	if(flush[1]){
		handType.push_back(FLUSH);
	}							
	
}

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
  int handPair=0;
  
  int firstPair=1;
  int firstKicker=1;

  for(int i=0; i+1<hand.size(); i++){
    if(hand[i]==hand[i+1]){
      handPair=hand[i];
    }
  }

  for(int i=0; i<board.size(); i++){
    if(i+3<board.size() && board[i]==board[i+3]){
      four=1;
      i+=3;
    } else if(i+2<board.size() && board[i]==board[i+2]){
      triple=1;
      for(int k=0; k<hand.size(); k++){
	if(hand[k]==board[i]){
	  myTriple=board[i];
	}
      }
      i+=2;
    }else if(i+1<board.size() && board[i]==board[i+1]){
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
  pairs.push_back(handPair);
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

  if(hand.back()==14){
    hand.insert(hand.begin(),1);
  }

  hand.erase( unique( hand.begin(), hand.end() ), hand.end() );
  std::sort (board.begin(), board.end());

  if(board.size()>0 && board.back()==14){

    board.insert(board.begin(),1);
  } 
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
      if(board[i]+3==board[i+3] && (hand[0]==board[i]-1 || hand[1]==board[i]-1)){
	firstStraightCard=board[i]-1;
	if(numStraightCards>0){
	  secondStraightCard=v2[0];
	}
      }else if(numStraightCards>0){
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
      if(board[i]+2==board[i+2]){
	if(hand[0]==board[i]-2 && hand[1]==board[i]-1){
	  firstStraightCard=board[i]-2;
	  secondStraightCard=board[i]-1;
	}else if(hand[0]==board[i]-1 || hand[1]==board[i]-1){
	  firstStraightCard=board[i]-1;
	  if(numStraightCards>0){
	    secondStraightCard=v2[0];
	  }
	}
      }else if(numStraightCards>0){
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
  *(out+5)=(float)pairs[0]/14.0;
  *(out+6)=(float)pairs[1]/2;
  *(out+7)=pairs[2];
  *(out+8)=pairs[3];
  *(out+9)=(float)pairs[4]/14;
  *(out+10)=(float)pairs[5]/14;
  *(out+11)=(float)pairs[6]/14;
  *(out+12)=(float)pairs[7]/14;
  *(out+13)=(float)pairs[8]/14;
  *(out+14)=straight[0];
  *(out+15)=straight[1];
  *(out+16)=straight[2];
  *(out+17)=(float)straight[3]/14;
  *(out+18)=(float)straight[4]/14;

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
