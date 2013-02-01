#ifndef __CARDHEURISTICS_HPP__
#define __CARDHEURISTICS_HPP__

#include <string>
#include <vector>

class CardHeuristics {
public: 
	enum HAND_TYPE {LOW_PAIR, MID_PAIR, HI_PAIR, TWO_PAIR, TRIPLE, FOUR, STRAIGHT_DRAW, STRAIGHT, FLUSH_DRAW, FLUSH};
	static void getHandType(const std::vector<std::string> &cards, const std::vector<std::string> &board, std::vector<HAND_TYPE> &handType);
	static std::string getEquityString(const std::vector<float> &weights, const std::vector<std::string> &hand, const std::vector<std::string> &board);
	static void haveMulti(const std::vector<int> &cards, std::vector<int> &have, int hi, int low);	
	static void haveStraight(const std::vector<int> &cards, std::vector<int> &have);
	static void haveFlush(const std::vector<int> &cards, std::vector<int> &have);
	static std::string getNumStrings(const std::vector<int> &hand, const std::vector<int> &board, const std::vector<float> &weights);
	static std::string convertSuit2String(int suit);
	static void getStringFromSuits(int i,int j, int * cardSuits, int have0, int have1, std::vector<std::string> &flushDraw, std::vector<std::string> &flush);
  static bool havePair(const std::vector<std::string> &had);
	static std::string getSuitStrings(const std::vector<int> &hand, const std::vector<int> &board, const std::vector<int> &handNums, const std::vector<int> &boardNums, const std::vector<float> &weights);
	static void getCards(const std::vector<std::string> &hand, 
					     const std::vector<std::string> &board, std::vector<int> handSuits, std::vector<int> handNums,std::vector<int> boardSuits,std::vector<int> boardNums);

  static void getPairs(std::vector<int> hand, std::vector<int> board, std::vector<int> &pairs);
  static void getStraight(std::vector<int> hand,
			       std::vector<int> board, std::vector<int> &straight);
  static void getFlush(std::vector<int> handSuits, 
			    std::vector<int> handNum, 
			    std::vector<int> boardSuits, 
			    std::vector<int> boardNum, std::vector<int> &flush);

  static void createBoardTextureInput(const std::vector<std::string> &myHand, 
			       const std::vector<std::string> &boardCards,
			       float *textureInputs); 
  
  static void createTextureFeatures(std::vector<int> flush, 
			     std::vector<int> pairs, 
			     std::vector<int> straight, 
			     float *out);
  
  static std::string getTextureString(std::vector<int> flush, 
			       std::vector<int> pairs, 
			       std::vector<int> straight);
  private:
  static int getSuit(char suit);
  static int getNum(char card);    
    
};

#endif  // __CARDHEURISTICS_HPP__
