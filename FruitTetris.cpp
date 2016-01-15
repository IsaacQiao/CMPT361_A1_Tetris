/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris

This code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
by Rui Ma (ruim@sfu.ca) on 2014-03-04. 

Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).
*/

#include "include/Angel.h"
#include <cstdlib>
#include <iostream>
#include <time.h> //*Isaac* For random

using namespace std;


// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400; 
int ysize = 720;

// *Isaac* define global tile x&y position to be used in newtile function
int xpos = 5;
int ypos = 19;

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos = vec2(xpos, ypos); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)
// *Isaac* set tile style as global to use in different functions
int randomtile;

// Update the color VBO of current tile
// *Isaac* set as global to use in different functions
vec4 newcolours[24];

// An array storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
vec2 allRotationsLshape[12][4] = 
	{//*Isaac* creat more shaps here
	 //*Isaac* each shape have its own order inside for easier use when change the color or shape
	{vec2(-2, 0), vec2(-1, 0), vec2( 0, 0), vec2( 1, 0)},//I1
	{vec2( 0, 1), vec2( 0, 0), vec2( 0,-1), vec2( 0,-2)},//I2
	{vec2(-1, 0), vec2( 0, 0), vec2( 1, 0), vec2( 2, 0)},//I3
	{vec2( 0, 2), vec2( 0, 1), vec2( 0, 0), vec2( 0,-1)},//I4	
	{vec2( 1, 0), vec2( 0, 0), vec2( 0,-1), vec2(-1,-1)},//S1
	{vec2( 0, 1), vec2( 0, 0), vec2( 1, 0), vec2( 1,-1)},//S2
	{vec2( 1, 1), vec2( 0, 1), vec2( 0, 0), vec2(-1, 0)},//S3
	{vec2(-1, 1), vec2(-1, 0), vec2( 0, 0), vec2( 0,-1)},//S4
	{vec2( 1, 0), vec2( 0, 0), vec2(-1, 0), vec2(-1,-1)},//L1
	{vec2( 0, 1), vec2( 0, 0), vec2( 0,-1), vec2( 1,-1)},//L2  
	{vec2( 1, 1), vec2( 1, 0), vec2( 0, 0), vec2(-1, 0)},//L3
	{vec2(-1, 1), vec2( 0, 1), vec2( 0, 0), vec2( 0,-1)}};//L4

// colors
// *Isaac* creat more colors here
vec4 white  = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = vec4(0.0, 0.0, 0.0, 1.0); 
vec4 allColors[5] = {vec4(1.0, 0.0, 0.0, 1.0), vec4(1.0, 0.5, 0.0, 1.0), 
	vec4(1.0, 1.0, 0.0, 1.0), vec4(0.0, 1.0, 0.0, 1.0), vec4(0.5, 0.0, 0.5, 1.0)};
// *Isaac* An array storing all possible colors
// allColors={red,orange,yellow,green,purple}

//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20]; 
// *Isaac* set as global to be use in different functions
bool reachtheboard = false;
// game stop bool
bool gamestop = false;


//An array containing the colour of each of the 10*20*2*3 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = tilepos.x + tile[i].x; 
		GLfloat y = tilepos.y + tile[i].y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1); 
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), .4, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), .4, 1);

		// Two points are used by two triangles each
		vec4 newpoints[6] = {p1, p2, p3, p2, p3, p4}; 

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*6*sizeof(vec4), 6*sizeof(vec4), newpoints); 
	}

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------

// Called at the start of play and every time a tile is placed
void newtile()
{
	//*Isaac* random style
	randomtile = rand() % 12; 

	// *Isaac* make sure the y pos is at 19 at the initial
	ypos = 20;

	// *Isaac* depends on the style, get the initial x pos
	// and also depends on the styles, make different y position
	//l2
	if (randomtile == 0){
		xpos = rand()%7+2;
	}
	//r2
	else if (randomtile == 2){
		xpos = rand()%7+1;
	}
	// none
	else if (randomtile == 1){
		xpos = rand()%10; 
		ypos = ypos-1;
	}
	else if (randomtile == 3){
		xpos = rand()%10; 
		ypos = ypos-2;
	}
	//left 1
	else if (randomtile == 7 || randomtile == 11){
		xpos = rand()%9+1;
	}
	//right 1
	else if (randomtile == 5 || randomtile == 9){
		xpos = rand()%9;
	}
	//left 1 right 1
	else{
		xpos = rand()%8+1;
	}
	// for other y's original position initilization
	if (randomtile == 5 || randomtile == 9 || randomtile == 6 || randomtile == 7 ||randomtile == 11 ||randomtile == 10)
	{
		ypos = ypos-1;
	}

	// Put the tile at the top of the board 
	// *Isaac* and a random x position, and spicified y position
	tilepos = vec2(xpos, ypos); 

	// Update the geometry VBO of current tile
	for (int i = 0; i < 4; i++)
		tile[i] = allRotationsLshape[randomtile][i]; // Get the 4 pieces of the new tile *Isaac* and a random style
	updatetile(); 

	// Update the color VBO of current tile

	// *Isaac* set vec4 newcolours[24]; as global to be use in stack up function
	// vec4 newcolours[24];

	// *Isaac* get a random color (one of the 5 fruits) for every 6 vertixes (for all the 4 fruits in this Tetris tile)
	for (int j = 6;j <= 24; j=j+6){
		int clonum = rand()%5;
		for (int i = j-6; i < j; i++){
			newcolours[i] = allColors[clonum]; // *Isaac* randomlize the color
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------
// *Isaac* define a function to reverse the color of the tile if needed
void reversecolor()
{
	// set a temp color24 to do record of original colors
	vec4 tempcolor[24];
	for (int i = 0; i <24; i++){
		tempcolor[i]=newcolours[i];
	}
	// and then reverse it into newcolours[i] array
	for (int i = 0; i <24; i++){
		newcolours[i]=tempcolor[23-i];
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}
//-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------
// *Isaac* define a function to shuffle the color of the tile
void shufflecolor()
{
	vec4 tempcolor[24];
	for (int i = 0; i <24; i++){
		tempcolor[i]=newcolours[i];
	}

	// depends on the different types, shuffle a->b->c->d as 1->2->3->4 or 4->3->2->1
	if (randomtile == 0 || randomtile == 3 || randomtile == 6 || randomtile == 7 || randomtile == 10 || randomtile == 11)
	{
		for (int i = 0; i <24; i++){
			newcolours[i] = tempcolor[(i+18)%24];
		}
	}
	else
	{
		for (int i = 0; i <24; i++){
			newcolours[i] = tempcolor[(i+6)%24];
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
}
//-------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------------------

void initGrid()
{
	// ***Generate geometry data
	vec4 gridpoints[64]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
	vec4 gridcolours[64]; // One colour per vertex
	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
		
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 0, 1);
	}
	// Make all grid lines white
	for (int i = 0; i < 64; i++)
		gridcolours[i] = white;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, 64*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}


void initBoard()
{
	// *** Generate the geometric data
	vec4 boardpoints[1200];
	for (int i = 0; i < 1200; i++)
		boardcolours[i] = black; // Let the empty cells on the board be black

	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{		
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), .5, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), .5, 1);
			
			// Two points are reused
			boardpoints[6*(10*i + j)    ] = p1;
			boardpoints[6*(10*i + j) + 1] = p2;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;
		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false; 


	// *** set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current tile initially
void initCurrentTile()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(3, &vaoIDs[0]);

	// Initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrentTile();

	// The location of the uniform variables in the shader program
	locxsize = glGetUniformLocation(program, "xsize"); 
	locysize = glGetUniformLocation(program, "ysize");

	// Game initialization
	newtile(); // create new next tile

	// set to default
	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}

//-------------------------------------------------------------------------------------------------------------------

// Rotates the current tile, if there is room
void rotate()
{      
	//-----------------------------
	// *Isaac* build whole function
	//-----------------------------

	// these ints are for calculate the current tile's position
	int boardposx0=tilepos.x+tile[0].x;
	int boardposy0=tilepos.y+tile[0].y;
	int boardposx1=tilepos.x+tile[1].x;
	int boardposy1=tilepos.y+tile[1].y;
	int boardposx2=tilepos.x+tile[2].x;
	int boardposy2=tilepos.y+tile[2].y;
	int boardposx3=tilepos.x+tile[3].x;
	int boardposy3=tilepos.y+tile[3].y;

	//depends on different tile types, rotate differently, and also keep the color order inside the tile.

	//0
	if ((randomtile == 0) && (boardposy2 != 19) && (boardposy2 != 0) && (boardposy2 != 1) && 
		(board[boardposx2][boardposy2+1]!=1) && (board[boardposx2][boardposy2-1]!=1) && (board[boardposx2][boardposy2-2]!=1))
	{
		randomtile = 1;
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[randomtile][i]; // Get the 4 pieces of the new tile *Isaac* and a random style
		updatetile(); 
		reversecolor();
	}
	//1
	else if ((randomtile == 1) && (boardposx1 != 0) && (boardposx1 != 9) && (boardposx1 != 8) &&
	 (board[boardposx1-1][boardposy1]!=1) && (board[boardposx1+1][boardposy1]!=1) && (board[boardposx1+2][boardposy1]!=1))
	 {
	 	randomtile = 2;
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[randomtile][i]; // Get the 4 pieces of the new tile *Isaac* and a random style
		updatetile(); 
	}
	//2
	else if ((randomtile == 2) && (boardposy1 != 19) && (boardposy1 != 0) && (boardposy1 != 18) && 
		(board[boardposx1][boardposy1+1]!=1) && (board[boardposx1][boardposy1-1]!=1) && (board[boardposx1][boardposy1+2]!=1))
	{
		randomtile = 3;
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[randomtile][i]; // Get the 4 pieces of the new tile *Isaac* and a random style
		updatetile(); 
		reversecolor();
	}
	//3
	else if ((randomtile == 3) && (boardposx2 != 0) && (boardposx2 != 9) && (boardposx2 != 1) &&
	 (board[boardposx2-1][boardposy2]!=1) && (board[boardposx2+1][boardposy2]!=1) && (board[boardposx2-2][boardposy2]!=1))
	 {
	 	randomtile = 0;
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[randomtile][i]; // Get the 4 pieces of the new tile *Isaac* and a random style
		updatetile(); 
	}
	//4
	else if ((randomtile == 4) && (boardposy1 != 19) && 
		(board[boardposx1][boardposy1+1]!=1) && (board[boardposx0][boardposy0-1]!=1))
	 {
	 	randomtile = 5;
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[randomtile][i]; // Get the 4 pieces of the new tile *Isaac* and a random style
		updatetile(); 
	}
	//5
	else if ((randomtile == 5) && (boardposx1 != 0) && 
		(board[boardposx1-1][boardposy1]!=1) && (board[boardposx0+1][boardposy0]!=1))
	 {
	 	randomtile = 6;
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[randomtile][i]; // Get the 4 pieces of the new tile *Isaac* and a random style
		updatetile(); 
		reversecolor();
	}
	//6
	else if ((randomtile == 6) && (boardposy2 != 0) && 
		(board[boardposx1-1][boardposy1]!=1) && (board[boardposx2][boardposy2-1]!=1))
	 {
	 	randomtile = 7;
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[randomtile][i]; // Get the 4 pieces of the new tile *Isaac* and a random style
		updatetile(); 
	}
	//7
	else if ((randomtile == 7) && (boardposx2 != 9) && 
		(board[boardposx3-1][boardposy3]!=1) && (board[boardposx2+1][boardposy2]!=1))
	 {
	 	randomtile = 4;
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[randomtile][i]; // Get the 4 pieces of the new tile *Isaac* and a random style
		updatetile(); 
		reversecolor();
	}
	//8
	else if ((randomtile == 8) && (boardposy1 != 19) &&
		(board[boardposx1][boardposy1+1]!=1) && (board[boardposx1][boardposy1-1]!=1) && (board[boardposx0][boardposy0-1]!=1))
	{
		randomtile = 9;
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[randomtile][i]; // Get the 4 pieces of the new tile *Isaac* and a random style
		updatetile(); 
	}
	//9
	else if ((randomtile == 9) && (boardposx1 != 0) &&
		(board[boardposx1-1][boardposy1]!=1) && (board[boardposx1+1][boardposy1]!=1) && (board[boardposx0+1][boardposy0]!=1))
	{
		randomtile = 10;
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[randomtile][i]; // Get the 4 pieces of the new tile *Isaac* and a random style
		updatetile();
		reversecolor();
	}
	//10
	else if ((randomtile == 10) && (boardposy2 != 0) &&
		(board[boardposx2][boardposy2+1]!=1) && (board[boardposx2][boardposy2-1]!=1) && (board[boardposx3][boardposy3+1]!=1))
	{
		randomtile = 11;
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[randomtile][i]; // Get the 4 pieces of the new tile *Isaac* and a random style
		updatetile(); 
	}
	//11
	else if ((randomtile == 11) && (boardposx2 != 9) &&
		(board[boardposx2-1][boardposy2]!=1) && (board[boardposx2+1][boardposy2]!=1) && (board[boardposx3-1][boardposy3]!=1))
	{
		randomtile = 8;
		// Update the geometry VBO of current tile
		for (int i = 0; i < 4; i++)
			tile[i] = allRotationsLshape[randomtile][i]; // Get the 4 pieces of the new tile *Isaac* and a random style
		updatetile();
		reversecolor();
	}
}

//-------------------------------------------------------------------------------------------------------------------

bool checkthreecolor(vec4 c1, vec4 c2, vec4 c3)
{
	//-----------------------------
	// *Isaac* build whole function
	//-----------------------------
	//this function is use to check if the three color input is same or not (and also not same as black)

	if ((c1[0]==c2[0]) && (c1[1] == c2[1]) && (c1[2] == c2[2]) && (c1[3] == c2[3]) &&
		(c1[0]==c3[0]) && (c1[1] == c3[1]) && (c1[2] == c3[2]) && (c1[3] == c3[3]) &&
		(c1[0]!=black[0] || c1[1]!=black[1] || c1[2]!=black[2] || c1[3]!=black[3] ))
	{
		return true;
	}
	else 
		return false;
}

//-------------------------------------------------------------------------------------------------------------------


void checkthreesame(){
	//-----------------------------
	// *Isaac* build whole function
	//-----------------------------
	// *Isaac* Checks if there are three same fruits that are in a row or column, 
	// they will be removed and the tiles above them  will  be  moved  down.

	// set three colors
	vec4 c1,c2,c3;
	// true if c1==c2==c3
	bool threesame = false;

	// check row
	for (int r = 0; r < 20; r++)
	{//check from the bot row

		for (int c = 2; c < 10; c++)
		{//check if three same exist in the same row from left+2 to right
			threesame = false;

			//record the current position of the check point
			int currentpos = r*60+c*6;

			//get the colors of current square and two squares to the left
			c1 = boardcolours[currentpos];
			c2 = boardcolours[currentpos-6];
			c3 = boardcolours[currentpos-12];
			
			threesame = checkthreecolor(c1,c2,c3);

			if (threesame)
			{
			//exist! begin deletion
				// *Isaac* fint where to delete
				int ii = 60*r+c*6+5;
				// *Isaac* clear color and occupicy
				for (int i = ii; i > (ii - 18); i--)
					boardcolours[i] = black;
				for (int x = c; x > c-3; x--)
					board[x][r] = false;

				// *Isaac* move down occupicy and color
				if (r != 19)
				{
					for (int y = r; y< 19 ; y++)
					{
						for (int x = c; x > c-3; x--)
							board[x][y] = board[x][y+1];
					}
					for (int y = r; y<19; y++)
					{
						int currentthreepos = 60*y+c*6+5;
						for (int threepos = currentthreepos; threepos > currentthreepos-18; threepos--)
						{
							boardcolours[threepos] = boardcolours[threepos+60];
						}
					}
				}
				// *Isaac* here draw the color on the background again.
				// *Isaac* [------------------------------------
				glutPostRedisplay();
				// *** set up buffer objects
				glBindVertexArray(vaoIDs[1]);
				glGenBuffers(2, &vboIDs[2]);
				// Grid cell vertex colours
				glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
				glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
				glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(vColor);

				//!!! once clear the current three! we need to recheck from begin because the deletion just happened may cause more three same!
				checkthreesame();
			}
		}
	}

	// check column
	for (int c = 0; c < 10; c++)
	{//check from left to right column
		for (int r = 2; r < 20; r++)
		{//check from bot+2 to top
			
			//same way as what i did in chack row.

			//check if three same exist
			threesame = false;
			int currentpos = r*60+c*6;
			c1 = boardcolours[currentpos];
			c2 = boardcolours[currentpos-60];
			c3 = boardcolours[currentpos-120];
			
			threesame = checkthreecolor(c1,c2,c3);

			if (threesame)
			{
			//exist! begin deletion
				// *Isaac* clear color and occupicy
				for (int n =0;n<3;n++)
				{
					int ii = 60*r+c*6+5-n*60;
					for (int i = ii; i > (ii - 6); i--)
					{
						boardcolours[i] = black;
					}
				}

				for (int y = r; y > r-3; y--)
					board[c][y] = false;

				// *Isaac* move down occupicy and color
				if (r != 19)
				{
					for (int y = r-2; y< 17 ; y++)
					{
							board[c][y] = board[c][y+3];
					}
					for (int y = r-2; y<17; y++)
					{
						int currentthreepos = 60*y+c*6+5;
						for (int threepos = currentthreepos; threepos > currentthreepos-6; threepos--)
						{
							boardcolours[threepos] = boardcolours[threepos+180];
						}
					}
				}
				// *Isaac* here draw the color on the background again.
				// *Isaac* [------------------------------------
				glutPostRedisplay();
				// *** set up buffer objects
				glBindVertexArray(vaoIDs[1]);
				glGenBuffers(2, &vboIDs[2]);
				// Grid cell vertex colours
				glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
				glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
				glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(vColor);
				checkthreesame();
			}
		}
	}


}

//-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
void checkfullrow(int row)
{
	//-----------------------------
	// *Isaac* build whole function
	//-----------------------------

	// assume row is full
	bool rowfull = true;

	// if NOTOCCUPIED existed --> row is not full
	for (int i = 0; i<10; i++)
	{
		if (board[i][row]==false){
			rowfull = false;
		}
	}

	//if row is full!!
	if (rowfull==true)
	{
		// *Isaac* fint where to delete
		int ii = 60*row;
		// *Isaac* clear color and occupicy
		for (int i = ii; i < ii+60 ; i++)
			boardcolours[i] = black;
		for (int x = 0; x<10; x++)
				board[x][row] = false;

		// *Isaac* move down occupicy and color
		if (row != 19)
		{
			for (int y = row; y< 19 ; y++)
			{
				for (int x = 0; x<10; x++)
					board[x][y] = board[x][y+1];
			}
			for (int i = ii+60; i<1200;i++)
				boardcolours[i-60] = boardcolours[i];
		}
		// *Isaac* here draw the color on the background again.
		// *Isaac* [------------------------------------
		glutPostRedisplay();
		// *** set up buffer objects
		glBindVertexArray(vaoIDs[1]);
		glGenBuffers(1, &vboIDs[3]);
		// Grid cell vertex colours
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
		glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(vColor);
	}

}

//-------------------------------------------------------------------------------------------------------------------

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile()
{
	//-----------------------------
	// *Isaac* build whole function
	//-----------------------------

	// *Isaac* set these ints for calculation
	int boardposx0=tilepos.x+tile[0].x;
	int boardposy0=tilepos.y+tile[0].y;
	int boardposx1=tilepos.x+tile[1].x;
	int boardposy1=tilepos.y+tile[1].y;
	int boardposx2=tilepos.x+tile[2].x;
	int boardposy2=tilepos.y+tile[2].y;
	int boardposx3=tilepos.x+tile[3].x;
	int boardposy3=tilepos.y+tile[3].y;
	int i0 = 60*boardposy0+6*boardposx0;
	int i1 = 60*boardposy1+6*boardposx1;
	int i2 = 60*boardposy2+6*boardposx2;
	int i3 = 60*boardposy3+6*boardposx3;

	// *Isaac* run 4 for loop for fill 4*6 color on the black background
	for (int i = i0; i < i0+6 ; i++)
		boardcolours[i] = newcolours[0];
	for (int i = i1; i < i1+6 ; i++)
		boardcolours[i] = newcolours[6];
	for (int i = i2; i < i2+6 ; i++)
		boardcolours[i] = newcolours[12];
	for (int i = i3; i < i3+6 ; i++)
		boardcolours[i] = newcolours[18];

	// *Isaac* here draw the color on the background again.
	// *Isaac* [------------------------------------
	glutPostRedisplay();
	// *** set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(1, &vboIDs[3]);
	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
	// *Isaac* ------------------------------------]

	// once set the tile, we need to check if there is a full row or same three
	for (int i = 19; i>=0; i--)
	{
		checkfullrow(i);
	}
	checkthreesame();

	// *Isaac* at this point, we finish what settile should do and thus, we need a new tile.
	if (! gamestop)
		newtile();
}

//-------------------------------------------------------------------------------------------------------------------

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetile(vec2 direction)
{	
	//-----------------------------
	// *Isaac* build whole function
	//-----------------------------

	// redefine reachtheboard bool
	reachtheboard = false;

	// *Isaac* these ints are for calculate the current tile's position
	int boardposx0=tilepos.x+tile[0].x;
	int boardposy0=tilepos.y+tile[0].y;
	int boardposx1=tilepos.x+tile[1].x;
	int boardposy1=tilepos.y+tile[1].y;
	int boardposx2=tilepos.x+tile[2].x;
	int boardposy2=tilepos.y+tile[2].y;
	int boardposx3=tilepos.x+tile[3].x;
	int boardposy3=tilepos.y+tile[3].y;

	// check if the tile reach the critical point

	if (direction.x == -1){
		//0
		if ((randomtile == 0) && ((boardposx0 == 0) || (board[boardposx0-1][boardposy0]==1)))
		{
			return false;
		}
		//1
		else if ((randomtile == 1) && ((boardposx0 == 0) || (board[boardposx0-1][boardposy0]==1) ||
			(board[boardposx1-1][boardposy1]==1) ||(board[boardposx2-1][boardposy2]==1) ||(board[boardposx3-1][boardposy3]==1)))
		{
			return false;
		}
		//2
		else if ((randomtile == 2) && ((boardposx0 == 0) || (board[boardposx0-1][boardposy0]==1)))
		{
			return false;
		}
		//3
		else if ((randomtile == 3) && ((boardposx0 == 0) || (board[boardposx0-1][boardposy0]==1) ||
			(board[boardposx1-1][boardposy1]==1) ||(board[boardposx2-1][boardposy2]==1) ||(board[boardposx3-1][boardposy3]==1)))
		{
			return false;
		}
		//4 & 6
		else if (((randomtile == 4) || (randomtile == 6)) && ((boardposx3 == 0) || (board[boardposx3-1][boardposy3]==1) || 
			board[boardposx1-1][boardposy1]==1))
		{
			return false;
		}
		//5 & 7
		else if (((randomtile == 5) || (randomtile == 7)) && ((boardposx0 == 0) || (board[boardposx0-1][boardposy0]==1) || 
			board[boardposx1-1][boardposy1]==1 || board[boardposx3-1][boardposy3]==1))
		{
			return false;
		}
		//8
		else if ((randomtile == 8) && ((boardposx3 == 0) || (board[boardposx2-1][boardposy2]==1) ||(board[boardposx3-1][boardposy3]==1)))
		{
			return false;
		}
		//9
		else if ((randomtile == 9) && ((boardposx0 == 0) || (board[boardposx0-1][boardposy0]==1) ||
			(board[boardposx1-1][boardposy1]==1) ||(board[boardposx2-1][boardposy2]==1) ))
		{
			return false;
		}
		//10
		else if ((randomtile == 10) && ((boardposx3 == 0) || (board[boardposx3-1][boardposy3]==1) || 
			board[boardposx0-1][boardposy0]==1))
		{
			return false;
		}
		//11
		else if ((randomtile == 11) && ((boardposx0 == 0) || (board[boardposx0-1][boardposy0]==1) ||
			(board[boardposx2-1][boardposy2]==1) ||(board[boardposx3-1][boardposy3]==1)))
		{
			return false;
		}
		
	}

	else if (direction.x == 1){
		//0
		if ((randomtile == 0) && ((boardposx3 == 9) || (board[boardposx3+1][boardposy3]==1)))
		{
			return false;
		}
		//1
		else if ((randomtile == 1) && ((boardposx0 == 9) || (board[boardposx0+1][boardposy0]==1) ||
			(board[boardposx1+1][boardposy1]==1) ||(board[boardposx2+1][boardposy2]==1) ||(board[boardposx3+1][boardposy3]==1)))
		{
			return false;
		}
		//2
		else if ((randomtile == 2) && ((boardposx3 == 9) || (board[boardposx3+1][boardposy3]==1)))
		{
			return false;
		}
		//3
		else if ((randomtile == 3) && ((boardposx0 == 9) || (board[boardposx0+1][boardposy0]==1) ||
			(board[boardposx1+1][boardposy1]==1) ||(board[boardposx2+1][boardposy2]==1) ||(board[boardposx3+1][boardposy3]==1)))
		{
			return false;
		}
		//4 & 6
		else if (((randomtile == 4) || (randomtile == 6)) && ((boardposx0 == 9) || (board[boardposx0+1][boardposy0]==1) || 
			board[boardposx2+1][boardposy2]==1))
		{
			return false;
		}
		//5 & 7
		else if (((randomtile == 5) || (randomtile == 7)) && ((boardposx3 == 9) || (board[boardposx0+1][boardposy0]==1) || 
			board[boardposx2+1][boardposy2]==1 || board[boardposx3+1][boardposy3]==1))
		{
			return false;
		}
		//8
		else if ((randomtile == 8) && ((boardposx0 == 9) || (board[boardposx0+1][boardposy0]==1) ||
			(board[boardposx3+1][boardposy3]==1)))
		{
			return false;
		}
		//9
		else if ((randomtile == 9) && ((boardposx3 == 9) || (board[boardposx0+1][boardposy0]==1) ||
			(board[boardposx1+1][boardposy1]==1) ||(board[boardposx3+1][boardposy2]==1) ))
		{
			return false;
		}
		//10
		else if ((randomtile == 10) && ((boardposx0 == 9) || (board[boardposx0+1][boardposy0]==1) || 
			board[boardposx1+1][boardposy1]==1))
		{
			return false;
		}
		//11
		else if ((randomtile == 11) && ((boardposx3 == 9) || (board[boardposx1+1][boardposy1]==1) ||
			(board[boardposx2+1][boardposy2]==1) ||(board[boardposx3+1][boardposy3]==1)))
		{
			return false;
		}
		
	}

	// *Isaac* check is the tile is move to a critical position
	else if( ((boardposy3==0) || (board[boardposx0][boardposy0-1]==1)|| (board[boardposx1][boardposy1-1]==1)||
	 (board[boardposx2][boardposy2-1]==1)|| (board[boardposx3][boardposy3-1]==1)) && (direction.y == -1))
	{
		// *Isaac* if YES -> set these positions to true which means they are occupied.
		for (int i = 0; i < 4; i ++){
			int boardtposx=tilepos.x+tile[i].x;
			int boardtposy=tilepos.y+tile[i].y;
			board[boardtposx][boardtposy] = true;
		}
		// *Isaac* issue! here!
		reachtheboard = true;
		return false;
	}	

	// not ceitical!!!!!!!!!!
	// *Isaac* move! here!
	tilepos = tilepos + direction;

	// *Isaac* once moved-> update the position of current tile and draw it!
	updatetile();
	glutPostRedisplay();
	
	// finally return true!!
	return true;
}
//-------------------------------------------------------------------------------------------------------------------
// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
	init();
}
//-------------------------------------------------------------------------------------------------------------------

// Draws the game
void display()
{

	glClear(GL_COLOR_BUFFER_BIT);

	glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	glUniform1i(locysize, ysize);

	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 24); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 64); // Draw the grid lines (21+11 = 32 lines)


	glutSwapBuffers();
}

//-------------------------------------------------------------------------------------------------------------------

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------

// Handle arrow key keypresses
void special(int key, int x, int y)
{
	switch(key) 
	{
		case GLUT_KEY_UP:
			rotate();
			break;

		case GLUT_KEY_DOWN:
			movetile(vec2(0, -1));
			if(reachtheboard){
				settile();
			}
			break;

		case GLUT_KEY_LEFT:
			movetile(vec2(-1, 0));
			if(reachtheboard){
				settile();
			}
			break;

		case GLUT_KEY_RIGHT:
			movetile(vec2(1, 0));
			if(reachtheboard){
				settile();
			}
			break;

	}
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' key restarts the game
			restart();
			break;
		case ' ':
			shufflecolor();
			break;
	}
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

void idle(void)
{
	glutPostRedisplay();
}

//-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------
// *Isaac* my designed Function for timmer moving
void dropdown(int value)
{
	
	glutTimerFunc(2000,dropdown,0);

	// *Isaac* if the tile reach a critical point(issue!), set the tile!
	movetile(vec2(0, -1));
	if(reachtheboard){
		settile();
	}
}
//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	srand (time(NULL)); // *Isaac* For random function call
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Fruit Tetris");
	glewInit();
	init();

	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	// *Isaac* write the timmer function here to call it at the begnning
	glutTimerFunc(2000,dropdown,0);

	glutMainLoop(); // Start main loop
	return 0;
}
