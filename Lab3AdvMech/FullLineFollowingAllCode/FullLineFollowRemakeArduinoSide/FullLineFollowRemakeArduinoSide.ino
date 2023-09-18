#include <QTRSensors.h>
QTRSensors qtr;
const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];
uint16_t linePosition;


const byte numChars = 32;
char receivedChars[numChars];
char tempChar[numChars]; // temporary array used for parsing


//motor command variables
int leftMotor; //int leftMotor
int rightMotor;

  
boolean newData = false;





//=====================================================

void setup() {
   pinMode(3, OUTPUT); //left motor
   pinMode(2,OUTPUT); //left motor
    Serial.begin(115200);
    qtr.setTypeRC(); //this allows us to read the line sensor from didgital pins

    //arduino pin sensornames i am using: 11,8,7,6,21,22,23,16 cant use #6 its a buzzer
    qtr.setSensorPins((const uint8_t[]){11, 8, 7, 15, 21, 22, 23, 16}, SensorCount);

    calibrateSensors();
    Serial.println("<Arduino is ready>");
}

//====================================================

void loop() {

    recvWithStartEndMarkers(); //this function is in charge of taking a peice of data that looks like <17.5,0,16> 
                               //turning it into a string looking like 17.5,0,16 and then setting newdata to true,
                               //letting the rest of the program know a packet of data is ready to be analyzed, does all this without blocking
                 
     
    
    if (newData == true) { //newData will be true when recvWithStartEndMarkers(); has finished recieving a whole set of data from Rpi (a set of data is denoted as being containted between <>)
      
      strcpy(tempChar, receivedChars); //this line makes a copy of recievedChars for parsing in parseData, I do this becasue strtok() will alter any string I give it,I want to preserve the origonal data
      parseData(); //right now parseData only parses a string of 3 numbers seperated by commas into floats
                   //so the string 17.5,0,16 becomes three floats; 17.5, 0, and 16
      sendDataToRpi();
                   
    }

    commandMotors(); //we want this to happen outside of our newdata=true loop so it is never blocked
}


//======================================================


void parseData(){

  //an interesting bug I found is that a char that was created in the Rpi and sent over to the arduino has a much higer chance
  //of being interpreted incorrectly by the c string functions (strtok, etc). I really have no idea why. I guess i need to



  char *strtokIndexer; //doing char * allows strtok to increment across my string properly frankly im not sure why... my kingdom for a proper c++ class

  
  strtokIndexer = strtok(tempChar,","); //sets strtokIndexer = to everything up to the first comma in tempChar /0 //this line is broken
  leftMotor = atoi(strtokIndexer); //converts strtokIndexer into a int
  

  strtokIndexer= strtok(NULL, ","); //setting the first input to null causes strtok to continue looking for commas in tempChar starting from where it left off, im not really sure why 
  rightMotor = atoi(strtokIndexer);

  


  //now that we have extracted the data from the Rpi as floats, we can use them to command actuators somewhere else in the code
  
}

//==========================================

void sendDataToRpi() {

   linePosition = qtr.readLineBlack(sensorValues);

   //HEY THIS IS IMPORTANT!!!!!!
   //Read the documentation on the QTR sensors if you want to learn more, but we can also examine what each individual sensor sees
   //by examining sensorValues at spesific places, for example, the below code prints out the outputs from the 0 and 7 sensor:
   //   Serial.print(sensorValues[0]);
   //   Serial.print(',');
   //   Serial.println(sensorValues[7]);
   //these range that can be printed here is 0-1000, 0 means that sensor is not over a line, and 1000 means it is directly over a line
   //you need to  figure out how to use that you can examine the status of each individual sensor to tell the Rpi weather 
   //your robot is over a line. My suggestion is to use an if statement that checks 2 sensors that usually wont both be seeing a line,
   //then change the value of lineposition to be the string 'cross'
   //HEY THIS IS IMPORTANT!!!!!!
  
     Serial.print(leftMotor);
     Serial.print(',');
     Serial.print(rightMotor);
     Serial.print(',');
     Serial.println(linePosition); //have this value change to 'cross' if your robot is on top of a cross.
newData = false;

}

//=======================================

void commandMotors(){
//analogWrite(3,leftMotor); 
//analogWrite(2,rightMotor);
//  
}

//==================================================================


void calibrateSensors(){

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // turn on Arduino's LED to indicate we are in calibration mode
                                   ///while calibrating, move the sensor over the line a couple times

  // 2.5 ms RC read timeout (default) * 10 reads per calibrate() call
  // = ~25 ms per calibrate() call.
  // Call calibrate() 200 times to make calibration take about 5 seconds.
  for (uint16_t i = 0; i < 200; i++)
  {
    qtr.calibrate();
  }
  digitalWrite(LED_BUILTIN, LOW); // turn off Arduino's LED to indicate we are through with calibration
  
}


//=========================================================


void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
                                                               
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();
                                                             
                                                                  
        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminates the string, frankly unsure why I need this
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}
