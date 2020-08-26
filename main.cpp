#include <SFML/Graphics.hpp>
#include "coinboard.h"
#include "stdio.h"


#define SCREENWIDTH 1024
#define SCREENHEIGHT 600

// 12 x 7 grid
#define WIDTH 12
#define HEIGHT 7

#define CELLWIDTH 57
#define CELLHEIGHT 67

#define SINGLEPLAYER 1
#define PLAYER 0
#define DEPTH 6




/* ******************
Sprite layout:
0 :     blue
1 :     yellow
2 :     center
3 :     paceable
4 :     blue win
5 :     yellow win
6 :     empty
7 :     placeable hinted
********************* */

sf::Vector2i world_to_map(int x, int y) {
    int xoffset = (SCREENWIDTH - WIDTH * CELLWIDTH) / 2;
    int xpos = x - xoffset;
    int xindex = xpos / CELLWIDTH;

    int yoffset;
    if (xindex % 2 == 0) {
        yoffset = ((SCREENHEIGHT - HEIGHT * CELLHEIGHT) / 2) - CELLHEIGHT / 4;
    } else {
        yoffset = ((SCREENHEIGHT - HEIGHT * CELLHEIGHT) / 2) + CELLHEIGHT / 4;
    }

    int ypos = y - yoffset;
    int yindex = ypos / CELLHEIGHT;

    return sf::Vector2i(xindex, yindex);
}

void handle_input(int mouse_button, int x, int y, int player, Coinboard* board) {
    if (mouse_button == sf::Mouse::Button::Right) {
        board->do_move(world_to_map(x,y), UNDO);
    } else if (mouse_button == sf::Mouse::Button::Left && board->game_running()) {
        board->do_move(world_to_map(x,y), PLACE);
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(SCREENWIDTH, SCREENHEIGHT), "Coin Game");
    sf::Texture spritesheet;
    spritesheet.create(32*4, 32*2);
    spritesheet.loadFromFile("sprites/sheet.png");

    Coinboard* board = new Coinboard();

    bool updated = true;

    sf::IntRect sprites[8];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2; j++) {
            sprites[j*4 + i] = sf::IntRect(i*32, j*32, 32, 32);
        }
    }

    int xoffset = (SCREENWIDTH - WIDTH * CELLWIDTH) / 2;
    int yoffset = (SCREENHEIGHT - HEIGHT * CELLHEIGHT) / 2;

    sf::Sprite cells[WIDTH][HEIGHT];
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            board->set_board(sf::Vector2i(i,j), 3);
            cells[i][j].setTexture(spritesheet);
            cells[i][j].setTextureRect(sprites[3]);
            cells[i][j].setScale(2.5f,2.5f);
            cells[i][j].setPosition(xoffset + i*CELLWIDTH, yoffset + j*CELLHEIGHT);
            if (i % 2 == 0) {
                cells[i][j].move(0, -CELLHEIGHT/4);
            } else {
                cells[i][j].move(0, CELLHEIGHT/4);
            }
        }
    }

    cells[5][3].setTextureRect(sprites[2]);
    cells[6][3].setTextureRect(sprites[2]);
    board->set_board(sf::Vector2i(5,3), 2);
    board->set_board(sf::Vector2i(6,3), 2);
    board->do_move(sf::Vector2i(0,0), PLACE);   //hacky way of getting hints for first move
    if (SINGLEPLAYER)   board->set_players(PLAYER, PLAYER ^ 1);
    else                board->set_players(0, -1);
    board->set_depth(DEPTH);

    bool bot_turn = false;
    if (PLAYER == 1 && SINGLEPLAYER) {
        bot_turn = true;
    }
    
    int turn = 0;




    while (window.isOpen())
    {
        if (board->game_running())
        
        if (bot_turn && board->game_running()) {
            board->do_bot_move();
            turn ^= 1;
            bot_turn = false;
            updated = true;
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            
            if (event.type == sf::Event::MouseButtonPressed) {
                int button = event.mouseButton.button;
                handle_input(event.mouseButton.button, event.mouseButton.x, event.mouseButton.y, turn, board);

                if (SINGLEPLAYER) {
                    if (board->get_board(board->get_last_move()) == turn) { 
                        turn ^= 1;
                        bot_turn = true;
                    }
                } else {
                    if (board->get_board(board->get_last_move()) < 2)
                        turn = board->get_board(board->get_last_move()) ^ 1;
                    else
                        turn = 0;
                }
                updated = true;
            }
        }

        window.clear(sf::Color(0x32,0x3c,0x39,0xFF));   // funky green
        for (int i = 0; i < WIDTH; i++) {
            for (int j = 0; j < HEIGHT; j++) {
                if (updated) {
                    cells[i][j].setTextureRect(sprites[board->get_board(sf::Vector2i(i, j))]);  // wow this is a fucked line
                }
                window.draw(cells[i][j]);
            }
        }
        if (updated) updated = false;
        window.display();
    }

    return 0;
}