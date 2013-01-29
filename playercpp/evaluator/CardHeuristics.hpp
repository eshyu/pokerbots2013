#ifndef __CARDHEURISTICS_HPP__
#define __CARDHEURISTICS_HPP__

#include <string>
#include <vector>

class CardHeuristics {
public: 
  static bool havePair(const std::vector<std::string> &had);

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
