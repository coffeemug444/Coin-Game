/* coinboard.h */

#ifndef COINBOARD_H
#define COINBOARD_H

#include <vector>
#include <SFML/System/Vector2.hpp>


enum Moves {PLACE = 0, UNDO};
typedef enum Moves Move_t;

class Coinboard {
private:

    int board [12][7];
    std::vector<sf::Vector2i> moves;

    bool running;

    int human;
    int bot;
    int recursion_depth;

    const int PLACE = 0;
    const int UNDO = 1;

    // returns a vector of all tiles in the board using a given ID
    std::vector<sf::Vector2i> get_board_used_by_id(int val);

    // returns a vector of all cell positions touching a given position
    inline std::vector<sf::Vector2i> get_neighbors(sf::Vector2i pos);

    // given two adjecent cells, returns the position of the next cell in the line
    sf::Vector2i get_next_cell(sf::Vector2i orig, sf::Vector2i neighbor);

    // iterates through every cell in the board and adds placeable cells to a vector
    std::vector<sf::Vector2i> find_avail_moves();

    // given the current board state, iterates through current available
    // moves and returns the optimal move for a given player (id)
    sf::Vector2i find_optimal_move(int id);

    // retrieves a list of the neighbors of a position and returns true
    // if at least 2 of the neighbors are previous moves
    bool check_neighbors(sf::Vector2i pos);

    // runs a depth first search of possible move pathways and applies scoring based on
    // who won and how many moves the winning move is. uses alpha beta pruning
    // to eliminate pathways that will not be travelled to save compute time.
    int minimax(sf::Vector2i last_move, int alpha, int beta, int depth, int id);

    // resets all of the empty cells to unhinted and reapplies all hints to moveable places
    void apply_hint();

    // checks the board to see if a given position is part of a winning line.
    // called extensively by the minimax algorithm so needs a position to check as
    // it is much faster than checking the entire board to see if the game is over
    bool check_winning_cell(sf::Vector2i pos);

    // takes the last move off the stack and sets it to an empty cell. resets the position
    // to an empty cell. Does not apply hints as this function is used extensively by the
    // minimax function and would slow the program substantially
    void undo_move();

    // given a winning cell position, returns a vector of the other two cell positions
    // in the winning line
    std::vector<sf::Vector2i> get_winning_cells(sf::Vector2i pos);

public:
    // returns the id of a given position. if the position is
    // outside of the bounds of the board returns 6 (id of empty cell)
    int get_board(sf::Vector2i pos);

    void set_board(sf::Vector2i pos, int val);

    void set_depth(int depth);

    void set_players(int h, int b);

    bool game_running();

    // applies the move the player made at the given position, or undoes the previous move
    void do_move(sf::Vector2i pos, Move_t action);

    // finds optimal move for bot player and applies it.
    void do_bot_move();
    
    sf::Vector2i get_last_move();

    Coinboard();
};

#endif // COINBOARD_H