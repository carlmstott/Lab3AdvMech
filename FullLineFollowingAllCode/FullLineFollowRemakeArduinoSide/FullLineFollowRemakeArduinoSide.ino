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
    qtr.setTypeRC();
    qtr.setSensorPins((const uint8_t[]){22, 23, 24, 25, 26, 27, 28, 29}, SensorCount);

    calibrateSensors();
    Serial.println("<Arduino is ready>");
}

//====================================================

void loop() {
//  if (Serial.available() > 0){
//    Serial.println("seeing something");
//  }

    recvWithStartEndMarkers(); //this function is in charge of taking a peice of data that looks like <17.5,0,16> 
                               //turning it into a string looking like 17.5,0,16 and then setting newdata to true,
                               //letting the rest of the program know a packet of data is ready to be analyzed, does all this without blocking
                               //something interesting about this function that I dont understand is that at higher baud rates (over 9600) the function does not properly recieve the data aroune 50% of the time
                               //it will leave in < symbols, and cut out didgets of the number, etc. I really am unsure why. But at lower baud rates (1200, for example), the function
                               //properly decodes the string it is sent around 95% of the time.
                               //UPDATE this issue has been solved! the problem was python writing bytes on top of other bytes,
                               //solved by adding wait
                               
    //recvUsingSerialReadBytes(); //unused, read function description to learn why
    
    if (newData == true) { //newData will be true when recvWithStartEndMarkers(); has finished recieving a whole set of data from Rpi (a set of data is denoted as being containted between <>)
      
      strcpy(tempChar, receivedChars); //this line makes a copy of recievedChars for parsing in parseData, I do this becasue strtok() will alter any string I give it,I want to preserve the origonal data
      parseData(); //right now parseData only parses a string of 3 numbers seperated by commas into floats
                   //so the string 17.5,0,16 becomes three floats; 17.5, 0, and 16
      linePosition = qtr.readLineBlack(sensorValues);
      
      sendDataToRpi();
                   
    }

    commandMotors(); //we want this to happen outside of our newdata=true loop so it is never blocked
}


//======================================================


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




//============================================

//When using the below function, we still get dropped bytes, so my
//conclusion is that the issue is that bytes are being dropped or sent our of order from the Rpi. I guess the 
//question is how to send data from the RPI without anything being dropped, as of now the solution is to use a very slow
//baud rate. UPDATE SOLUTON FOUND: in python I put a sleep timer inbetween each byte send wich prevents the bytes from
//being written on top of each other

 // void recvUsingSerialReadBytes(){
 // Serial.readBytes(receivedChars,20);
 //newData=true;
//}

//============================================


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
//the values we send back to the Rpi will be thinks like sensor readings. for now I have the Arduino sending the same values 
//it was sent back to the Rpi for debugging reasons, but any numbers can be sent from this function (as strings) and will be parsed as floats
//by the Rpi
  
   //well I lerned you cant mix charachters, I cant combo charachters made in arduino and charachters I got from Rpi and then send that result back to Rpi.
     Serial.print(leftMotor);
     Serial.print(',');
     Serial.print(rightMotor);
     Serial.print(',');
     Serial.println(linePosition);
//we only need to send LinePosition, weve established that I can properly send motor commands
     //Serial.println(receivedChars);

newData = false;

     //Serial.println(tempChar); //ok this prints perfectly on the Rpi, so the issue is NOT with recvWithStartEndMarkers()
}

//=======================================

void commandMotors(){
//analogWrite(3,leftMotor); 
//analogWrite(2,rightMotor);
//  
}
