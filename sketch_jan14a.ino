#include <Servo.h>
#include <SoftwareSerial.h>

Servo panServo; // Pan servo
Servo tiltServo; // Tilt servo

int panAngle = 80;
int tiltAngle = 80;

int waittime = 1000; // 1 second (in microseconds)
unsigned long interval = 15000000; // 15 seconds (in microseconds)

int topLeft, topRight, downLeft, downRight;

SoftwareSerial esp8266(2, 3); // RX, TX

String AP = "room_1_dmn";    // AP NAME
String PASS = "caminar1";     // AP PASSWORD
String API = "Z57TVIDWWI8LO7V5"; // Write API KEY
String HOST = "api.thingspeak.com";
String PORT = "80";
String field = "field1";
int countTrueCommand;
unsigned long countTrueCommand1;
unsigned long countTimeCommand;
boolean found = false;

unsigned long previousMicros = 0;

void setup()
{
  Serial.begin(9600);
  esp8266.begin(115200);
  sendCommand("AT", 5, "OK", 0);
  sendCommand("AT+CWMODE=1", 5, "OK", 0);
  sendCommand("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"", 20, "OK", 0);
  panServo.attach(6);
  tiltServo.attach(5);

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
}

void sunTracking()
{
  // Read values from photoresistors
  topLeft = analogRead(A2);
  topRight = analogRead(A1);
  downLeft = analogRead(A0);
  downRight = analogRead(A3);

  unsigned long currentMicros = micros();

  // Perform actions only if enough time has passed
  if (currentMicros - previousMicros >= waittime)
  {
    previousMicros = currentMicros; // Save the last time we executed the action

    if (topLeft > topRight)
    {
      panAngle = panAngle - 1;
    }

    if (downLeft > downRight)
    {
      panAngle = panAngle - 1;
    }

    if (topLeft < topRight)
    {
      panAngle = panAngle + 1;
    }

    if (downLeft < downRight)
    {
      panAngle = panAngle + 1;
    }

    if (panAngle > 160)
    {
      panAngle = 160;
    }

    if (panAngle < 0)
    {
      panAngle = 0;
    }

    if (topLeft > downLeft)
    {
      tiltAngle = tiltAngle - 1;
    }
    if (topRight > downRight)
    {
      tiltAngle = tiltAngle - 1;
    }
    if (topLeft < downLeft)
    {
      tiltAngle = tiltAngle + 1;
    }
    if (topRight < downRight)
    {
      tiltAngle = tiltAngle + 1;
    }
    if (tiltAngle > 120)
    {
      tiltAngle = 120;
    }
    if (tiltAngle < 90)
    {
      tiltAngle = 90;
    }

    panServo.write(tiltAngle);
    tiltServo.write(panAngle);

    // Print the values for debugging
    Serial.print("Pan Angle: ");
    Serial.print(panAngle);
    Serial.print(", Tilt Angle: ");
    Serial.println(tiltAngle);
  }
}

void loop()
{
  unsigned long currentMicros = micros();
  sunTracking(); // Continuous tracking
  Serial.println("--------------------------------------------------");
  Serial.println(currentMicros - countTrueCommand1);
  if (currentMicros - countTrueCommand1 >= interval)
  {
    int valSensor = analogRead(A0);
    countTrueCommand1 = micros();
    String getData = "GET /update?api_key=" + API + "&" + field + "=" + String(valSensor);
    sendCommand("AT+CIPMUX=1", 5, "OK", currentMicros);
    sendCommand("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT, 15, "OK", currentMicros);
    sendCommand("AT+CIPSEND=0," + String(getData.length() + 4), 4, ">", currentMicros);
    esp8266.println(getData);
    //delay(1500);

    sendCommand("AT+CIPCLOSE=0", 5, "OK", currentMicros);
  }
}

void sendCommand(String command, int maxTime, char readReplay[], unsigned long currentMicros)
{
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  countTrueCommand1 = currentMicros;
  while (countTimeCommand < (maxTime * 1))
  {
    esp8266.println(command); //at+cipsend
    if (esp8266.find(readReplay)) //ok
    {
      found = true;
      break;
    }

    countTimeCommand++;
  }

  if (found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
    return;
  }

  if (found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
    return;
  }

  found = false;
}
