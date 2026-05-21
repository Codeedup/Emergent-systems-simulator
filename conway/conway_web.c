/*
 * Course: Digital Systems 2025-26
 * Week 6: Emergence in Grids: Conway's Game of Life in a web browser
 * 
 * NOTE: EMSCRIPTEN_KEEPALIVE above a function ensures the function is exported to JS
 * 
 * by Evan Raskob 2025-26
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // uint8_teger types
#include <unistd.h> // Required for usleep() (pausing the animation)
#include <time.h>   // Required to seed the random number generator
#include <string.h> // memcpy
#include <emscripten.h>


//these are in characters, not pixels
int WIDTH = 48;
int HEIGHT = 60;
uint8_t life_chance = 25; // 25 percent chance of new life

uint8_t *life_grid = NULL;   // current state of life
uint8_t *buffer_grid = NULL; // will be used when we change states, so we do them all at once


//------------------------------------------------------
// Function Definitions come first, for ease of reading


// --- Getters for JS ---

// --- Part 1a & 1b starts here: -------------------
//----------------------------------------------
EMSCRIPTEN_KEEPALIVE
int get_width() {
    return WIDTH;
}

EMSCRIPTEN_KEEPALIVE
void set_width(int new_width ) { 
    WIDTH = new_width;
 }

EMSCRIPTEN_KEEPALIVE
int get_height() { 
    return HEIGHT;
 }

EMSCRIPTEN_KEEPALIVE
void set_height(int new_height) { 
    HEIGHT = new_height;
 }

//----------------------------------------------


// JS needs to know where the memory is and how big it is
EMSCRIPTEN_KEEPALIVE
uint8_t* get_grid_ptr() { return life_grid; }

EMSCRIPTEN_KEEPALIVE
void set_life_chance(uint8_t l) { life_chance=l; }

int count_alive_neighbors(uint8_t *grid, int cx, int cy, int w, int h);               // conway's rules
void copy_from_buffer(uint8_t *original_grid, uint8_t *buffer_grid, int buffer_size); // copy
void next_state(uint8_t *current_grid, uint8_t *buffer_grid, const int w, const int h);   // calculate and set next state of grid
void cleanup();
int setup_conway();
void swap_buffers();
void next_state(uint8_t *current_grid, uint8_t *buffer_grid, const int w, const int h);
int iterate();


// --- Part 1c is here: -----------------------
//---------------------------------------------
/**
 * Set the value of a cell at x,y
 * Doesn't do error checking!!! This is dangerous (remember overflow errors?)
 */
EMSCRIPTEN_KEEPALIVE
int set_cell(int x, int y) {
    if (life_grid == NULL) return 1; // bail if no grid!

    //checking if the values are within the grid
    if (x < 0 || x >= WIDTH) return 1;
    if (y < 0 || y >= HEIGHT) return 1;
    // set cell at the appropriate index in the life grid to alive
   
    
    int index = y * WIDTH + x;
    life_grid[index] = 1;
    return 0; // end normally
}

EMSCRIPTEN_KEEPALIVE
int get_cell(int x, int y) {
    if (life_grid == NULL) return 0; // if no grid what should we do? Return dead no matter what?
    // same as above - checking x and y are in the grid 
    if (x < 0 || x >= WIDTH) return 0;
    if (y < 0 || y >= HEIGHT) return 0;

    // get cell value at the appropriate index in the life grid
    // return cell value to JavaScript 

    int index = y * WIDTH + x; 
    return life_grid[index];
}   
//---------------------------------------------


//------------------------------------------------------
//
// Main function that runs when this program is executed
//
EMSCRIPTEN_KEEPALIVE
int setup_conway()
{
    // Reminder about memory allocation:
    // we use `sizeof(uint8_t)` because malloc doesn't know what an array or an uint8_teger is; it only knows "bytes"
    // which are returned as a general (void *) type. Because an uint8_t might be 4 bytes on one machine and
    // 8 bytes on another, we multiply our grid size by sizeof(uint8_t) to make sure it is the actual memory
    // size for the machine we are on.

    // Double buffering -- two different image grids that we swap between
    // NOTE: sincel we're using pouint8_ts, we could alternate grids but this example
    // shows copying so we can get more practice doing that.

    life_grid = (uint8_t *)malloc(WIDTH * HEIGHT * sizeof(uint8_t));   // current state of life
    buffer_grid = (uint8_t *)malloc(WIDTH * HEIGHT * sizeof(uint8_t)); // will be used when we change states, so we do them all at once

    // Safety check: always ensure the OS actually gave you the memory
    if (life_grid == NULL || buffer_grid == NULL)
    {
        return 1;
    }

    // 1. Seed the random number generator with the current time.
    //    Using a specific number means we get a predictable sequence of
    //    random numbers, which can also be useful.
   srand(time(NULL));

    // 2. Initialize the grid with random life (chance to be alive)
    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        // roll the dice and decide whether to create new "life"
        // use % modulus because we get a large number, potentially and we want a percentage
        if (rand() % 100 < life_chance)
        {
            life_grid[i] = 1; // new life!
        }
        else
        {
            life_grid[i] = 0; // nothing (dead?)
        }
    }

    // NOTE: We DO NOT free() here! We need this memory to persist.

    return 0; // exit program
}


EMSCRIPTEN_KEEPALIVE
int iterate() {
    next_state(life_grid, buffer_grid, WIDTH, HEIGHT);
    return 1;
}

// Memory Cleanup: Call this from JS if you ever destroy the game
EMSCRIPTEN_KEEPALIVE
void cleanup() {
    if (life_grid != NULL) free(life_grid);
    if (buffer_grid != NULL) free(buffer_grid);
}


// update state quickly by swapping buffer pointers, no copying needed
void swap_buffers() {
    uint8_t *temp = life_grid;
    life_grid = buffer_grid;
    buffer_grid = temp;
    temp = NULL;
}

/**
 * Update the
 */
void next_state(uint8_t *current_grid, uint8_t *buffer_grid, const int w, const int h)
{
    // loop through the grids, counting alive neighbours for each cell and then setting
    // the state (1=alive, 0=dead) based on the count
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            int index = (y * w) + x; // index in the array
            int alive_neighbors = count_alive_neighbors(current_grid, x, y, w, h);
            int is_alive = current_grid[index];

            // Conway's Rules
            if (is_alive == 1 && (alive_neighbors < 2 || alive_neighbors > 3))
            {
                buffer_grid[index] = 0; // Death
            }
            else if (is_alive == 0 && alive_neighbors == 3)
            {
                buffer_grid[index] = 1; // Birth
            }
            else
            {
                buffer_grid[index] = is_alive; // Stasis
            }
        }
    }
    

    // actually update state
    swap_buffers();

    //copy_from_buffer(current_grid, buffer_grid, w * h);
}

/**
 * Count neighbours for the cell at (cx,cy) in a grid of dimensions w x h
 */
int count_alive_neighbors(uint8_t *grid, int cx, int cy, int w, int h)
{
    int count = 0;

    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
                continue;

            int nx = cx + dx;
            int ny = cy + dy;

            // --- TOROIDAL WRAP-AROUND ---
            // Instead of a hard border, we wrap the edges to the opposite side.
            // In C, the modulo operator (%) handles this easily.
            // We add the width before modulo to prevent negative numbers due to dx.
            nx = (nx + w) % w;
            ny = (ny + h) % h;

            int neighbor_index = (ny * w) + nx;
            count += grid[neighbor_index];
        }
    }
    return count;
}