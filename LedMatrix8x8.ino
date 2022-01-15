 #include <Wire.h> 
 #include <LiquidCrystal_I2C.h>
 #include "pitches.h" 

 LiquidCrystal_I2C lcd(0x27,16,2);
 
 unsigned char i;
 unsigned char j; 
 unsigned long tinit;
/*Port Definitions*/
  int Max7219_pinCLK = 10;
  int Max7219_pinCS = 9;
  int Max7219_pinDIN = 8;

 int melody[] = {NOTE_D3, NOTE_CS3, NOTE_C3};
 int noteDurations[] = {4, 4, 1};
 
 unsigned char fail[8] = {0x00,0x00,0x24,0x00,0x18,0x24,0x00,0x00};
 volatile int food[2];
 volatile int initial = 0;
 volatile int snakeLength = 1;
 volatile int snake[64][2];
 volatile int movement[2];
 volatile unsigned char gameDisplay[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
 volatile int score = 0;
 volatile int sound = 0;

 int VRx = A0;
 int VRy = A1;
 int SW = 2;

int xPosition = 0;
int yPosition = 0;
int SW_state = 0;
int mapX = 0;
int mapY = 0;
 
void Write_Max7219_byte(unsigned char DATA) 
{   
    digitalWrite(Max7219_pinCS,LOW);    
    for(int i = 8; i >= 1; i--){     
      digitalWrite(Max7219_pinCLK,LOW);
      digitalWrite(Max7219_pinDIN,DATA&0x80);// Extracting a bit data
      DATA = DATA<<1;
      digitalWrite(Max7219_pinCLK,HIGH);
      }                             
} 
 
 
void Write_Max7219(unsigned char address,unsigned char dat)
{
    digitalWrite(Max7219_pinCS,LOW);
    Write_Max7219_byte(address);           //address，code of LED
    Write_Max7219_byte(dat);               //data，figure on LED 
    digitalWrite(Max7219_pinCS,HIGH);
}
 
void Init_MAX7219(void)
{
 Write_Max7219(0x09, 0x00);       //decoding ：BCD
 Write_Max7219(0x0a, 0x03);       //brightness 
 Write_Max7219(0x0b, 0x07);       //scanlimit；8 LEDs
 Write_Max7219(0x0c, 0x01);       //power-down mode：0，normal mode：1
 Write_Max7219(0x0f, 0x00);       //test display：1；EOT，display：0
} 
 
void setup()
{
  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(3,0);
  lcd.print("SNAKE GAME");
  lcd.setCursor(3,1);
  lcd.print("score = ");
  lcd.setCursor(12,1);
  lcd.print(score);
 
  pinMode(Max7219_pinCLK,OUTPUT);
  pinMode(Max7219_pinCS,OUTPUT);
  pinMode(Max7219_pinDIN,OUTPUT);
  delay(50);
  Init_MAX7219();

  Serial.begin(9600); 
  
  pinMode(VRx, INPUT);
  pinMode(VRy, INPUT);
  pinMode(SW, INPUT_PULLUP);
  randomSeed(analogRead(0));
}
 
 
void loop()
{ 
  xPosition = analogRead(VRx);
  yPosition = analogRead(VRy);
  SW_state = digitalRead(SW);
  mapX = map(xPosition, 0, 1023, -505, 518);
  mapY = map(yPosition, 0, 1023, -494, 529);
  
  if(!isFail()){
    if(initial == 0){
    generateFood();
    generateInitialSnakeDot();
    initial++;
    }

  if(isGoal()){
    score++;
    lcd.setCursor(12,1);
    lcd.print(score);
    
    generateFood();
    snakeLength++;
    snake[snakeLength - 1][0] = -1;
    snake[snakeLength - 1][1] = -1; 
  }    
    
  if(mapX > 0)
    {generateMoveRight();}
  else if (mapX < 0)
      {generateMoveLeft();}
    else if (mapY > 0)
      {generateMoveDown();}
      else if (mapY < 0)
        {generateMoveUp();}
  makeMove();
  displayGame();
  }
  else{
    lcd.setCursor(3,0);
    lcd.print(" GAME OVER");
    displayFail();
    /*if(sound == 0){
     soundFail();
     sound++; 
    }*/
    if(SW_state == 0)
     restartGame();      
  }
}

unsigned char intToDisplayable(int row){
  switch (row) {
  case 0:
    return 0x80;
  case 1:
    return 0x40;
  case 2:
    return 0x20;
  case 3:
    return 0x10;
  case 4:
    return 0x08;
  case 5:
    return 0x04;
  case 6:
    return 0x02;
  case 7:
    return 0x01;
  default:
    return 0x00;
}
}

void generateFood(){
  food[0] = random(0, 7);
  food[1] = random(0, 7);
}

void generateInitialSnakeDot(){
  snake[0][0] = 7;
  snake[0][1] = 3;
}

void restartGame(){
  score = 0;
  snakeLength = 1;
  generateFood();
  generateInitialSnakeDot();

  lcd.setCursor(3,0);
  lcd.print("SNAKE GAME");
  lcd.setCursor(3,1);
  lcd.print("score = ");
  lcd.setCursor(12,1);
  lcd.print(score);

  sound = 0;
}

void clearGameDisplay(){
  for(int i = 0; i < 8; i++)
    gameDisplay[i] = 0;
}

void calculateDisplay(){
  clearGameDisplay();  
  for(int i = 0; i < snakeLength; i++){
    gameDisplay[snake[i][0]] += intToDisplayable(snake[i][1]);
  }
  if(!isSnakeOverFood())
    gameDisplay[food[0]] += intToDisplayable(food[1]);
}

boolean isSnakeOverFood(){
  for(int i = 0; i < snakeLength; i++)
    if(snake[i][0] == food[0] && snake[i][1] == food[1])
      return true;
  return false;
}

void displayGame(){
  calculateDisplay();
  tinit = millis();
   while(millis()-tinit < 300){
    for(int i = 0; i <= 7; i++)
      Write_Max7219(intToDisplayable(i),gameDisplay[i]);
   }
}

boolean isGoal(){
  if(food[0] == snake[0][0] && food[1] == snake[0][1])
    return true;
  return false;
}

boolean isFail(){
  if(snake[0][0] < 0 || snake[0][0] > 7 || snake[0][1] < 0 || snake[0][1] > 7)
    return true;
  for(int i = snakeLength - 1; i > 0; i--)
    if(snake[i][0] == snake[0][0] && snake[i][1] == snake[0][1])
      return true;
  return false;
}

void displayFail(){
  for(i=0;i<=7;i++)
    Write_Max7219(intToDisplayable(i),fail[i]);
}

void soundFail(){
   for (int thisNote = 0; thisNote < 3; thisNote++) {
      int noteDuration = 1000/noteDurations[thisNote];
      tone(7, melody[thisNote],noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(7);
  }
}

void moveSnake(){
  for(int i = snakeLength - 1; i > 0; i--){
    snake[i][0] = snake[i - 1][0];
    snake[i][1] = snake[i - 1][1];    
  }
}

void makeMove(){
  if(!((snake[0][0] + movement[0]) == snake[1][0] && (snake[0][1] + movement[1]) == snake[1][1])){
    moveSnake();
    snake[0][0] += movement[0];
    snake[0][1] += movement[1];
  }
  else{
    moveSnake();
    snake[0][0] -= movement[0];
    snake[0][1] -= movement[1];
  }
}

void generateMoveUp(){
    movement[0] = -1;
    movement[1] = 0;
}

void generateMoveDown(){
    movement[0] = 1;
    movement[1] = 0;
}

void generateMoveLeft(){
    movement[1] = -1;
    movement[0] = 0;
}

void generateMoveRight(){
    movement[1] = 1;
    movement[0] = 0;
}
