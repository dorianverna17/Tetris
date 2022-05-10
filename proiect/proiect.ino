/* Arduino libraries needed in the computation */
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

// Define hardware type, size, and output pins:
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN 3

// Setup for software SPI:
#define DATA_PIN 2
#define CLK_PIN 4
MD_MAX72XX myDisplay = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

/*
 * declare the matrix that represents the tetris game board
 * 8 rows and 32 columns (4 * 8x8 matrices)
 */
#define ROWS 8
#define COLUMNS 32
int board[8][32];

/* auxiliary for the piece generation */
int piece[ROWS][ROWS];

/* count for piece generation */
int count;
int first_it;

void generateRandomPiece();
void displayMatrix();
void putPieceInMatrix();
void MovePieces();

void setup() {
  /* Intialize the object: */
  myDisplay.begin();
  /* Set the intensity (brightness) of the display (0-15): */
  myDisplay.control(MD_MAX72XX::INTENSITY, 0);
  /* Clear the display: */
  myDisplay.clear();

  /* init the board with zeros */
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLUMNS; j++)
      board[i][j] = 0;
  }

  /* set a random seed */
  randomSeed(millis());

  /* begin serial for debugging */
  Serial.begin(9600);

  /* initialize counter for piece generation */
  count = 0;
  /* tells if it is the first iteration */
  first_it = 1;
}

void loop() {
  if (count == 8 || first_it == 1) {
    generateRandomPiece();
    putPieceInMatrix();
    first_it = 0;
  }
  
  count++;
  if (count == 9)
    count = 0;

  displayMatrix();
  movePieces();
  delay(1000);
}

/* In this function I move the pieces */
void movePieces() {
  for (int i = 0; i < ROWS; i++) {
    for (int j = COLUMNS - 1; j >= 1; j--) {
      board[i][j] = board[i][j - 1];
    }
  }
  for (int i = 0; i < ROWS; i++) {
    board[i][0] = 0; 
  }
}

/* this function displays the matrix */
void displayMatrix() {
  /* display each row of each matrix */
  uint8_t row = 0;
  
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < ROWS; j++) {
      row = 0;
      for (int k = 0; k < ROWS; k++) {
        if (board[j][i * ROWS + k])
          row ^= (1 << k);
      }
      if (i == 0)
      Serial.println(row);
      myDisplay.setRow(i, i, j, row);
    }
  }
}

/* this function replaces the first
 * matrix with the one with the piece
 */
void putPieceInMatrix() {
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < ROWS; j++) {
      board[i][j] = piece[i][j];
      Serial.print(piece[i][j]);
      Serial.print(" ");
    }
    Serial.println("\n");
  }
  Serial.println("\n");
}

/*
 * piece is generated within the
 * boundaries of the first matrix
 */
void generateRandomPiece() {
  /* auxiliary variables */
  int starting_point;
  int vertical;
  int horizontal;
  int intersection_point;
 
  /* initialize the matrix with zeros */
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < ROWS; j++) {
      piece[i][j] = 0;
    }
  }

  /* starting_point */
  starting_point = random(0, 8);
  piece[starting_point][0] = 1;
  
  vertical = random(2, 4);
  if (vertical == 0)
    vertical++;
  horizontal = random(2, 4);
  if (horizontal == 0)
    horizontal++;
  if (starting_point + vertical >= ROWS) {
    vertical = ROWS - starting_point - 1;
  }
  intersection_point = random(0, horizontal);

  /* iterate over the next columns */
  for (int i = 0; i < horizontal; i++) {
      piece[starting_point][i] = 1;
      if (intersection_point == i) {
        for (int j = 0; j < vertical; j++)
          piece[starting_point + j][i] = 1;
      }
  }
}
