/*
* Course: Digital Systems 2025-26
* Week 6: Reaction-Diffusion Patterns (Gray-Scott Model) in a web browser
* 
* A reaction-diffusion system models how chemicals A and B react with each other
* and diffuse through space, creating emergent self-organizing patterns.
* 
* NOTE: EMSCRIPTEN_KEEPALIVE above a function ensures the function is exported to JS
* 
* by Evan Raskob 2025-26
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <emscripten.h>

// Grid dimensions
int WIDTH = 128;
int HEIGHT = 128;

// --- GRAY-SCOTT PARAMETERS ---
// Changing 'f' (feed) and 'k' (kill) alters the "species" of pattern.
// These specific values produce mitosis-like spots and stripes.

double diffusion_A = 1.0f;      // Diffusion rate of chemical A
double diffusion_B = 0.5f;      // Diffusion rate of chemical B
double feed_rate = 0.028f;      // Feed rate (f)  (adding A)
double kill_rate = 0.062f;      // Kill rate (k) (removing B)
double time_step = 1.0f;        // Time step for integration (with normalized kernel)

// Chemical concentrations
double *chemical_A = NULL;      // Concentration of chemical A
double *chemical_B = NULL;      // Concentration of chemical B
double *buffer_A = NULL;        // Buffer for chemical A calculations
double *buffer_B = NULL;        // Buffer for chemical B calculations
uint8_t *display_grid = NULL;  // Grid for display (visualization of B)


// --- Function Prototypes ---
void update_generation(double *A_curr, double *B_curr, double *A_next, double *B_next);
void update_reaction_diffusion();
void update_display();
int iterate();
void set_cell(int x, int y); // set A/B concetration of cell at (x,y)
int get_cell(int x, int y); //added a get_cell prototype because it was missing and appears below
void cleanup();

// ------------------------------
// --- Getters/Setters for JS ---
// ------------------------------


// --- Part 3a (like Conway 1a & 1b) starts here: -------------------
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

EMSCRIPTEN_KEEPALIVE
uint8_t* get_grid_ptr() { return display_grid; }

EMSCRIPTEN_KEEPALIVE
void set_feed_rate(double f) { feed_rate = f; }

EMSCRIPTEN_KEEPALIVE
float get_feed_rate() { return feed_rate; }

EMSCRIPTEN_KEEPALIVE
void set_kill_rate(double k) { kill_rate = k; }

EMSCRIPTEN_KEEPALIVE
void set_diffusion_A(float d) { diffusion_A = d; }

EMSCRIPTEN_KEEPALIVE
void set_diffusion_B(float d) { diffusion_B = d; }

EMSCRIPTEN_KEEPALIVE
void set_time_step(float t) { time_step= t; } //Bug fix ? 

//------------------------------------------------


// --- Part 3a continues here: -------------------
//----------------------------------------------
/**
 * Set cell value at index (used in JavaScript to 'paint' cells)
 */


    // hint: this one is like the version in Game of Life,
    // but there are *two* arrays we need to set values for!
    // the cell in chemical_A should be set to the diffusion value for A,
    // and for chemical_B use the diffusion value for B .
    // See the setup function below for an example of how this is done

EMSCRIPTEN_KEEPALIVE
void set_cell(int x, int y) { //changed to void to match prototype above
    if (chemical_A == NULL || chemical_B == NULL) return ; 
    
    int radius = 5;

    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int px = x + dx;
            int py = y + dy;
    //checking if the values are within the grid
    if (px < 0 || px >= WIDTH) return ;
    if (py < 0 || py >= HEIGHT) return ;
    // set cell at the appropriate index 

    
    
    int index = py * WIDTH + px;
    chemical_A[index] = 0.4;
    chemical_B[index] = 1.5;
    }
}
}


    // hint: this one is like the version in Game of Life,
    // but there are *two* arrays so which do we use?
    // The answer is neither A nor B, instead use the display grid
    // because that is the calculated value of the system.
    // Again, see below to undertand how it is calculated
EMSCRIPTEN_KEEPALIVE
int get_cell(int x, int y) {
    if (display_grid == NULL) return 0; 
    // same as above - checking x and y are in the grid 
    if (x < 0 || x >= WIDTH) return 0;
    if (y < 0 || y >= HEIGHT) return 0;

    // get cell value at the appropriate index in the display gird
    // return cell value to JavaScript 

    int index = y * WIDTH + x; 
    return display_grid[index];
}   


// Initialize the reaction-diffusion system
EMSCRIPTEN_KEEPALIVE
int setup_reaction_diffusion()
{
    int size = WIDTH * HEIGHT;
    srand(time(NULL));

    // Allocate memory for chemical concentrations
    chemical_A = (double *)malloc(size * sizeof(double));
    chemical_B = (double *)malloc(size * sizeof(double));
    buffer_A = (double *)malloc(size * sizeof(double));
    buffer_B = (double *)malloc(size * sizeof(double));
    display_grid = (uint8_t *)malloc(size * sizeof(uint8_t));
    
    if (chemical_A == NULL || chemical_B == NULL || 
        buffer_A == NULL || buffer_B == NULL || display_grid == NULL)
    {
        return 1; // Memory allocation failed
    }
        
    // Initialize the grid: A = 1 everywhere, B = 0 everywhere
    for (int i = 0; i < size; i++)
    {
        chemical_A[i] = 1.0f;
        chemical_B[i] = 0.0f;
        buffer_A[i] = 1.0f;
        buffer_B[i] = 0.0f;
    }
    
    // Create a seed: add chemical B in the center region
    // This seed will grow and create patterns
    int center_x = WIDTH / 2;
    int center_y = HEIGHT / 2;
    int seed_radius = 8;
    
    for (int y = center_y - seed_radius; y <= center_y + seed_radius; y++)
    {
        for (int x = center_x - seed_radius; x <= center_x + seed_radius; x++)
        {
            int dx = x - center_x;
            int dy = y - center_y;
            if (dx * dx + dy * dy <= seed_radius * seed_radius)
            {
                // Toroidal wrap
                int wrapped_x = (x + WIDTH) % WIDTH;
                int wrapped_y = (y + HEIGHT) % HEIGHT;
                int idx = wrapped_y * WIDTH + wrapped_x;
                double noise = (rand() % 100) / 1000.0; 
                chemical_B[idx] = 0.4 + noise; // Add noise between 0.0 and 0.10
                chemical_A[idx] = 0.5;  
            }
        }
    }
    
    update_display();
    return 0;
}
    
/**
 * Calculate the Laplacian (second derivative) using proper 3x3 kernel convolution.
 * In physics, it calculates how a substance diffuses.
 * In image filtering, this exact same math is used for edge detection!
 * Notice the weights: Center is negative, neighbors are positive. 
 * It measures the difference between a cell and its surroundings.
 * This kernel is a normalized Laplacian that properly approximates ∇²
 * Kernel (sums to 0, which is crucial for Laplacian):
 *  0.05  0.20  0.05
 *  0.20 -1.00  0.20
 *  0.05  0.20  0.05
 */

 // If we wrote it out, it would look like this;
// const double laplacian_kernel[3][3] = {
//     {0.05, 0.20, 0.05},
//     {0.20,-1.00, 0.20},
//     {0.05, 0.20, 0.05}
// };

double get_laplacian(double *grid, int x, int y, int w, int h)
{
    // Get all 8 neighbors with toroidal wrapping
    int nw = ((y - 1 + h) % h) * w + ((x - 1 + w) % w);  // NW
    int n  = ((y - 1 + h) % h) * w + x;                   // N
    int ne = ((y - 1 + h) % h) * w + ((x + 1) % w);      // NE
    int w_idx = y * w + ((x - 1 + w) % w);              // W
    int c  = y * w + x;                                   // C
    int e  = y * w + ((x + 1) % w);                      // E
    int sw = ((y + 1) % h) * w + ((x - 1 + w) % w);     // SW
    int s  = ((y + 1) % h) * w + x;                       // S
    int se = ((y + 1) % h) * w + ((x + 1) % w);         // SE
    
    // Apply the normalized Laplacian kernel
    double laplacian = 0.05 * (grid[nw] + grid[ne] + grid[sw] + grid[se])
                     + 0.20 * (grid[n] + grid[s] + grid[e] + grid[w_idx])
                     - 1.00 * grid[c];
    return laplacian;
}

/**
* Update the reaction-diffusion system
* Implements the Gray-Scott model:
*   dA/dt = D_a * ∇²A - A*B² + f*(1-A)
*   dB/dt = D_b * ∇²B + A*B² - (k+f)*B
*/
void update_reaction_diffusion()
{
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            int idx = y * WIDTH + x;
            
            double a = chemical_A[idx];
            double b = chemical_B[idx];
            
            // Calculate Laplacian for diffusion
            double lap_a = get_laplacian(chemical_A, x, y, WIDTH, HEIGHT);
            double lap_b = get_laplacian(chemical_B, x, y, WIDTH, HEIGHT);
            
            // Reaction-diffusion equations
            double reaction = a * b * b;
            
            buffer_A[idx] = a + time_step * (diffusion_A * lap_a - reaction + feed_rate * (1.0f - a));
            buffer_B[idx] = b + time_step * (diffusion_B * lap_b + reaction - (kill_rate + feed_rate) * b);
        }
    }
    
    // Swap buffers
    double *temp_a = chemical_A;
    double *temp_b = chemical_B;
    chemical_A = buffer_A;
    chemical_B = buffer_B;
    buffer_A = temp_a;
    buffer_B = temp_b;
}

/**
* Update the display grid based on current chemical concentrations
* We visualize the concentration of B on the display
*/
void update_display()
{
    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
        // Map B concentration (0-1) to grayscale (0-255)
        double v = chemical_B[i];
        //if (v > 1.0) v = 1.0;
        display_grid[i] = (uint8_t)(v * 255.0);
    }
}

/**
* Main iteration function called from JavaScript
*/
EMSCRIPTEN_KEEPALIVE
int iterate()
{
    // SIMULATION STEP: Chemical reactions happen very slowly.
    // If we only updated once per frame, the animation would take hours.
    // We run the math 15 times for every 1 time we draw to the screen.

    for (int step = 0; step < 6; step++) { //turned this down so that it would run smoother for the three JS
        update_reaction_diffusion();
    }
    update_display();
    return 1;
}

/**
* Memory cleanup
*/
EMSCRIPTEN_KEEPALIVE
void cleanup()
{
    if (chemical_A != NULL) free(chemical_A);
    if (chemical_B != NULL) free(chemical_B);
    if (buffer_A != NULL) free(buffer_A);
    if (buffer_B != NULL) free(buffer_B);
    if (display_grid != NULL) free(display_grid);
}
