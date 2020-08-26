#include "coinboard.h"
#include <algorithm>
#include "stdio.h"

#define INF 100000  //big enough lmao

// finds optimal move for bot player and applies it.
void Coinboard::do_bot_move() {
    sf::Vector2i pos = find_optimal_move(bot);
    set_board(pos, bot);
    moves.push_back(pos);

    bool check_win = check_winning_cell(pos);
    if (check_win) {
        std::vector<sf::Vector2i> cells = get_winning_cells(pos);
        for (int i = 0; i < 3; i++) {
            set_board(cells[i], bot + 4);
        }
        // printf("\n");
        running = false;
        return;
    }

    apply_hint();
}

// applies the move the player made at the given position, or undoes the previous move
void Coinboard::do_move(sf::Vector2i pos, Move_t action) {
    if (action == UNDO) {
        if (!running) {
            if (get_board(moves.back()) == (bot + 4) && bot != -1) {    // if the last move was a winning move by the bot, and a bot exists, undo twice
                undo_move();
            }
            undo_move();
        } else {
            if (bot != -1) { // if there is a bot (singleplayer game), undo twice
                undo_move();
            }
            undo_move();
        }

        if (human == 1 && moves.size() == 2 && bot != -1) {
            do_bot_move();
        }
        apply_hint();
        return;
    }

    std::vector<sf::Vector2i> avail_moves = find_avail_moves();
    bool found = false;
    for (int i = 0; i < avail_moves.size(); i++) {
        if (pos == avail_moves[i]) found = true;
    }
    if (!found) return;

    set_board(pos, human);
    moves.push_back(pos);

    bool check_win = check_winning_cell(pos);
    if (check_win) {
        std::vector<sf::Vector2i> cells = get_winning_cells(pos);
        for (int i = 0; i < 3; i++)
            set_board(cells[i], human + 4);
        std::vector<sf::Vector2i> hintedcells = get_board_used_by_id(7);
        for (int i = 0; i < hintedcells.size(); i++) {
            set_board(hintedcells[i], 3);
        }
        running = false;
        return;
    }

    apply_hint();
}

// returns a vector of all tiles in the board using a given ID
std::vector<sf::Vector2i> Coinboard::get_board_used_by_id(int val) {
    std::vector<sf::Vector2i> cells;
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 7; j++)
            if (board[i][j] == val)
                cells.push_back(sf::Vector2i(i, j));
    return cells;
}

// returns a vector of all cell positions touching a given position
inline std::vector<sf::Vector2i> Coinboard::get_neighbors(sf::Vector2i pos) {
    std::vector<sf::Vector2i> neighbors;
    int i = 1;
    if (pos.x % 2 == 0) i = -1;

	pos.y -= 1;
	neighbors.push_back(pos);	//top
	pos.y += 2;
	neighbors.push_back(pos);	//bottom
	pos.y -= 1;
	
	pos.x += 1;
	neighbors.push_back(pos);	//right
	pos.x -= 2;
	pos.y += i;
	neighbors.push_back(pos);	//left indented
	
	pos.y -= i;
	neighbors.push_back(pos);	//left
	pos.y += i;
	pos.x += 2;
	neighbors.push_back(pos);	//right indented
	
	return neighbors;
}

// resets all of the empty cells to unhinted and reapplies all hints to moveable places
void Coinboard::apply_hint() {
    std::vector<sf::Vector2i> hinted = get_board_used_by_id(7);
    for (int i = 0; i < hinted.size(); i++)
        set_board(hinted[i], 3);
    
    std::vector<sf::Vector2i> avail = find_avail_moves();
    for (int i = 0; i < avail.size(); i++)
        set_board(avail[i], 7);
    
}

// given two adjecent cells, returns the position of the next cell in the line
sf::Vector2i Coinboard::get_next_cell(sf::Vector2i orig, sf::Vector2i neighbor) {
    sf::Vector2i dif = neighbor - orig;
	if (dif.x == 0)
		return orig + 2*dif;
		
	sf::Vector2i next = neighbor;
	next.x += dif.x;
	if (dif.y != 0)
		return next;
	
	
	int i = 1;
	if (int(neighbor.x) % 2 == 0)
		i = -1;
	
	next.y += i;
	return next;
}

// takes the last move off the stack and sets it to an empty cell. resets the position
// to an empty cell. Does not apply hints as this function is used extensively by the
// minimax function and would slow the program substantially
void Coinboard::undo_move() {
    if (moves.size() > 2) {
        board[moves.back().x][moves.back().y] = 3;
        moves.pop_back();
    }

    if (!running) {
        running = true;
        std::vector<sf::Vector2i> cells = get_board_used_by_id(4);
        for (int i = 0; i < cells.size(); i++)
            board[cells[i].x][cells[i].y] = 0;
        cells = get_board_used_by_id(5);
        for (int i = 0; i < cells.size(); i++)
            board[cells[i].x][cells[i].y] = 1;

    }
}

// given the current board state, iterates through current available
// moves and returns the optimal move for a given player (id)
sf::Vector2i Coinboard::find_optimal_move(int id) {
    std::vector<sf::Vector2i> am = find_avail_moves();
    sf::Vector2i best_move;
    int best_score;

    if (id == 0)
        best_score = INF;
    else
        best_score = -INF;
    
    for (int i = 0; i < am.size(); i++) {
        board[am[i].x][am[i].y] = id;
        moves.push_back(am[i]);
        int move_score = minimax(am[i], -INF, INF, recursion_depth, id ^ 1);
        if (id == 0 && move_score < best_score) {
            best_move = am[i];
            best_score = move_score;
        }
        if (id == 1 && move_score > best_score) {
            best_move = am[i];
            best_score = move_score;
        }
        undo_move();
    }
    return best_move;
}

// checks the board to see if a given position is part of a winning line.
// called extensively by the minimax algorithm so needs a position to check as
// it is much faster than checking the entire board to see if the game is over
bool Coinboard::check_winning_cell(sf::Vector2i pos) {
    int id = board[pos.x][pos.y];
    std::vector<sf::Vector2i> n = get_neighbors(pos);

    for (int i = 0; i < 3; i++)
        if ((get_board(n[2*i]) == id) && (get_board(n[2*i + 1]) == id))
            return true;
    
    for (int i = 0; i < 6; i++) {
        if (get_board(n[i]) == id) {
            sf::Vector2i next = get_next_cell(pos, n[i]);
            if (get_board(next) == id) {
                return true;
            }
        }
    }
    return false;
}

// runs a depth first search of possible move pathways and applies scoring based on
// who won and how many moves the winning move is. uses alpha beta pruning
// to eliminate pathways that will not be travelled to save compute time.
int Coinboard::minimax(sf::Vector2i last_move, int alpha, int beta, int depth, int id) {
    if (check_winning_cell(last_move))
        return (depth + 1) * (2*board[last_move.x][last_move.y] - 1);
    
    if (depth == 0)
        return 0;
    
    std::vector<sf::Vector2i> m = find_avail_moves();

    if (id == 1) {
        int maxEval = -INF;
        for (int i = 0; i < m.size(); i++) {
            board[m[i].x][m[i].y] = id;
            moves.push_back(m[i]);
            int eval = minimax(m[i], alpha, beta, depth - 1, id ^ 1);
            maxEval = std::max(eval, maxEval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                undo_move();
                break;
            }
            undo_move();
        }
        return maxEval;
    } else {
        int minEval = INF;
        for (int i = 0; i < m.size(); i++) {
            board[m[i].x][m[i].y] = id;
            moves.push_back(m[i]);
            int eval = minimax(m[i], alpha, beta, depth - 1, id ^ 1);
            minEval = std::min(eval, minEval);
            beta = std::min(beta, eval);
            if (beta <= alpha) {
                undo_move();
                break;
            }
            undo_move();
        }
        return minEval;
    }
}


// retrieves a list of the neighbors of a position and returns true
// if at least 2 of the neighbors are previous moves
bool Coinboard::check_neighbors(sf::Vector2i pos) {
    int c = 0;
    std::vector<sf::Vector2i> neighbors = get_neighbors(pos);
    for (int d = 0; d < 6; d++)
        if (get_board(neighbors[d]) < 3)
            c++;
            if (c >= 2) return true;
    return false;
}

// given a winning cell position, returns a vector of the other two cell positions
// in the winning line
std::vector<sf::Vector2i> Coinboard::get_winning_cells(sf::Vector2i pos) {
    std::vector<sf::Vector2i> cells;

    int id = board[pos.x][pos.y];
    std::vector<sf::Vector2i> n = get_neighbors(pos);

    for (int i = 0; i < 3; i++)
        if ((get_board(n[2*i]) == id) && (get_board(n[2*i + 1]) == id)) {
            cells.push_back(pos);
            cells.push_back(n[2*i]);
            cells.push_back(n[2*i + 1]);
        }
    
    for (int i = 0; i < 6; i++) {
        if (get_board(n[i]) == id) {
            sf::Vector2i next = get_next_cell(pos, n[i]);
            if (get_board(next) == id) {
                cells.push_back(pos);
                cells.push_back(n[i]);
                cells.push_back(next);
            }
        }
    }
    return cells;

}

// iterates through every cell in the board and adds placeable cells to a vector
std::vector<sf::Vector2i> Coinboard::find_avail_moves() {
    std::vector<sf::Vector2i> avail_moves;
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 7; j++) {
            int board_id = board[i][j];
            if (board_id == 3 || board_id == 7) {
                sf::Vector2i pos = sf::Vector2i(i,j);
                if(check_neighbors(pos))
                    avail_moves.push_back(pos);
            }
        }
    }
    return avail_moves;
}

void Coinboard::set_board(sf::Vector2i pos, int val) {
    board[pos.x][pos.y] = val;
}

// returns the id of a given position. if the position is
// outside of the bounds of the board returns 6 (id of empty cell)
inline int Coinboard::get_board(sf::Vector2i pos) {
	if (pos.x < 0 || pos.x >= 12 || pos.y < 0 || pos.y >= 7)
		return 6;
	return board[pos.x][pos.y];
}

void Coinboard::set_depth(int depth) {
    recursion_depth = depth;
}

void Coinboard::set_players(int h, int b) {
    human = h;
    bot = b;
}

bool Coinboard::game_running() {
    return running;
}

sf::Vector2i Coinboard::get_last_move() {
    return moves.back();
}

// initializer
Coinboard::Coinboard() {
    human = 0;
    bot = 1;
    recursion_depth = 6;

    running = true;

    moves.push_back(sf::Vector2i(5,3));
    moves.push_back(sf::Vector2i(6,3));
}