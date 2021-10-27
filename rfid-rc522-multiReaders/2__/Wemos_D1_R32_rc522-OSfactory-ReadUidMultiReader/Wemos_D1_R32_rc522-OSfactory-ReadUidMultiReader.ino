/**
 * This codes enables you to read multiple rfid readers
 *
 * * 
 *  * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      
 *             Reader/PCD   Wemos_D1 R32
 * Signal      Pin         
 * -----------------------------------------------------------------------------------------
 * <1st Reader>
 * RST/Reset   RST          14           
 * SPI SS      SDA(SS)      5  / VSPI_SS         
 * SPI MOSI    MOSI         23 / VSPI-MOSI  
 * SPI MISO    MISO         19 / VSPI_MiSO
 * SPI SCK     SCK          18 / VSPI_SCK
 * 
 * <2nd Reader>
 *             SDA(SS)      13  
 * 
 * <3rd Reader>
 *             SDA(SS)      12
 *  
*/

#include <SPI.h>
// Download and install from https://github.com/miguelbalboa/rfid
#include <MFRC522.h>

// CONSTANTS
// The number of RFID readers
const byte numReaders = 3;
// Each reader has a unique Slave Select pin
const byte ssPins[] = {5, 13, 12};
// They'll share the same reset pin
const byte resetPin = 14;


// The correct IDs should be changed by you!!!
// The sequence of NFC tag IDs required to unlock the Lock
const String correctIDs[] = {"9BD60C22", "5A573086"};


// GLOBALS
// Initialise an array of MFRC522 instances representing each reader
MFRC522 mfrc522[numReaders];
// The tag IDs currently detected by each reader
String currentIDs[numReaders];
// Init array that will store new NUID 
byte nuidPICC[numReaders][4];

#define DEBUG (1)

String dump_byte_array(byte *buffer, byte bufferSize);
void printHex(byte *buffer, byte bufferSize);
void printDec(byte *buffer, byte bufferSize);


/**
   Initialisation
*/
void setup() {

  // Initialise serial communications channel with the PC
  Serial.begin(9600);
  Serial.println(F("Serial communication started"));


  // Initialise the SPI bus
  SPI.begin();

  for (uint8_t i = 0; i < numReaders; i++) { // Initialise the reader // Note that SPI pins on the reader must always be connected to certain // Arduino pins (on an Uno, MOSI=> pin11, MISO=> pin12, SCK=>pin13)
    // The Slave Select (SS) pin and reset pin can be assigned to any pin
    mfrc522[i].PCD_Init(ssPins[i], resetPin);

    // Set the gain to max - not sure this makes any difference...
    // mfrc522[i].PCD_SetAntennaGain(MFRC522::PCD_RxGain::RxGain_max);

#ifdef DEBUG
    // Dump some debug information to the serial monitor
    Serial.print(F("Reader #"));
    Serial.print(i);
    Serial.print(F(" initialised on pin "));
    Serial.print(String(ssPins[i]));
    Serial.print(F(". Antenna strength: "));
    Serial.print(mfrc522[i].PCD_GetAntennaGain());
    Serial.print(F(". Version : "));
    mfrc522[i].PCD_DumpVersionToSerial();
#endif

    // Slight delay before activating next reader
    delay(100);
  }

#ifdef DEBUG
  Serial.println(F("--- END SETUP ---"));
#endif
}

/**
   Main loop
*/
void loop() {

  // Assume that the tags have not changed since last reading
  boolean changedValue = false;

  // Loop through each reader
  for (uint8_t i = 0; i < numReaders; i++) {

    // Initialise the sensor
    //mfrc522[i].PCD_Init();

    // String to hold the ID detected by each sensor
    String readRFID = "";

    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if ( ! mfrc522[i].PICC_IsNewCardPresent())
      continue;
  
    // Verify if the NUID has been readed
    if ( ! mfrc522[i].PICC_ReadCardSerial())
      continue;
    

    Serial.print(F(">>> PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522[i].PICC_GetType(mfrc522[i].uid.sak);
    Serial.println(mfrc522[i].PICC_GetTypeName(piccType));

    // Check is the PICC of Classic MIFARE type
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      Serial.println(F("Your tag is not of type MIFARE Classic."));
      
      continue;
    }

    if (mfrc522[i].uid.uidByte[0] != nuidPICC[i][0] || 
      mfrc522[i].uid.uidByte[1] != nuidPICC[i][1] || 
      mfrc522[i].uid.uidByte[2] != nuidPICC[i][2] || 
      mfrc522[i].uid.uidByte[3] != nuidPICC[i][3] ) {

      Serial.print(F("Reader #"));
      Serial.print(i);
      Serial.print(F(": "));
      Serial.println(F("A new card has been detected."));
      Serial.println();
      
      

      // Store NUID into nuidPICC array
      for (byte j = 0; j < 4; j++) {
        nuidPICC[i][j] = mfrc522[i].uid.uidByte[j];
      }

      Serial.println(F("The NUID tag is:"));
      Serial.print(F("In hex: "));
      printHex(mfrc522[i].uid.uidByte, mfrc522[i].uid.size);
      Serial.println();
      Serial.print(F("In dec: "));
      printDec(mfrc522[i].uid.uidByte, mfrc522[i].uid.size);
      Serial.println();
      
    } else {
      Serial.print(F("Reader #"));
      Serial.print(i);
      Serial.print(F(": "));
      Serial.println(F("Card read previously."));
      Serial.println();      
    #ifdef DEBUG
      Serial.println(F("---"));
    #endif
    }

    // Extract the ID from the tag
    readRFID = dump_byte_array(mfrc522[i].uid.uidByte, mfrc522[i].uid.size);
 

    // If the current reading is different from the last known reading
    if (readRFID != currentIDs[i]) {
      // Set the flag to show that the rfid state has changed
      changedValue = true;
      // Update the stored value for this sensor
      currentIDs[i] = readRFID;
    }



    // If the reading fails to match the correct ID for this sensor
    if (currentIDs[i] != correctIDs[i]) {
      //do something,later.  
    }



    // If the reading is correct with corresponding ID
    if (currentIDs[i] == correctIDs[i]) {
      //Do something, later.
    }

    // Halt PICC
    mfrc522[i].PICC_HaltA();
    // Stop encryption on PCD
    mfrc522[i].PCD_StopCrypto1();
    
  }//for (uint8_t i = 0; i < numReaders; i++) {


#ifdef DEBUG
  // If the changedValue flag has been set, at least one sensor has changed
  if (changedValue) {
    // Dump to serial the current state of all sensors
    for (uint8_t k = 0; k < numReaders; k++) {
      Serial.print(F("Reader #"));
      Serial.print(String(k));
      Serial.print(F(" on Pin #"));
      Serial.print(String((ssPins[k])));
      Serial.print(F(" detected tag: "));
      Serial.println(currentIDs[k]);
    }
    Serial.println(F("---"));
  }
#endif

}

/**
   Helper function to return a string ID from byte array
*/
String dump_byte_array(byte *buffer, byte bufferSize) {
  String read_rfid = "";
  for (byte i = 0; i < bufferSize; i++) {
    read_rfid = read_rfid + String(buffer[i], HEX);
  }
  return read_rfid;
}

/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
