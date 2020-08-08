// HDSP2000 Display
// Referencing: 
// 1. https://www.freetronics.com.au/blogs/news/8494795-controlling-vintage-hdsp2000-displays-with-arduino#.XyOZC8BS-M-
// 2. https://hackaday.io/project/167123-retro-time-dot-matrix-led-clocks 

//Basic bit manipulation macros/defines
#define bit_set(p, m) ((p) |= (m))
#define bit_clear(p, m) ((p) &= ~(m))
#define bit_flip(p, m)  ((p) ^= (m))
#define bit_get(p, m) ((p) & (m))

#define clock_set   bit_set(PORTB, 1 << PB1)
#define clock_clear bit_clear(PORTB, 1 << PB1)
#define data_set    bit_set(PORTB, 1 << PB0)
#define data_clear  bit_clear(PORTB, 1 << PB0)
#define data_write(n) (n)?(data_set):(data_clear)

#define column_set(c)   bit_set(PORTD, 1 << (PD2 + c))
#define column_clear(c) bit_clear(PORTD, 1 << (PD2 + c))
#define column_write(c, n) (n)?(column_set(c)):(column_clear(c))

struct CharMap
{
  char c;
  int v1;
  byte b1;
  byte b2;
  byte b3;
  byte b4;
  byte b5;
};
const int cmap_len = 95;

#define DISPLAY_CHARS 8
#define MAX_CHARS 32

volatile char display_chars[MAX_CHARS];     // The current character buffer to display
volatile int  display_length = 0;           // The length of the character string
volatile int  display_curr_char = 0;        // Current character displayed on the left
volatile int  display_curr_column = 0;      // Current column flashed out, 0-4
volatile unsigned long display_scroll_speed = 200;  // Millis to scroll the display
volatile unsigned long display_last_scroll = 0; // last millis() scrolled.
volatile bool display_scroll_complete = false;

// This is where the information on each character is
// character s can be adjusted by changing the HEX codes
// data is read out in 5 columns according to 1s and 0s

struct CharMap cmap[] = {
  {' ', 32, 0x00,  0x00,  0x00,  0x00,  0x00},
  {'!', 33, 0x00,  0x00,  0xF2,  0x00,  0x00},
  {'"', 34, 0x00,  0xE0,  0x00,  0xE0,  0x00},   //removed as confuses arduino IDE
  {'#', 35, 0x28,  0xFE,  0x28,  0xFE,  0x28},
  {'$', 36, 0x24,  0x54,  0xFE,  0x54,  0x48},
  {'%', 37, 0xC4,  0xC8,  0x10,  0x26,  0x46},
  {'&', 39, 0x6C,  0x92,  0xAA,  0x44,  0x0A},
  {'\'',39, 0x00,  0xA0,  0xC0,  0x00,  0x00},  //removed as confuses arduino IDE
  {'(', 40, 0x00,  0x38,  0x44,  0x82,  0x00},
  {')', 41, 0x00,  0x82,  0x44,  0x38,  0x00},
  {'*', 42, 0x28,  0x10,  0x7C,  0x10,  0x28},
  {'+', 43, 0x10,  0x10,  0x7C,  0x10,  0x10},
  {',', 44, 0x00,  0x0A,  0x0C,  0x00,  0x00},
  {'-', 45, 0x10,  0x10,  0x10,  0x10,  0x10},
  {'.', 46, 0x00,  0x06,  0x06,  0x00,  0x00},
  {'/', 47, 0x04,  0x08,  0x10,  0x20,  0x40},   //removed as confuses arduino IDE
  
  //default font

  {'0', 48, 0x7C,  0x8A,  0x92,  0xA2,  0x7C},
  {'1', 49, 0x00,  0x42,  0xFE,  0x02,  0x00},
  {'2', 50, 0x42,  0x86,  0x8A,  0x92,  0x62},
  {'3', 51, 0x84,  0x82,  0xA2,  0xD2,  0x8C},
  {'4', 52, 0x18,  0x28,  0x48,  0xFE,  0x08},
  {'5', 53, 0xE4,  0xA2,  0xA2,  0xA2,  0x9C},
  {'6', 54, 0x3C,  0x52,  0x92,  0x92,  0x0C},
  {'7', 55, 0x80,  0x8E,  0x90,  0xA0,  0xC0},
  {'8', 56, 0x6C,  0x92,  0x92,  0x92,  0x6C},
  {'9', 57, 0x60,  0x92,  0x92,  0x94,  0x78},

  /*
  // dice font
  {'0', 48, 0xFE, 0x82, 0x82, 0x82, 0xFE}, //  0
  {'1', 49, 0x00, 0x00, 0x10, 0x00, 0x00},  //  1
  {'2', 50, 0x00, 0x00, 0x44, 0x00, 0x00}, //  2
  {'3', 51, 0x02, 0x00, 0x10, 0x00, 0x80},   //  3
  {'4', 52, 0x82, 0x00, 0x00, 0x00, 0x82},   //  4
  {'5', 53, 0x82, 0x00, 0x10, 0x00, 0x82},   //  5
  {'6', 54, 0x92, 0x00, 0x00, 0x00, 0x92},   //  6
  {'7', 55, 0x8A, 0x00, 0x20, 0x00, 0x8A}, //  7
  {'8', 56, 0x92, 0x00, 0x24, 0x00, 0x92},   //  8
  {'9', 57, 0x92, 0x00, 0x92, 0x00, 0x92}, //  9

  //old school font
  {'0', 48, 0xFE, 0x82, 0x82, 0x82, 0xFE}, //  0
  {'1', 49, 0x00, 0x00, 0xFE, 0x00, 0x00},  //  1
  {'2', 50, 0x9E, 0x92, 0x92, 0x92, 0xF2},   //  2
  {'3', 51, 0xC6, 0x82, 0x92, 0x92, 0xEE},   //  3
  {'4', 52, 0xF0, 0x10, 0x10, 0x7E, 0x10},   //  4
  {'5', 53, 0xF2, 0x92, 0x92, 0x92, 0x9E},  //  5
  {'6', 54, 0xFE, 0x92, 0x92, 0x92, 0x9E},   //  6
  {'7', 55, 0x80, 0x80, 0x80, 0x80, 0xFE},  //  7
  {'8', 56, 0xEE, 0x92, 0x92, 0x92, 0xEE},   //  8
  {'9', 57, 0xF2, 0x92, 0x92, 0x92, 0xFE},  //  9

  // robot font
  {'0', 48, 0xFE, 0xFE, 0x82, 0xFE, 0xFE},  //  0
  {'1', 49, 0x00, 0x80, 0xFE, 0xFE, 0x00},  //  1
  {'2', 50, 0xDE, 0x9E, 0x92, 0xF6, 0xF6},   //  2
  {'3', 51, 0xC6, 0x82, 0x92, 0xFE, 0xEE},   //  3
  {'4', 52, 0xF0, 0x30, 0x10, 0xFE, 0xFE},  //  4
  {'5', 53, 0xF6, 0xB6, 0x92, 0xDE, 0xDE},  //  5
  {'6', 54, 0xFE, 0xBE, 0x92, 0xDE, 0xDE},  //  6
  {'7', 55, 0xC0, 0x80, 0x80, 0xFE, 0xFE},   //  7
  {'8', 56, 0xEE, 0xBE, 0x92, 0xFE, 0xEE},  //  8
  {'9', 57, 0xF6, 0xB6, 0x92, 0xFE, 0xFE},  //  9

  //Commodore -64 font
  {'0', 48, 0x7C, 0xFE, 0x92, 0xA2, 0x7C}, //0
  {'1', 49, 0x02, 0x22, 0xFE, 0xFE, 0x02},  //  1
  {'2', 50, 0x46, 0x8E, 0x9A, 0xF2, 0x62},  //  2
  {'3', 51, 0x82, 0x92, 0x92, 0xFE, 0x7C},  //  3
  {'4', 52, 0x18, 0x28, 0xFE, 0xFE, 0x08},  //  4
  {'5', 53, 0xE4, 0xF6, 0x92, 0x9E, 0x8C},  //  5
  {'6', 54, 0x7C, 0xFE, 0x92, 0xD2, 0x4C},  //  6
  {'7', 55, 0xC0, 0xCE, 0x9E, 0xB0, 0xE0},  //  7
  {'8', 56, 0x6C, 0xFE, 0x92, 0x92, 0x6C},  //  8
  {'9', 57, 0x64, 0x92, 0x92, 0xFE, 0x7C},  //  9
  */
  
  {':', 58, 0x10,  0x10,  0x10,  0x10,  0x10}, //modified
  {';', 59, 0x00,  0x6A,  0x6C,  0x00,  0x00},
  {'<', 60, 0x10,  0x28,  0x44,  0x82,  0x00},
  {'=', 61, 0x28,  0x28,  0x28,  0x28,  0x28},
  {'>', 62, 0x00,  0x82,  0x44,  0x28,  0x10},
  {'?', 63, 0x40,  0x80,  0x8A,  0x90,  0x60},
  {'@', 64, 0x4C,  0x92,  0x9E,  0x82,  0x7C},

  {'A', 65, 0x7E,  0x88,  0x88,  0x88,  0x7E},
  {'B', 66, 0xFE,  0x92,  0x92,  0x92,  0x6C},
  {'C', 67, 0x7C,  0x82,  0x82,  0x82,  0x44},
  {'D', 68, 0xFE,  0x82,  0x82,  0x44,  0x38},
  {'E', 69, 0xFE,  0x92,  0x92,  0x92,  0x82},
  {'F', 70, 0xFE,  0x90,  0x90,  0x90,  0x80},
  {'G', 71, 0x7C,  0x82,  0x92,  0x92,  0x5E},
  {'H', 72, 0xFE,  0x10,  0x10,  0x10,  0xFE},
  {'I', 73, 0x00,  0x82,  0xFE,  0x82,  0x00},
  {'J', 74, 0x04,  0x02,  0x82,  0xFC,  0x80},
  {'K', 75, 0xFE,  0x10,  0x28,  0x44,  0x82},
  {'L', 76, 0xFE,  0x02,  0x02,  0x02,  0x02},
  {'M', 77, 0xFE,  0x40,  0x30,  0x40,  0xFE},
  {'N', 78, 0xFE,  0x20,  0x10,  0x08,  0xFE},
  {'O', 79, 0x7C,  0x82,  0x82,  0x82,  0x7C},
  {'P', 80, 0xFE,  0x90,  0x90,  0x90,  0x60},
  {'Q', 81, 0x7C,  0x82,  0x8A,  0x84,  0x7A},
  {'R', 82, 0xFE,  0x90,  0x98,  0x94,  0x62},
  {'S', 83, 0x62,  0x92,  0x92,  0x92,  0x8C},
  {'T', 84, 0x80,  0x80,  0xFE,  0x80,  0x80},
  {'U', 85, 0xFC,  0x02,  0x02,  0x02,  0xFC},
  {'V', 86, 0xF8,  0x04,  0x02,  0x04,  0xF8},
  {'W', 87, 0xFC,  0x02,  0x0C,  0x02,  0xFC},
  {'X', 88, 0xC6,  0x28,  0x10,  0x28,  0xC6},
  {'Y', 89, 0xE0,  0x10,  0x0E,  0x10,  0xE0},
  {'Z', 90, 0x86,  0x8A,  0x92,  0xA2,  0xC2},
  {'[', 91, 0x00,  0xFE,  0x82,  0x82,  0x00},
  {'\\',92, 0x40,  0x20,  0x10,  0x08,  0x04},   //removed as confuses arduino IDE
  {']', 93, 0x00,  0x82,  0x82,  0xFE,  0x00},
  {'^', 94, 0x20,  0x40,  0x80,  0x40,  0x20},
  {'_', 95, 0x02,  0x02,  0x02,  0x02,  0x02},
  {'`', 96, 0x00,  0x80,  0x40,  0x20,  0x00},   //removed as confuses arduino IDE
  {'a', 97, 0x04,  0x2A,  0x2A,  0x2A,  0x1E},
  {'b', 98, 0xFE,  0x12,  0x12,  0x12,  0x0C},
  {'c', 99, 0x1C,  0x22,  0x22,  0x22,  0x22},
  {'d', 100, 0x0C,  0x12,  0x12,  0x12,  0xFE},
  {'e', 101, 0x1C,  0x2A,  0x2A,  0x2A,  0x1A},
  {'f', 102, 0x00,  0x10,  0x7E,  0x90,  0x40},
  {'g', 103, 0x12,  0x2A,  0x2A,  0x2A,  0x3C},
  {'h', 104, 0xFE,  0x10,  0x10,  0x10,  0x0E},
  {'i', 105, 0x00,  0x00,  0x5E,  0x00,  0x00},
  {'j', 106, 0x04,  0x02,  0x02,  0xBC,  0x00},
  {'k', 107, 0x00,  0xFE,  0x08,  0x14,  0x22},
  {'l', 108, 0x00,  0x82,  0xFE,  0x02,  0x00},
  {'m', 109, 0x3E,  0x20,  0x1C,  0x20,  0x3E},
  {'n', 110, 0x3E,  0x10,  0x20,  0x20,  0x1E},
  {'o', 111, 0x1C,  0x22,  0x22,  0x22,  0x1C},
  {'p', 112, 0x3E,  0x28,  0x28,  0x28,  0x10},
  {'q', 113, 0x10,  0x28,  0x28,  0x28,  0x3E},
  {'r', 114, 0x3E,  0x10,  0x20,  0x20,  0x10},
  {'s', 115, 0x12,  0x2A,  0x2A,  0x2A,  0x24},
  {'t', 116, 0x20,  0x20,  0xFC,  0x22,  0x24},
  {'u', 117, 0x3C,  0x02,  0x02,  0x02,  0x3C},
  {'v', 118, 0x38,  0x04,  0x02,  0x04,  0x38},
  {'w', 119, 0x3C,  0x02,  0x0C,  0x02,  0x3C},
  {'x', 120, 0x22,  0x14,  0x08,  0x14,  0x22},
  {'y', 121, 0x20,  0x12,  0x0C,  0x10,  0x20},
  {'z', 122, 0x22,  0x26,  0x2A,  0x32,  0x22},
  {'{', 123, 0x00,  0x10,  0x6C,  0x82,  0x82},
  {'|', 124, 0x00,  0x00,  0xEE,  0x00,  0x00},
  {'}', 125, 0x82,  0x82,  0x6C,  0x10,  0x00},
  {'~', 126, 0x20,  0x40,  0x40,  0x40,  0x80}
};

int column[]  = {2, 3, 4, 5, 6};
int data_pin  = 8;
int clock_pin = 9;


void setup() {
  Serial.begin(9600);
  for (int i = 0; i < 5; i++) {
    pinMode(column[i], OUTPUT);
  }
  for (int i = 0; i < 5; i++) {
    digitalWrite(column[i], LOW);
  }
  pinMode(data_pin, OUTPUT);
  pinMode(clock_pin, OUTPUT);
  digitalWrite(data_pin, LOW);
  digitalWrite(clock_pin, HIGH);

  SetupTimer();
}

void SetupTimer()
{
  cli(); // Stop global interrupt
  
  TCCR1A = 0; // Timer/Counter Control Register 1 A
  TCCR1B = 0; // Timer/Counter Control Register 1 B
  
  TCCR1B |= (1 << WGM12); // Configure Timer 1 for CTC mode
  TIMSK1 |= (1 << OCIE1A); // Tell CTC to call interrupt routine in addition to setting CTC flag

  // CS12  CS11  CS10  Description
  //  0     0     1    clk / 1
  //  0     1     0    clk / 8
  //  0     1     1    clk / 64
  //  1     0     0    clk / 256
  //  1     1     0    clk / 1024
  OCR1A = 125; // CTC Compare value
  TCCR1B |= (1 << CS12); // CS10, CS11, CS12 bits control prescaler. 
  // 1. With 16 MHz and clock divisor of 64, Timer 1 increments 62,500 times/sec
  // 2. With CTC = 250, Timer 1 reaches CTC ~1000 times/sec
  
  sei(); // enable global interrupt again
}

void loop() {
  //ShowWordForMillis('1', '2', '3', '4', '5', '6', '7', '8', 2000);

  //unsigned long tmp = millis();
  //Serial.println(tmp);

  if (Serial.available()) {
    ShowTextFor(Serial.readString(), 10000);
  }
  
  ShowTextFor("How are you?", 5000);
  // delay(5000);
  
  ShowTextFor("Hello", 2000);
  // delay(2000);
}

void ShowWordForMillis(char c8, char c7, char c6, char c5, char c4, char c3, char c2, char c1, unsigned long mil) {
  unsigned long end_time;
  end_time = millis() + mil;
  
  while (end_time > millis()) {
    ShowWord (c8, c7, c6, c5, c4, c3, c2, c1);
  }
}


void ShowWord(char c8, char c7, char c6, char c5, char c4, char c3, char c2, char c1) {
  char chars[8] = {c1,c2,c3,c4,c5,c6,c7,c8};
  boolean thisBit;

  // for each column
  //Serial.println("Starting to look for data");
  for (int i = 0; i < 5; i++)    // 5 columns
  {
    // look up column i(0) for char 1
    byte thisCharData;

    for (int thisChar = 0; thisChar < 8; thisChar++) {
      thisCharData = getColumnByte(chars[thisChar], i);
      // write 8 values
      for (int j = 1; j < 8; j++) {
        digitalWrite(clock_pin, HIGH);
        //Serial.println(thisCharData & (1<<j), BIN);
        thisBit = (thisCharData & (1 << j));
        //Serial.print(thisBit, BIN);
        digitalWrite(data_pin, thisBit); // first binary value character 8, reads for right to left, binary from left to right
        digitalWrite(clock_pin, LOW);
      }
    }

    digitalWrite(column[i], HIGH);
    delayMicroseconds(2000);
    digitalWrite(column[i], LOW);
    //Serial.print(i);
  }

}

byte getColumnByte(char c, int k)
{
  byte chardata ;
  //Serial.print ("the letter is - ");
  //Serial.println (c);
  //Serial.print ("the position is - ");
  //Serial.println (k);
  for (int i = 0 ; i < cmap_len ; i++) {
    //Serial.print (" i= ");
    //Serial.print (i);
    //Serial.print (" char = ");
    //Serial.println (cmap[i].c);
    if (c == cmap[i].c) {
      switch (k) {
        case 0:
          chardata = cmap[i].b1;
          break;
        case 1:
          chardata = cmap[i].b2;
          break;
        case 2:
          chardata = cmap[i].b3;
          break;
        case 3:
          chardata = cmap[i].b4;
          break;
        case 4:
          chardata = cmap[i].b5;
          break;
      }

      //Serial.print("found it - ");
      //Serial.println(chardata, BIN);
      break;
    }
  }
  //Serial.print ( "char data = ");
  //Serial.println (chardata, BIN);
  return chardata;
}

void ShowTextInterrupt(String tmpString) {
  // Set to the smaller of input string or buffer size
  display_length = min(tmpString.length(), MAX_CHARS);

  for (int i=0; i<display_length; i++) {
    display_chars[i] = tmpString.charAt(i);
  }

  display_curr_char = 0;
  display_curr_column = 0;
  display_last_scroll = millis();
}

void ShowTextFor(String tmpString, unsigned long duration) {
  ShowTextInterrupt(tmpString);

  delay(duration);

  // Wait until the scrolling is complete.
  while (!IsDisplayScrollComplete()) {}
}

bool IsDisplayScrollComplete() {
  if (display_length < DISPLAY_CHARS) {
    return true;
  } else {
    // Check if until it has completed scrolling
    return display_scroll_complete;
  }
}


// ISR(TIM1_COMPA_vect) // ATTiny84 format
ISR(TIMER1_COMPA_vect)
{
  // Push out whatever is in the data for one level
  RefreshDisplay();
}


void RefreshDisplay() {
  boolean thisBit;
  int display_prev_column = display_curr_column;

  // Check if we need to scroll by 1 character
  if ((millis() - display_last_scroll) > display_scroll_speed) {
    display_curr_char = display_curr_char+1;
    // Flag as not complete
    display_scroll_complete = false;

    // Last character has scrolled off screen
    if (display_curr_char >= display_length) {
      // Wraparound to allow text to scroll in from the right
      display_curr_char = -DISPLAY_CHARS;
      // Flag as complete, allow for next text
      display_scroll_complete = true;
    }
    
    display_last_scroll = millis();
  }

  // Check if the current text string fits in the display
  if (display_length <= DISPLAY_CHARS) {
    // show all, no need to scroll
    display_curr_char = 0;
  } else {
    // Use the current char
  }

  // Start with deactivating the previous column
  //digitalWrite(column[display_curr_column], LOW);
  column_write(display_curr_column, LOW);
  display_curr_column = (display_curr_column + 1) % 5; // Wrap around to 0


  // temp variable to look up column for each char
  byte thisCharData;

  // Loop from (this character + DISPLAY_CHARS - 1) back to (this character) and display all of them
  for (int thisChar = (display_curr_char+DISPLAY_CHARS-1); thisChar >= display_curr_char; thisChar--) {
    
    // Read the character data if it is part of the string
    if (thisChar >= 0 && thisChar < display_length) {
      thisCharData = getColumnByte(display_chars[thisChar], display_curr_column);
    } else {
      thisCharData = 0; // Not part of string, just display zero
    }
    
    // write 7 values
    for (int j = 1; j < 8; j++) {
      //digitalWrite(clock_pin, HIGH);
      clock_set;
      thisBit = (thisCharData & (1 << j));
      //digitalWrite(data_pin, thisBit); // first binary value character 8, reads for right to left, binary from left to right
      data_write(thisBit);
      //digitalWrite(clock_pin, LOW);
      clock_clear;
    }
  }

  // Activate this column until next ISR
  //digitalWrite(column[display_curr_column], HIGH);
  column_write(display_curr_column, HIGH);
    
}

