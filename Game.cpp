#include "Game.h"
#include "Board.h"
#include "Player.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>

using namespace std;

class GameImpl
{
  public:
    GameImpl(int nRows, int nCols);
    int rows() const;
    int cols() const;
    bool isValid(Point p) const;
    Point randomPoint() const;
    bool addShip(int length, char symbol, string name);
    int nShips() const;
    int shipLength(int shipId) const;
    char shipSymbol(int shipId) const;
    string shipName(int shipId) const;
    Player* play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause);
    ~GameImpl();
private:
    int m_rows;
    int m_cols;
    int m_nShips;
    struct ShipType {
        int length;
        char symbol;
        string nm;
    };
    vector<ShipType*> m_shipTypes;
    
    void takeTurn(Player* myTurn, Player* opponent, Board& opponentBoard, bool& shotHit, bool& shipDestroyed, int& shipId);
    
};

void waitForEnter()
{
    cout << "Press enter to continue: ";
    cin.ignore(10000, '\n');
}

GameImpl::GameImpl(int nRows, int nCols) : m_rows(nRows), m_cols(nCols), m_nShips(0)
{
    // This compiles but may not be correct
}

GameImpl::~GameImpl()
{
    ShipType* trash;
    while ( !m_shipTypes.empty() )
    {
        trash = m_shipTypes.back();
        m_shipTypes.pop_back();
        delete trash;
    }
}

int GameImpl::rows() const
{
    return m_rows;  // This compiles but may not be correct
}

int GameImpl::cols() const
{
    return m_cols;  // This compiles but may not be correct
}

bool GameImpl::isValid(Point p) const
{
    return p.r >= 0  &&  p.r < rows()  &&  p.c >= 0  &&  p.c < cols();
}

Point GameImpl::randomPoint() const
{
    return Point(randInt(rows()), randInt(cols()));
}

bool GameImpl::addShip(int length, char symbol, string name)
{
    ShipType* add = new ShipType { length, symbol, name };
    m_shipTypes.push_back(add);
    m_nShips++;
    return true;  // This compiles but may not be correct
}

int GameImpl::nShips() const
{
    return m_nShips;  // This compiles but may not be correct
}

int GameImpl::shipLength(int shipId) const
{
    return m_shipTypes[shipId]->length;  // This compiles but may not be correct
}

char GameImpl::shipSymbol(int shipId) const
{
    return m_shipTypes[shipId]->symbol;  // This compiles but may not be correct
}

string GameImpl::shipName(int shipId) const
{
    return m_shipTypes[shipId]->nm;  // This compiles but may not be correct
}

Player* GameImpl::play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause)
{
    if ( !p1->placeShips(b1) || !p2->placeShips(b2) )
        return nullptr;
    while (!b1.allShipsDestroyed() && !b2.allShipsDestroyed())
    {
        bool shotHit = false;
        bool shipDestroyed = false;
        int shipId = -1;
        takeTurn(p1, p2, b2, shotHit, shipDestroyed, shipId);
        if ( b2.allShipsDestroyed() )
        {
            cout << p1->name() << " wins!" << endl;
            if ( p2->isHuman() )
            {
                cout << "Here's where " << p1->name() << "'s ships were:" << endl;
                b1.display(false);
            }
            return p1;
        }
        if ( shouldPause )
            waitForEnter();
        takeTurn(p2, p1, b1, shotHit, shipDestroyed, shipId);
        
        if ( b1.allShipsDestroyed() )
            break;
        
        if ( shouldPause )
            waitForEnter();

    }
    cout << p2->name() << " wins!" << endl;
    if ( p1->isHuman() )
    {
        cout << "Here's where " << p2->name() << "'s ships were:" << endl;
        b2.display(false);
    }
    return p2;  // This compiles but may not be correct


}

void GameImpl::takeTurn( Player* myTurn, Player* opponent, Board& opponentBoard, bool& shotHit, bool& shipDestroyed, int& shipId )
{
    cout << myTurn->name() << "'s turn. Board for " << opponent->name() << ":" << endl;
    opponentBoard.display( myTurn->isHuman() );
    
    Point attackPt = myTurn->recommendAttack();
    if (!opponentBoard.attack(attackPt, shotHit, shipDestroyed, shipId))
    {
        cout << myTurn->name() << " wasted a shot at (" << attackPt.r << "," << attackPt.c << ")." << endl;
        myTurn->recordAttackResult(attackPt, false, false, false, shipId);
        opponent->recordAttackByOpponent(attackPt);
    }
    else
    {
        cout << myTurn->name() << " attacked (" << attackPt.r << "," << attackPt.c << ") and ";
        if ( shotHit && !shipDestroyed )
            cout << "hit something";
        else if ( shotHit && shipDestroyed )
            cout << "destroyed the " << shipName(shipId);
        else
            cout << "missed";
        
        cout <<", resulting in:" << endl;
        opponentBoard.display( myTurn->isHuman() );
        myTurn->recordAttackResult(attackPt, true, shotHit, shipDestroyed, shipId);
        opponent->recordAttackByOpponent(attackPt);
    }
}


//******************** Game functions *******************************

// These functions for the most part simply delegate to GameImpl's functions.
// You probably don't want to change any of the code from this point down.

Game::Game(int nRows, int nCols)
{
    if (nRows < 1  ||  nRows > MAXROWS)
    {
        cout << "Number of rows must be >= 1 and <= " << MAXROWS << endl;
        exit(1);
    }
    if (nCols < 1  ||  nCols > MAXCOLS)
    {
        cout << "Number of columns must be >= 1 and <= " << MAXCOLS << endl;
        exit(1);
    }
    m_impl = new GameImpl(nRows, nCols);
}

Game::~Game()
{
    delete m_impl;
}

int Game::rows() const
{
    return m_impl->rows();
}

int Game::cols() const
{
    return m_impl->cols();
}

bool Game::isValid(Point p) const
{
    return m_impl->isValid(p);
}

Point Game::randomPoint() const
{
    return m_impl->randomPoint();
}

bool Game::addShip(int length, char symbol, string name)
{
    if (length < 1)
    {
        cout << "Bad ship length " << length << "; it must be >= 1" << endl;
        return false;
    }
    if (length > rows()  &&  length > cols())
    {
        cout << "Bad ship length " << length << "; it won't fit on the board"
             << endl;
        return false;
    }
    if (!isascii(symbol)  ||  !isprint(symbol))
    {
        cout << "Unprintable character with decimal value " << symbol
             << " must not be used as a ship symbol" << endl;
        return false;
    }
    if (symbol == 'X'  ||  symbol == '.'  ||  symbol == 'o')
    {
        cout << "Character " << symbol << " must not be used as a ship symbol"
             << endl;
        return false;
    }
    int totalOfLengths = 0;
    for (int s = 0; s < nShips(); s++)
    {
        totalOfLengths += shipLength(s);
        if (shipSymbol(s) == symbol)
        {
            cout << "Ship symbol " << symbol
                 << " must not be used for more than one ship" << endl;
            return false;
        }
    }
    if (totalOfLengths + length > rows() * cols())
    {
        cout << "Board is too small to fit all ships" << endl;
        return false;
    }
    return m_impl->addShip(length, symbol, name);
}

int Game::nShips() const
{
    return m_impl->nShips();
}

int Game::shipLength(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipLength(shipId);
}

char Game::shipSymbol(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipSymbol(shipId);
}

string Game::shipName(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipName(shipId);
}

Player* Game::play(Player* p1, Player* p2, bool shouldPause)
{
    if (p1 == nullptr  ||  p2 == nullptr  ||  nShips() == 0)
        return nullptr;
    Board b1(*this);
    Board b2(*this);
    return m_impl->play(p1, p2, b1, b2, shouldPause);
}

