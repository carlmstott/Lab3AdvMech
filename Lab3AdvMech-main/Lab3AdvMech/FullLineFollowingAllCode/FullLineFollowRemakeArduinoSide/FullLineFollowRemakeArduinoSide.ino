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
int isCross=0;

  
boolean newData = false;





//=====================================================

void setup() {
   pinMode(3, OUTPUT); //left motor
   pinMode(2,OUTPUT); //left motor
    Serial.begin(115200);
    qtr.setTypeRC(); //this allows us to read the line sensor from didgital pins

    //arduino pin sensornames I am using: 7, 18, 23 aka A5 PIN A1 DID NOT WORK WITH ANY SENSOR!!, 20, 21, 22, 8, 6. UNHOOK THE BLUE JUMPER LABELED BUZZER ON THE ASTAR or pin 6 will cause the buzzer to activate.
//    qtr.setEmitterPins(4, 5);
//     QTRReadMode::On;
    qtr.setSensorPins((const uint8_t[]){7, 18, 23, 20, 21, 22, 8, 6}, SensorCount);

    calibrateSensors();
    qtr.setEmitterPins(4, 5);
     QTRReadMode::On;
    Serial.println("<Arduino is ready>");
}

//====================================================

void loop() {

//////////////////////////////////////////REPAIRING THE READLINEBLACK FUNCTION SO THE LINEPOSITION VARIABLE IS ACTUALLY USEFULL FOR STUDENTS//////////////////////////////////////////////
linePosition = qtr.readLineBlack(sensorValues);

//REPAIR NUMBER ONE:  every sensor reading under 300 is a noisy and messes up the lineposition measurment, so this for loop filters it out
for (int i=0; i<= 7; i++){
   if (sensorValues[i] <300){
       sensorValues[i]=0;     
    }
}


// REPAIR NUMBER 2: checking if all my sensorvalues are zero and then setting lineposition to zero, sometimes if all sensorvalues are zero lineposition will be set to 7k and this fixes that
if (sensorValues[0]==0 && sensorValues[1]==0 && sensorValues[2]==0 && sensorValues[3]==0 && sensorValues[4]==0 && sensorValues[5]==0 && sensorValues[6]==0 && sensorValues[7]==0){
  linePosition=0;
}

//REPAIR NUMBER THREE: if only sensor 0 or sensor 8 are reading measurments, then the low level function that calculates linePosition will set it to values that do not represent where the sensor array
//is actually located relative to the line, the below loops fix that by setting linePosition to  1000 if ONLY sensor 0 sees anything, and setting linePosition to 5000 if ONLY sensor 7 sees anything
if (sensorValues[0] >0 && sensorValues[1]==0 && sensorValues[2]==0 && sensorValues[3]==0 && sensorValues[4]==0 && sensorValues[5]==0 && sensorValues[6]==0 && sensorValues[7]==0){
  linePosition=1000;
} 
if (sensorValues[7] >0 && sensorValues[0]==0 && sensorValues[1]==0 && sensorValues[2]==0 && sensorValues[3]==0 && sensorValues[4]==0 && sensorValues[5]==0 && sensorValues[6]==0){
  linePosition=5000;
}

//REPAIR NUMBER 4: there are still situations where linePosition is somehow greater than 5000 or 0<linePosition<1000, so I am hard capping linePosition to be between 1000 and 5000 when linePosition is greater
//than zero.
if (linePosition > 5000){
  linePosition = 5000;
}
if (linePosition < 1000 && linePosition > 0){
  linePosition = 1000;
}


if ((sensorValues[7] > 500) && (sensorValues[0] > 500)){
  isCross = 1;
}else{
    isCross = 0;
  }

//now linePosition finally is a value between 1000 and 5000 that is proportonal to the sensor's position relative to the line, and the value isCross is set to true when both sensor 0 and 7 are over a line


//////////////////////////////////////////////////////////////////////////////////////////////////////////END OF REPAIRS TO LINEPOSITION//////////////////////////////

  

    recvWithStartEndMarkers(); //this function is in charge of taking a peice of data that looks like <17,16> 
                               //turning it into a string looking like 17,16 and then setting newdata to true,
                               //letting the rest of the program know a packet of data is ready to be analyzed, does all this without blocking
    if (newData == true) { //newData will be true when recvWithStartEndMarkers(); has finished recieving a whole set of data from Rpi (a set of data is denoted as being containted between <>)
      
      strcpy(tempChar, receivedChars); //this line makes a copy of recievedChars for parsing in parseData, I do this becasue strtok() will alter any string I give it,I want to preserve the origonal data
      parseData(); //right now parseData only parses a string of 2 numbers seperated by commas into floats
                   //so the string 17.5,16 becomes two floats; 17.5 and 16


      //THIS SECTION IS WHERE YOU SHOULD SERIAL PRINT WHAT YOU WANT TO SEND TO YOUR RPI
      Serial.print(linePosition);
      Serial.print(",");
      Serial.println(isCross);
      newData = false;

      
      //sendDataToRpi(); //unused
                   
    }



    commandMotors(); //we want this to happen outside of our newdata=true loop so it is never blocked
}


//======================================================


void parseData(){


  char *strtokIndexer; //doing char * allows strtok to increment across my string properly frankly im not sure why... something to do with pointers that I dont expect students to understand

  
  strtokIndexer = strtok(tempChar,","); //sets strtokIndexer = to everything up to the first comma in tempChar /0 //this line is broken
  leftMotor = atoi(strtokIndexer); //converts strtokIndexer into a int
  

  strtokIndexer= strtok(NULL, ","); //setting the first input to null causes strtok to continue looking for commas in tempChar starting from where it left off, im not really sure why 
  rightMotor = atoi(strtokIndexer);

  
  //now that we have extracted the data from the Rpi as floats, we can use them to command actuators somewhere else in the code
  
}

//==========================================


//=======================================

void commandMotors(){
//analogWrite(3,leftMotor); 
//analogWrite(2,rightMotor);
//  
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

//==================================================================

void calibrateSensors(){

  //THE SENSORS ONLY CALIBRATE WHEN YOU UPLOAD NEW ARDUINO CODE TO THE ASTAR. after that the sensors STAY calibrated as long as the Astar has power.

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // turn on Arduino's LED to indicate we are in calibration mode
                                   ///while calibrating, move the sensor over the line a couple times

  // 2.5 ms RC read timeout (default) * 10 reads per calibrate() call
  // = ~25 ms per calibrate() call.
  // Call calibrate() 400 times to make calibration take about 10s seconds.
  for (uint16_t i = 0; i < 400; i++)
  {
    qtr.calibrate();
  }
  digitalWrite(LED_BUILTIN, LOW); // turn off Arduino's LED to indicate we are through with calibration
  
}

    //UNUSED
//void sendDataToRpi() {
//
//
//
//   //HEY THIS IS IMPORTANT!!!!!!
//   //Read the documentation on the QTR sensors if you want to learn more, but as well as examining where the line is relative to the whole array of sensors, we can also examine what each INDIVIDUAL sensor sees
//   //by examining sensorValues at spesific places (AFTER the readLineBlack function has been called on it), for example, the below code prints out the outputs from the 0 and 7 sensor seperated by a comma:
//   //   Serial.print(sensorValues[0]);
//   //   Serial.print(',');
//   //   Serial.println(sensorValues[7]);
//   //these range that can be printed here is 0-1000, 0 means that the INDIVIDUAL sensor is not over a line, and 1000 means it is directly over a line
//   //you need to figure out how to use that you can examine the status of each individual sensor to tell the Rpi if 
//   //your robot is over a cross. My suggestion is to use an if statement that checks 2 sensors that usually wont both be seeing a line,
//   //then change the value of lineposition to be the string 'cross'
//  //example logic to detect if your linesensor is over a cross:
//  //
//  //if(sensorValues[7] > 750 && sensorValues[0] >750){ //meaning both the far left and right sensors see a value above 750, meaning your robot is VERY LIKLY over a cross
//  // iscross=1; //now I will be sending the string 'cross' to the rpi INSTEAD of a number between 0-7000 that rearesents the line sensor array's position relative to the cross
//  //}
//  //
//   //HEY THIS IS IMPORTANT!!!!!!
//  
////     Serial.print(leftMotor);
////     Serial.print(',');
////     Serial.print(rightMotor);
////     Serial.print(',');
//  //if(iscross == 1){ //this commented out if/else statement is a continuation of the example of logic used to send the rpi the value 8000 both left and right sensors see the line, this is a value that LinePosition will never send, so you can have a catch in the Rpi code that checks if linePosition is greater than 7000
//  //Serial.println('8000')
//  //iscross=0;
//  //} else {
//     Serial.println(linePosition);
//  //}
//newData = false;
//
//}
