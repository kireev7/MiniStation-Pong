#include <TVout.h>

#define WHEEL_ONE_PIN 0 //analog
#define WHEEL_TWO_PIN 1 //analog

#define PADDLE_HEIGHT 14
#define PADDLE_WIDTH 1

#define RIGHT_PADDLE_X (TV.hres()-4)
#define LEFT_PADDLE_X 2

#define IN_GAMEA 0 //in game state - draw constants of the game box
#define IN_GAMEB 0 //in game state - draw the dynamic part of the game

#define GAME_OVER 2 //game over state

#define LEFT_SCORE_X (TV.hres()/2-15)
#define RIGHT_SCORE_X (TV.hres()/2+10)
#define SCORE_Y 4

#define MAX_Y_VELOCITY 8
#define PLAY_TO 7

#define LEFT 0
#define RIGHT 1

TVout TV;
unsigned char x, y;


int wheelOnePosition = 0;
int wheelTwoPosition = 0;
int rightPaddleY = 0;
int leftPaddleY = 0;
unsigned char ballX = 0;
unsigned char ballY = 0;
char ballVolX = 2;
char ballVolY = 2;

int leftPlayerScore = 0;
int rightPlayerScore = 0;

int frame = 0;
int state = IN_GAMEB;

void processInputs() {
  wheelOnePosition = analogRead(WHEEL_ONE_PIN);
  wheelTwoPosition = analogRead(WHEEL_TWO_PIN);

  delay(10);
}

void drawGameScreen() {
  //  TV.clear_screen();
  //draw right paddle
  rightPaddleY = ((wheelOnePosition / 8) * (TV.vres() - PADDLE_HEIGHT)) / 128;
  x = RIGHT_PADDLE_X;
  for (int i = 0; i < PADDLE_WIDTH; i++) {
    TV.draw_line(x + i, rightPaddleY, x + i, rightPaddleY + PADDLE_HEIGHT, 1);
  }

  //draw left paddle
  leftPaddleY = ((wheelTwoPosition / 8) * (TV.vres() - PADDLE_HEIGHT)) / 128;
  x = LEFT_PADDLE_X;
  for (int i = 0; i < PADDLE_WIDTH; i++) {
    TV.draw_line(x + i, leftPaddleY, x + i, leftPaddleY + PADDLE_HEIGHT, 1);
  }

  //draw score
  TV.print_char(LEFT_SCORE_X, SCORE_Y, '0' + leftPlayerScore);
  TV.print_char(RIGHT_SCORE_X, SCORE_Y, '0' + rightPlayerScore);
 
  //draw ball
  TV.set_pixel(ballX, ballY, 2);
}

//player == LEFT or RIGHT
void playerScored(byte player) {
  if (player == LEFT) leftPlayerScore++;
  if (player == RIGHT) rightPlayerScore++;

  //check for win
  if (leftPlayerScore == PLAY_TO || rightPlayerScore == PLAY_TO) {
    state = GAME_OVER;
  }

  ballVolX = -ballVolX;
}



void drawBox() {
  TV.clear_screen();

  //draw net
  for (int i = 1; i < TV.vres() - 4; i += 6) {
    TV.draw_line(TV.hres() / 2, i, TV.hres() / 2, i + 3, 1);
  }
  // had to make box a bit smaller to fit tv
  TV.draw_line(0, 0, 0, 95, 1 ); // left
  TV.draw_line(0, 0, 126, 0, 1 ); // top
  TV.draw_line(126, 0, 126, 95, 1 ); // right
  TV.draw_line(0, 95, 126, 95, 1 ); // bottom


  state = IN_GAMEB;
}

void setup()  {
  //Serial.begin(9600);
  x = 0;
  y = 0;
  TV.begin(_NTSC);       //for devices with only 1k sram(m168) use TV.begin(_NTSC,128,56)

  ballX = TV.hres() / 2;
  ballY = TV.vres() / 2;
}

void loop() {
  if (state == IN_GAMEA) {
    drawBox();
  }

  if (state == IN_GAMEB) {
    if (frame % 2 == 0) { //every third frame
      ballX += ballVolX;
      ballY += ballVolY;

      // change if hit top or bottom
      if (ballY <= 1 || ballY >= TV.vres() - 1)
      { ballVolY = -ballVolY;
        delay(100);
      }

      // test left side for wall hit
      if (ballVolX < 0 && ballX == LEFT_PADDLE_X + PADDLE_WIDTH - 1 && ballY >= leftPaddleY && ballY <= leftPaddleY + PADDLE_HEIGHT) {
        ballVolX = -ballVolX;
        ballVolY += 2 * ((ballY - leftPaddleY) - (PADDLE_HEIGHT / 2)) / (PADDLE_HEIGHT / 2);
        delay(100);
      }

      // test right side for wall hit
      if (ballVolX > 0 && ballX == RIGHT_PADDLE_X && ballY >= rightPaddleY && ballY <= rightPaddleY + PADDLE_HEIGHT) {
        ballVolX = -ballVolX;
        ballVolY += 2 * ((ballY - rightPaddleY) - (PADDLE_HEIGHT / 2)) / (PADDLE_HEIGHT / 2);
        delay(100);
      }

      //limit vertical speed
      if (ballVolY > MAX_Y_VELOCITY) ballVolY = MAX_Y_VELOCITY;
      if (ballVolY < -MAX_Y_VELOCITY) ballVolY = -MAX_Y_VELOCITY;

      // Scoring
      if (ballX <= 1) {
        playerScored(RIGHT);
        // sound
        delay(100);
      }
      if (ballX >= TV.hres() - 1) {
        playerScored(LEFT);
        // sound
        delay(100);
      }
    }

    drawGameScreen();
  }

  if (state == GAME_OVER) {
    drawGameScreen();
    TV.print_str(29, 25, "GAME");
    TV.print_str(68, 25, "OVER");
    
    //reset the scores
    leftPlayerScore = 0;
    rightPlayerScore = 0;
    state = IN_GAMEB;
  }


  TV.delay_frame(1);
  if (++frame == 60) frame = 0; //increment and/or reset frame counter
}
