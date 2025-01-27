#include <SDL2/SDL.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <cmath>
#include <map>

using namespace std;

const int SCREEN_WIDTH = 800, SCREEN_HEIGHT = 600;

enum GameState
{
    STATE_HOMEPAGE,
    STATE_TWO_GAME,
    STATE_ONE_GAME,
    STATE_EXIT
};

// function for loading textures
SDL_Texture *loadImage(SDL_Renderer *renderer, const char *filePath)
{
    // initialize image
    SDL_Surface *surface = SDL_LoadBMP(filePath);
    if (!surface)
    {
        cerr << "Unable to load image: " << filePath << "! SDL_Error: " << SDL_GetError() << endl;
        return nullptr;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

class Button
{
private:
    SDL_Rect rect;
    SDL_Color outline;

public:
    Button(SDL_Rect imgRect, SDL_Color borderColor)
    {
        rect = imgRect;
        outline = borderColor;
    }
    bool isClicked(int mouseX, int mouseY)
    {
        SDL_Point p = {mouseX, mouseY};
        return SDL_PointInRect(&p, &rect);
    }
    void renderButton(SDL_Renderer *renderer)
    {
        // outline it
        SDL_SetRenderDrawColor(renderer, outline.r, outline.g, outline.b, outline.a);
        SDL_RenderDrawRect(renderer, &rect);
    }
};

class Player
{
private:
public:
    char player;
    char winner, win_type;
    int win_index;

    Player()
    {
        reset();
    }
    void reset()
    {
        player = 'o';
        winner = '#';
        win_type = '-';
        win_index = 0;
    }
    void switchPlayer()
    {
        player = (player == 'o') ? 'x' : 'o';
    }
    void setWinner()
    {
        winner = player;
    }
};

class ReferenceBoard
{
public:
    char board[3][3];

    ReferenceBoard()
    {
        reset('-');
    }
    void reset(char symbol)
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                board[i][j] = symbol;
            }
        }
    }
    bool isFull()
    {
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                if (board[i][j] == '-')
                    return false;
            }
        }
        return true;
    }
    bool fillCell(int row, int col, char player)
    {
        if (board[row][col] == '-')
        {
            board[row][col] = player;
            return true;
        }
        return false;
    }
    bool checkWin(Player &curr) const
    {
        // check rows and columns
        for (int i = 0; i < 3; ++i)
        {
            if (board[i][0] == curr.player && board[i][1] == curr.player && board[i][2] == curr.player)
            {
                curr.win_index = i;
                curr.win_type = 'h';
                return true;
            }
            if (board[0][i] == curr.player && board[1][i] == curr.player && board[2][i] == curr.player)
            {
                curr.win_index = i;
                curr.win_type = 'v';
                return true;
            }
        }
        // check diagonals
        if (board[0][0] == curr.player && board[1][1] == curr.player && board[2][2] == curr.player)
        {
            curr.win_type = 'm';
            return true;
        }
        if (board[0][2] == curr.player && board[1][1] == curr.player && board[2][0] == curr.player)
        {
            curr.win_type = 's';
            return true;
        }
        return false;
    }
};

class Board
{
private:
    SDL_Rect rect;
    int cellSize;

    void drawX(SDL_Renderer *renderer, int x1, int y1, int x2, int y2)
    {
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        SDL_RenderDrawLine(renderer, x2, y1, x1, y2);
    }
    void drawO(SDL_Renderer *renderer, int x, int y, int radius)
    {
        for (int angle = 0; angle < 360; angle++)
        {
            int dx = static_cast<int>(radius * cos(angle * M_PI / 180));
            int dy = static_cast<int>(radius * sin(angle * M_PI / 180));
            SDL_RenderDrawPoint(renderer, x + dx, y + dy);
        }
    }

public:
    Board(int x, int y, int w, int h)
    {
        rect = {x, y, w, h};
        cellSize = w / 3;
    }
    bool isClicked(int mouseX, int mouseY)
    {
        SDL_Point p = {mouseX, mouseY};
        return SDL_PointInRect(&p, &rect);
    }
    void findCell(int mouseX, int mouseY, int &row, int &col)
    {
        row = (mouseY - rect.y) / cellSize;
        col = (mouseX - rect.x) / cellSize;
    }
    void renderBoard(SDL_Renderer *renderer, char XOBoard[3][3], Player curr)
    {
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        // draw horizontal line
        for (int i = 1; i < 3; i++)
        {
            SDL_RenderDrawLine(renderer, rect.x, rect.y + i * cellSize, rect.x + rect.w, rect.y + i * cellSize);
        }
        // draw vertical lines
        for (int i = 1; i < 3; i++)
        {
            SDL_RenderDrawLine(renderer, rect.x + i * cellSize, rect.y, rect.x + i * cellSize, rect.y + rect.w);
        }

        // draw X and O
        for (int i = 0; i < 3; i++)
        {
            // Loop through rows
            for (int j = 0; j < 3; j++)
            {
                int x = rect.x + j * cellSize + cellSize / 2;
                int y = rect.y + i * cellSize + cellSize / 2;

                if (XOBoard[i][j] == 'x')
                {
                    drawX(renderer, x - cellSize / 4, y - cellSize / 4, x + cellSize / 4, y + cellSize / 4);
                }
                if (XOBoard[i][j] == 'o')
                {
                    drawO(renderer, x, y, cellSize / 4);
                }
            }
        }
        // check for win and find from where to where to draw win line
        if (curr.winner != '#')
        {
            if (curr.win_type == 'm') // main diagonal win line
            {
                SDL_RenderDrawLine(renderer, rect.x, rect.y, rect.x + rect.w, rect.y + rect.w);
            }
            else if (curr.win_type == 's') // secondary diagonal win line
            {
                SDL_RenderDrawLine(renderer, rect.x + rect.w, rect.y, rect.x, rect.y + rect.w);
            }
            else if (curr.win_type == 'h') // horizontal win line
            {
                SDL_RenderDrawLine(renderer, rect.x, rect.y + (curr.win_index + 0.5) * cellSize, rect.x + rect.w, rect.y + (curr.win_index + 0.5) * cellSize);
            }
            else if (curr.win_type == 'v') // verical win line
            {
                SDL_RenderDrawLine(renderer, rect.x + (curr.win_index + 0.5) * cellSize, rect.y, rect.x + (curr.win_index + 0.5) * cellSize, rect.y + rect.w);
            }
        }
    }
};

void cleanup(SDL_Window *window, SDL_Renderer *renderer, map<string, SDL_Texture *> &textures)
{
    for (auto &pair : textures)
    {
        if (pair.second)
        {
            SDL_DestroyTexture(pair.second);
        }
    }
    textures.clear();
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << endl;
        return -1;
    }
    // create window
    SDL_Window *window = SDL_CreateWindow("Cit Cat Coe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_Quit();
        return -1;
    }
    // create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
    // load the icon
    SDL_Surface *icon = SDL_LoadBMP("icon.bmp");
    if (icon)
    {
        SDL_SetWindowIcon(window, icon);
        SDL_FreeSurface(icon);
    }
    // initialize a hash map for textures
    map<string, SDL_Texture *> textures;
    // load title
    SDL_Rect title_Rect = {(SCREEN_WIDTH - 582) / 2, 90, 582, 96};
    textures["title"] = loadImage(renderer, "assets/title.bmp");

    // load cat_stand image
    SDL_Rect cat_stand_Rect = {600, 450, 140, 100};
    textures["cat_stand"] = loadImage(renderer, "assets/cat_stand.bmp");
    // load cat2 image
    SDL_Rect cat_sit_Rect = {600, 450, 110, 110};
    textures["cat_sit"] = loadImage(renderer, "assets/cat_sit.bmp");

    // load twoplayer background
    SDL_Rect twoPlayer_BG_Rect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 60, 200, 80};
    textures["twoPlayer_BG"] = loadImage(renderer, "assets/twoplayer.bmp");
    // load one player background
    SDL_Rect onePlayer_BG_Rect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 50, 200, 80};
    textures["onePlayer_BG_Rect"] = loadImage(renderer, "assets/twoplayer.bmp");

    // load back button background
    SDL_Rect backButton_BG_Rect = {20, 20, 40, 40};
    textures["backButton_BG"] = loadImage(renderer, "assets/back.bmp");

    // load again image
    SDL_Rect playAgain_Rect = {370, 490, 60, 60};
    textures["playAgain"] = loadImage(renderer, "assets/playAgain.bmp");

    // load cit image
    SDL_Rect cit_Rect = {40, 250, 180, 90};
    textures["cit"] = loadImage(renderer, "assets/cit.bmp");
    // load cit_turn image
    textures["cit_turn"] = loadImage(renderer, "assets/cit_turn.bmp");
    // load cit_win image
    textures["cit_win"] = loadImage(renderer, "assets/cit_win.bmp");

    // load coe image
    SDL_Rect coe_Rect = {570, 255, 180, 90};
    textures["coe"] = loadImage(renderer, "assets/coe.bmp");
    // load coe_turn image
    textures["coe_turn"] = loadImage(renderer, "assets/coe_turn.bmp");
    // load coe_win image
    textures["coe_win"] = loadImage(renderer, "assets/coe_win.bmp");

    // error check for all the immages turned into textures
    for (auto &pair : textures)
    {
        if (!pair.second)
        {
            cerr << "Error: Failed to load texture '" << pair.first << "'!" << endl;
            cleanup(window, renderer, textures);
            return -1;
        }
    }

    // initialize colors
    SDL_Color black = {0, 0, 0, 255};
    SDL_Color white = {255, 255, 255, 255};

    // initialize the buttons
    Button onePlayerButton(onePlayer_BG_Rect, white);
    Button twoPlayerButton(twoPlayer_BG_Rect, white);
    Button backButton(backButton_BG_Rect, black);
    Button playAgainButton(playAgain_Rect, black);
    //  initialize the 3x3 board
    Board mainBoard((SCREEN_WIDTH - 300) / 2, (SCREEN_HEIGHT - 300) / 2, 300, 300);
    // initialize the reference board
    ReferenceBoard refBoard;
    Player twoPlayer;

    GameState currentState = STATE_HOMEPAGE;
    bool quit = false;
    SDL_Event e;
    while (!quit)
    {
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX = e.button.x;
                int mouseY = e.button.y;

                if ((currentState == STATE_ONE_GAME || currentState == STATE_TWO_GAME))
                {
                    // reset reffrence board and player order if play again or back button button pressed
                    if (playAgainButton.isClicked(mouseX, mouseY) || backButton.isClicked(mouseX, mouseY))
                    {
                        refBoard.reset('-');
                        twoPlayer.reset();
                        // if back button pressed chang state to homepaage
                        if (backButton.isClicked(mouseX, mouseY))
                        {
                            currentState = STATE_HOMEPAGE;
                        }
                    }
                    if (mainBoard.isClicked(mouseX, mouseY) && twoPlayer.winner == '#')
                    {
                        int row, col;
                        mainBoard.findCell(mouseX, mouseY, row, col);
                        if (refBoard.fillCell(row, col, twoPlayer.player))
                        {
                            // check if there is a winner and finds the index and type of win
                            if (refBoard.checkWin(twoPlayer))
                            {
                                twoPlayer.setWinner();
                            }
                            twoPlayer.switchPlayer();
                        }
                    }
                }
                if (currentState == STATE_HOMEPAGE)
                {
                    if (onePlayerButton.isClicked(mouseX, mouseY))
                        currentState = STATE_ONE_GAME;
                    if (twoPlayerButton.isClicked(mouseX, mouseY))
                        currentState = STATE_TWO_GAME;
                }
            }
        }
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);

        if (currentState == STATE_HOMEPAGE)
        {
            // render title
            SDL_RenderCopy(renderer, textures["title"], nullptr, &title_Rect);
            // render cat stand img
            SDL_RenderCopy(renderer, textures["cat_stand"], nullptr, &cat_stand_Rect);
            // render two player bg
            SDL_RenderCopy(renderer, textures["twoPlayer_BG"], nullptr, &twoPlayer_BG_Rect);
            // render one player bg
            SDL_RenderCopy(renderer, textures["onePlayer_BG"], nullptr, &onePlayer_BG_Rect);
            // render game modes buttons
            onePlayerButton.renderButton(renderer);
            twoPlayerButton.renderButton(renderer);
        }
        else if (currentState == STATE_ONE_GAME)
        {
        }
        else if (currentState == STATE_TWO_GAME)
        {
            // render the 3x3 board
            mainBoard.renderBoard(renderer, refBoard.board, twoPlayer);
            // render back button bg
            SDL_RenderCopy(renderer, textures["backButton_BG"], nullptr, &backButton_BG_Rect);
            backButton.renderButton(renderer);
            // render cat sit img
            SDL_RenderCopy(renderer, textures["cat_sit"], nullptr, &cat_sit_Rect);

            if (refBoard.isFull())
            {
                // render the play again button if there is a draw
                SDL_RenderCopy(renderer, textures["playAgain"], nullptr, &playAgain_Rect);
                playAgainButton.renderButton(renderer);
                // render cit and coe
                SDL_RenderCopy(renderer, textures["cit"], nullptr, &cit_Rect);
                SDL_RenderCopy(renderer, textures["coe"], nullptr, &coe_Rect);
            }
            else
            {
                if (twoPlayer.winner == '#')
                {
                    // if there is no winner render the players depending on their turn
                    SDL_RenderCopy(renderer, textures[twoPlayer.player == 'o' ? "cit_turn" : "cit"], nullptr, &cit_Rect);
                    SDL_RenderCopy(renderer, textures[twoPlayer.player == 'o' ? "coe" : "coe_turn"], nullptr, &coe_Rect);
                }
                else
                {
                    // if there is a winner render the play again button and the win message
                    SDL_RenderCopy(renderer, textures["playAgain"], nullptr, &playAgain_Rect);
                    playAgainButton.renderButton(renderer);
                    // render cit or coe win message depending in winner
                    SDL_RenderCopy(renderer, textures[twoPlayer.winner == 'o' ? "cit_win" : "cit"], nullptr, &cit_Rect);
                    SDL_RenderCopy(renderer, textures[twoPlayer.winner == 'o' ? "coe" : "coe_win"], nullptr, &coe_Rect);
                }
            }
        }
        SDL_RenderPresent(renderer);
    }
    cleanup(window, renderer, textures);
    return 0;
}