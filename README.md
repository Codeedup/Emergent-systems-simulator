# Final Project for Methods 2: Digital Systems 2025-26

*by Evan Raskob <e.raskob@arts.ac.uk>*


# What you will do

This can be thought of as one large project, or two smaller ones, plus a short reflective writing. 

The first project leverages the concepts we explored in C: digital representation of numbers, bitwise operations, data types and 2D "artificial life" in the form of Conway's Game of Life, Reaction-diffusion, and also fractals. 

The second project may or may not incorporate the first, depending on your creative and technical direction. This is where you demonstrate your basic proficiency in 3D world-building, especially the maths and techniques needed to realise complex 3D structures and basic simulations like L-Systems and Boids.

On both projects you have considerable leeway to make creative decisions and hopefully have some fun with the generative audio-visual material we introduced in class.

Detailed instructions follow. Please read them! Don't use an AI to summarise them, they are a summary already! If you don't understand anything then *ask Evan directly* in class or via Slack. 


# What you will submit

Download this git repository containing a JavaScript web app and follow the instructions in this document to complete the project. Please keep the directory structure as close as possible to the original, adding folders as needed when you make different versions of the basic examples that you feel you need to show, and adding a folder called `reflections` in the root with the reflective exercise and any screenshots or video recordings of your process.

When finished, **remove the node_modules folder** by deleting it and zip all the files and folders into a zip archive just like the git repo was and upload it all to Moodle. *Don't change the folder structure or you will lose marks.* 

An even better way to archive your work is to put these files into your own new git repository, save your commits and then either clone the repository and zip it up bare, or use our github website to download the zip.

# Notes on reflection and AI 

As we discussed in class, a big part of what we are learning is how to manage our own development process, and understand and communicate our work. This includes documenting our work for our future selves, as well as for others, and reflecting on it.

Reflection is also fundamental to an iterative software and conceptual development process, because after each stage of completion we have to think about what we’ve done and then come up with a future plan that's better than our current one and takes into account our failures and successes and everything in between.

If you are outsourcing your reflection to an AI, *you are not learning* and *you are not planning*. At some point you *have* to look back over your work and reflect, in your own words and thoughts, in order to move forwards. Heck, you need to do it *just to stay where you are an not slip backwards*.

As such, your writing don't have to be amazingly worded, they just need to be coherent and your own.

# Getting started

This again uses Bun (https://bun.sh) as a package manager, so make sure you have it installed or nothing will work.

Then, run `bun install` to install the necessary libraries and `npm run dev` to start the server. You should see a list of projects. *The projects won't work until you finish the code.*

## How this project is structured

Referring to the file system, `vite` (the server) uses `index.html` as the starting page so in that file you will see a list of links to the examples. When you add or modify examples of your own, you add or remove from this list. Makes sense?

The examples are each in their own folder, named something that briefly describes them like *conway*. When you add an example, do the same thing -- put it in the root folder and create a new folder for it with a brief name (no spaces, they get confusing in html).

the `src` folder has some general code in it that applies to all projects, like the CSS. The `main.js` there doesn't do anything, it's for the `index.html` file in case you wanted to add anything to it (I'm not sure that you do).

## Compiling C examples with Emscripten

Generally, the C code examples can be compiled using Emscripten using the same looking command, the only difference is the input and output file names at the start. For example, to compile the Conway example, first open a terminal (VS Code's terminal isn't always reliable FYI) and change directory (remember the `cd` command? Remember on Mac you can copy a folder in the Finder and paste the full path into the terminal?) and then run:

```bash
emcc conway_web.c -o conway_web.js \
    -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
    -s MODULARIZE=1 \
    -s EXPORT_ES6=1 \
    -s ALLOW_MEMORY_GROWTH=1
```

*Note: Because we used EMSCRIPTEN_KEEPALIVE, the setup_conway, iterate, get_grid_ptr, etc., functions are automatically exported to JS.*


### Aside: Why we used C 

C is more efficient for large Game of Life boards and the reaction diffusion examples. You will notice it is a lot faster than JavaScript when it gets to a 1000x1000 grid. I didn't do a full side-by-side test but you are welcome to do it yourself and report back!


# Assignment Tasks

At each stage, take some screenshots, record some reflections on your progress and challenges (remember the prompts from Methods 1).

## Passing data between C and JavaScript

The first task will be to adapt the Game of Life example to pass data from JavaScript to the compiled C program so we can add some interaction to the Conway board. 

**For this part you will work on the conway_web.c and conway.html files in the /conway folder.** 

### 1. Starting out: set and get functions

Remember setters and getters from Methods 1? Let's do that with Conway's Game of Life.

#### 1a. Setting

At the top of the file, but below where `WIDTH` and `HEIGHT` are defined, create two new functions (called `set_width` and `set_height`) that let us set the WIDTH and HEIGHT of the Conway board. They will need to take a number as argument (what type? `int`? `float`? `uint8_t`?) and will have type `void` because they don't return anything, and make sure to put `EMSCRIPTEN_KEEPALIVE` in the line just before each declaration so that JavaScript can access them in the web browser. Look at the other set/get functions after them for reference on how it's done.

Then, in your JavaScript in `index.html` use these new functions to set your Conway board to 800x600. 

*HINT: setting the width and height of it is a start but you also need to think about what to do next to apply these to the simulation because `WIDTH` and `HEIGHT` just **store** values, they don't actually create or modify arrays or program memory. There's at least one more step (in Javascript, not the C file) that you need to take to actually change the Conway board.*


#### 1b. Getting

Now we need the reverse -- two functions, one to get the width and the other to get the height from JavaScript. they should have an `int` type to be most compatible with JavaScript.


#### 1c. Set/get cell data

At some point we will need to get the value of a particular cell from the Conway board from JavaScript, and also to set the value of a cell to 1 (ALIVE). The *get* function should look up the cell value at the coordinate `(x,y)` and return an `int` (Note: probably a `uint8_t` is more appropriate but less compatible and not necessary here so we use `int` instead). The *set* function should set the value at coordinate `(x,y)` to `1`.

You probably also want to create a function in your C code that can translate an (x,y) coordinate in an image to the appropriate index in your data array. You don't have to, though -- it's pretty simple to type out.


### 2. An interface using bitwise operations

It would be great if we could explicitly set regions of the Conway board to the different shapes (gliders, spaceships, pinwheels) from his notes (see [Week 6 notes](https://www.notion.so/evanteaching/Week-6-Artificial-Life-1-Emergence-in-2D-Grids-3259e67390e7807e9377c633a15b0260)). Let's make our board interactive so we can "draw" into it using the mouse or touchscreen to apply different bitwise operations directly to the Conway cells. Let's try implementing that.

#### 2a. Pause/restart

First, we probably need a way to *pause* the simulation because it will be too hard to draw whilst the simulation is changing the board at the same time. 

In the JavaScript code we toggle a boolean variable between true (simulation is running) and false (simulation is paused). Since `render()` is where the animation happens, the code should check if the simulation is paused inside there.

Your example *automatically* pauses the system whilst you are drawing (e.g. when the mouse is down) and un-pauses it when the mouse is up.

Automatically pausing the simulation in response to the mouse being pressed means capturing mouse events like `mousedown` and `mousemove` and running functions on the event object that you receive from them, which will contain the location of the mouse cursor, relative to the element that the event-catpuring function has been called from, in the properties `event.clientX` and `event.clientY`
(assuming you name your captured event `event` and not something else like just `e`). 

**For you to do:** Since this part was tricky to implement, I've done the hard part of setting up event handlers for you. Look at the code and add a button in HTML and a function in JavaScript that stops and resumes the simulation. How you do it is up to you, it can be a toggle button (pause/restart) or two buttons (pause and start). There are comments in the code to help you with this.

The current example scales the image up by some factor so pixel (10, 20) isn't necessarily equivalent to cell (10, 20) in Game of Life. Look at how the function translates pixel coordinates to Game of Life board coordinates (look for the `addEventHandler()` lines in the HTML files ) **and write comments in the code explaining what each part does:** how it captures the mouse press event and handles transforming the coordinates so that they align to the coordinates of the simulation's grid.


#### 2b. bitwise pixel drawing

By default, the pixel drawing methods just set the cell to alive.

Modify these to add bitwise operations to your clickable Game of Life Board by adding another radio button (or use keys or mouse buttons, up to you) to select between different bitwise operations (like `AND, OR, XOR, NOT`) to make it so you can click on a pixel in the `canvas` element of the Game of Life in JavaScript and it will perform that bitwise operation on the appropriate cell in the Game of Life board. So, when you click on a pixel it takes the current value of that cell (`true` or `false`) and performs a bitwise operation with `true`. If you set it the operation to `OR` and click on a pixel in the image it should set the Game of Life cell to ALIVE (`true`) no matter what, but if you set it to `XOR` it would only be ALIVE if the current value was DEAD (`false`), otherwise it would be DEAD because XOR returns `false` unless both arguments are `true`.

*Note: If you are using bitwise operations, you don't simply set the value of the cell to ALIVE. You need to think about how you want to implement this -- in C, in the same function with extra arguments? With different functions? In JavaScript by getting the value first and then doing bitwise ops? Up to you, as long as you make it work.*

You decide which pixel operations are interesting, and make other creative judgements about how this should work as long as it uses bitwise operations and some form of interaction such as using the mouse movements and buttons or keyboard, or whatever else you can think of. 

Briefly, you will be doing something like clicking the mouse, getting the value of the cell that corresponds to the area of the image you clicked in, doing a bitwise operation on it, and then setting it to something. That's why we made set/get functions previously.

Take a short video of each operation you try out, maybe 5-10 seconds at most. **Remember that you need to show documentation of your process otherwise your work will not be marked.**


*HINT:*

In this example, if you use a grayscale image that means you only have one colour value per pixel, which makes things easier. Remember that pixel data is represented by an unsigned 8 bit integer for each colour channel, which is normally R,G,B,A (in JavaScript's canvas) or just Gray for grayscale. (HINT: [we did this in class](https://git.arts.ac.uk/IU000242-Methods-2-Digital-Systems/glitch-lab/blob/main/web/web_glitch.c))


**Advanced**

1. **Better drawing:** Instead of drawing pixel-by-pixel, how about drawing with a small rectangle or ball-shaped cursor? This way you can change multiple pixels at the same time and make more of an effect. You can implement the pixel tools in C or JavaScript, up to you.

2. **Gliders and life presets:** like in class, create a function or two that automatically draw gliders into the grid, and use it to place a few fun "lifeforms" at the start. 

3. (Very optional) **Lifeform painting:** How about painting with gliders or other lifeforms?? What would that interface look like? How would it work? 

4. (Very very optional) **Breakout UI panel:** If you're feeling very advanced, why not create a smaller 1-1 version of the Game Of Life to draw on instead of clicking the larger rendered image? Think of it like a mini-control panel.

## 3. Artificial lives

Maybe Conway isn't your thing. Let's use another artificial life or other generative algorithm like slime moulds or Gray Scott *reaction diffusion* like in the next example. 

**For this part you will work on the rd_web.c and rd.html files in the /rd folder.** 

### 3a Fix rd_web.c or roll your own

**You will need to make the same changes to `rd_web.c` as in Task 1a, 1b, and 1c** if you want to use Reaction-diffusion. **OR** add your own simulation using my examples as a guide!

If you are using reaction diffusion continue 3 below:

### 3b Reaction-Diffusion Patterns (Gray-Scott Model)

Let's find some cool patterns.

As we discussed in class, an alternative approach to simulating artificial life uses **reaction-diffusion systems**, which model how two chemicals (A and B) react with each other and simultaneously diffuse (spread) through space. This creates beautiful, self-organizing patterns without any explicit "rules" like Conway's Game of Life.

The **Gray-Scott model** implemented here uses these differential equations:
- **$dA/dt$** $= D_a · ∇²A - A·B² + f·(1-A)$  
- **$dB/dt$** $= D_b · ∇²B + A·B² - (k+f)·B$

Where:
- **$D_a, D_b$** are diffusion rates (how fast chemicals spread)
- **$f$** is the feed rate (how much A is replenished)
- **$k$** is the kill rate (how much B is consumed)
- **$∇²$** (Laplacian) calculates the difference between a cell and its neighbors

The system starts with chemical A everywhere at concentration 1.0, and a seed of chemical B in the center. The chemicals then interact: A + B → react, but A diffuses faster than B. By adjusting the feed and kill rates, you can create waves, spirals, stripes, and many other patterns—all emerging from simple local rules without any top-down control.

To compile the reaction-diffusion version use Emscripten and the same looking command as before:

```bash
emcc rd_web.c -o rd_web.js \
    -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
    -s MODULARIZE=1 \
    -s EXPORT_ES6=1 \
    -s ALLOW_MEMORY_GROWTH=1
```

Then open `rd.html` in a browser. Use the sliders to adjust the parameters and watch the patterns change in real-time. 

Try clicking and dragging in the grid to perturb the simulation. You need to vigorously move the mouse since it's only drawing pixel-by-pixel.

**Find some presents that you like and take screenshots of the preset values and results on screen.** You can use [the presets from class](https://www.notion.so/evanteaching/Week-6-Artificial-Life-1-Emergence-in-2D-Grids-3259e67390e7807e9377c633a15b0260?source=copy_link#3259e67390e7800db247d7e5b16d6b9e) as a guide, they don't always work exactly due to differences in precision.

## 4. 3D Worlds

Here you can make a choice -- bring your 2D world into 3D, or do a separate 3D project. Up to you.

You should be demonstrating that you can efficiently represent fractal/chaotic data structures like trees and fractals (L-Systems) or iterated equations (Mandelbrot) or behaviours (Boids, particles, etc.).

You should use matrixes and vectors to set up complex motions like movements, rotations, relative transformations like with the L-systems and planetary examples. Instead of using ThreeJS's built in Scenegraph to automatically position and transform your 3D objects, instead do it manually like we did in class. In other words, show us your work! Create transformation matrixes and apply them to your objects so that each has a local space that gets translated into a global space when it is rendered by the camera.

You should try to be efficient in how you render your world, using instancing and cached geometry as much as possible (see the examples from the L-systems lab for a refresher).

Make this part creative and fun by creating a concept and designing your world around it. For example, "Oceanic bliss" with fish and sand and corals. Or maybe a flowering forest, with bees as spheres with circle wings and vision cones to show orientation, pollen particles, etc.  Give it a title.

**Things you might do in your world:**

*(but don't have to)*

1. Create different trees using L-Systems, along with different animated creatures using Boids or other emergent behaviours
2. Generate sounds using code (offline, but provide us with code) and load up rendered sound files and play them as ambience
3. Use the 2D conway or reaction diffusion or mandelbrot or other fractals and simulations as textures (in the `/canvas_texture` folder you will find an example of drawing a Conway grid into a 3D texture. Do what you like with it!)

### Example

In the `/canvas_texture` folder you can find a fully-working example that uses another 2D canvas to run Game of Life and uses that as a texture on a 3D cube in a separate webgl canvas. If you got your Game of Life working, then it should just work! (It won't do anything until you finish it because it links to the Emscripten compiled JavaScript file, check the source code).

You can, of course, hide the 2D canvas by setting the CSS display property to 'none' or create an offscreen canvas (see the links at the end of this doc for the ThreeJS examples). 

![3D Conway game of life from the canvas example](/src/assets/3d_conway.png)


## Reflection

We discussed how computer simulations and mathematics have their own assumptions built in, like how modern digital maps and grids have a history in military targeting and how computer programming also has a long history in military applications in terms of targeting systems and reliability in war and defense, instead of leaving space for error, fuzzy contexts, creative outputs, exploration. 

How do the assumptions of the tool-makers and ruling elite get embedded and perpetuated in the tools they build and techniques they use, even if they seem universal like with mathematical notation and simulations? For example, how does our simulations of "digital life" using simple rules possibly reinforce the idea that animals and natural systems have no real agency and only we (as humans) do? How can we miss seeing mathematical abilities in other cultures when we only look for systems that we are familiar with? Remember Hardaway's "The View From Nowhere".

This doesn't mean that our tools are *bad* and need to be thrown away. We can critique a system that we find useful so we are aware of its limitations, and get a sense of what can be improved or done away with. Our relationship with our tools and systems is *complicated* but that doesn't mean we can't still *have fun and make nice things.* 

Write 200-300 words reflecting on your experience in the class -- pick a topic or tool or concept from the syllabus and reflect on your experience with it. Ask yourself "Why? Why this system? What's missing? What's included? Who or what does it serve? Is that good/bad/other? What was my experience with it?"

Think about how that topic or concept or tool affects the broader world, and how your own experience might be helpful to a wider audience. This doesn't need to be political, or radical, or too personal, just give it some thought. **Don't use AI, put it in your own words and don't feel like you need to share it with anyone.** This is for you and your tutors, unless you really want to share it with others.

# How you will be assessed: the Learning Outcomes

## LO1 Apply principles of binary logic in the design and use of basic electronic systems (Knowledge)

We never got into *actual* electronics but we did look at simulated electronics -- bytes, bits, and how [numbers can be represented in circuits and computer memory](https://www.notion.so/evanteaching/Week-1-Computing-the-Real-World-2f49e67390e7804c93aed5941fc4d472?source=copy_link#2f89e67390e7805b9711cbb30853340d). This was all in [week 1](https://www.notion.so/evanteaching/Week-1-Computing-the-Real-World-2f49e67390e7804c93aed5941fc4d472). 

Then, we further explored this at a higher level in [Week 2](https://www.notion.so/evanteaching/Week-2-Data-realities-and-formats-2f69e67390e780e18ef8e3b3a2be4b2f) when we looked at extracting headers and loading audio and then later on image data. This also involved knowing about how variables are implemented in different languages and how that maps to low-level hardware and software design.

To an extent, we also used binary logic to create waveforms when we synthesised audio and glitched images in C, and especially when we looked at extracting colour components (e.g. R,G,B values) from pixel values in images. 

You will be assessed on how well you demonstrate in your bitwise pixel operations that you understand basic binary by correctly implementing and using simple operations like AND, OR, and XOR and using data types correctly (int, unsigned int, etc). 

*A large part of this comes from your documentation of your process, and comments in source code.*

# LO2 Identify and apply basic concepts of linear algebra such as vectors and matrixes (Knowledge)

This LO refers to the quiz questions you will get (that is the *identify* portion) and to later weeks with WebGL and linear algebra. We used vector structures to represent both points and actual vectors (in the linear algebra sense) and then explored how matrix maths could be used to create 3D transformations of those points and vectors.

You will be assessed on how well you demonstrate that you can use matrixes and vectors to set up complex motions like movements, rotations, relative transformations like with the L-systems and planetary examples. That is why you need to be explicit about how you use them, you will not get full marks if you just use ThreeJS to handle this for you.

*A large part of this comes from your documentation of your process, and comments in source code.*


# LO3 Experiment with different methods of representing, storing, manipulating images and large data sets in digital systems (Enquiry)

In class this was the glitch/BMPs/sound labs; conway and reaction diffusion labs; L-systems labs; and boids. This refers more to the techniques we used to manipulate these data structures than the conceptual side of what they meant and were used for. 

For example:

* [Week 2: Data realities and formats](https://www.notion.so/evanteaching/Week-2-Data-realities-and-formats-2f69e67390e780e18ef8e3b3a2be4b2f) looked at how data is represented in C, then in Week 3 we worked on breaking/glitching that data to understand it better.

* [Week 4: Infinite detail from simple loops](https://www.notion.so/evanteaching/Week-4-Infinite-detail-from-simple-loops-30d9e67390e780b7b1ecd7af08cf5878#3179e67390e7802fb6d1def8d46913bd) introduced fractals (Mandelbrot) and L-Systems. 

Generally, you will be assessed on how correctly your code accessed and manipulated images, artificial life simulations, and how advanced your L-Systems representations and renderers were implemented (this could vary based on what you used). A lot of this is correctly using, manipulating, and copying between arrays that represent different types of data (pixels, vectors, matrices, etc.).

*A large part of this comes from your documentation of your process, and comments in source code.*

# LO4 Identify how power relations are organized, embedded and perpetuated in digital infrastructure (Enquiry)

You will be assessed mainly on your reflection but also on your documentation of your process. Hopefully, you worked on this assignment but also kept a part of your brain thinking "Why? Why this system? What's missing? What's included? Who or what does it serve? Is that good/bad/other?"

An excellent reflection situates both your process and the class material within the broader community by referencing outside inspirations such as open source tools. It demonstrates insight into the developer identity, how design choices relate to equity and power, the nature of feedback and iterative processes.


# Further resources

https://threejs.org/manual/#en/textures

https://threejs.org/manual/#en/canvas-textures

