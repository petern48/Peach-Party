#include "StudentWorld.h"
#include "GameConstants.h"
#include <string>
#include <sstream>
using namespace std;

const int LENGTHOFGAME = 99;
const int LEFT = 180;
const int RIGHT = 0;
const int UP = 90;
const int DOWN = 270;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h, and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath)
{
    // Initialize data members (done in .h file)
}

int StudentWorld::init()
{
    // Load the board data file in using Board Class
    ostringstream oss;
    oss << assetPath() << "board0" << getBoardNumber() << ".txt";
    string boardFile = oss.str();

    // Load positions into the board
    Board::LoadResult result = m_board.loadBoard(boardFile);

    if (result != Board::load_success) {
        if (result == Board::load_fail_file_not_found)
            cerr << "Could not find " << boardFile << endl;
        else if (result == Board::load_fail_bad_format)
            cerr << "Board was improperly formatted" << endl;

        return GWSTATUS_BOARD_ERROR;
    }

    // if result == load_success
    else {
        cerr << "Successfully loaded board" << endl;

        // Determine where each actor is supposed to be on board, save in single STL container of pointers
        for (int x = 0; x < SPRITE_WIDTH; x++)
            for (int y = 0; y < SPRITE_HEIGHT; y++) {
                Actor* a = nullptr;
                switch (m_board.getContentsOf(x,y)) {
                case Board::empty:
                    break;
                case Board::player:
                    m_Peach = new Peach(x, y, this);
                    m_Yoshi = new Yoshi(x, y, this);
                    a = new BlueCoinSquare(x, y, this); // Players start on a BlueCoinSquare
                    break;
                case Board::blue_coin_square:
                    a = new BlueCoinSquare(x, y, this);
                    break;
                case Board::red_coin_square:
                    a = new RedCoinSquare(x, y, this);
                    break;
                case Board::up_dir_square:
                    a = new DirectionSquare(x, y, this, UP);
                    break;
                case Board::down_dir_square:
                    a = new DirectionSquare(x, y, this, DOWN);
                    break;
                case Board::left_dir_square:
                    a = new DirectionSquare(x, y, this, LEFT);
                    break;
                case Board::right_dir_square:
                    a = new DirectionSquare(x, y, this, RIGHT);
                    break;
                case Board::event_square:
                    a = new EventSquare(x, y, this);
                    break;
                case Board::bank_square:
                    a = new BankSquare(x, y, this);
                    break;
                case Board::star_square:
                    a = new StarSquare(x, y, this);
                    break;
                case Board::bowser:
                    a = new Bowser(x, y, this);
                    m_actors.push_back(a);
                    a = new BlueCoinSquare(x, y, this); // Bowser starts on BlueCoinSquare
                    break;
                case Board::boo:
                    a = new Boo(x, y, this);
                    m_actors.push_back(a);
                    a = new BlueCoinSquare(x, y, this); // Boo starts on BlueCoinSquare
                    break;
                }
                if (a != nullptr)
                    m_actors.push_back(a);
            }
    }
    m_bankMoney = 0;
	startCountdownTimer(LENGTHOFGAME);
    return GWSTATUS_CONTINUE_GAME;
}

// GameWorld repeatedly calls this roughly 20 times per second. move() represents a single tick
int StudentWorld::move()
{
    // If game over
    if (timeRemaining() <= 0) {
        playSound(SOUND_GAME_FINISHED);

        // Chooses random if tie
        Player* winner = getWinner();

        // Call setFinalScore
        setFinalScore(winner->getStars(), winner->getCoins());

        // if Peach won
        if (winner == m_Peach)
            return GWSTATUS_PEACH_WON;
        else
            return GWSTATUS_YOSHI_WON;
    }

    // Ask all active actors to doSomething
    m_Peach->doSomething();
    m_Yoshi->doSomething();

    list<Actor*>::iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++)
        if ((*it) != nullptr && (*it)->isAlive())
            (*it)->doSomething();


    list<Actor*>::iterator temp;
    // Delete any actors that have become inactive/dead during this tick
    for (it = m_actors.begin(); it != m_actors.end();) {
        if ((*it) != nullptr && !(*it)->isAlive()) { // Short Circuit
            temp = it;
            it++;
            delete* temp;
            m_actors.erase(temp);
        }
        else
            it++;
    }

    // Update the status text on top of the screen
    string gameStats = getStatsString();
    setGameStatText(gameStats);

    // If game not over return
    return GWSTATUS_CONTINUE_GAME;
}

// Function may run twice, so set nullptrs
void StudentWorld::cleanUp()
{
    if (m_Peach != nullptr) {
        delete m_Peach;
        m_Peach = nullptr;
    }
    if (m_Yoshi != nullptr) {
        delete m_Yoshi;
        m_Yoshi = nullptr;
    }
    // Delete other objects
    list<Actor*>::iterator it;
    list<Actor*>::iterator temp;
    for (it = m_actors.begin(); it != m_actors.end();) {
        if ((*it) != nullptr) {
            temp = it;
            it++;
            delete *temp;
            m_actors.erase(temp); // Erase pointer
        }
    }
}


StudentWorld::~StudentWorld() {
    cleanUp();
}


Player* StudentWorld::getOtherPlayer(Player* p) const {
    if (p == m_Peach)
        return m_Yoshi;
    else
        return m_Peach;
}

vector<Player*> StudentWorld::getAllPlayers() const {
    vector<Player*> v;
    v.push_back(m_Peach);
    v.push_back(m_Yoshi);
    return v;
}

Player* StudentWorld::getWinner() const {
    // Peach has more stars
    if (m_Peach->getStars() > m_Yoshi->getStars())
        return m_Peach;
    // Yoshi has more stars
    else if (m_Yoshi->getStars() > m_Peach->getStars())
        return m_Yoshi;
    // Same number of stars
    else {
        // Peach has more coins
        if (m_Peach->getCoins() > m_Yoshi->getCoins())
            return m_Peach;
        // Yoshi has more coins
        else if (m_Yoshi->getCoins() > m_Peach->getCoins())
            return m_Yoshi;
        // Choose randomly
        else {
            if (randInt(1, 2) == 1)
                return m_Peach;
            else 
                return m_Yoshi;
        }
    }
}

bool StudentWorld::isValidSquare(int x, int y) {
    // If not perfectly on a square
    if (x % SPRITE_WIDTH != 0 || y % SPRITE_HEIGHT != 0)
        return false;

    // Convert back to 16x16 Board coordinates
    int boardX = x / SPRITE_WIDTH; // Rounds down for int division
    int boardY = y / SPRITE_HEIGHT;
    if (m_board.getContentsOf(boardX, boardY) == Board::empty)
        return false;
    return true;
}

string StudentWorld::getStatsString() const {
    ostringstream oss;
    oss << "P1 Roll: " << m_Peach->getRoll() << " Stars: " << m_Peach->getStars() << " $$: " << m_Peach->getCoins();
    if (m_Peach->hasVortex())
        oss << " VOR";
    oss << " | Time: " << timeRemaining() << " | " << "Bank: " << getBankCoins() << " | ";
    oss << "P2 Roll: " << m_Yoshi->getRoll() << " Stars: " << m_Yoshi->getStars() << " $$: " << m_Yoshi->getCoins();
    if (m_Yoshi->hasVortex())
        oss << " VOR";

    return oss.str();
}

Actor* StudentWorld::getSquareAt(int x, int y) const {
    list<Actor*>::const_iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++)
        if ((*it) != nullptr && (*it)->isSquare() && (*it)->getX() == x && (*it)->getY() == y)
            return (*it);
    return nullptr;
}

Actor* StudentWorld::getRandomSquare() const {
    int sizeOfContainer = m_actors.size();
    int randomInt = randInt(0, sizeOfContainer);

    list<Actor*>::const_iterator it = m_actors.begin();

    // Iterate to the random object
    for (int i = 0; i < randomInt; i++)
        it++;
    // Iterate forward until find a square
    while (true) {
        if ((*it) != nullptr && (*it)->isSquare()) // Found a square
            break;
        it++;
        // Loop around if needed
        if (it == m_actors.end())
            it = m_actors.begin();
    }
    return *it;
}

vector<Actor*> StudentWorld::getAllBaddies() const {
    vector<Actor*> v;
    list<Actor*>::const_iterator it;
    for (it = m_actors.begin(); it != m_actors.end(); it++)
        if ((*it) != nullptr && (*it)->isBaddie())
            v.push_back((*it));
    return v;
}