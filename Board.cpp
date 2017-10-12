#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

class Ship{
public:
    Ship ( int id ): shipId (id) {}
    vector<Point> unHitPts;  // get from Game::length
    int shipId;
};


class BoardImpl
{
  public:
    BoardImpl(const Game& g);
    void clear();
    void block();
    void unblock();
    bool placeShip(Point topOrLeft, int shipId, Direction dir);
    bool unplaceShip(Point topOrLeft, int shipId, Direction dir);
    void display(bool shotsOnly) const;
    bool attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId);
    bool allShipsDestroyed() const;
    ~BoardImpl();

  private:
      // TODO:  Decide what private members you need.  Here's one that's likely
      //        to be useful:
    const Game& m_game;
    vector<Ship*> shippy;
    char displayGrid[MAXROWS][MAXCOLS];
    
    //helper functions:
    bool isValidPlacement( const Point& topOrLeft, const Direction& dir = HORIZONTAL, const int& length = 1) const;
    bool addShipToBoard(const Point& topOrLeft, const Direction& dir, const int& shipId);
    bool markOnBoard(const Point& topOrLeft, Ship* target, const Direction& dir, const char& replaceThat, const char& withThis, const int& length );
    bool replace(const Point& pt, char oldVal, char newVal);
    
    bool shipIdTaken( const int& id ) const;
    int isOccupiedBy(const Point& pt, Ship* target) const;
    void deleteShip(vector<Ship*>::iterator itr);
};


int BoardImpl::isOccupiedBy(const Point& pt, Ship* target) const
{
        for ( int j = 0; j < target->unHitPts.size(); j++)
        {
            if (target->unHitPts[j].r == pt.r && target->unHitPts[j].c == pt.c)
                return j;
        }
    return -1;
}

bool BoardImpl::shipIdTaken( const int& id ) const
{
    for ( int i = 0; i< shippy.size(); i++)
    {
        if ( shippy[i]->shipId == id )
            return true;
    }
    return false;
}


bool lesserId( Ship* s1, Ship* s2)
{
    return ( s1->shipId < s2->shipId );
}

bool BoardImpl::replace(const Point& pt, char oldVal, char newVal)
{
    if ( displayGrid[pt.r][pt.c] == oldVal )
    {
        displayGrid[pt.r][pt.c] = newVal;
        return true;
    }
    return false;
}

BoardImpl::BoardImpl(const Game& g)
 : m_game(g)
{
    clear();
    // This compiles, but may not be correct
}

void BoardImpl::clear()
{
    for ( int r = 0; r < MAXROWS; r++)
    {
        for ( int c = 0; c < MAXCOLS; c++)
            displayGrid[r][c] = '.';
    }
}

BoardImpl::~BoardImpl()
{
    while ( !shippy.empty() )
        deleteShip(shippy.begin());
}

void BoardImpl::deleteShip(vector<Ship*>::iterator itr)
{
    delete *itr;
    shippy.erase(itr);
}


void BoardImpl::block()
{
      // Block cells with 50% probability
    for (int r = 0; r < m_game.rows(); r++)
        for (int c = 0; c < m_game.cols(); c++)
        {
            if (randInt(2) == 0)
            {
                displayGrid[r][c] = '#';
            }
        }
    
}

void BoardImpl::unblock()
{
    for (int r = 0; r < m_game.rows(); r++)
        for (int c = 0; c < m_game.cols(); c++)
        {
            if ( displayGrid[r][c] == '#' )
                displayGrid[r][c] = '.';
        }
}

bool BoardImpl::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    if ( shipId >= m_game.nShips() || shipId < 0 )
        return false;
    if ( shipIdTaken(shipId))
        return false;
    if ( !isValidPlacement(topOrLeft, dir, m_game.shipLength(shipId)))
        return false;
    
    if ( !addShipToBoard(topOrLeft, dir, shipId) )  // checks for overlap with blocks and with other ships
    {
        return false;
    }

    std::sort(shippy.begin(), shippy.end(), lesserId);   // sorts by id so correct ship can be accessed at vector[ID]
    return true;
}

bool BoardImpl::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    if ( !shipIdTaken(shipId))
        return false;
    
    vector<Ship*>::iterator p;
    for ( p = shippy.begin(); (*p)->shipId != shipId; p++) ;
    
    if ( !markOnBoard(topOrLeft, *p, dir, m_game.shipSymbol(shipId), '.', m_game.shipLength(shipId)) )
       return false;
    
    deleteShip(p);
    return true;
}

void BoardImpl::display(bool shotsOnly) const
{
    cout << "  ";
    for ( int c = 0; c < m_game.cols(); c++)
        cout << c;
    cout << endl;
    for ( int r = 0; r < m_game.rows(); r++)
    {
        cout << r << ' ';
        for ( int c = 0; c < m_game.cols(); c++)
        {
            if ( shotsOnly )
            {
                if ( displayGrid[r][c] != '.' && displayGrid[r][c] != 'o' && displayGrid[r][c] != 'X')
                    cout << '.';
                else
                    cout << displayGrid[r][c];
            }
            else
                cout << displayGrid[r][c];
        }
        cout << endl;
    }
    
}

bool BoardImpl::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    if ( !isValidPlacement(p) )
        return false;
    if ( displayGrid[p.r][p.c] == 'X' || displayGrid[p.r][p.c] == 'o')  ///previously attacked location
        return false;
    int hit;
    shotHit = false;
    shipDestroyed = false;
    displayGrid[p.r][p.c] = 'o';
    for ( int i = 0; i < shippy.size(); i++)
    {
        hit = isOccupiedBy(p, shippy[i]);
        if ( hit != -1 )
        {
            shippy[i]->unHitPts.erase(shippy[i]->unHitPts.begin() + hit);
            displayGrid[p.r][p.c] = 'X';
            shotHit = true;
            
            if ( shippy[i]->unHitPts.empty())
            {
                shipId = i;
                shipDestroyed = true;
            }
            break;
        }
    }
    
    return true; // This compiles, but may not be correct
}

bool BoardImpl::allShipsDestroyed() const
{
   
    for ( int i = 0; i < shippy.size(); i++)
    {
        if ( !shippy[i]->unHitPts.empty() )
            return false;
    }
   
    return true; // This compiles, but may not be correct
}

bool BoardImpl::isValidPlacement( const Point& topOrLeft, const Direction& dir, const int& length) const
{
    if ( topOrLeft.r > m_game.rows() || topOrLeft.c > m_game.cols() || topOrLeft.r < 0 || topOrLeft.c < 0)
        return false;
    
    switch (dir)
    {
        case HORIZONTAL:
            if ( topOrLeft.c + length > m_game.cols() )
                return false;
            break;
        case VERTICAL:
            if ( topOrLeft.r + length > m_game.rows() )
                return false;
            break;
    }
    return true;
}

bool BoardImpl::addShipToBoard(const Point& topOrLeft, const Direction& dir, const int& shipId)
{
    Ship* toAdd = new Ship ( shipId );
    if ( !markOnBoard(topOrLeft, toAdd, dir, '.', m_game.shipSymbol(shipId), m_game.shipLength(shipId)) )
    {
        markOnBoard(topOrLeft, toAdd, dir,  m_game.shipSymbol(shipId), '.', m_game.shipLength(shipId));
        delete toAdd;
        return false;
    }

    shippy.push_back(toAdd);
    return true;
}

bool BoardImpl::markOnBoard(const Point& topOrLeft, Ship* target, const Direction& dir, const char& replaceThat, const char& withThis, const int& length )
{
    switch (dir)
    {
        case HORIZONTAL:
            for ( int i = 0; i < length; i++)
            {
                if ( !replace(Point(topOrLeft.r,topOrLeft.c+i), replaceThat, withThis))
                    return false;
                if ( replaceThat == '.' )
                    target->unHitPts.push_back(Point(topOrLeft.r, topOrLeft.c+i));
            }
            break;
        case VERTICAL:
            for ( int i = 0; i < length; i++)
            {
                if ( !replace(Point(topOrLeft.r+i,topOrLeft.c), replaceThat, withThis))
                    return false;
                if ( replaceThat == '.')
                    target->unHitPts.push_back(Point(topOrLeft.r+i, topOrLeft.c));
            }
            break;
    }
    return true;
}

//******************** Board functions ********************************

// These functions simply delegate to BoardImpl's functions.
// You probably don't want to change any of this code.

Board::Board(const Game& g)
{
    m_impl = new BoardImpl(g);
}

Board::~Board()
{
    delete m_impl;
}

void Board::clear()
{
    m_impl->clear();
}

void Board::block()
{
    return m_impl->block();
}

void Board::unblock()
{
    return m_impl->unblock();
}

bool Board::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->placeShip(topOrLeft, shipId, dir);
}

bool Board::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->unplaceShip(topOrLeft, shipId, dir);
}

void Board::display(bool shotsOnly) const
{
    m_impl->display(shotsOnly);
}

bool Board::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    return m_impl->attack(p, shotHit, shipDestroyed, shipId);
}

bool Board::allShipsDestroyed() const
{
    return m_impl->allShipsDestroyed();
}



