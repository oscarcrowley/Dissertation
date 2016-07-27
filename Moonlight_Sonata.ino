//bit buffer key map 

struct bit_buffer {
    int bytes;
    int maxbit;
    byte *bit_buffer;
}BIT_BUFFER;

void initialise_bit_buffers(int n) {
    int bytes = n/8;
    if (n%8) {
        bytes++;
    }

    BIT_BUFFER.bit_buffer = (byte *)malloc(bytes);
    memset(BIT_BUFFER.bit_buffer, 0, bytes);
    BIT_BUFFER.bytes = bytes;
    BIT_BUFFER.maxbit = n;
}

void enable_bit(int n) {
    if (n >= BIT_BUFFER.maxbit) {
        return;
    }

    int byte = n/8;
    int pos = n%8;
    unsigned char *p = BIT_BUFFER.bit_buffer + byte;
    *p |= (1 << pos);
}

void disable_bit(int n) {
    if (n >= BIT_BUFFER.maxbit) {
        return;
    }

    int byte = n/8;
    int pos = n%8;
    unsigned char *p = BIT_BUFFER.bit_buffer + byte;
    *p &= ~(1 << pos);
}

void set_bit(int bit, int state) {
    if (bit >= BIT_BUFFER.maxbit) {
        return;
    }

    if (state == 0) {
        disable_bit(bit);
    } else {
        enable_bit(bit);
    }
}

int is_bit_enabled(int n) {
    if (n >= BIT_BUFFER.maxbit) {
        return 0;
    }

    int byte = n/8;
    int pos = n%8;
    unsigned char *p = BIT_BUFFER.bit_buffer + byte;
    return *p & (1 << pos);
}

int is_bit_disabled(int n) {
    if (n >= BIT_BUFFER.maxbit) {
        return 0;
    }

    return !is_bit_enabled(n);
}

void toggle_bit(int n) {
    if (n >= BIT_BUFFER.maxbit) {
        return;
    }

    int byte = n/8;
    int pos = n%8;
    unsigned char *p = BIT_BUFFER.bit_buffer + byte;
    *p ^= (1 << pos);
}

int countSetBits(unsigned char n) {
    unsigned int count = 0;
    while (n)
    {
        n &= (n-1) ;
        count++;
    }
    return count;
}

int count_enabled_bits() {

    int count = 0;
    for(int i=0;i<BIT_BUFFER.bytes;i++) {

        unsigned char *p = BIT_BUFFER.bit_buffer + i;
        count = count + countSetBits(*p);
    }

    return count;
}

int count_disabled_bits() {

    return (BIT_BUFFER.maxbit) - count_enabled_bits();
}

//end of bit buffer helpers -----------------------------------------------------------

byte commandByte;
byte noteByte;
byte velocityByte;
byte noteOn = 144;
const byte number_of_notes = 4;
const byte steps_in_song = 31;

byte song[steps_in_song][number_of_notes] = { //Moonlight Sonata 
  {50,57,0,0},
  {62,0,0,0},
  {65,0,0,0},
  {57,0,0,0},
  {62,0,0,0},
  {65,0,0,0},
  {57,0,0,0},
  {62,0,0,0},
  {65,0,0,0},
  {57,0,0,0},
  {62,0,0,0},
  {65,0,0,0},
  {48,57,0,0},
  {62,0,0,0},
  {65,0,0,0},
  {57,0,0,0},
  {62,0,0,0},
  {65,0,0,0},
  {57,0,0,0},
  {62,0,0,0},
  {65,0,0,0},
  {57,0,0,0},
  {62,0,0,0},
  {65,0,0,0},
  {46,58,0,0},
  {62,0,0,0},
  {65,0,0,0},
  {58,0,0,0},
  {62,0,0,0},
  {65,0,0,0},
  {43,58,0,0}
}; //use -1 to denote a wildcard for single notes later on in the code

byte song_position = 0;

void setup(){
  initialise_bit_buffers(128);
  
  Serial.begin(31250);
  pinMode(13,OUTPUT);

  set_pin_output();
  turn_all_pins_off();  
}

void checkMIDI(){
  do{
    if (Serial.available()){
      commandByte = Serial.read();//read first byte
      noteByte = Serial.read();//read next byte
      velocityByte = Serial.read();//read final byte
      
      if ((commandByte == noteOn) && (velocityByte > 0)){//if note on message
        set_bit(noteByte, 1);
      } 
      else { 
        set_bit(noteByte, 0);
      }      
    }
  }
  while (Serial.available() > 2);//when at least three bytes available
}

void loop(){  
  checkMIDI();

  show_LEDs(song[song_position]);

  if (song_position >= steps_in_song) { //total number of positional notes
    turn_all_pins_on();
  } else {
    show_LEDs(song[song_position]); 
  }

  if (do_keys_match()) {
    song_position = song_position + 1;
  }
 
  delay(100);
  digitalWrite(13,LOW);//turn led off
}

int do_keys_match() {

  int bits_required = 0;
  for(int i=0;i<number_of_notes;i++) {
    if (song[song_position][i] != 0) {
      bits_required++;
    }
  }

  if (count_enabled_bits() != bits_required) {
    return 0;
  }

  for(int i=0;i<bits_required;i++) {
    if (is_bit_disabled(song[song_position][i])) {
      return 0;
    }
  }

  return 1;
}

void show_LEDs(byte keys[2]) {

  turn_all_pins_off();

  for (int i=0;i<number_of_notes;i++){
    int pin = pin_for_code(keys[i]);
    digitalWrite(pin, HIGH);
  }
  
}

void turn_all_pins_off() {  
  set_pins(LOW);
}

void turn_all_pins_on() {
  set_pins(HIGH);
}

void set_pins(int state) {

  int first_pin = 4;
  int pin_count = 8;

  for (int i=0;i<pin_count;i++){
    digitalWrite(i+first_pin,state);
  }
}

void set_pin_output() {

  int first_pin = 4;
  int pin_count = 8;

  for (int i=0;i<pin_count;i++){
    pinMode(i+first_pin,OUTPUT);
  }
}

//end helpers to turn all pins on or off

//map keyboard code to physical port (led)
int pin_for_code(byte code) { 


  const byte map_size = 8;

  byte keymap[map_size][2] = {
    {43,4},
    {46,5},
    {48,6},
    {50,7},
    {57,8},
    {58,9},
    {62,10},
    {65,11}
  };

  for(int i=0;i<map_size;i++) {
    if (keymap[i][0] == code) {
      return keymap[i][1];
    }
  }

  return 0;
  
  /*if (code == 43) {
    return 4;
  }
  else if (code == 46) {
    return 5;
  }
  else if (code == 48) {
    return 6;
  }
  else if (code == 50) {
    return 7;
  }
  else if (code == 57) {
    return 8;
  }  
  else if (code == 58) {
    return 9;
  }  
  else if (code == 62) {
    return 10;
  }  
  else if (code == 65) {
    return 11;
  }     
    return 0;*/
}


