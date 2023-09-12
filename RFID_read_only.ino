//-------------------------------------**
//RFID commands
//-------------------------------------
unsigned char ReadSingle[7] = { 0XBB, 0X00, 0X22, 0X00, 0X00, 0X22, 0X7E };
unsigned char ReadMulti[10] = { 0XBB, 0X00, 0X27, 0X00, 0X03, 0X22, 0XFF, 0XFF, 0X4A, 0X7E };
unsigned char ReadStop[7] = { 0XBB, 0X00, 0X28, 0X00, 0X00, 0X28, 0X7E };
unsigned char setModeHighSens[8] = { 0XBB, 0X00, 0XF5, 0X00, 0X01, 0X00, 0XF6, 0X7E };
unsigned char setRegion[8] = { 0XBB, 0X00, 0X07, 0X00, 0X01, 0X03, 0X0B, 0X7E };       // europe
unsigned char setPower[9] = { 0XBB, 0X00, 0XB6, 0X00, 0X02, 0X0A, 0X28, 0XEA, 0X7E };  //26dBm- max power
//**-----------------------------------
//RFID commands
//**-----------------------------------**
int setSettings = true;  //set to true to always start with the proper settings
//-------------------------------------**
//RFID variables
//-------------------------------------
unsigned char readData[32];
unsigned char savedReadData[32];
unsigned char readEpc[12];
unsigned char savedReadEpc[12];
unsigned int currentReadLength = 0;
unsigned int validReadLength = 0;
bool isReading = false;
bool isValidRead = false;
bool savedValidRead = false;
int runRFID = 0;
int runRFIDInterval = 250;
unsigned long oldMillisRFID = 0;
unsigned long newMillis = 0;
bool getFeedback=true;
//**-----------------------------------
//RFID variables
//**-----------------------------------**

//**-----------------------------------**
//Barcode variables
//**-----------------------------------
const int barPin = 27;
int barStat = LOW;
unsigned long oldMillisBar=0;
bool getFeedbackBar=false;




//**-----------------------------------
//Barcodes variables
//**-----------------------------------**




void setup() {
  pinMode(barPin, OUTPUT);
  Serial2.begin(115200);
  delay(50);
  Serial1.begin(9600,SERIAL_8N1,14,12);	
  Serial.begin(115200);
  delay(500);

}

void loop() {
  newMillis = millis();
  //-----------------------------------------------------preload settings
  if (setSettings == true) {
    delay(200);
    Serial.write(setRegion, 8);
    delay(200);
    Serial.write(setModeHighSens, 8);
    delay(200);
    Serial.write(setPower, 9);
    delay(200);
    setSettings = false;
  }
  //-----------------------------------------------------preload settings************

  if (newMillis - oldMillisRFID >= runRFIDInterval && isReading == false) {
    runRFID = 1;
    oldMillisRFID = newMillis;
  }
  if (runRFID == 1) {
    Serial2.write(ReadSingle, 7);
    readRFID();
    barInput();
    runRFID = 0;
  }
}
//------------------------------------------------------**************

//-----------------------------------------------------handle RFID read input
void readRFID() {

  while (Serial2.available()) {  //Checks if serial input is available
    unsigned char rc = Serial2.read();

    if (rc == 0XBB) {  //read has started
      isReading = true;
      currentReadLength = 0;
      for (int i = 0; i < 32; i++) {  //loop *readData 64 times
        readData[i] = 0X00;           //Fill the *readData with 0X00 to clear all positions before saving new data (only for visual and inspection perposes)
      }
    }

    if (isReading) {
      readData[currentReadLength] = rc;
      if (currentReadLength == 1 && rc == 0X02) {
        validReadLength = 1;
        isValidRead = true;
        for (int i = 0; i < 32; i++) {  //Loop *lastValidReadData 64 times*
          savedReadData[i] = 0X00;      //Clear all chars
        }
        savedReadData[0] = 0XBB;  //Write header to lastValidReadData 0XAA
      }
      if (isValidRead) {                      //If incomming data is valid
        savedReadData[validReadLength] = rc;  //Taking the data form *LastValidReadData and putting it in *rc at the position decided by currentReadLength
        if(getFeedback==true){
          Serial.print(savedReadData[validReadLength], HEX);
          Serial.print(" ");
        }
        validReadLength++;  //saving the length of the saved string
      }

      if (validReadLength > 7 && validReadLength < 20) {             //Taking out the EPC code from the whole string of incomming data after reading
        readEpc[validReadLength - 8] = readData[currentReadLength];  //Removing the first 8 characters from the incomming data and saving this in *readepc
      }
    }
    currentReadLength++;


    if (rc == 0X7E) {
      if(getFeedback==true){
        Serial.println();
      }  
      isValidRead = false;
      savedValidRead = 1;
      isReading = false;
    }
    delay(5);
    Serial2.flush();
    if(getFeedback==true){
      Serial.flush();
    }
  }
  //------------------------------------------------------**************
}

void barInput(){


  
}