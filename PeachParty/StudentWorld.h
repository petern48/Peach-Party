#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Board.h"
#include <string>

#include <list>
#include <vector>
#include "Actor.h"

// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld 
{
public:
  StudentWorld(std::string assetPath);
  virtual int init();
  virtual int move();
  virtual void cleanUp();
  virtual ~StudentWorld();

  int getBankCoins() const { return m_bankMoney; }
  void depositBankCoins(int c) { m_bankMoney += c; }
  void resetBankCoins() { m_bankMoney = 0; }

  bool isValidSquare(int x, int y);
  Activatable* getSquareAt(int x, int y) const;
  Activatable* getRandomSquare() const;
  
  Player* getOtherPlayer(Player* p) const; // UNSURE if works properly TODO
  std::vector<Activatable*> getAllBaddies() const;


private:
	Player* getWinner() const;
	std::string getStatsString() const;

	Board m_board;
	std::list<Activatable*> m_actors;
	Yoshi* m_Yoshi = nullptr;
	Peach* m_Peach = nullptr;
	int m_bankMoney = 0;
};

#endif // STUDENTWORLD_H_
