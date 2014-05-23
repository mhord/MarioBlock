/***************************************************************************
MarioBlock.ino

Example code for using a SparkFun AudioWidget (WIG-11125) to play a SMB
coin cha-ching when it is moved.

Uses the Wake-on-Shake (SEN-11447) to remain dormant until moved.

16 Apr 2013 - Mike Hord, SparkFun Electronics

This code is beerware- if you find it useful, please buy me (or, for that
matter, any other SparkFun employee you met) a pint next time you meet us at
the local.

***************************************************************************/

//-----------------------------------------------------------------------------
// Pin definitions
#define RESET A0   // Pull this low for at least 6ms to reset the audio module. 
                   //  Allow at least 300ms after reset before interacting with
                   //  the chip.
#define NEXT  3    // Play the next track. If you wish to advance the pointer
                   //  but not play the next track, you must immediately follow
                   //  a toggle of this pin with a toggle of the PLAY pin. Note
                   //  that the pointer for pin operations is different to the
                   //  pointer for the 2-wire serial controller. See above for
                   //  more instructions.
#define PREV  9    // Just like NEXT, but backwards.
#define PLAY  4    // Play, or stop, the current track. This stops playback, so
                   //  upon the next toggle of the PLAY pin, playback will
                   //  begin from the start of the current track. It does NOT
                   //  pause playback.
#define BUSY  5    // HIGH during playback.
#define DOUT  6    // Data input from a microcontroller.
#define DCLK  7    // Data clock from a microcontroller.

// Opcode definitions- the 2-wire serial (which is NOT I2C- it is close to SPI
//  without a CS or MISO, but the Arduino can't clock data slowly enough to
//  suit the widget when using the internal SPI peripheral).
#define OPCODE_PLAY_PAUSE  0xFFFE   // Play a track, or pause the current
                                    //  playback. Note that this requires that
                                    //  you first send a number from 0-511 to
                                    //  the module to indicate which track you
                                    //  wish to play back- simply sending the
                                    //  PLAY_PAUSE opcode has no effect other
                                    //  than pausing playback.
#define OPCODE_STOP        0xFFFF   // Stops the current playback. Subsequent
                                    //  toggling of the PLAY pin will start
                                    //  playback as though no previous track
                                    //  were playing.
#define OPCODE_VOL         0xFFF0   // Set the volume. 0xFFF0 is muted, 0xFFF7
                                    //  is max. Defaults to max on startup.

byte file = 0;   // This is a file counter used to internally track the pointer
                 //  for playback. There is no way to read the internal pointer
                 //  and find out which track will play if you simply strobe the
                 //  PLAY pin, or which track you'll be moving to if you strobe
                 //  NEXT or PREV.

// For this project, we never leave setup().
void setup()
{
  Serial.begin(57600);  // Not strictly necessary, unless the while(1)
                        //   at the end of setup() is removed.
  pinSetup();           // For conciseness of code, this is moved to a function.
  resetModule();        // The module must be reset before being used.
  delay(100);   // Experimentally determined that some time padding is needed
                //   after the module reset is completed before we can play a
                //   sound successfully.
  digitalWrite(NEXT, LOW);
  delay(10);
  digitalWrite(NEXT, HIGH);
  //sendCommand((unsigned int) file);  // Select a file; defaults to the 0th file.
  //sendCommand(OPCODE_PLAY_PAUSE);    // Play back the file we selected.
  while(1);    // Wait here until the Wake-on-Shake pulls the power.
}

// loop() doesn't really do anything in this project; execution halts at the
//   while(1) statement at the end of setup(). However, if you remove that,
//   you can play around with various commands and methods of playing sounds
//   with the module.
void loop() 
{
  if (Serial.available() > 0)
  {
    byte command = Serial.read();
    switch(command)
    {
      case 'p':
        sendCommand((unsigned int) file);
        sendCommand(OPCODE_PLAY_PAUSE);
        break;
      case 's':
        sendCommand(OPCODE_STOP);
        break;
      case 'n':
        if (file++ == 11) file = 0;
        digitalWrite(NEXT, LOW);
        delay(10);
        digitalWrite(NEXT, HIGH);
        break;
      case 'b':
        if (file-- == 255) file = 10;
        digitalWrite(PREV, LOW);
        delay(10);
        digitalWrite(PREV, HIGH);
        break;
      case '+':
        if (file++ == 11) file = 0;
        digitalWrite(NEXT, LOW);
        delay(10);
        digitalWrite(NEXT, HIGH);
        while(digitalRead(BUSY) == HIGH);
        digitalWrite(PLAY, LOW);
        delay(10);
        digitalWrite(PLAY, HIGH);
        break;
      case '-':
        if (file-- == 255) file = 10;
        digitalWrite(PREV, LOW);
        delay(10);
        digitalWrite(PREV, HIGH);
        while(digitalRead(BUSY) == HIGH);
        digitalWrite(PLAY, LOW);
        delay(10);
        digitalWrite(PLAY, HIGH);
        break;
      case 't':
        digitalWrite(PLAY, LOW);
        delay(10);
        digitalWrite(PLAY, HIGH);
        break;
      case 'x':
        sendCommand(OPCODE_PLAY_PAUSE);
        break;
    } 
  }
}

void pinSetup()
{
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, LOW);
  pinMode(NEXT, OUTPUT);
  digitalWrite(NEXT, HIGH);
  pinMode(PREV, OUTPUT);
  digitalWrite(PREV, HIGH);
  pinMode(PLAY, OUTPUT);
  digitalWrite(PLAY, HIGH);
  
  pinMode(DOUT, OUTPUT);
  digitalWrite(DOUT, HIGH);
  pinMode(DCLK, OUTPUT);
  digitalWrite(DOUT, HIGH);
  
  pinMode(BUSY, INPUT);
}

void resetModule()
{
  digitalWrite(RESET, LOW);
  delay(100);
  digitalWrite(RESET, HIGH);
  delay(300);
}

void sendCommand(unsigned int command)
{
  digitalWrite(DCLK, LOW);
  delayMicroseconds(1900);
  for (byte i = 0; i < 16; i++)
  {
    delayMicroseconds(100);
    digitalWrite(DCLK, LOW);
    digitalWrite(DOUT, LOW);
    if ((command & 0x8000) != 0)
    {
      digitalWrite(DOUT, HIGH);
    }
    delayMicroseconds(100);
    digitalWrite(DCLK, HIGH);
    command = command<<1;
  }
}
      
    
