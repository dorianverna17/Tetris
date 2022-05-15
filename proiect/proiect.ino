/* Arduino libraries needed in the computation */
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
/* for LCD */
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

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
#define MOVE_LEFT 44
#define MOVE_RIGHT 46
#define ROTATE_LEFT 48
#define ROTATE_RIGHT 50
#define START_STOP 52

/* lcd display */
LiquidCrystal_I2C lcd(0x27, 16, 2);

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
  /* the length of the piece */
  int horizontal;
  int vertical;
} moving_piece;

/* auxiliary for the piece generation */
int piece[ROWS][ROWS];

/* count for piece generation */
int stopped;
int first_it;
int speed_pieces;
int difficulty_speed;

/* piece that is currently moving */
moving_piece piece_mov;

/* button state variables */
int left_move;
int right_move;
int left_rotate;
int right_rotate;
int start_stop;
/* used for play/pause */
int continue_button;

/* the score of a game */
int score;
/* score that needs to e achieved in order to win */
int win_score;

/* 0 - nothing, 1 - win, 2 - lose */
int win_lose;

int game_started;

void generateRandomPiece();
void displayMatrix();
void putPieceInMatrix();
void MovePieces();
void printMatrixSerial();
void givePrize();
void reset_game();

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
  randomSeed(analogRead(POTENTIOMETER) + random(0, 1024));

  /* begin serial for debugging */
  Serial.begin(9600);

  /*
   * tells when the moving piece reached the
   * bottom and another one can be created
   */
  stopped = 1;
  
  /* tells if it is the first iteration */
  first_it = 1;

  /* game not started yet */
  game_started = 0;

  /* game not lost and not won */
  win_lose = 0;

  speed_pieces = analogRead(POTENTIOMETER);

  /* begin lcd */
  lcd.init();
  lcd.init();
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Hello! Select");
  lcd.setCursor(0, 1);
  lcd.print("your difficulty");
}

void loop() {
  if (game_started == 0) {
    int potentiometer_value = analogRead(POTENTIOMETER);
    
    if (abs(potentiometer_value - speed_pieces) > 10) {
      lcd.clear();
      lcd.setCursor(6, 0);
      if (potentiometer_value <= 333) {
        lcd.print("EASY");
        difficulty_speed = 800;
        win_score = 3;
      } else if (potentiometer_value > 333 && potentiometer_value <= 666) {
        lcd.print("MEDIUM");
        difficulty_speed = 400;
        win_score = 5;
      } else {
        lcd.print("HARD");
        difficulty_speed = 100;
        win_score = 7;
      }
      lcd.setCursor(2, 1);
      lcd.print("Press Start"); 
    }

    start_stop = digitalRead(START_STOP);
    if (start_stop == 1) {
      game_started = 1;
      score = 0;
      speed_pieces = difficulty_speed;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Score:");
      lcd.setCursor(8, 0);
      lcd.print(score);

      /* didn't lost or won yet */
      win_lose = 0;
    }
  } else {
    start_stop = digitalRead(START_STOP);
    if (start_stop == 1) {
      continue_button = 0;
      delay(100);
      while (continue_button == 0) {
        lcd.setCursor(0, 1);
        lcd.print("Game Paused");
        continue_button = digitalRead(START_STOP);
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Score:");
    }

    lcd.setCursor(8, 0);
    lcd.print(score);

    lcd.setCursor(10, 0);
    lcd.print("/");

    lcd.setCursor(12, 0);
    lcd.print(win_score);

    if (win_score == score) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Congrats!");
      lcd.setCursor(0, 1);
      lcd.print("You Won!");
      win_score = 1;
      delay(5000);
      givePrize();
      reset_game();
    }

    if (win_score == 2) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Bad Luck...");
      lcd.setCursor(0, 1);
      lcd.print("You Lost:(");
      delay(5000);
      reset_game();
    }
    
    if (stopped == 1 || first_it == 1) {
      generateRandomPiece();
      putPieceInMatrix();
      first_it = 0;
      stopped = 0;
    }

    eraseRows();

    displayMatrix();

    /* read the state of the move left button */
    left_move = digitalRead(MOVE_LEFT);
    /* read the state of the move right button */
    right_move = digitalRead(MOVE_RIGHT);
    // TODO -> matrice de incadrare
    /* read the state of the rotate left button */
    left_rotate = digitalRead(ROTATE_LEFT);
    /* read the state of the rotate right button */
    right_rotate = digitalRead(ROTATE_RIGHT);
  
    //  displayPiece();
    movePieces();
    delay(speed_pieces);

//      printMatrixSerial();
  }
}

void givePrize() {
  
}

void reset_game() {
  setup();
}

int checkIntersectionOfPiece() {
  /* declare auxiliaries */
  int x, y;
  
  for (int i = 0; i < piece_mov.size_piece; i++) {
    x = piece_mov.x[i];
    y = piece_mov.y[i];
    if (y == COLUMNS - 1)
      return true;
    else if (y >= 0 && y < COLUMNS - 1 && board[x][y + 1] == 1)
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

int checkRotate() {
  return true;
}

int matrix_rotated[ROWS][COLUMNS];

int rotateRight() {
  int found_col1 = 0, found_col2 = 0;
  int start_col1 = 0, start_col2 = COLUMNS - 1;

  while (found_col1 == 0 || found_col2 == 0) {
    for (int i = 0; i < ROWS; i++) {
      if (board[i][start_col1] == 2)
        found_col1 = 1;
      if (board[i][start_col2] == 2)
        found_col2 = 1;
    }
    if (found_col1 == 0)
      start_col1++;
    if (found_col2 == 0)
      start_col2--;
  }

  int found_row1 = 0, found_row2 = 0;
  int start_row1 = 0, start_row2 = ROWS - 1;

  while (found_row1 == 0 || found_row2 == 0) {
    for (int i = start_col1; i <= start_col2; i++) {
      if (board[start_row1][i] == 2)
        found_row1 = 1;
      if (board[start_row2][i] == 2)
        found_row2 = 1;
    }
    if (found_row1 == 0)
      start_row1++;
    if (found_row2 == 0)
      start_row2--;
  }

  for (int i = start_row1; i <= start_row1 + (start_col2 - start_col1); i++) {
    for (int j = start_col1; j <= start_col1 + (start_row2 - start_row1); j++) {
      matrix_rotated[i][j] = board[start_row1 + (j - start_col1)][start_col2 - (i - start_row1)];
    }
  }

  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLUMNS; j++) {
      if (board[i][j] == 2)
        board[i][j] = 0;
      if (matrix_rotated[i][j] == 2) {
        board[i][j] = 2;
      }
    }
  }
  
  /* reinitialize matrix and matrix rotated */
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLUMNS; j++) {
      matrix_rotated[i][j] = 0;
    }
  }

  int iterator = 0;

  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLUMNS; j++) {
      if (board[i][j] == 2) {
        piece_mov.x[iterator] = i;
        piece_mov.y[iterator] = j;
        iterator++;
      }
    }
  }

  piece_mov.size_piece = iterator;

  int auxiliary = piece_mov.horizontal;
  piece_mov.horizontal = piece_mov.vertical;
  piece_mov.vertical = auxiliary;
  return true;
}

int rotateLeft() {
  int found_col1 = 0, found_col2 = 0;
  int start_col1 = 0, start_col2 = COLUMNS - 1;

  while (found_col1 == 0 || found_col2 == 0) {
    for (int i = 0; i < ROWS; i++) {
      if (board[i][start_col1] == 2)
        found_col1 = 1;
      if (board[i][start_col2] == 2)
        found_col2 = 1;
    }
    if (found_col1 == 0)
      start_col1++;
    if (found_col2 == 0)
      start_col2--;
  }

  int found_row1 = 0, found_row2 = 0;
  int start_row1 = 0, start_row2 = ROWS - 1;

  while (found_row1 == 0 || found_row2 == 0) {
    for (int i = start_col1; i <= start_col2; i++) {
      if (board[start_row1][i] == 2)
        found_row1 = 1;
      if (board[start_row2][i] == 2)
        found_row2 = 1;
    }
    if (found_row1 == 0)
      start_row1++;
    if (found_row2 == 0)
      start_row2--;
  }

  // TODO
  // Asta trebuie modificata
  for (int i = start_row1; i <= start_row1 + (start_col2 - start_col1); i++) {
    for (int j = start_col1; j <= start_col1 + (start_row2 - start_row1); j++) {
      matrix_rotated[i][j] = board[start_row2 - (j - start_col1)][start_col2 - (i - start_row1)];
    }
  }

  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLUMNS; j++) {
      if (board[i][j] == 2)
        board[i][j] = 0;
      if (matrix_rotated[i][j] == 2) {
        board[i][j] = 2;
      }
    }
  }
  
  /* reinitialize matrix and matrix rotated */
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLUMNS; j++) {
      matrix_rotated[i][j] = 0;
    }
  }

  int iterator = 0;

  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLUMNS; j++) {
      if (board[i][j] == 2) {
        piece_mov.x[iterator] = i;
        piece_mov.y[iterator] = j;
        iterator++;
      }
    }
  }

  piece_mov.size_piece = iterator;

  int auxiliary = piece_mov.horizontal;
  piece_mov.horizontal = piece_mov.vertical;
  piece_mov.vertical = auxiliary;
  return true;
}

int checkLeftMove() {
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLUMNS; j++) {
      if (board[i][j] == 2) {
        if (i == 0)
          return false;
        if (board[i - 1][j] == 1)
          return false;
      }
    }
  }
  return true;
}

int MovePieceLeft() {
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLUMNS; j++) {
      if (board[i][j] == 2) {
        board[i - 1][j] = board[i][j];
        board[i][j] = 0;
      }
    }
  }
  for (int i = 0; i < piece_mov.size_piece; i++) {
    piece_mov.x[i]--;
  }
  return true;
}

int checkRightMove() {
  for (int i = 0; i < ROWS; i++) {
    for (int j = 0; j < COLUMNS; j++) {
      if (board[i][j] == 2) {
        if (i == ROWS - 1)
          return false;
        if (board[i + 1][j] == 1)
          return false;
      }
    }
  }
  return true;
}

int MovePieceRight() {
  for (int i = ROWS - 1; i >= 0; i--) {
    for (int j = 0; j < COLUMNS; j++) {
      if (board[i][j] == 2) {
        board[i + 1][j] = board[i][j];
        board[i][j] = 0;
      }
    }
  }
  for (int i = 0; i < piece_mov.size_piece; i++) {
    piece_mov.x[i]++;
  }
  return true;
}

/* function that erases the rows that are full */
void eraseRows() {
  int check_row;
  int i = COLUMNS - 1;

  while (i >= 0) {
    check_row = 1;
    for (int j = 0; j < ROWS; j++) {
      if (board[j][i] != 1) {
        check_row = 0;
        break;
      }
    }
    if (check_row == 1) {
      score += 1;
      for (int j = 0; j < ROWS; j++) {
        board[j][i] = 0;
      }
      for (int j = 0; j < ROWS; j++) {
        for (int k = i; k >= 1; k--) {
          board[j][k] = board[j][k - 1];
        }
      }
      for (int j = 0; j < ROWS; j++) {
        board[j][0] = 0;
      }
      i++;
    }
    i--;
  }
}

/* In this function I move the pieces */
void movePieces() {
  /* check if we can move left */
  if (left_move == 1) {
    if (checkLeftMove()) {
      MovePieceLeft();
    }
  } else if (right_move == 1) {
    if (checkRightMove()) {
      MovePieceRight();
    }
  } else if (left_rotate == 1) {
    if (checkRotate()) {
      rotateLeft();
    }
  } else if (right_rotate == 1) {
    if (checkRotate()) {
      rotateRight();
    }
  }
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
    if (piece_mov.y[i] == 0) {
      board[piece_mov.x[i]][piece_mov.y[i]] = 2;
    }
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
    if (board[i][0] == 1)
      return true;
  }
  return false;
}

/* this function replaces the first
 * matrix with the one with the piece
 */
void putPieceInMatrix() {
  int iterator = 0;

  if (checkIfLost())
    win_score = 2;

  for (int i = 0; i < piece_mov.size_piece; i++) {
    piece_mov.y[i] -= piece_mov.horizontal;
  }
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
  /* this is for constructing piece_mov */
  int iterator = 0;

  /* starting_point */
  starting_point = random(0, ROWS);
  piece_mov.x[iterator] = starting_point;
  piece_mov.y[iterator] = 0;
  iterator++;
  
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
      if (i != 0) {
        piece_mov.x[iterator] = starting_point;
        piece_mov.y[iterator] = i;
        iterator++;
      }
      if (intersection_point == i) {
        for (int j = 0; j < vertical; j++)
          if (j != 0) {
            piece_mov.x[iterator] = starting_point + j;
            piece_mov.y[iterator] = i;
            iterator++;
          }
      }
  }
  piece_mov.size_piece = iterator;
  piece_mov.horizontal = horizontal;
  piece_mov.vertical = vertical;
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
