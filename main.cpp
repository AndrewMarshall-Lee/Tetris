#ifndef UNICODE
#define UNICODE
#endif


#include <iostream>
#include <thread>
#include <vector>
using namespace std;

#include <stdio.h>
#include <Windows.h>
#include <winuser.h>



//tetris shapes
wstring shapes[7];

//map size
int nFieldWidth = 12;
int nFieldHeight = 18;

//empty array to store all map data
unsigned char *pField = nullptr;

//screen size
int nScreenWidth = 80;
int nScreenHeight = 30;


int Rotate(int px, int py, int r)
{  // For rotation, on 4x4 grid
   // 0: i = y*w + x
   // 90: i = 12 + y -(x*4)
   // 180: i = 15 - (y*4)
   // 270: i = 3 + y + (x*4)
    switch (r%4)
    {
        case 0: return py * 4 + px;
        case 1: return 12 + py - (px * 4);
        case 2: return 15 - (py * 4) - px;
        case 3: return 3 - py + (px * 4);
    }
    return 0;
}

bool DoesItFit(int nshape, int nrot, int posx, int posy)
{
 for(int px =0; px<4;px++)
        for(int py =0; py < 4; py++)
        {
            //get piece index
            int pi = Rotate(px, py, nrot);

            // get index into field
            int fi = (posy + py)* nFieldWidth + (posx + px);

            if (posx +px >= 0 && posx + px < nFieldWidth){
                    if (posx +py >= 0 && posy + py < nFieldHeight)
                    {
                        if (shapes[nshape][pi] == L'X' && pField[fi] != 0)
                        {
                            return false; //stops out of bounds
                        }

                    }
            }
        }


 return true;
}
int main()
{
   //create shapes
    shapes[0].append(L"..X.");
    shapes[0].append(L"..X.");
    shapes[0].append(L"..X.");
    shapes[0].append(L"..X.");

    shapes[1].append(L"..X.");
    shapes[1].append(L".XX.");
    shapes[1].append(L".X..");
    shapes[1].append(L"....");

    shapes[2].append(L".X..");
    shapes[2].append(L".XX.");
    shapes[2].append(L"..X.");
    shapes[2].append(L"....");

    shapes[3].append(L"....");
    shapes[3].append(L".XX.");
    shapes[3].append(L".XX.");
    shapes[3].append(L"....");

    shapes[4].append(L"..X.");
    shapes[4].append(L".XX.");
    shapes[4].append(L"..X.");
    shapes[4].append(L"....");

    shapes[5].append(L"....");
    shapes[5].append(L".XX.");
    shapes[5].append(L"..X.");
    shapes[5].append(L"..X.");

    shapes[6].append(L"....");
    shapes[6].append(L".XX.");
    shapes[6].append(L".X..");
    shapes[6].append(L".X..");




    //create buffer screen
    wchar_t *screen = new wchar_t [nScreenHeight*nScreenWidth];
    for(int i = 0; i < nScreenWidth*nScreenHeight; i++) screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

    //create background (boundary and nothing else) screen
    pField = new unsigned char[nFieldWidth*nFieldHeight];
    for (int x = 0; x < nFieldWidth; x++){ // draw boundary
        for (int y = 0; y < nFieldHeight; y++){
            pField[y*nFieldWidth + x] = (x == 0 || x == nFieldWidth - 1 || y == nFieldHeight -1) ? 9 : 0;
        }
    }

	//game loop

	bool GameOver = false;
    bool GameWin = false;


	//test
	int nCurrentPiece = 1;

	int nCurrentRotation = 0;

	int nCurrentX = nFieldWidth/2;

	int nCurrentY = 0;

    //zero is default state, nothing changes
    bool bKey[4];
    bool bRotateHold = false;

    int nSpeed = 20;
    int nSpeedCounter = 0;
    bool bForceDown = false;
    int nPiece = 0;
    int Level = 1;
    int maxLevel = 1;
    int nScore = 0;
    vector<int> vLines;


	while(!GameOver)
       {
           //Game timing
        this_thread::sleep_for(50ms);
        nSpeedCounter++;

        bForceDown = (nSpeedCounter == nSpeed);



           //Input
        for (int k = 0; k < 4; k++)								// R   L   D Z
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

           //Game Logic
        if (bKey[1]){
            if (DoesItFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)){
                nCurrentX = nCurrentX - 1;
            }
        }
        if (bKey[0]){
            if (DoesItFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)){
                nCurrentX = nCurrentX + 1;
            }
        }

        if (bKey[2]){
            if (DoesItFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY+1)){
                nCurrentY = nCurrentY + 1;
            }
        }


        if (bKey[3]){
            nCurrentRotation += (!bRotateHold && DoesItFit(nCurrentPiece, nCurrentRotation+1, nCurrentX, nCurrentY)) ? 1 : 0;
            bRotateHold = true;
        }
        else{
            bRotateHold = false;
        }


        if (bForceDown){
            if (DoesItFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY+1)){
                nCurrentY = nCurrentY + 1;
            }
            else{
                //lock current piece
                for(int px =0; px<4;px++){
                    for(int py =0; py < 4; py++){
                        if (shapes[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X'){
                            pField[(nCurrentY + py) *nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
                        }
                    }
                }
                nPiece++;
                if(nPiece % 10 == 0){
                    if (nSpeed >= 10){
                            nSpeed--;
                            Level++;
                    }
                }

                //check have line
                for(int py =0; py<4;py++){
                    if(nCurrentY + py < nFieldHeight-1){
                        bool bLine = true;
                        for(int px = 1; px < nFieldWidth;px++){
                            bLine &= pField[(nCurrentY + py)*nFieldWidth+px] !=0;
                        }
                        if (bLine){
                            for (int px = 1; px < nFieldWidth - 1; px++){
                                pField[(nCurrentY + py) * nFieldWidth + px] = 8;
                            }

                            //Check what on Earth vectors function is
                            vLines.push_back(nCurrentY+py);
                        }
                    }
                }
                nScore += 25;
                if (!vLines.empty()) nScore += (1 << vLines.size())* 100 * Level;

                //choose next piece
                nCurrentPiece = rand()%7;
                nCurrentRotation = 0;
                nCurrentX = nFieldWidth/2;
                nCurrentY = 0;


                //if piece doesn't fit
                GameOver = !DoesItFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
            }
            nSpeedCounter = 0;
        }

           //Render Output
           //Draw frame
        for (int x = 0; x < nFieldWidth; x++){
            for (int y = 0; y < nFieldHeight; y++){
            //each tile is drawn based on value in array and a set of characters, e.g. 0 = " ", 1 = "A"
                screen[(y + 2)*nScreenWidth + (x + 2)]= L" ABCDEFG=#"[pField[y*nFieldWidth + x]];
            }
        }

        //Draw piece
        for(int px = 0; px < 4; px++){
            for(int py = 0; py < 4; py++){
                if (shapes[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X'){

                    screen[(nCurrentY+ py + 2)*nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;
                }
            }
        }

        //Display score and level
        swprintf_s(&screen[1* nScreenWidth+ nFieldWidth + 6], 16, L"Goal: Level %d", maxLevel);
        swprintf_s(&screen[2* nScreenWidth+ nFieldWidth + 6], 16, L"Level: %d", Level);
        swprintf_s(&screen[3* nScreenWidth+ nFieldWidth + 6], 16, L"Score: %8d", nScore);

        //Move all pieces down on line creation
        if (!vLines.empty()){
            WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth*nScreenHeight, { 0,0 }, &dwBytesWritten);
            this_thread::sleep_for(400ms);

            for (auto &v : vLines){
                for (int px = 1; px < nFieldWidth - 1; px++){
                    for (int py = v; py > 0; py--){
                        pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
                    }
                    pField[px] = 0;
                }
            }
            vLines.clear();
        }

        //Check if game is won
        if (Level > maxLevel){
            GameWin = true;
            GameOver = true;
        }


        //Display Frame
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth*nScreenHeight, { 0,0 }, &dwBytesWritten);
       }

    //Game over

    CloseHandle(hConsole);
    if (GameWin){
        cout << "Congratulations, you win! Your final score was: " << nScore << endl;
    }
    else{
        cout << "Game over :(. Your final score was: " << nScore << endl;
    }


    system("pause");


    return 0;
}
