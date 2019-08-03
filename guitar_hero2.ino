//set pins
const int buttonPressedPin = 2; //or gate
const int pausePin = 3;
const int redButtonPin = 4;
const int blueButtonPin = 5;
const int greenButtonPin = 6;
const int whiteButtonPin = 7;
const int speakerPin = 8;
const int whiteLightPin = 9;
const int redLightPin = 10;
const int blueLightPin = 11;
const int greenLightPin = 12;
const int clockPin = 13;

//create variables to be used in interrupts
volatile bool checkButtonsNow = false;
volatile bool changeLightsNow = false;
volatile bool paused = true;

//buttons states of on or off
bool whiteState = 0;
bool redState = 0;
bool blueState = 0;
bool greenState = 0;

//light states on or off
int newLights;
bool whiteLight = 0; //state the player has to match
bool whiteLight1 = 0;
bool whiteLight2 = 0;
bool whiteLight3 = 0;
bool redLight = 0; //state the player has to match
bool redLight1 = 0;
bool redLight2 = 0;
bool redLight3 = 0;
bool blueLight = 0; //state the player has to match
bool blueLight1 = 0;
bool blueLight2 = 0;
bool blueLight3 = 0;
bool greenLight = 0; //state the player has to match
bool greenLight1 = 0;
bool greenLight2 = 0;
bool greenLight3 = 0;

//tones
const int whiteTone = 196; //Note G3
const int redTone = 392; //Note G4
const int blueTone = 784; //Note G5
const int greenTone = 988; //Note B5
const int toneDuration = 200;
const int lowTone = 100;
const int highTone = 1000;
const int marioKartLowTone = 440;
const int marioKartHighTone = 880;

//game logic
bool correct = 0;
int countFirst3 = 0;
int points = 0;
int highscore = 0;

//display messages
String pointsMessage = "Points: ";
String highscoreMessage = " Highscore: ";
String displayPoints;
String displayHighscore;

//compare values
const int changeLightsTime = 15000;
volatile int pauseTime = 0;
int nowTime;
const int debounceTime = 0;
int pauseState;
volatile bool pauseButton;

void setup() {
  //disable interrupts for setup
  cli(); 

  //log to serial monitor
  Serial.begin(9600);

  //set pinModes
  pinMode(buttonPressedPin, INPUT);
  pinMode(whiteButtonPin, INPUT);
  pinMode(redButtonPin, INPUT);
  pinMode(blueButtonPin, INPUT);
  pinMode(greenButtonPin, INPUT);
  pinMode(pausePin, INPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(greenLightPin, OUTPUT);
  pinMode(blueLightPin, OUTPUT);
  pinMode(redLightPin, OUTPUT);
  pinMode(whiteLightPin, OUTPUT);

  //set pin from or gate and pin for the pause button as interrupts
  attachInterrupt(digitalPinToInterrupt(buttonPressedPin), buttonPress, RISING);
  attachInterrupt(digitalPinToInterrupt(pausePin), pause, RISING);

  //set clock prescaler
  TCCR1A = 0x0;
  TCCR1B = 0x0;
  TCCR1B |= (1 << CS12);
  TCCR1B |= (1 << CS10);
  
  //allow timer overflow interrupt
  TIMSK1 = 0x2;
  
  //timer compare value for led speed
  OCR1A = changeLightsTime;
  
  sei(); //enable interrupts
}

void loop() {
  if (changeLightsNow & !paused) { 
    playTones();
    changeLights();
  } 
  if(checkButtonsNow & !paused) { 
    correct = 0;
    checkButtons();
  }
  if(pauseButton) {
    pauseState = digitalRead(pausePin);
    if(TCNT1 < pauseTime){
      nowTime = TCNT1 + 15000;
    } else {
      nowTime = TCNT1;
    }
    if (nowTime - pauseTime > debounceTime) {
      if (digitalRead(pausePin) == pauseState){
        paused = !paused;
      }
    }
    pauseButton = false;
  }
}

//interrupt for pause button
void pause() {
  pauseButton = true;
  pauseTime = TCNT1;
}

//interupt for any of note button
void buttonPress() {
  checkButtonsNow = true;
}

//change lights when timer hits OCR1A
ISR (TIMER1_COMPA_vect) {
  changeLightsNow = true;
}

//play tones for corresponding input and compute points and highscore
void playTones() {
  if(correct){
    points++;
    if (points > highscore) {
      highscore = points;
    }
    displayPoints = pointsMessage + points;
    displayHighscore = highscoreMessage + highscore;
    Serial.print(displayPoints);
    Serial.println(displayHighscore);
    if(whiteState == 1){
      tone(speakerPin, whiteTone, toneDuration);
      delay(toneDuration);
    } 
    if (redState == 1) {
      tone(speakerPin, redTone, toneDuration);
      delay(toneDuration);
    }
    if (blueState == 1) {
      tone(speakerPin, blueTone, toneDuration);
      delay(toneDuration);
    }
    if (greenState == 1) {
      tone(speakerPin, greenTone, toneDuration);
    }
  } 
  //first three low beeps for start
  else if(countFirst3 < 3) {
    tone(speakerPin, marioKartLowTone, 500);
  }
  //high pitch to indicate start game
  else if(countFirst3 == 3) {
    tone(speakerPin, marioKartHighTone, 500); 
  }
  //tone for wrong answer
  else if(countFirst3 >3) {
    points = 0;
    for(int count = 0; count <=3; count++){  
      tone(speakerPin, highTone, toneDuration);
      delay(25);
      tone(speakerPin, lowTone, toneDuration);
      delay(25);
    }
    displayPoints = pointsMessage + points;
    displayHighscore = highscoreMessage + highscore;
    Serial.print(displayPoints);
    Serial.println(displayHighscore);
  }
}

void changeLights() {
  //send a high signal for the registers' clock
  digitalWrite(clockPin,HIGH);
  
  //keep track of first 3 rows at beginning so that user doesnt get wrong answer before game starts
  if(countFirst3 <= 3){
    countFirst3++;
  }
  
  //keep track of values shifting down lights
  whiteLight = whiteLight1;
  whiteLight1 = whiteLight2;
  whiteLight2 = whiteLight3;
  redLight = redLight1;
  redLight1 = redLight2;
  redLight2 = redLight3;
  blueLight = blueLight1;
  blueLight1 = blueLight2;
  blueLight2 = blueLight3;
  greenLight = greenLight1;
  greenLight1 = greenLight2;
  greenLight2 = greenLight3;
  
  //set incoming light states randomly to high or low
  newLights = random(1,16);
  whiteLight3 = (newLights >> 0) & 1;
  redLight3 = (newLights >> 1) & 1;
  blueLight3 = (newLights >> 2) & 1;
  greenLight3 = (newLights >> 3) & 1;

  //turn clock signal off
  digitalWrite(clockPin,LOW);
  
  //write new light3 states to board, other states are taken care of with registers on the breadboard
  digitalWrite(whiteLightPin, whiteLight3);
  digitalWrite(redLightPin, redLight3);
  digitalWrite(blueLightPin, blueLight3);
  digitalWrite(greenLightPin, greenLight3);

  //reset clock and changeLightsNow
  TCNT1 = 0;
  changeLightsNow = false;
}

//used to poll button states if interrupt is fired
void checkButtons() {
  delay(50);
    whiteState = buttonCheck(whiteButtonPin);
    redState = buttonCheck(redButtonPin);
    blueState = buttonCheck(blueButtonPin);
    greenState = buttonCheck(greenButtonPin);

    if(whiteState == whiteLight && redState == redLight && blueState == blueLight && greenState == greenLight){
      correct = 1;
    }
    else {
      correct = 0;
    }

    //reset interrupt bool checkButtonsNow
    checkButtonsNow = false;
}

//used to check state of note buttons
bool buttonCheck(int pin){
  if (digitalRead(pin) > 0){
    return true;
  }
  else {
    return false;
  }
}

