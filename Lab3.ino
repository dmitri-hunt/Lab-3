#include "IRremote.h"
#include <Wire.h>
#include <DS3231.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

DS3231 clock;
RTCDateTime dt;

#define ENABLE 5
#define DIRA 3
#define DIRB 4

int receiver = 6; // Signal Pin of IR receiver to Arduino Digital Pin 6
int motor_state = 1;
int motor_on = 0;
volatile int second_flag = 0;
/*-----( Declare objects )-----*/
IRrecv irrecv(receiver);     // create instance of 'irrecv'
decode_results results;      // create instance of 'decode_results'



ISR(TIMER1_COMPA_vect){//timer1 interrupt 1Hz toggles pin 13 (LED)
//generates pulse wave of frequency 1Hz/2 = 0.5kHz (takes two cycles for full wave- toggle high then toggle low)
  second_flag = 1;
}

void update_lcd(){
  lcd.setCursor(0,0);
  if(motor_on){
    switch(motor_state){
      case 1:
        lcd.print("CC 1/4  ");
        break;
      case 2:
        lcd.print("CC 1/2  ");
        break;
      case 3:
        lcd.print("CC 3/4  ");
        break;
      case 4:
        lcd.print("CC Full");
        break;
      case 5:
        lcd.print("C 1/4  ");
        break;
      case 6:
        lcd.print("C 1/2  ");
        break;
      case 7:
        lcd.print("C 3/4  ");
        break;
      case 8:
        lcd.print("C Full  ");
        break;
    }
  }
  else{
    lcd.print("OFF     ");
  }
  dt = clock.getDateTime();
  if(dt.hour<10){
    lcd.setCursor(0,1);
    lcd.print("0");
    lcd.setCursor(1,1);
    lcd.print(dt.hour);
  }
  else{
    lcd.setCursor(0,1);
    lcd.print(dt.hour);
  }
  lcd.setCursor(2,1);
  lcd.print(":");
  if(dt.minute<10){
    lcd.setCursor(3,1);
    lcd.print("0");
    lcd.setCursor(4,1);
    lcd.print(dt.minute);
  }
  else{
    lcd.setCursor(3,1);
    lcd.print(dt.minute);
  }
  lcd.setCursor(5,1);
  lcd.print(":");
  if(dt.second<10){
    lcd.setCursor(6,1);
    lcd.print("0");
    lcd.setCursor(7,1);
    lcd.print(dt.second);
  }
  else{
    lcd.setCursor(6,1);
    lcd.print(dt.second);
  }

}
/*-----( Function )-----*/
void translateIR() // takes action based on IR code received

// describing Remote IR codes 

{

  switch(results.value)

  {
  case 0xFFA25D:
    motor_on=!motor_on;
    break;
  
  case 0xFF629D: 
    if(((motor_state < 4) && (motor_state > 0))||((motor_state < 8) && (motor_state > 4))){
      motor_state ++;
    }
    else{
      motor_state = motor_state - 3;
    }
    break;
    
  case 0xFF22DD: 
    if(motor_state > 4){
      motor_state = motor_state - 4;
    }
    break;
    
  case 0xFFC23D:
    if(motor_state < 5){
      motor_state = motor_state + 4; 
    }
    break;
    
  case 0xFFA857: 
    if(((motor_state < 5) && (motor_state > 1))||(motor_state > 5)){
      motor_state --;
    }
    break;
    
  delay(100); // Do not get immediate repeat


} //END translateIR

void run_motor(){
  if(motor_on){
    switch(motor_state){
    case 1:
      digitalWrite(ENABLE, HIGH);
      digitalWrite(DIRB, LOW);
      analogWrite(DIRA, 100);
      break;
    case 2:
      digitalWrite(ENABLE, HIGH);
      digitalWrite(DIRB, LOW);
      analogWrite(DIRA,150);
      break;
    case 3:
      digitalWrite(ENABLE, HIGH);
      digitalWrite(DIRB, LOW);
      analogWrite(DIRA, 200);
      break;
    case 4:
      digitalWrite(ENABLE, HIGH);
      digitalWrite(DIRB, LOW);
      analogWrite(DIRA, 250);
      break;
    case 5:
      digitalWrite(ENABLE, HIGH);
      digitalWrite(DIRA, LOW);
      analogWrite(DIRB, 100);
      break;
    case 6:
      digitalWrite(ENABLE, HIGH);
      digitalWrite(DIRA, LOW);
      analogWrite(DIRB, 150);
      break;
    case 7:
      digitalWrite(ENABLE, HIGH);
      digitalWrite(DIRA, LOW);
      analogWrite(DIRB, 200);
      break;
    case 8:
      digitalWrite(ENABLE, HIGH);
      digitalWrite(DIRA, LOW);
      analogWrite(DIRB, 250);
      break;
    }
  }
  else{
    digitalWrite(ENABLE, LOW);
  }
}

void setup() {
  cli();//stop interrupts

  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();//allow interrupts
  
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("IR Receiver Button Decode"); 
  irrecv.enableIRIn(); // Start the receiver
  //---set pin direction
  pinMode(ENABLE,OUTPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(DIRB,OUTPUT);

  clock.begin();
  clock.setDateTime(__DATE__, __TIME__);

  lcd.begin(16, 2);
}

void loop() {
  // put your main code here, to run repeatedly:  
  run_motor();
  if(second_flag){
    update_lcd();
    second_flag = 0;
  }
  if (irrecv.decode(&results)) // have we received an IR signal?
  {
    translateIR(); 
    irrecv.resume(); // receive the next value
  }
}
