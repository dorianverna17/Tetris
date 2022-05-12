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

/* other useful pins */
#define POTENTIOMETER A0

/*
 * declare the matrix that represents the tetris game board
 * 8 rows and 32 columns (4 * 8x8 matrices)
 */
#define ROWS 8
#define COLUMNS 32
#define MAX_PIECE_SIZE 4
int board[8][32];

/* structure that defines the position of the piece */
typedef struct moving_piece_struct {
  int size_piece;
  int x[2 * MAX_PIECE_SIZE];
  int y[2 * MAX_PIECE_SIZE];
} moving_piece;

/* auxiliary for the piece generation */
int piece[ROWS][ROWS];

/* count for piece generation */
int stopped;
int first_it;
int speed_pieces;

/* piece that is currently moving */
moving_piece piece_mov;

void generateRandomPiece();
void displayMatrix();
void putPieceInMatrix();
void MovePieces();
void printMatrixSerial();

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

  /*
   * tells when the moving piece reached the
   * bottom and another one can be created
   */
  stopped = 1;
  /* tells if it is the first iteration */
  first_it = 1;
}

void loop() {
  if (stopped == 1 || first_it == 1) {
    generateRandomPiece();
    putPieceInMatrix();
    first_it = 0;
    stopped = 0;
  }

  displayMatrix();
  displayPiece();
  movePieces();
  delay(speed_pieces);

//  printMatrixSerial();

  speed_pieces = analogRead(POTENTIOMETER);
//  Serial.println(speed_pieces);
}

int checkIntersectionOfPiece() {
  /* declare auxiliaries */
  int x, y;
  
  for (int i = 0; i < piece_mov.size_piece; i++) {
    x = piece_mov.x[i];
    y = piece_mov.y[i];
    if (y == COLUMNS - 1)
      return true;
    else if (board[x][y + 1] == 1)
      return true;
  }
  return false;
}

void stopPiece() {
  /* declare auxiliaries */
  int x, y;
  
  for (int i = 0; i < piece_mov.size_piece; i++) {
    x = piece_mov.x[i];
    y = piece_mov.y[i];
    board[x][y] = 1;
  }
  piece_mov.size_piece = 0;
  stopped = 1;
}

/* In this function I move the pieces */
void movePieces() {
  if (checkIntersectionOfPiece()) {
    stopPiece();
  }
  for (int i = 0; i < ROWS; i++) {
    for (int j = COLUMNS - 1; j >= 1; j--) {
      if (board[i][j - 1] == 2) {
        board[i][j] = board[i][j - 1];
        board[i][j - 1] = 0;
      }
    }
  }
  for (int i = 0; i < piece_mov.size_piece; i++) {
    piece_mov.y[i]++;
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
      myDisplay.setRow(i, i, j, row);
    }
  }
}

/* function to check if the game has been lost */
int checkIfLost() {
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < ROWS; j++) {
      if (board[i][j] != 0 && piece[i][j] == 2)
        return true;
    }
  }
  return false;
}

/* this function replaces the first
 * matrix with the one with the piece
 */
void putPieceInMatrix() {
  int iterator = 0;

  if (checkIfLost()) {
    Serial.println("You lost!");
    while (1);
  }
  
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < ROWS; j++) {
      if (piece[i][j] == 2) {
        board[i][j] = piece[i][j];
        piece_mov.x[iterator] = i;
        piece_mov.y[iterator] = j;
        iterator++;
      }
    }
  }
  piece_mov.size_piece = iterator;
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
  starting_point = random(0, ROWS);
  piece[starting_point][0] = 2;
  
  vertical = random(2, MAX_PIECE_SIZE);
  if (vertical == 0)
    vertical++;
  horizontal = random(2, MAX_PIECE_SIZE);
  if (horizontal == 0)
    horizontal++;
  if (starting_point + vertical >= ROWS) {
    vertical = ROWS - starting_point - 1;
  }
  intersection_point = random(0, horizontal);

  /* iterate over the next columns */
  for (int i = 0; i < horizontal; i++) {
      piece[starting_point][i] = 2;
      if (intersection_point == i) {
        for (int j = 0; j < vertical; j++)
          piece[starting_point + j][i] = 2;
      }
  }
}

void printMatrixSerial() {
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLUMNS; j++) {
      Serial.print(board[i][j]);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println();
  Serial.println();
}

void displayPiece() {
  for (int i = 0; i < piece_mov.size_piece; i++) {
    Serial.print("(");
    Serial.print(piece_mov.x[i]);
    Serial.print(", ");
    Serial.print(piece_mov.y[i]);
    Serial.print(") ");
  }
  Serial.println();
}
