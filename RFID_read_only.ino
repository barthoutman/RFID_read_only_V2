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

//**-----------------------------------**
//barInput variables
//**-----------------------------------
const int barPin = 27;
int barState = LOW;
unsigned long oldMillisBar=0;
bool getFeedbackBar=false;

unsigned char barScanCode[12];															//char that stores the imcomming data trough the barcode scanner
unsigned char barSavedData[12];														//Saving the data out of barScanCode 
unsigned int barCurrentReadLength=0;													//used to store the incomming data in the correct location in scanCode
bool barIsReading=false;																//barcode has been read or not
bool barEndOfRead=false;																//stops saving the incoming data after barCurrentReadLength == 12
long barScanTimer=0;																		//makes a non blocking pause in the bar code scanner after it has succesfully read a code
unsigned char epcCodeBar[12];	
//**-----------------------------------
//barInput variables
//**-----------------------------------**

//-------------------------------------**
//readRFID variables
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
int runRFIDInterval = 100;
unsigned long oldMillisRFID = 0;
unsigned long newMillis = 0;
bool getFeedback=true;
//**-----------------------------------
//readRFID variables
//**-----------------------------------**

//**-----------------------------------**
//readRFID variables
//**-----------------------------------
unsigned char writeRFIDData [32];
//**-----------------------------------
//readRFID variables
//**-----------------------------------**


const int userButton=26;
int userButtonState=0;
bool runBarInput=false;

int statusCount = 0;                                  // 0: startup| 1: turned on| 2: send barcode read command| 3: scanning Barcode| 4: Barcode scanned succesfully| 5: RFID read command sent| 6: RFID read succesful| 7: Building Write command| 8: Command build succesful| 9: RFID Write command sent| 10: RFID Write succesful| 11: RFID read command sent| 12: Comparing string (RFID EPC, and barcode)| 13: RFID write correct 
int printStatusCountTimer = 500;
unsigned long oldMillisStatusPrinter = 0;

void setup() {
  pinMode(barPin,OUTPUT);
  pinMode(userButton,INPUT_PULLDOWN);
  Serial.begin(9600);
  delay(100);
  Serial2.begin(115200);
  delay(100) ;
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

  if (newMillis - oldMillisStatusPrinter >= printStatusCountTimer) {
    runRFID = 1;
    oldMillisStatusPrinter = newMillis; 
    Serial.println(statusCount);
  }
 
  if (statusCount< 1) statusCount = 1;

  userButtonState = digitalRead(userButton);
  if (userButtonState==HIGH) {
    runBarInput=true;
    barInput();
    if (statusCount==1)statusCount = 2;
  }

  if (userButtonState==LOW) {
    runBarInput=false;
    digitalWrite(barPin, LOW);
  }

  if (statusCount == 4 && userButtonState == LOW){
    Serial2.write(ReadSingle, 7);
    readRFID();
    delay(10);
    runRFID = 0;
    int isSent = 1;

    if (isSent==1){
      oldMillisRFID = newMillis;
    }

    if (newMillis - oldMillisRFID >= runRFIDInterval) {
      runRFID = 1;
      oldMillisRFID = newMillis; 
      if (statusCount==4)statusCount = 5;
      isSent==0;
    }
  }
}
//------------------------------------------------------**************

void barInput(){

  if(runBarInput==true){
    digitalWrite(barPin, HIGH);
    delay(10);

    while (Serial.available()){
      if (statusCount==2)statusCount = 3;
     barIsReading=true;
      if (barEndOfRead==false){
        unsigned char sc = Serial.read();
        barScanCode[barCurrentReadLength] = sc;
        if (barCurrentReadLength>=12 && barScanCode[12]==13){
          Serial.print("Barcode: ");
          barEndOfRead=true;
          digitalWrite(barPin, LOW);
          delay(50);
          for(int i=0; i<12; i++){
            barSavedData[i]=barScanCode[i];
            epcCodeBar[i]=barScanCode[i];
            Serial.print(epcCodeBar[i]);
            Serial.print(" ");
            if(i>=11){Serial.println();}
          }
          delay(50); 
          statusCount = 4; 
        }
      }

      barCurrentReadLength++;  
        if(barEndOfRead == true)
       {
          barCurrentReadLength=0;
          barEndOfRead = false;
          barIsReading = false;
          for(int i = 0; i<12; i++)
            {
              barScanCode[i]=0;
           }
        }
        Serial.flush();
    }
  }

  else {
    barIsReading = false;
    for(int i=0; i<12; i++){
      barScanCode[i]=0;
      digitalWrite(barPin, LOW);
    }
  }
}

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
        Serial.print("RFID: ");
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

      if (validReadLength > 7 && validReadLength < 21) {             //Taking out the EPC code from the whole string of incomming data after reading
        readEpc[validReadLength - 9] = readData[currentReadLength];  //Removing the first 8 characters from the incomming data and saving this in *readepc
      }
    }
    currentReadLength++;


    if (rc == 0X7E && isValidRead) {
      if(getFeedback==true){
        Serial.println();
      }  
      if (isValidRead == true){
        statusCount = 6;
        Serial.print("EPC: ");
        for (int i = 0; i < 12; i++){
          Serial.print(readEpc[i], HEX);
          Serial.print(" ");
          if(i==11){Serial.println();}
        }
      }
      isValidRead = false;
      savedValidRead = 1;
      isReading = false;
    }
    delay(2);
    Serial2.flush();
    if(getFeedback==true){
      Serial.flush();
    }
  }
  //------------------------------------------------------**************
}

unsigned char generateChecksum(int cStart, int cEnd, unsigned char* dataArray) {						//Run the checksum calculation
	long int dataCounter = 0;																					//Start the *dataCounter at 0. This adds up to be the total sum of all dataArray data.
	for(int i = cStart; i < cEnd; i++) {																		//Loop through the whole write data to calculate what the checksum should be
		dataCounter += (int)dataArray[i];																		//Add dataArray up as an int instead of a char at the value of the location placed by *i
	}
	return dataCounter % 256;																					//Devide datacounter by 256 and take the rest. This way the number here can never be higher then 256
}

void buildWriteCommand() {

  writeRFIDData[0] = 0XBB;    // Header
  writeRFIDData[1] = 0X00;    // Type
  writeRFIDData[2] = 0X49;    // Command (write)
  writeRFIDData[3] = 0X00;    // Parameter length (1/2) Length from Passcode 1/4 up to the end of the EPC
  writeRFIDData[4] = 0X17;    // Parameter length (2/2) Total length is 23 -> Hex 0X17 

  writeRFIDData[5] = 0X00;    //Passcode 1/4
  writeRFIDData[6] = 0X00;    //Passcode 2/4
  writeRFIDData[7] = 0X00;    //Passcode 3/4
  writeRFIDData[8] = 0X00;    //Passcode 4/4

  writeRFIDData[9] = 0X01;    // Memory bank (00 is RFU, 01 is EPC, 10 is TID and 11 is USER)
  writeRFIDData[10] = 0X00;   // Memory address offset 1/2
  writeRFIDData[11] = 0X00;   // Memory address offset 1/2
  writeRFIDData[12] = 0X00;   // Data length 1/2
  writeRFIDData[13] = 0X08;   // Data length 1/2

  writeRFIDData[14] = savedReadData[20];   // crc code 1/2 read location 20
  writeRFIDData[15] = savedReadData[21];   // crc code 1/2 read location 21

  writeRFIDData[16] = savedReadData[6];   // PC code 1/2 read location 6
  writeRFIDData[17] = savedReadData[7];   // PC code 1/2 read location 7

  for (int i = 0; i<12; i++){
    writeRFIDData[18+i] = epcCodeBar[i];   //12x EPC from Barcode scanner from barInput()
  }

	writeRFIDData[30] = generateChecksum(1, 30, writeRFIDData);
	writeRFIDData[31] = 0X7E;
}