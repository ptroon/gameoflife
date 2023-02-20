#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raylib.h"

using namespace std;
#include <iostream>
#include <cstdio>
#include <array>
#include <cstdlib>
#include <iomanip>

// Game of Life written in C++ and RayLib/RayGUI
// written by P.Troon 2023

int ddActive = 0;
bool ddEditMode = false;

int spinnerTicks=5;
bool spinnerEditMode = true;

char *btn_text = new char[256] {"RUN SIMULATION"};

const int win_width = 1000;
const int win_height = 1000;
const int offset = 80;

bool exitProgram = false;
const int gap = 20;

const int max_x = win_width / gap;                      // number of cells on X
const int max_y = (win_height / gap) - (offset / gap);  // number of cells on Y
int max_cell = max_x * max_y;                           // the maximum number of cells in the grid

int cell_arr[max_y+2][max_x+2] = {0};                   // the 2D array for holding all the cells - uses basic zero-based array plus extra cells on width & length that is used to prevent bounding errors

const int max_live = 50;                                // maximum number of cells that can have a live cell on initiation

bool RunningSimulation = false;                         // boolean to determine if the simulation is running or not.

// Timer used to control the speed of the evolution ticks.
typedef struct Timer {
    double startTime;
    double lifeTime;
} Timer;

void startTimer (Timer *timer, double lifetime) {

    timer->startTime = GetTime();
    timer->lifeTime = lifetime;
}

bool timerDone (Timer *timer) {
    return GetTime() - timer->startTime >= timer->lifeTime;
}

double getElapsed (Timer *timer) {
    return GetTime() - timer->startTime;
}

void updateTimer(Timer* timer)
{
    // subtract this frame from the timer if it's not allready expired
    if (timer != NULL && timer->lifeTime > 0)
        timer->lifeTime -= GetFrameTime();
}

float evolutionTicks = (float) spinnerTicks / 10;       // speed for the evolution to run at
Timer evolutionTimer = {0};                             // Timer for the evolution.


void initCellAssignment () {
//
// initialises the cells randomly for a pre-defined number of live cells and places them into the 2D array
//

    int temp_row = 0;
    int temp_col = 0;

    for (int i=1; i<max_live; i++) {                            

        int rcell = rand() % max_cell + 1;                      // get a random number between 1 and the specified max number of cells
        temp_row = (rcell / max_x);                             // This calculates the row on which the cell exists in the 2D array (starts at 1) e.g. 1-50, 51-100 etc.
        temp_col = rcell - (temp_row * max_x);                  // This gives the cell (col) location on the row already discovered.
        temp_row++; //temp_col++;                               //
        cell_arr[temp_row][temp_col] = 1;                       // apply the cell into the 2D row/col array.
    }

}

void injectTestCells() {
// TESTING: used to inject a known cell design into the array so I can test the evolution rules.

    cell_arr[20][20] = 1;
    cell_arr[20][21] = 1;
    cell_arr[20][22] = 1;
    cell_arr[19][21] = 1;
    cell_arr[21][21] = 1;    

}



void drawCells () {

    for (int r=0; r <= max_y; r++) {
        for (int c=0; c <= max_x; c++) {
            
            if (cell_arr[r][c] > 0) {
                DrawRectangle(c*gap, r*gap, gap, gap, GREEN);
            }
        }
    }
}


void drawUI () {

    GuiSetState(STATE_NORMAL);  
    if (GuiButton((Rectangle){ 10, 950, 200, 30 }, btn_text)) { 
    
        RunningSimulation = !RunningSimulation; 
        if (RunningSimulation) {
             startTimer(&evolutionTimer, evolutionTicks);
             btn_text = "PAUSE SIMULATION";
        } else {
            btn_text = "RUN SIMULATION";
        }
    }
    
    GuiSetStyle(TEXTBOX, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    if (GuiSpinner((Rectangle){ 300, 950, 125, 30 }, "Tick speed", &spinnerTicks, 0, 10, spinnerEditMode)) { 
        //spinnerEditMode = !spinnerEditMode; 
        evolutionTicks = (float) spinnerTicks / 10;
    }

}


void evolveCells () {
// uses the Conwway ruleset to manage all the cells in the grid
// CPU intensive

int temp_arr[max_y+2][max_x+2] = {};    // temp array used to hold the new cells structure
int neighbors[8] = {};                  // the 8 neighboring cells round the chosen one
int counter=0;

    for (int r=1; r <= max_y; r++) {
        for (int c=1 ; c <= max_x; c++) {
            
            neighbors[0] = cell_arr[r-1][c-1];      // top left
            neighbors[1] = cell_arr[r-1][c];        // top center
            neighbors[2] = cell_arr[r-1][c+1];      // top right
            neighbors[3] = cell_arr[r][c-1];        // left
            neighbors[4] = cell_arr[r][c+1];        // right
            neighbors[5] = cell_arr[r+1][c-1];      // bottom left
            neighbors[6] = cell_arr[r+1][c];        // bottom center
            neighbors[7] = cell_arr[r+1][c+1];      // bottom right                                                              

            counter = neighbors[0]+neighbors[1]+neighbors[2]+neighbors[3]+neighbors[4]+neighbors[5]+neighbors[6]+neighbors[7];  // get the count of cells around the main cell.

            if (cell_arr[r][c]>0 && counter<2) {                        // any live cell with fewer than 2 neighbors will die
                temp_arr[r][c] = 0;
            }

            if (cell_arr[r][c]>0 && (counter==2 || counter==3)) {       // any live cell with between 2 & 3 neighbors will live on
                temp_arr[r][c] = 1;
            }

            if (cell_arr[r][c]>0 && counter>3) {                        // any live cell with more than 3 neighbors will die
                temp_arr[r][c] = 0;
            }

            if (cell_arr[r][c]==0 && counter==3) {                      // any void cell with exactly 3 neighbors will reproduce
                temp_arr[r][c] = 1;
            }

        }
    }

    swap(cell_arr, temp_arr);   // swap temp to llive 2D array

}


int main () {

    InitWindow(win_width, win_height, "Game of Life (2023)");
    // FPS should be low as this has very simple graphic use
    SetTargetFPS(20);
    //initCellAssignment();
    GuiLoadStyle("terminal.rgs");   // Set the GUI template to Terminal style
   

    while (! exitProgram)
    {

        exitProgram = WindowShouldClose();

        BeginDrawing();
        ClearBackground(DARKGRAY);

        // draw vertical lines
        for (int i=0; i<=win_width; i+=gap) {
            DrawLine(i, 0, i, win_height-offset+gap, GRAY);
        }

        // draw horizontal lines
        for (int j=0; j<=win_height-offset+gap; j+=gap) {
            DrawLine(0, j, win_width, j, GRAY);
        }
        
        
        drawCells();
        
        if (RunningSimulation) {    // only run evolution when actually running

            updateTimer(&evolutionTimer);
            if (timerDone(&evolutionTimer)) {
                evolveCells();
                startTimer(&evolutionTimer, evolutionTicks);
            }
        } else {

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

                Vector2 mousePos = GetMousePosition();
                Rectangle canvas = {0, 0, win_width, win_height-offset+gap};
                
                if (CheckCollisionPointRec(mousePos, canvas)) {
                    int boundX = mousePos.x / gap;
                    int boundY = mousePos.y / gap;
                    cell_arr[boundY][boundX] = 1;
                }
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {

                Vector2 mousePos = GetMousePosition();
                Rectangle canvas = {0, 0, win_width, win_height-offset+gap};
                
                if (CheckCollisionPointRec(mousePos, canvas)) {
                    int boundX = mousePos.x / gap;
                    int boundY = mousePos.y / gap;
                    cell_arr[boundY][boundX] = 0;
                }

            }
        }

        drawUI();
        EndDrawing();

        if (IsKeyPressed(KEY_SPACE)) {
            initCellAssignment();
        }
    }



    CloseWindow();
    return 0;
    
}