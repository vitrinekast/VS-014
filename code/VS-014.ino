/*****************************************************************************************************************************
  echoTrek - 8Bit Digital Delay / LO-FI Bitcrusher / Reverse Speech DSP Pedal Effects  for Guitar, Voice, Synths, etc.
  Works with Arduino UNO R3 / NANO / PRO MINI  - See the schematics for wiring details. By J. CesarSound - ver 1.0 - Jan/2021.
  
  Accessed From: https://projecthub.arduino.cc/CesarSound/c4f2fdea-9035-44a8-9e2c-60ea6ff38159
******************************************************************************************************************************/

#define audio_in A0
#define repeat_selector 9
#define rev_selector 10
#define delay_time_in A7

#define gran_len 100;
const unsigned int d_size = 1900;  //Delay memory buffer size
unsigned int val, d_val, d_time;
int i, j;
byte count = 2;
bool rev = 0;
bool gran = 0;
bool repeat = 0;
int granCount = 0;
char delay_data[d_size + 1] = { NULL };    //Delay  memory buffer
char delay_data_1[d_size + 1] = { NULL };  //Delay memory buffer


void setup() {
  pinMode(repeat_selector, INPUT_PULLUP);
  pinMode(rev_selector, INPUT_PULLUP);

  InitTimer1();               //Set up timer 1 for 16.384kHz
  setPwmFrequency(5, 1);      //function for setting PWM High frequency 62.475kHz on  pin D5
  analogReference(INTERNAL);  //Use 1.1v aref voltage.
  

  d_time = d_size;
}

void loop () {
  
}
void sampling() {
  d_time = map(analogRead(delay_time_in), 0, 1100, 0, d_size);
  val = map(analogRead(audio_in), 0, 900, 0, 255);
  if (digitalRead(rev_selector) == HIGH) {
    rev = 0;
  } else {
    rev = 1;
  }

  if (digitalRead(repeat_selector) == HIGH) {
    repeat = 0;
  } else {
    repeat = 1;
  }
  delay_sound();
  analogWrite(5, d_val);
}

void delay_sound() {
  if(gran) granCount++;
  
  if(gran && granCount > gran_len) {
    i = random(0, d_size - gran_len);
    granCount = 0;
  }

  i = i + 1;
  if (i > d_time) i = 0;
  delay_data[i] = val;

  if (i == d_time) j = 0;
  if (!repeat) delay_data_1[i] = delay_data[i];
  else {
    if (delay_data_1[i] < delay_data[i]) {
      delay_data_1[i] = delay_data[i];
    }
  }
  j = j + 1;
  if (j > d_time) j = 0;
  if (!rev) d_val = delay_data_1[j];
  if (rev) d_val = delay_data_1[d_time - j];
}


//Set  up timer 1 for 16.384kHz (975)
void InitTimer1() {
  cli();        //Disable global  interrupts
  TCCR1A = 0;   //Reset Timer 1 Counter Control Register A
  TCCR1B = 0;   //Reset Timer 1 Counter Control Register B
  TCNT1 = 0;    //initialize counter  value to 0
  OCR1A = 975;  //Set Timer 1 to desired frequency: 16384Hz (16,000,000  / 16384) - 1 = 975
  //Turn on CTC (clear timer on compare match) mode:
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS10);    //CS10 Prescalar = 1 (no prescalar  used); CS11 Prescalar = 8
  TIMSK1 |= (1 << OCIE1A);  //Enable timer interrupt
  sei();                    //Enable interrupts
}

ISR(TIMER1_COMPA_vect) {
  sampling();
}

//PWM  62.475kHz high frequency DAC for pins 5 & 6 (others are 32k)
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if (pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if (pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if (pin == 3 || pin == 11) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x07; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}