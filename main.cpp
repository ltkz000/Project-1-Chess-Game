#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <SFML/Network.hpp>
#include<iostream>
#include<sstream>
#include<math.h>
#include<stack>
#include<vector>
#include<algorithm>// min, max

using namespace sf;
using namespace std;

const int size = 56;
Vector2f offset(28, 28);
int castling = 0;
int point = 0;

int board[8][8] = 
{-1,-2,-3,-4,-5,-3,-2,-1,
-6,-6,-6,-6,-6,-6,-6,-6,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0,
6, 6, 6, 6, 6, 6, 6, 6,
1, 2, 3, 4, 5, 3, 2, 1};

struct Chess{
    Sprite s;
    int index, cost;
};

class GameManager
{
public:
    //Chess array
    Chess chessArray[33];

    //Positive move
    Vector2f positiveMove[32];
    int positiveCount;

    //Moved stack
    stack<Vector2f> posS;
    //Moved che ss index
    stack<int> nS;

    //Game play
    void Play();
    void initVariables();
    void Undo();
    void move(int n, Vector2f oldPos, Vector2f newPos);

    //AI
    void positiveXe(int n, int x, int y, int grid[9][9]);
    void positiveTuong(int n, int x, int y, int grid[9][9]);
    void positiveMa(int n, int x, int y, int grid[9][9]);
    void positiveVua(int n, int x, int y, int grid[9][9]);
    void positiveTot(int n, int x, int y, int grid[9][9]);

    void increasePositive(int i, int j);
    void positiveMoving(int n);

    int costMove();
    
    int minimax(int depth, bool luot);
    int alpha_beta(int depth, bool luot, int alpha, int beta);
    Vector2f getNextMove(bool luot);
};

void GameManager::initVariables()
{
    positiveCount = 0;
    int k = 0;
    for(int i = 0; i < 8; ++i)
    {
        //Textures and position
        for(int j = 0; j < 8; ++j)
        {
            int n = board[i][j];
            if(!n) continue;
            int x = abs(n) - 1;
            int y = n > 0 ? 1 : 0;
            chessArray[k].index = n;
            chessArray[k].s.setTextureRect(IntRect(x*size, y*size, size, size));
            chessArray[k].s.setPosition(size*j + offset.x, size*i + offset.y);
    
            //Quantitative 
            int v = 0;
            int g;
            g = abs(chessArray[k].index);
            if(g == 1) v = 50;
            else if(g == 2 || g == 3) v = 30;
            else if(g == 4) v = 90;
            else if(g == 5) v = 900;
            else if(g == 6) v = 10;
            chessArray[k].cost = chessArray[k].index/g*v;
            k++;
        }
    }
}

void GameManager::increasePositive(int i, int j)
{
    positiveMove[positiveCount] = Vector2f(i*size, j*size) + offset;
    ++positiveCount;
}

void GameManager::positiveTot(int n, int x, int y, int grid[9][9])
{
    int k = grid[x][y]/abs(grid[x][y]); //k = -1 || k = 1

    //First move
    if((y==1 || y==6) && grid[x][y-k] == 0 && grid[x][y - 2*k] == 0 && y - 2*k >= 0 && y - 2*k < 8) increasePositive(x, y - 2*k);
    
    //Normal move
    if(grid[x][y-k] == 0 && (y-k) >= 0 && (y-k) < 8) increasePositive(x, y-k);

    //Cross-eating
    if(grid[x+1][y-k]*grid[x][y] < 0 && y-k >= 0 && y-k < 8 && x+1 < 8) increasePositive(x+1, y-k);
    if(grid[x-1][y-k]*grid[x][y] < 0 && y-k >= 0 && y-k < 8 && x-1 >= 0) increasePositive(x-1, y-k);
}

void GameManager::positiveVua(int n, int x, int y, int grid[9][9])
{
    //Move and eat on both sides
    if((grid[x+1][y] == 0 || grid[x+1][y] * grid[x][y] < 0) && x+1 < 8) increasePositive(x+1, y);
    if((grid[x-1][y] == 0 || grid[x-1][y] * grid[x][y] < 0) && x-1 >= 0) increasePositive(x-1, y);

    //Cross forward
    if((grid[x+1][y+1] == 0 || grid[x+1][y+1] * grid[x][y] < 0) && x+1 < 8 && y+1 < 8) increasePositive(x+1, y+1);
    if((grid[x-1][y+1] == 0 || grid[x-1][y+1] * grid[x][y] < 0) && x-1 >= 0 && y+1 < 8) increasePositive(x-1, y+1);

    //Cross backward
    if((grid[x-1][y-1] == 0 || grid[x-1][y-1] * grid[x][y] < 0) && x-1 >= 0 && y-1 >= 0) increasePositive(x-1, y-1);
    if((grid[x+1][y-1] == 0 || grid[x+1][y-1] * grid[x][y] < 0) && x+1 < 8 && y-1 >= 0) increasePositive(x+1, y-1);

    //Forward and backward
    if((grid[x][y+1] == 0 || grid[x][y+1] * grid[x][y] < 0) && y+1 < 8) increasePositive(x, y+1);
    if((grid[x][y-1] == 0 || grid[x][y-1] * grid[x][y] < 0) && y-1 >= 0) increasePositive(x, y-1);

    // //Castling
    // if(x == 4 && y == 0 && grid[x+1][y] == 0 && grid[x+2][y] == 0 && grid[7][y] == -1){
    //     increasePositive(x+2, y);
    //     ++castling;
    // }
    // if(x == 3 && y == 7 && grid[x-1][y] == 0 && grid[x-2][y] == 0 && grid[0][y] == 1){
    //     increasePositive(x-2, y);
    //     ++castling;
    // }
}

void GameManager::positiveMa(int n, int x, int y, int grid[9][9])
{
    if((grid[x+2][y+1] == 0 || grid[x+2][y+1] * grid[x][y] < 0) && x+2 < 8 && y+1 < 8) increasePositive(x+2, y+1);
    if((grid[x+2][y-1] == 0 || grid[x+2][y-1] * grid[x][y] < 0) && x+2 < 8 && y-1 >= 0) increasePositive(x+2, y-1);
    
    if((grid[x-2][y+1] == 0 || grid[x-2][y+1] * grid[x][y] < 0) && x-2 >= 0 && y+1 < 8) increasePositive(x-2, y+1);
    if((grid[x-2][y-1] == 0 || grid[x-2][y-1] * grid[x][y] < 0) && x-2 >= 0 && y-1 >= 0) increasePositive(x-2, y-1);
    
    if((grid[x+1][y+2] == 0 || grid[x+1][y+2] * grid[x][y] < 0) && x+1 < 8 && y+2 < 8) increasePositive(x+1, y+2);
    if((grid[x+1][y-2] == 0 || grid[x+1][y-2] * grid[x][y] < 0) && x+1 < 8 && y-2 >= 0) increasePositive(x+1, y-2);

    if((grid[x-1][y+2] == 0 || grid[x-1][y+2] * grid[x][y] < 0) && x-1 >= 0 && y+2 < 8) increasePositive(x-1, y+2);
    if((grid[x-1][y-2] == 0 || grid[x-1][y-2] * grid[x][y] < 0) && x-1 >= 0 && y-2 >= 0) increasePositive(x-1, y-2);
}

void GameManager::positiveTuong(int n, int x, int y, int grid[9][9])
{
    for(int i=x+1, j=y+1; (i<8 && j<8); ++i, ++j)
    {
        if(grid[i][j] != 0){
            if(grid[i][j] * grid[x][y] < 0) increasePositive(i, j);
            break;
        }
        increasePositive(i,j);
    }

    for(int i=x+1, j=y-1; (i<8 && j>=0); ++i, --j)
    {
        if(grid[i][j] != 0){
            if(grid[i][j] * grid[x][y] < 0) increasePositive(i, j);
            break;
        }
        increasePositive(i,j);
    }

    for(int i=x-1, j=y+1; (i>=0 && j<8); --i, ++j)
    {
        if(grid[i][j] != 0){
            if(grid[i][j] * grid[x][y] < 0) increasePositive(i, j);
            break;
        }
        increasePositive(i,j);
    }

    for(int i=x-1, j=y-1; (i>=0 && j>=0); --i, --j)
    {
        if(grid[i][j] != 0){
            if(grid[i][j] * grid[x][y] < 0) increasePositive(i, j);
            break;
        }
        increasePositive(i,j);
    }
}

void GameManager::positiveXe(int n, int x, int y, int grid[9][9])
{
    //Both sides
    for(int i=x+1; i<8; ++i)
    {
        if(grid[i][y] != 0){
            if(grid[i][y] * grid[x][y] < 0) increasePositive(i, y);
            break;
        }
        increasePositive(i, y);
    }

    for(int i=x-1; i>=0; --i)
    {
        if(grid[i][y] != 0){
            if(grid[i][y] * grid[x][y] < 0) increasePositive(i, y);
            break;
        }
        increasePositive(i, y);
    }

    //For and Backward
    for(int j=y+1; j<8; ++j)
    {
        if(grid[x][j] != 0){
            if(grid[x][j] * grid[x][y] < 0) increasePositive(x, j);
            break;
        }
        increasePositive(x, j);
    }

    for(int j=y-1; j>=0; --j)
    {
        if(grid[x][j] != 0){
            if(grid[x][j] * grid[x][y] < 0) increasePositive(x, j);
            break;
        }
        increasePositive(x, j);
    }
}

void GameManager::move(int n, Vector2f oldPos, Vector2f newPos)
{
    posS.push(oldPos);
    posS.push(newPos);
    nS.push(n);

    // y = pawn positon
    int y = int((newPos - offset).y/size);

    //Check pawn -> queen
    if(y == 0 && chessArray[n].index == 6)
    {
        nS.push(100);//Undo()
        chessArray[n].index = 4;
        chessArray[n].cost = 90;
        chessArray[n].s.setTextureRect(IntRect(3*size, size, size, size));
    }
    if(y == 7 && chessArray[n].index == -6)
    {
        nS.push(-100);
        chessArray[n].index = -4;
        chessArray[n].cost = -90;
        chessArray[n].s.setTextureRect(IntRect(3*size, 0, size, size));
    }

    //Castling
    // int kx, ky;
    // kx = int((newPos- offset).x/size);

    // if(castling != 0 && chessArray[n].index == 5){
    //     castling = 0;
    //     move(1, Vector2f(((kx-1)*size + offset.x), newPos.y), Vector2f(((kx+1)*size + offset.x), newPos.y));
    // }

    // if(castling != 0 && chessArray[n].index == -5){
    //     castling = 0;
    //     move(-1, Vector2f(((kx+1)*size + offset.x), newPos.y), Vector2f(((kx-1)*size + offset.x), newPos.y));
    // }

    //Move
    for(int i = 0; i < 32; ++i)
    {
        if(chessArray[i].s.getPosition() == newPos)
        {
            //Remove captured chess
            chessArray[i].s.setPosition(-100, -100);
            posS.push(newPos);
            posS.push(Vector2f(-100, -100));
            nS.push(i);
            break;
        }
    }
    chessArray[n].s.setPosition(newPos);
}

void GameManager::Undo()
{
    if(!nS.empty()){
        int n = nS.top();
        nS.pop();
        Vector2f p = posS.top();
        posS.pop();
        Vector2f q = posS.top();
        posS.pop();

        //Undo pawn -> queen
        if(n == 100){
            n = nS.top();
            nS.pop();
            chessArray[n].index = 6;
            chessArray[n].cost = 10;
            chessArray[n].s.setTextureRect(IntRect(5*size, size, size, size));
        }
        if(n == -100){
            n = nS.top();
            nS.pop();
            chessArray[n].index = -6;
            chessArray[n].cost = -10;
            chessArray[n].s.setTextureRect(IntRect(5*size, 0, size, size));
        }
        chessArray[n].s.setPosition(q);

        if(p == Vector2f(-100, -100)) Undo();
    }else return;
}

int GameManager::costMove()
{
    int s = 0;
    for(int i = 0; i < 32; ++i)
    {
        if(chessArray[i].s.getPosition() == Vector2f(-100, -100)) continue;
        s += chessArray[i].cost;
    }
    return s;
}

void GameManager::positiveMoving(int n)
{
    Vector2f pos = chessArray[n].s.getPosition() - offset; 
    int x = pos.x/size;
    int y = pos.y/size;

    int grid[9][9];
    Vector2i vitri;

    //Init index
    for(int i = 0; i < 8; ++i)
    {
        for(int j = 0; j < 8; ++j)
        {
            grid[i][j] = 0;
        }
    }

    for(int j = 0; j < 32; ++j)
    {
        vitri = Vector2i(chessArray[j].s.getPosition() - offset);
        grid[vitri.x/size][vitri.y/size] = chessArray[j].index;
    }

    //Positive move
    if(abs(chessArray[n].index) == 1) positiveXe(n, x, y, grid);//Castle
    else if(abs(chessArray[n].index) == 2) positiveMa(n, x, y, grid);//Knight
    else if(abs(chessArray[n].index) == 3) positiveTuong(n, x, y, grid);//Bishop
    else if(abs(chessArray[n].index) == 4){
        positiveXe(n, x, y, grid);
        positiveTuong(n, x, y, grid);
    }//Queen
    else if(abs(chessArray[n].index) == 5) positiveVua(n, x, y, grid);//King
    else positiveTot(n, x, y, grid); //Pawn
}

int GameManager::alpha_beta(int depth, bool luot, int alpha, int beta)
{
    if(depth == 0){
        return costMove();
    }

    Vector2f positiveMovetemp[32];

    if(luot == true){
        int bestMove = -10000;

        //Player
        for(int j = 16; j < 32; ++j)
        {
            if(chessArray[j].s.getPosition() == Vector2f(-100, -100)) continue;

            positiveMoving(j);
            int coun = positiveCount;
            positiveCount = 0;
            for(int i = 0; i < coun; ++i) positiveMovetemp[i] = positiveMove[i];
            for(int i = 0; i < coun; ++i)
            {
                move(j, chessArray[j].s.getPosition(), positiveMovetemp[i]);
                bestMove = max(bestMove, alpha_beta(depth-1, !luot, alpha, beta));
                //Undo
                Undo();
                alpha = max(alpha, bestMove);
                if(beta <= alpha) return bestMove;
            }
        }
        return bestMove;
    }
    else{
        int bestMove = 10000;

        //Computer
        for(int j = 0; j < 16; ++j)
        {
            if(chessArray[j].s.getPosition() == Vector2f(-100, -100)) continue;

            positiveMoving(j);
            int coun = positiveCount;
            positiveCount = 0;
            for(int i = 0; i < coun; ++i) positiveMovetemp[i] = positiveMove[i];
            for(int i = 0; i < coun; ++i)
            {
                move(j, chessArray[j].s.getPosition(), positiveMovetemp[i]);
                bestMove = min(bestMove, alpha_beta(depth-1, !luot, alpha, beta));
                //Undo
                Undo();
                beta = min(beta, bestMove);
                if(beta <= alpha) return bestMove;
            }
        }
        return bestMove;
    }
}

//Computer alpha_beta
Vector2f GameManager::getNextMove(bool luot)
{
    Vector2f oldPos, newPos, oldPostemp, newPostemp;
    int minimaxtemp = 10000, minimax = 10000;
    int count1, n;
    Vector2f positiveMovetemp[32];

    for(int i = 0; i < 16; ++i)
    {
        //Skip eaten chess
        if(chessArray[i].s.getPosition() == Vector2f(-100, -100)) continue;

        //-------------//
        positiveMoving(i);
        count1 = positiveCount;
        positiveCount = 0;

        //Set temp variables
        for(int k = 0; k < count1; ++k) positiveMovetemp[k] = positiveMove[k];
        
        oldPostemp = chessArray[i].s.getPosition();

        for(int j = 0; j < count1; ++j)
        {
            move(i, oldPostemp, positiveMovetemp[j]);
            int alpha = -10000, beta = 10000;
            int temp = alpha_beta(4, !luot, alpha, beta);
            if(minimaxtemp > temp){
                newPostemp = positiveMovetemp[j];
                minimaxtemp = temp;
            }
            Undo();
        }
        if(minimax > minimaxtemp){
            minimax = minimaxtemp;
            oldPos = oldPostemp;
            newPos = newPostemp;
            n = i;
        }
    }

    //Picked move
    posS.push(oldPos);
    nS.push(n);
    return newPos;
}

// int GameManager::minimax(int depth, bool luot)
// {
//     if(depth == 0){
//         return costMove();
//     }

//     Vector2f positiveMovetemp[32];

//     if(luot == true){
//         int bestMove = -10000;

//         //Player
//         for(int j = 16; j < 32; ++j)
//         {
//             if(chessArray[j].s.getPosition() == Vector2f(-100, - 100)) continue;

//             positiveMoving(j);
//             int coun = positiveCount;
//             positiveCount = 0;
//             for(int i = 0; i < coun; ++i) positiveMovetemp[i] = positiveMove[i];
//             for(int i = 0; i < coun; ++i)
//             {
//                 move(j, chessArray[j].s.getPosition(), positiveMovetemp[i]);
//                 bestMove = max(bestMove, minimax(depth - 1, !luot));
//                 //Undo
//                 Undo();
//             }
//         }
//         return bestMove;
//     }else{
//         int bestMove = 10000;

//         //Computer
//         for(int j = 0; j < 16; ++j)
//         {
//             if(chessArray[j].s.getPosition() == Vector2f(-100, -100)) continue;

//             positiveMoving(j);
//             int coun = positiveCount;
//             positiveCount = 0;
//             for(int i = 0; i < coun; ++i) positiveMovetemp[i] = positiveMove[i];
//             for(int i = 0; i < coun; ++i)
//             {
//                 move(j, chessArray[j].s.getPosition(), positiveMovetemp[i]);
//                 bestMove = min(bestMove, minimax(depth - 1,!luot));
//                 //Undo
//                 Undo();
//             }
//         }
//         return bestMove;
//     }
// }

// //Computer minimax
// Vector2f GameManager::getNextMove(bool luot)
// {
//     Vector2f oldPos, newPos, oldPostemp, newPostemp;
//     int minimaxtemp = 10000, minimax1 = 10000;
//     int coun, n;
//     Vector2f positiveMovetemp[32];

//     for(int i = 0; i < 16; ++i)
//     {
//         //Skip eaten chess
//         if(chessArray[i].s.getPosition() == Vector2f(-100, -100)) continue;

//         //-------------//
//         positiveMoving(i);
//         coun = positiveCount;
//         positiveCount = 0;

//         //Set temp variables
//         for(int k = 0; k < coun; ++k) positiveMovetemp[k] = positiveMove[k];
        
//         oldPostemp = chessArray[i].s.getPosition();

//         for(int j = 0; j < coun; ++j)
//         {
//             move(i, oldPostemp, positiveMovetemp[j]);
//             int temp = minimax(4, !luot);
//             if(minimaxtemp > temp){
//                 newPostemp = positiveMovetemp[j];
//                 minimaxtemp = temp;
//             }
//             Undo();
//         }
//         if(minimax1 > minimaxtemp){
//             minimax1 = minimaxtemp;
//             oldPos = oldPostemp;
//             newPos = newPostemp;
//             n = i;
//         }
//     }

//     //Picked move
//     posS.push(oldPos);
//     nS.push(n);
//     return newPos;
// }

//------------------------------------------------------------//

void GameManager::Play()
{
    RenderWindow window(VideoMode(800, 504), "Chess Game!");

    Vector2f mousePosView;

    //Board and chess textures
    Sprite board, posmove;
    Texture chessTex, boardTex, posmoveTex;
    chessTex.loadFromFile("Textures/figures1.png");
    boardTex.loadFromFile("Textures/chessboard1.png");
    posmoveTex.loadFromFile("Textures/positive.png");

    //Init textures
    for(int i = 0; i < 32; ++i)
    {
        chessArray[i].s.setTexture(chessTex);
    }
    board.setTexture(boardTex);
    board.setPosition(offset);
    posmove.setTexture(posmoveTex);

    //Init endgame text
    Text endgameText;
    Font font;
    font.loadFromFile("Fonts/EvilEmpire-4BBVK.ttf");
    endgameText.setFont(font);
    endgameText.setCharacterSize(75);

    //Init guide text
    Text guideText;
    guideText.setPosition(525.f, 28.f);
    guideText.setFont(font);
    guideText.setCharacterSize(25);
    guideText.setFillColor(Color::White);
    guideText.setString("GUIDE:\nPress 'Escape' to Undo");

    //Init background;
    Sprite background;
    Texture backgroundTex;
    backgroundTex.loadFromFile("Textures/test.png");
    background.setTexture(backgroundTex);

    //Exit Button
    RectangleShape exitRec;
    exitRec.setPosition(665.f, 400.f);
    exitRec.setSize(Vector2f(100.f, 45.f));
    exitRec.setFillColor(Color(70, 70, 70, 200));

    Text exitText;
    exitText.setString("Exit");
    exitText.setFont(font);
    exitText.setCharacterSize(25);
    exitText.setFillColor(Color::White);
    exitText.setPosition(exitRec.getPosition().x + exitRec.getGlobalBounds().width / 2.f - exitText.getGlobalBounds().width / 2.f,
                            exitRec.getPosition().y + exitRec.getGlobalBounds().height / 2.f - exitText.getGlobalBounds().height + 2.f);
    
    //Undo Button
    RectangleShape undoRec;
    undoRec.setPosition(515.f, 400.f);
    undoRec.setSize(Vector2f(100.f, 45.f));
    undoRec.setFillColor(Color(70, 70, 70, 200));

    Text undoText;
    undoText.setString("Undo");
    undoText.setFont(font);
    undoText.setCharacterSize(25);
    undoText.setFillColor(Color::White);
    undoText.setPosition(undoRec.getPosition().x + undoRec.getGlobalBounds().width / 2.f - undoText.getGlobalBounds().width / 2.f,
                            undoRec.getPosition().y + undoRec.getGlobalBounds().height / 2.f - undoText.getGlobalBounds().height + 2.f);

    //Init()
    initVariables();

    //Game
    bool luotchoi = true;
    Vector2f oldPos, newPos;
    int n = 0, click = 0, count = 0;

    //Mouse click position
    Vector2i mousePos;

    while(window.isOpen())
    {
        Event e;
        while(window.pollEvent(e))
        {
            mousePosView = window.mapPixelToCoords(Mouse::getPosition(window));

            if(e.type == Event::Closed){
                window.close();
            }

            if(exitRec.getGlobalBounds().contains(mousePosView)){
                exitRec.setFillColor(Color(150, 150, 150, 255));
                if(Mouse::isButtonPressed(Mouse::Left)){
                    exitRec.setFillColor(Color(20, 20, 20, 200));
                    window.close();
                }
            }else{
                exitRec.setFillColor(Color(70, 70, 70, 200));
            } 

            //Undo
            if(e.type == Event::KeyPressed){
                if(e.key.code == Keyboard::Escape){
                    Undo();
                    Undo();
                }   
            }
        
            if(undoRec.getGlobalBounds().contains(mousePosView)){
                undoRec.setFillColor(Color(150, 150, 150, 255));
                if(Mouse::isButtonPressed(Mouse::Left)){
                    undoRec.setFillColor(Color(20, 20, 20, 200));
                    Undo();
                    Undo();
                }
            }else{
                undoRec.setFillColor(Color(70, 70, 70, 200));
            }   

            //Click
            if(e.type == Event::MouseButtonPressed){
                if(e.mouseButton.button == Mouse::Left){
                    mousePos = Mouse::getPosition(window) - Vector2i(offset);
                    click++;
                }
            }
        }

        if(luotchoi == true){//Human//
            if(click == 1){

                bool isMove = false;

                for(int i = 16; i < 32; ++i)
                {
                    if(chessArray[i].s.getGlobalBounds().contains(mousePos.x + offset.x, mousePos.y + offset.y)){
                        isMove = true;
                        n = i;
                        oldPos = chessArray[n].s.getPosition();
                        chessArray[n].s.setColor(Color::Yellow);
                    }
                }

                if(!isMove){
                    click = 0;
                }else{
                    positiveMoving(n);
                    count = positiveCount;
                    positiveCount = 0;
                }
            }
            if(click == 2){
                int x = mousePos.x/size;
                int y = mousePos.y/size;
                newPos = Vector2f(x*size, y*size) + offset;

                //move only in positiveMove
                for(int i = 0; i < count; ++i)
                {
                    if(positiveMove[i] == newPos){
                        move(n, oldPos, newPos);
                        luotchoi = !luotchoi;
                    }
                }

                //reset
                count = 0;
                click = 0;
            }
        }
        else{//Computer//
            newPos = getNextMove(luotchoi);
            int c = nS.top();
            nS.pop();
            oldPos = posS.top();
            posS.pop();
            move(c, oldPos, newPos);
            luotchoi = !luotchoi;
            
            //reset
            click = 0;
        }

        //Render//
        window.draw(background);

        window.draw(exitRec);

        window.draw(exitText);

        window.draw(undoRec);

        window.draw(undoText);

        window.draw(guideText);

        window.draw(board);

        for(int i = 0; i < count; ++i)
        {
            posmove.setPosition(positiveMove[i]);
            window.draw(posmove);
        }

        for(int i = 0; i < 32; ++i)
        {
            window.draw(chessArray[i].s);
        }

        if(costMove() - point == 900){
            endgameText.setFillColor(Color::Yellow);
            endgameText.setString("YOU WIN");
            endgameText.setPosition(504.f / 2.f - endgameText.getGlobalBounds().width / 2.f
                            , 460.f / 2.f - endgameText.getGlobalBounds().height / 2.f);
        }else if(costMove() - point == -900){
            endgameText.setFillColor(Color::Red);
            endgameText.setString("YOU LOSE");
            endgameText.setPosition(504.f / 2.f - endgameText.getGlobalBounds().width / 2.f
                            , 460.f / 2.f - endgameText.getGlobalBounds().height / 2.f);
        }

        point = costMove();

        //Init point text
        Text pointText;
        pointText.setPosition(525.f, 50.f + guideText.getGlobalBounds().height);
        pointText.setFont(font);
        pointText.setCharacterSize(25);
        pointText.setFillColor(Color::White);
        stringstream ss;
        ss << "Points: " << point;
        pointText.setString(ss.str());

        window.draw(endgameText);

        window.draw(pointText);

        window.display();
    }
}

int main()
{
    GameManager cGame;
    cGame.Play();
}

