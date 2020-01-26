
#define PIXELS  300  // Number of pixels in the string
#define PIXELS3 900
#define HALFPIXELS 5

#define PIXEL_PORT    PORTD
#define PIXEL_DDR     DDRD
#define PIXEL_BIT     2

#define T1H  800    // Width of a 1 bit in ns
#define T1L  450    // Width of a 1 bit in ns

#define T0H  400    // Width of a 0 bit in ns
#define T0L  850    // Width of a 0 bit in ns

#define RES 50000    // Width of the low gap between bits to cause a frame to latch

#define NS_PER_SEC (1000000000L)

#define CYCLES_PER_SEC (F_CPU)

#define NS_PER_CYCLE ( NS_PER_SEC / CYCLES_PER_SEC )

#define NS_TO_CYCLES(n) ( (n) / NS_PER_CYCLE )


byte pixelArray[PIXELS3];


inline void sendBit( bool bitVal ) {
  
    if (  bitVal ) {        // 0 bit
      
    asm volatile (
      "sbi %[port], %[bit] \n\t"        // Set the output bit
      ".rept %[onCycles] \n\t"                                // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      "cbi %[port], %[bit] \n\t"                              // Clear the output bit
      ".rept %[offCycles] \n\t"                               // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      ::
      [port]    "I" (_SFR_IO_ADDR(PIXEL_PORT)),
      [bit]   "I" (PIXEL_BIT),
      [onCycles]  "I" (NS_TO_CYCLES(T1H) - 2),    // 1-bit width less overhead  for the actual bit setting, note that this delay could be longer and everything would still work
      [offCycles]   "I" (NS_TO_CYCLES(T1L) - 2)     // Minimum interbit delay. Note that we probably don't need this at all since the loop overhead will be enough, but here for correctness

    );
                                  
    } else {          // 1 bit

    // **************************************************************************
    // This line is really the only tight goldilocks timing in the whole program!
    // **************************************************************************


    asm volatile (
      "sbi %[port], %[bit] \n\t"        // Set the output bit
      ".rept %[onCycles] \n\t"        // Now timing actually matters. The 0-bit must be long enough to be detected but not too long or it will be a 1-bit
      "nop \n\t"                                              // Execute NOPs to delay exactly the specified number of cycles
      ".endr \n\t"
      "cbi %[port], %[bit] \n\t"                              // Clear the output bit
      ".rept %[offCycles] \n\t"                               // Execute NOPs to delay exactly the specified number of cycles
      "nop \n\t"
      ".endr \n\t"
      ::
      [port]    "I" (_SFR_IO_ADDR(PIXEL_PORT)),
      [bit]   "I" (PIXEL_BIT),
      [onCycles]  "I" (NS_TO_CYCLES(T0H) - 2),
      [offCycles] "I" (NS_TO_CYCLES(T0L) - 2)

    );
      
    }
    
    // Note that the inter-bit gap can be as long as you want as long as it doesn't exceed the 5us reset timeout (which is A long time)

    
}  

  
inline void sendByte( byte bythe ) {
    
    for( byte bit = 0 ; bit < 8 ; bit++ ) {
      
      sendBit( bitRead( bythe , 7 ) );                // WS2812B wants bit in highest-to-lowest order
                                                     // so send highest bit (bit #7 in an 8-bit byte since they start at 0)
      bythe <<= 1;                                    // and then shift left so bit 6 moves into 7, 5 moves into 6, etc
      
    }           
} 


void ledsetup() {
  
  bitSet( PIXEL_DDR , PIXEL_BIT );
  
}


inline void sendPixel( byte r, byte g , byte b )  {  
  
  sendByte(g);          // WS2812B wants colors in green then red then blue order
  sendByte(r);
  sendByte(b);
  
}


void show() {
  delayMicroseconds( (RES / 1000UL) + 1);       // Round up since the delay must be _at_least_ this long (too short might not work, too long is not a problem)
}


// Display a single color on the whole string
void showColor( byte r , byte g , byte b ) {
  for( int p=0; p<PIXELS3; p++ ) {
    pixelArray[p] = r;
    pixelArray[p+1] = g;
    pixelArray[p+2] = b;
    p++;p++;
  }
}

void showArray() {
    cli();
    for (int k = 0; k < PIXELS3; k++) {
        sendPixel(pixelArray[k],pixelArray[k+1],pixelArray[k+2]);
        k++;k++;
    }
    sei();
    show();
}


const byte dim_curve[] = {
    0,   1,   1,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,
    3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,   4,   4,   4,   4,   4,
    4,   4,   4,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   6,   6,   6,
    6,   6,   6,   6,   6,   7,   7,   7,   7,   7,   7,   7,   8,   8,   8,   8,
    8,   8,   9,   9,   9,   9,   9,   9,   10,  10,  10,  10,  10,  11,  11,  11,
    11,  11,  12,  12,  12,  12,  12,  13,  13,  13,  13,  14,  14,  14,  14,  15,
    15,  15,  16,  16,  16,  16,  17,  17,  17,  18,  18,  18,  19,  19,  19,  20,
    20,  20,  21,  21,  22,  22,  22,  23,  23,  24,  24,  25,  25,  25,  26,  26,
    27,  27,  28,  28,  29,  29,  30,  30,  31,  32,  32,  33,  33,  34,  35,  35,
    36,  36,  37,  38,  38,  39,  40,  40,  41,  42,  43,  43,  44,  45,  46,  47,
    48,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,
    63,  64,  65,  66,  68,  69,  70,  71,  73,  74,  75,  76,  78,  79,  81,  82,
    83,  85,  86,  88,  90,  91,  93,  94,  96,  98,  99,  101, 103, 105, 107, 109,
    110, 112, 114, 116, 118, 121, 123, 125, 127, 129, 132, 134, 136, 139, 141, 144,
    146, 149, 151, 154, 157, 159, 162, 165, 168, 171, 174, 177, 180, 183, 186, 190,
    193, 196, 200, 203, 207, 211, 214, 218, 222, 226, 230, 234, 238, 242, 248, 255,
};

void getRGB(int hue, int sat, int val, int colors[3]) { 
  /* convert hue, saturation and brightness ( HSB/HSV ) to RGB
     The dim_curve is used only on brightness/value and on saturation (inverted).
     This looks the most natural.      
  */
 
  val = dim_curve[val];
  sat = 255-dim_curve[255-sat];
 
  int r;
  int g;
  int b;
  int base;
 
  if (sat == 0) { // Acromatic color (gray). Hue doesn't mind.
    colors[0]=val;
    colors[1]=val;
    colors[2]=val;  
  } else  { 
 
    base = ((255 - sat) * val)>>8;
 
    switch(hue/60) {
    case 0:
        r = val;
        g = (((val-base)*hue)/60)+base;
        b = base;
    break;
 
    case 1:
        r = (((val-base)*(60-(hue%60)))/60)+base;
        g = val;
        b = base;
    break;
 
    case 2:
        r = base;
        g = val;
        b = (((val-base)*(hue%60))/60)+base;
    break;
 
    case 3:
        r = base;
        g = (((val-base)*(60-(hue%60)))/60)+base;
        b = val;
    break;
 
    case 4:
        r = (((val-base)*(hue%60))/60)+base;
        g = base;
        b = val;
    break;
 
    case 5:
        r = val;
        g = base;
        b = (((val-base)*(60-(hue%60)))/60)+base;
    break;
    }
 
    colors[0]=r;
    colors[1]=g;
    colors[2]=b; 
  }   
}


String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
boolean con = false;

byte rgb[3] = {255, 50, 50};
int index = 0;
byte p = 0;
int mode = 0; 
bool runFlag = false;

void setup() {
  ledsetup();
  showColor(0,0,0);
  Serial.begin(500000);
  randomSeed(analogRead(0));
  pinMode(LED_BUILTIN, OUTPUT);
}

int hooo = 0;
int jep = 1;

unsigned long timeMillis = 0;

int col[3];

void loop() {
  
  if (runFlag == false) {
    if (timeMillis <= millis()) {
      showArray();
      for (int pix = 0; pix < PIXELS; pix++) {
        
        getRGB((hooo+pix*jep)%360, 255, 200, col);/*
        pixelArray[pix*3] = col[0];
        pixelArray[pix*3+1] = col[1];
        pixelArray[pix*3+2] = col[2];*/
        switch (pix % 3) {
          case 0:
            pixelArray[pix*3] = 255;
            pixelArray[pix*3+1] = 0;
            pixelArray[pix*3+2] = 255;
          break;
          case 1:
            pixelArray[pix*3] = 255;
            pixelArray[pix*3+1] = 0;
            pixelArray[pix*3+2] = 255;
          break;
          case 2:
            pixelArray[pix*3] = 255;
            pixelArray[pix*3+1] = 0;
            pixelArray[pix*3+2] = 255;
          break;
        }
      }
      hooo = (hooo+jep)%360;
      timeMillis = millis() + 30;
    }
  }
  

  
  if (stringComplete) {
    stringComplete = false;

    if (inputString.substring(1,4).equals("END")) {
      index = 0;
      showArray();
      mode = 0;
      Serial.print("#RDY\n");
    }
    else if (inputString.substring(1,4).equals("ALL")) {
      mode = 1;
    }
    else if (inputString.substring(1,4).equals("CON")) {
      runFlag = true;
      digitalWrite(LED_BUILTIN,HIGH);
      showColor(0, 0, 0);
      showArray();
    }
    else if (inputString.substring(1,4).equals("DIS")) {
      runFlag = false;
      digitalWrite(LED_BUILTIN,LOW);
      showColor(0, 0, 0);
      showArray();
    }
    inputString = "";
  }
}



void serialEvent() {
  while (Serial.available()) {
    
    if (mode == 1) {
      byte inChar = (byte)Serial.read();
      pixelArray[index] = inChar;
      if (index++ >= PIXELS3) {
        mode = 0; index = 0; stringComplete = true;
      }
    }
    
    else {
      byte inChar = Serial.read();
      if ((char)inChar == '\n') {
        stringComplete = true;
      }
      inputString += (char) inChar;
    }
  }
}
