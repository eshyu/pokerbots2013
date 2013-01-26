#ifndef __CARDHEURISTICS_HPP__
#define __CARDHEURISTICS_HPP__

#include <string>
#include <vector>

class CardHeuristics {
public: 
  std::vector<int> getPairs(std::vector<int> hand, std::vector<int> board);
  std::vector<int> getStraight(std::vector<int> hand,
			       std::vector<int> board);
  std::vector<int> getFlush(std::vector<int> handSuits, 
			    std::vector<int> handNum, 
			    std::vector<int> boardSuits, 
			    std::vector<int> boardNum);

  void createBoardTextureInput(const std::vector<std::string> &myHand, 
			       const std::vector<std::string> &boardCards,
			       double *textureInputs); 
  
  void createTextureFeatures(std::vector<int> flush, 
			     std::vector<int> pairs, 
			     std::vector<int> straight, 
			     double *out);
  
  std::string getTextureString(std::vector<int> flush, 
			       std::vector<int> pairs, 
			       std::vector<int> straight);
  
private:
  int getSuit(char suit);
  int getNum(char card);    
    
};

#endif  // __CARDHEURISTICS_HPP__
