# Battleship-Proj3
Battleship game for command line built using C++, for CS32


main classes:
Game
Board
Player (base class)
  -HumanPlayer
  -GoodPlayer
    - implemented my own MiniMax and probability algorithm to attack (beats MediocrePlayer ~96% of the time)
  -MediocrePlayer
    - uses recursion to place its ships
  -AwfulPlayer
  

