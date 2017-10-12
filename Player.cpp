#include "Player.h"
#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
using namespace std;


void setAvailablePts( vector<Point> &vec, int nRows, int nCols)
{
    for ( int r = 0; r < nRows; r++)
        for ( int c = 0; c < nCols; c++)
            vec.push_back(Point(r,c));
}

int findIfAvailable(const vector<Point>& vec, const Point& target)
{
    for ( int i = 0;  i < vec.size(); i++)
    {
        if ( vec[i].r == target.r && vec[i].c == target.c )
            return i;
    }
    return -1;
}

//*********************************************************************
//  AwfulPlayer
//*********************************************************************

class AwfulPlayer : public Player
{
public:
    AwfulPlayer(string nm, const Game& g);
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
                                    bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
private:
    Point m_lastCellAttacked;
};

AwfulPlayer::AwfulPlayer(string nm, const Game& g)
: Player(nm, g), m_lastCellAttacked(0, 0)
{}

bool AwfulPlayer::placeShips(Board& b)
{
    // Clustering ships is bad strategy
    for (int k = 0; k < game().nShips(); k++)
        if ( ! b.placeShip(Point(k,0), k, HORIZONTAL))
            return false;
    return true;
}

Point AwfulPlayer::recommendAttack()
{
    if (m_lastCellAttacked.c > 0)
        m_lastCellAttacked.c--;
    else
    {
        m_lastCellAttacked.c = game().cols() - 1;
        if (m_lastCellAttacked.r > 0)
            m_lastCellAttacked.r--;
        else
            m_lastCellAttacked.r = game().rows() - 1;
    }
    return m_lastCellAttacked;
}

void AwfulPlayer::recordAttackResult(Point /* p */, bool /* validShot */,
                                     bool /* shotHit */, bool /* shipDestroyed */,
                                     int /* shipId */)
{
    // AwfulPlayer completely ignores the result of any attack
}

void AwfulPlayer::recordAttackByOpponent(Point /* p */)
{
    // AwfulPlayer completely ignores what the opponent does
}

//*********************************************************************
//  HumanPlayer
//*********************************************************************

bool getLineWithTwoIntegers(int& r, int& c)
{
    bool result(cin >> r >> c);
    if (!result)
        cin.clear();  // clear error state so can do more input operations
    cin.ignore(10000, '\n');
    return result;
}

// TODO:  You need to replace this with a real class declaration and
//        implementation.
class HumanPlayer : public Player
{
public:
    HumanPlayer(string nm, const Game& g) : Player(nm, g) {}
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId)  {  }
    virtual void recordAttackByOpponent(Point p) {  }
    virtual bool isHuman() const { return true; }
private:
    
    //helper:
    void printMsg ( string msg, char hOrV = '0' ) const;
};

void HumanPlayer::printMsg(string msg, char hOrV ) const
{
    if ( msg == "enter")
    {
        cout << "Enter row and column of ";
        hOrV == 'h' ? cout << "left" : cout << "top";
        cout << "most cell (e.g. 3 5): ";
    }
    else if ( msg == "2ints")
        cout << "You must enter two integers." << endl;
    else if ( msg == "cantPlace" )
        cout << "The ship can not be placed there." << endl;
    else if ( msg == "hOrV" )
        cout << "Direction must be h or v." << endl;
    else if ( msg == "attack")
        cout << "Enter the row and column to attack (e.g, 3 5): ";
}

bool HumanPlayer::placeShips( Board& b )
{
    char hOrV;
    int r, c;
    bool placed;
    Direction dir;
    
    cout << name() << " the Human must place " << game().nShips() << " ships." << endl;
    b.display(0);
    
    for ( int i = 0; i < game().nShips(); i++)
    {
        placed = false;
        cout << "Enter h or v for direction of "<< game().shipName(i) << " (length " << game().shipLength(i) << "): ";
        cin >> hOrV;
        cin.ignore(10000, '\n');
        switch (hOrV)
        {
            case 'h':
            case 'v':
                hOrV == 'h' ? dir = HORIZONTAL : dir = VERTICAL;
                while ( !placed )
                {
                    printMsg("enter", hOrV);
                    if (!getLineWithTwoIntegers(r, c))
                    {
                        printMsg("2ints");
                        continue;
                    }
                    if (!b.placeShip(Point(r,c), i, dir))
                    {
                        printMsg("cantPlace");
                        continue;
                    }
                    placed = true;
                    b.display(0);
                }
                break;
            default:
                printMsg("hOrV");
                i--;
                break;
        }
    }
    return true;
}

Point HumanPlayer::recommendAttack(){
    
    int r, c;
    printMsg("attack");
    
    while (!getLineWithTwoIntegers(r, c))
    {
        printMsg("2ints");
        printMsg("attack");
    }
    
    return Point(r,c);
}



//*********************************************************************
//  MediocrePlayer
//*********************************************************************   //  recursive placeships

// TODO:  You need to replace this with a real class declaration and
//        implementation.

// Remember that Mediocre::placeShips(Board& b) must start by calling
// b.block(), and must call b.unblock() just before returning.

class MediocrePlayer : public Player
{
public:
    MediocrePlayer(string nm, const Game& g) : Player(nm, g), currentState(1), transitionPt(0,0)
    {setAvailablePts(availablePts, g.rows(), g.cols());}
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
                                    bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p) {  };
private:
    int currentState;
    Point transitionPt;
    vector<Point> availablePts;
    
    // helper functions:
    bool placeShipsHelper (Board& b, const int& n, const int& cellsAttempted ) const;
    Point randomPointFromSet ( const Point& center );
    // int findIfAvailable ( const Point& target ) const;
};


bool MediocrePlayer::placeShips(Board& b)
{
    for ( int i = 0; i < 50; i++)
    {
        b.block();
        
        if ( placeShipsHelper(b, game().nShips(), 0) )
        {
            b.unblock();
            return true;
        }
        b.unblock();
    }

    b.unblock();
    return false;
}

bool MediocrePlayer::placeShipsHelper(Board &b, const int& n, const int& cellsAttempted) const
{
    if ( n == 0 )
        return true;
    if ( cellsAttempted >= game().rows() * game().cols() )   // if impossible with this block configuration
        return false;
    
    int row = cellsAttempted / game().cols();      // starts at (0,0), then (0,1) - (0,9), then (1,0) - (1,9)...
    int col = cellsAttempted % game().cols();
    
    Direction dir;
    
    if ( b.placeShip(Point(row,col), n-1, HORIZONTAL) )
        dir = HORIZONTAL;
    else if ( b.placeShip(Point(row,col), n-1, VERTICAL))
        dir = VERTICAL;
    else
        return placeShipsHelper(b, n, cellsAttempted + 1);
    
    if ( placeShipsHelper(b, n - 1, 0))
        return true;
    else
    {
        b.unplaceShip(Point(row,col), n-1, dir);
        return placeShipsHelper(b, n, cellsAttempted + 1);
    }
    
}



Point MediocrePlayer::recommendAttack()
{
    Point x;
    int n;
    
    if ( availablePts.empty() )
        return Point(0,0);
    
    if ( currentState == 1 )
    {
        n = randInt(availablePts.size());
        x = availablePts.at(n);
    }
    else                            /////// ( currentState == 2 )
    {
        
        bool allTaken = true;
        
        for ( int i = 0; i < 9; i++)
        {
            if ( findIfAvailable(availablePts, Point(transitionPt.r - 4 + i, transitionPt.c)) != -1 || findIfAvailable(availablePts, Point(transitionPt.r, transitionPt.c - 4 + i)) != -1 )
            {
                allTaken = false;
                break;
            }
        }
        
        if ( allTaken )
        {
            currentState = 1;
            return recommendAttack();
        }
        
        do {
            x = randomPointFromSet(transitionPt);
            n = findIfAvailable(availablePts, x);
        } while ( n == -1 );
    }
    
    availablePts.erase(availablePts.begin() + n);
    return x;
}

void MediocrePlayer::recordAttackResult(Point p, bool validShot, bool shotHit,
                                        bool shipDestroyed, int shipId)
{
    if ( !validShot )
    {
        currentState = 1;
        return;
    }
    
    if ( currentState == 1)
    {
        if ( !shotHit )
            return;
        
        if ( shipDestroyed )
            return;
        
        if ( shotHit && !shipDestroyed )
        {
            currentState = 2;
            transitionPt = p;
            return;
        }
    }
    
    if ( currentState == 2 )
    {
        if ( !shotHit )
            return;
        if ( shipDestroyed )
        {
            currentState = 1;
            return;
        }
        
        if ( shotHit && !shipDestroyed )
            return;
    }
}

Point MediocrePlayer::randomPointFromSet ( const Point& center )
{
    if ( randInt(2) == 0)
        return Point(center.r, center.c - 4 + randInt(9));
    return Point(center.r - 4 + randInt(9), center.c);
}



//*********************************************************************
//  GoodPlayer
//*********************************************************************


class GoodPlayer : public Player
{
public:
    GoodPlayer(string nm, const Game& g);
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
                                    bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
private:
    int currentState;
    char oppGrid[MAXROWS][MAXCOLS];
    Point transitionPt;
    Direction dir;
    Point topOrLeft;
    Point botOrRight;
    
    queue<Point> ptsToExplore;
    
    int biggerShip ( const int& id1, const int& id2) const;
    int calcProb(const Point& p, const int& biggestShipLeft ) const;
    
    bool collateral;
    vector<int> shipLengths;
    void mostProbableFirst (const Point& p, Point& p1, Point& p2, Point& p3, Point& p4 ) const;
    int hitCount;
    
    bool justPlaceThemIfPossible(Board& b, int n, int cellsAttempted );
    vector<Point> openPoints;
    Point bestMove();
    int shipsGone;
};

GoodPlayer::GoodPlayer(string nm, const Game& g) : Player(nm, g), currentState(1), transitionPt(Point(0,0)), dir(HORIZONTAL), topOrLeft(Point(0,0)), botOrRight(Point(0,1)), collateral(false), hitCount(0), shipsGone(0)
{
    for ( int r = 0; r < g.rows(); r++)
        for ( int c = 0; c < g.cols(); c++)
        {
            oppGrid[r][c] = '.';
            openPoints.push_back(Point(r,c));
        }
    
    for ( int i = 0; i < game().nShips(); i++)
    {
        shipLengths.push_back(game().shipLength(i));
    }
    sort(shipLengths.begin(), shipLengths.end() );
    
}

bool GoodPlayer::placeShips(Board& b)    /////////////////////////////////////////////////////////////
{
    vector<Point> shipLocations;
    int idOfBiggest = 0;
    for ( int i = 0; i < game().nShips(); i++)
    {
        idOfBiggest = biggerShip(idOfBiggest, i);
    }
    
    int placeRow = 0;
    int placeCol = game().cols()/game().nShips() - 1;
    Direction dir = HORIZONTAL;
    
    if ( !b.placeShip(Point(placeRow, placeCol), idOfBiggest, HORIZONTAL) )
        return justPlaceThemIfPossible(b, game().nShips(), 0); /////////////////
    else
        shipLocations.push_back(Point(placeRow,placeCol));
    
    placeCol = game().cols()/game().nShips() + 2;
    for ( int i = 0; i < game().nShips(); i++)
    {
        
        if ( i != idOfBiggest )
        {
            placeRow = placeRow + game().rows()/game().nShips();
            placeCol = game().cols() - placeCol;
            dir = HORIZONTAL;
            if (  !b.placeShip(Point(placeRow,placeCol), i, dir) || i == game().nShips() -1  )
            {
                if ( !b.placeShip(Point(game().rows()/2 + 1, 0 ), i, VERTICAL))
                {
                    int i = 0;
                    while ( !b.placeShip(game().randomPoint(), i, dir)) {
                        i++;
                        if ( i > 5 )
                        {
                            for ( int i = 0; i < shipLocations.size(); i++)
                            {
                                for ( int n = 0 ; n < game().nShips(); n++)
                                {
                                    if (b.unplaceShip(shipLocations[i], n, HORIZONTAL) || b.unplaceShip(shipLocations[i], n, VERTICAL) )
                                        break;
                                }
                            }
                            return justPlaceThemIfPossible(b, game().nShips(), 0);
                        }
                    };
                }
                else
                    shipLocations.push_back(Point(game().rows()/2 + 1, 0 ));
            }
            else
                shipLocations.push_back(Point(placeRow,placeCol));
            placeCol += 1;
            placeRow += 1;
        }
    }
    
    return true;
}


int GoodPlayer::biggerShip ( const int& id1, const int& id2) const
{
    if ( game().shipLength(id1) >= game().shipLength(id2) )
        return id1;
    return id2;
}

bool GoodPlayer::justPlaceThemIfPossible(Board& b, int n, int cellsAttempted )
{
    if ( n == 0 )
        return true;
    if ( cellsAttempted >= game().rows() * game().cols() )   // if impossible with this block configuration
        return false;
    
    int row = cellsAttempted / game().cols();      // starts at (0,0), then (0,1) - (0,9), then (1,0) - (1,9)...
    int col = cellsAttempted % game().cols();
    
    Direction dir;
    
    if ( b.placeShip(Point(row,col), n-1, HORIZONTAL) )
        dir = HORIZONTAL;
    else if ( b.placeShip(Point(row,col), n-1, VERTICAL))
        dir = VERTICAL;
    else
        return justPlaceThemIfPossible(b, n, cellsAttempted + 1);
    
    if ( justPlaceThemIfPossible(b, n - 1, 0))
        return true;
    else
    {
        b.unplaceShip(Point(row,col), n-1, dir);
        return justPlaceThemIfPossible(b, n, cellsAttempted + 1);
    }
    
}



Point GoodPlayer::recommendAttack()  //// shiplengths remaining, pt surrounded by o
{
    if ( currentState == 1)
    {
        Point a;
        do{
            a = bestMove();
        }
        while ( oppGrid[a.r][a.c] != '.') ;
        return a;
        
    }
    
    if ( currentState == 2)
    {
        Point a1, a2, a3, a4;
        mostProbableFirst(ptsToExplore.front(), a1, a2, a3, a4);
        
        if ( a1.r >= 0 && a1.r < game().rows() && a1.c >= 0 && a1.c < game().cols() && oppGrid[a1.r][a1.c] == '.')
            return Point(a1.r,a1.c);
        else if ( a2.r >= 0 && a2.r < game().rows() && a2.c >= 0 && a2.c < game().cols() && oppGrid[a2.r][a2.c] == '.' )
            return Point(a2.r,a2.c);
        else if ( a3.r >= 0 && a3.r < game().rows() && a3.c >= 0 && a3.c < game().cols() && oppGrid[a3.r][a3.c] == '.' )
            return Point(a3.r,a3.c);
        else if ( a4.r >= 0 && a4.r < game().rows() && a4.c >= 0 && a4.c < game().cols() && oppGrid[a4.r][a4.c] == '.')
            return Point(a4.r,a4.c);
        else
        {
            ptsToExplore.pop();
            if ( !ptsToExplore.empty() )
            {
                return recommendAttack();
            }
            else
            {
                currentState = 1;
                return recommendAttack();
            }
        }
    }
    
    if ( currentState == 3 )
    {
        if ( dir == HORIZONTAL )
        {
            if ( botOrRight.c + 1 < game().cols() && oppGrid[botOrRight.r][botOrRight.c+1] == '.' )
                return Point(botOrRight.r, botOrRight.c + 1);
            else if ( topOrLeft.c -1 >= 0 && oppGrid[topOrLeft.r][topOrLeft.c-1] == '.')
                return Point(topOrLeft.r, topOrLeft.c - 1);
            else
            {
                collateral = true;
                currentState = 2;
                return recommendAttack();    /// check to make sure id is the one that was hit...if not, keep going
            }
        }
        if ( dir == VERTICAL )
        {
            if ( botOrRight.r + 1 < game().rows() && oppGrid[botOrRight.r+1][botOrRight.c] == '.' )
                return Point(botOrRight.r+1, botOrRight.c);
            else if ( topOrLeft.r-1 >= 0 && oppGrid[topOrLeft.r-1][topOrLeft.c] == '.')
                return Point(topOrLeft.r-1, topOrLeft.c);
            else
            {
                collateral = true;
                currentState = 2;
                return recommendAttack();
            }
        }
        
    }
    
    return game().randomPoint(); //never actually executed
}



void GoodPlayer::mostProbableFirst (const Point& p, Point& d1, Point& d2, Point& d3, Point& d4 ) const
{
    Point left (p.r, p.c-1);
    Point up (p.r-1, p.c);
    Point right (p.r, p.c+1);
    Point down (p.r+1, p.c);
    if (calcProb(left, shipLengths.back()) > calcProb(right, shipLengths.back()) )
    {
        d1 = left;
        d3 = right;
    }
    else
    {
        d1 = right;
        d3 = left;
    }
    if ( calcProb(up, shipLengths.back() ) > calcProb(down, shipLengths.back())  )
    {
        d2 = up;
        d4 = down;
    }
    else
    {
        d2 = down;
        d4 = up;
    }
    
}

void GoodPlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId)
{
    if (validShot)
    {
        if ( shotHit )
        {
            oppGrid[p.r][p.c] = 'X';
            hitCount++;
        }
        else
            oppGrid[p.r][p.c] = 'o';
    }
    
    if ( shipDestroyed )
    {
        shipsGone += game().shipLength(shipId);
        if ( shipsGone != hitCount )
            collateral = true;
        else
            collateral = false;
    }
    
    if ( currentState == 1)
    {
        if ( shipDestroyed )
        {
            shipLengths.erase(find(shipLengths.begin(), shipLengths.end(), game().shipLength(shipId)));
            return;
        }
        else if (shotHit)
        {
            transitionPt = p;
            topOrLeft = p;
            currentState = 2;
            ptsToExplore.push(p);
            return;
        }
        else
            return;
        
    }
    
    if ( currentState == 2)
    {
        if ( shipDestroyed )
        {
            shipLengths.erase(find(shipLengths.begin(), shipLengths.end(), game().shipLength(shipId)));
            if (!collateral)
                currentState  = 1;
            return;
        }
        else if (shotHit)
        {
            if ( transitionPt.r == p.r-1 || transitionPt.r == p.r+1)
            {
                dir = VERTICAL;
                if ( p.r - 1 == transitionPt.r )
                    botOrRight = p;
                else
                {
                    topOrLeft = p;
                    botOrRight = transitionPt;
                }
            }
            else if ( transitionPt.c == p.c-1 || transitionPt.c == p.c+1)
            {
                dir = HORIZONTAL;
                if ( p.c - 1 == transitionPt.c )
                    botOrRight = p;
                else
                {   topOrLeft = p;
                    botOrRight = transitionPt;
                }
            }
            currentState = 3;
        }
        else
            return;
    }
    
    if ( currentState == 3 )
    {
        if ( shotHit )
        {
            if ( dir == VERTICAL && p.r > botOrRight.r )
                botOrRight = p;
            else if ( dir == VERTICAL && p.r < topOrLeft.r )
            {
                topOrLeft = p;
            }
            else if ( dir == HORIZONTAL && p.c > botOrRight.c )
                botOrRight = p;
            else if ( dir == HORIZONTAL && p.c < topOrLeft.c )
            {
                topOrLeft = p;
            }
        }
        
        if ( shipDestroyed )
        {
            shipLengths.erase(find(shipLengths.begin(), shipLengths.end(), game().shipLength(shipId)));
            if ( ((dir == VERTICAL )&&  ((botOrRight.r - topOrLeft.r + 1) == game().shipLength(shipId) ))|| ( dir == HORIZONTAL && (botOrRight.c - topOrLeft.c + 1 == game().shipLength(shipId)) ))  ////   check if shipLength
            {
                if ( collateral )
                {
                    currentState = 2;
                    transitionPt = ptsToExplore.front();
                }
                else
                {
                    currentState = 1;
                    while ( !ptsToExplore.empty() ) ptsToExplore.pop();
                }
                return;
            }
            else
            {
                currentState = 2;
            }
        }
        else if ( shotHit )
        {
            ptsToExplore.push(p);
            return;
        }
        else
        {
            return;
        }
    }
    
    
}
void GoodPlayer::recordAttackByOpponent(Point p)
{
    ///////////not used
    
    
}


int GoodPlayer::calcProb(const Point& p, const int& biggestShipLeft ) const
{
    
    int Xleft = 0;
    int Xright = 0;
    int Ydown = 0;
    int Yup = 0;
    for ( int x = 1; x + p.c < game().cols()  ; x++)
    {
        if ( oppGrid[p.r][x+p.c] == '.' )
            Xright++;
        else
            break;
    }
    for ( int x = 1; p.c - x >= 0 ; x++)
    {
        if ( oppGrid[p.r][p.c-x] == '.')
            Xleft++;
        else
            break;
    }
    for ( int y = 1; y + p.r < game().rows()  ; y++)
    {
        if ( oppGrid[p.r+y][p.c] == '.' )
            Ydown++;
        else
            break;
        
    }
    for ( int y = 1; p.r - y >= 0 ; y++)
    {
        if ( oppGrid[p.r-y][p.c] == '.')
            Yup++;
        else
            break;
    }
    
    if ( 1 + Yup + Ydown < biggestShipLeft && 1 + Xleft + Xright < biggestShipLeft )
        return -3;
    
    if ( 1 + Yup + Ydown < biggestShipLeft || 1 + Xleft + Xright < biggestShipLeft )
        return -2;
    
    if ( (Xleft == 0 && Ydown == 0 )|| ( Xright == 0 && Yup == 0) || (Xleft == 0 && Yup==0) || (Xright==0&&Ydown==0))
        return -1;
    
    if ( Xleft == 0 || Xright == 0 || Ydown == 0 || Yup == 0)
        return (Xleft+Xright+Yup+Ydown) - 1;
    
    return (Xleft+Xright+Yup+Ydown);
}


Point GoodPlayer::bestMove()
{
    vector<int> probables;
    int maxIndex = 0;
    
    for ( int i = 0; i < openPoints.size(); i++)
    {
        probables.push_back( calcProb(openPoints.at(i), shipLengths.back()) );
        if ( probables[i] > probables[maxIndex])
            maxIndex = i;
    }
    
    Point attackPt = openPoints.at(maxIndex);
    openPoints.erase(openPoints.begin() + maxIndex);
    
    return attackPt;
    
}

//*********************************************************************
//  createPlayer
//*********************************************************************

Player* createPlayer(string type, string nm, const Game& g)
{
    static string types[] = {
        "human", "awful", "mediocre", "good"
    };
    
    int pos;
    for (pos = 0; pos != sizeof(types)/sizeof(types[0])  &&
         type != types[pos]; pos++)
        ;
    switch (pos)
    {
        case 0:  return new HumanPlayer(nm, g);
        case 1:  return new AwfulPlayer(nm, g);
        case 2:  return new MediocrePlayer(nm, g);
        case 3:  return new GoodPlayer(nm, g);
        default: return nullptr;
    }
}


