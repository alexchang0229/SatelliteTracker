#include <Sgp4.h>
#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <RTCZero.h>
#include <AccelStepper.h>

// To be modified by user //
#define SECRET_SSID "ChangVan2017"                                     // Your Network name.
#define SECRET_PASS "ohcanada"                                         // Your Network password.
#define DEBUG                                                          //Enable serial output.
const int timeZone = -7;                                               //Your Time zone.
char TLE[500];                                                         //Variable to store satellite TLEs.
char satnames[4][30] = {"RADARSAT-2", "NEOSSAT", "M3MSAT ", "SCISAT"}; // Names of satellites.
char satURL[4][30] = {"/satcat/tle.php?CATNR=32382", "/satcat/tle.php?CATNR=39089",
                      "/satcat/tle.php?CATNR=41605", "/satcat/tle.php?CATNR=27858"}; // URL of Celestrak TLEs for satellites (In same order as names).
char TLE1[4][70];
char TLE2[4][70];
float myLat = 52.12;
float myLong = -106.663;
float myAlt = 482; // Your latitude, longitude and altitude.
int numSats = 4;   // Number of satellites to track.

// Azimuth stepper pins //
#define AZmotorPin1 9  // IN1 on the ULN2003 driver
#define AZmotorPin2 10 // IN2 on the ULN2003 driver
#define AZmotorPin3 11 // IN3 on the ULN2003 driver
#define AZmotorPin4 12 // IN4 on the ULN2003 driver
// Elevation stepper pins //
#define ELmotorPin1 2
#define ELmotorPin2 3
#define ELmotorPin3 4
#define ELmotorPin4 5

int satAZsteps;
int satELsteps;
int turns = 0;
float oneTurn = 4096;        // Number of steps per one rotation for stepper motor.
#define MotorInterfaceType 8 // Define the AccelStepper interface type; 4 wire motor in half step mode:
AccelStepper stepperAZ = AccelStepper(MotorInterfaceType, AZmotorPin1, AZmotorPin3, AZmotorPin2, AZmotorPin4);
AccelStepper stepperEL = AccelStepper(MotorInterfaceType, ELmotorPin1, ELmotorPin3, ELmotorPin2, ELmotorPin4);

Sgp4 sat;
RTCZero rtc;

char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password (use for WPA, or use as key for WEP)
int i;
int k;
int SAT;
int nextSat;
int AZstart;
long passEnd;
int satVIS;
char satname[] = " ";
int passStatus = 0;
char server[] = "104.168.149.178"; //Web address to get TLE (CELESTRAK)
int year;
int mon;
int day;
int hr;
int minute;
double sec;
int today;
long nextpassEpoch;
long upcomingPasses[4];
int status = WL_IDLE_STATUS;
unsigned long unixtime;
unsigned long testTime = 1593789900;
unsigned long timeNow = 0;

// Used for Network Time Protocol (NTP)
unsigned int localPort = 2390;        // local port to listen for UDP packets
IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
const int NTP_PACKET_SIZE = 48;       // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE];   //buffer to hold incoming and outgoing packets
WiFiUDP Udp;                          // A UDP instance to let us send and receive packets over UDP

// Initialize the Ethernet client library
WiFiClient client;

void setup()
{

//Initialize serial and wait for port to open:
#ifdef DEBUG
  Serial.begin(9600);
  while (!Serial)
  {
    delay(10);
  }
#endif
  // Setup stepper movements //
  stepperEL.setMaxSpeed(1000);
  stepperEL.setCurrentPosition(-227); // Elevation stepper starts at -227 steps (20 degrees above horizon).
  stepperEL.setAcceleration(100);
  stepperAZ.setMaxSpeed(1000);
  stepperAZ.setCurrentPosition(0); // Azimuth stepper starts at 0.
  stepperAZ.setAcceleration(100);

  // check for the presence of wifi shield:
  if (WiFi.status() == WL_NO_SHIELD)
  {
#ifdef DEBUG
    Serial.println("WiFi shield not present");
#endif
    while (true)
      ; // don't continue:
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED)
  {
#ifdef DEBUG
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
#endif
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 5 seconds for connection:
    delay(5000);
  }
#ifdef DEBUG
  Serial.println("Connected to wifi");
#endif

  sat.site(myLat, myLong, myAlt); //set location latitude[°], longitude[°] and altitude[m]
  rtc.begin();

  // Get Unix time //
  while (timeNow == 0)
  {
    unixtime = readLinuxEpochUsingNTP();
    rtc.setEpoch(unixtime);
    today = rtc.getDay();
    timeNow = rtc.getEpoch();
  }
#ifdef DEBUG
  Serial.println("unixtime: " + String(unixtime));
#endif

  // Get TLEs //
  for (SAT = 0; SAT < numSats; SAT++)
  {
    getTLE(SAT);
    sat.init(satname, TLE1[SAT], TLE2[SAT]); //initialize satellite parameters
    sat.findsat(timeNow);
    upcomingPasses[SAT] = Predict(1);
  }
  nextSat = nextSatPass(upcomingPasses);
  sat.init(satname, TLE1[nextSat], TLE2[nextSat]);
  Predict(1);

// Print obtained TLE in serial. //
#ifdef DEBUG
  for (SAT = 0; SAT < numSats; SAT++)
  {
    Serial.println("TLE set #:" + String(SAT));
    for (i = 0; i < 70; i++)
    {
      Serial.print(TLE1[SAT][i]);
    }
    Serial.println();
    for (i = 0; i < 70; i++)
    {
      Serial.print(TLE2[SAT][i]);
    }
    Serial.println();
  }
  Serial.println("Next satellite: " + String(nextSat));
#endif
}

void loop()
{
  timeNow = rtc.getEpoch(); // Update time.
  sat.findsat(timeNow);
  satAZsteps = round(sat.satAz * oneTurn / 360); //Convert degrees to stepper steps
  satELsteps = -round(sat.satEl * oneTurn / 360);
#ifdef DEBUG
  invjday(sat.satJd, timeZone, true, year, mon, day, hr, minute, sec);
  Serial.println("\nLocal time: " + String(day) + '/' + String(mon) + '/' + String(year) + ' ' + String(hr) + ':' + String(minute) + ':' + String(sec));
  Serial.println("azimuth = " + String(sat.satAz) + " elevation = " + String(sat.satEl) + " distance = " + String(sat.satDist));
  Serial.println("latitude = " + String(sat.satLat) + " longitude = " + String(sat.satLon) + " altitude = " + String(sat.satAlt));
  Serial.println("AZStep pos: " + String(stepperAZ.currentPosition()));
#endif

  while (true)
  {
    if (nextpassEpoch - timeNow < 300 && nextpassEpoch + 5 - timeNow > 0)
    {
#ifdef DEBUG
      Serial.println("Status: Pre-pass");
      Serial.println("Next satellite is #: " + String(satnames[nextSat]) + " in: " + String(nextpassEpoch - timeNow));
#endif
      prepass();
      break;
    }
    if (sat.satVis != -2)
    {
#ifdef DEBUG
      Serial.println("Status: In pass");
#endif
      inPass();
      break;
    }
    if (timeNow - passEnd < 120)
    {
#ifdef DEBUG
      Serial.println("Status: Post-pass");
#endif
      postpass();
      break;
    }
    if (sat.satVis == -2)
    {
#ifdef DEBUG
      Serial.println("Status: Standby");
      Serial.println("Next satellite is: " + String(satnames[nextSat]) + " in: " + String(nextpassEpoch - timeNow));
#endif
      standby();
      break;
    }
  }
  delay(20);

  // Update TLE & Unix time everyday.//
  if (passStatus == 0 && today != rtc.getDay())
  {
    for (SAT = 0; SAT < numSats; SAT++)
    {
      getTLE(SAT);
    }
    unixtime = readLinuxEpochUsingNTP();
    rtc.setEpoch(unixtime);
    today = rtc.getDay();
#ifdef DEBUG
    Serial.println("Updating TLEs and time");
#endif
  }
}

int nextSatPass(long _nextpassEpoch[4])
{ // Replace with number of satellites
  for (i = 0; i < numSats; ++i)
  {
    if (_nextpassEpoch[0] - timeNow >= _nextpassEpoch[i] - timeNow)
    {
      _nextpassEpoch[0] = _nextpassEpoch[i];
      nextSat = i;
    }
  }
  return nextSat;
}

void standby()
{

  // Azimuth //
  stepperAZ.runToNewPosition(0);
  // ELEVATION //
  stepperEL.runToNewPosition(-227); //Standby at 20 degrees above horizon

  digitalWrite(AZmotorPin1, LOW);
  digitalWrite(AZmotorPin2, LOW);
  digitalWrite(AZmotorPin3, LOW);
  digitalWrite(AZmotorPin4, LOW);

  digitalWrite(ELmotorPin1, LOW);
  digitalWrite(ELmotorPin2, LOW);
  digitalWrite(ELmotorPin3, LOW);
  digitalWrite(ELmotorPin4, LOW);
}

void prepass()
{
  // Pass is less than 300 seconds (5 mins) away, move antenna to start location and wait.
  if (AZstart < 360 && AZstart > 180)
  {
    AZstart = AZstart - 360; //Goes to start counter-clockwise if closer.
  }
  stepperAZ.runToNewPosition(AZstart * oneTurn / 360);
  stepperEL.runToNewPosition(0);

  digitalWrite(AZmotorPin1, LOW);
  digitalWrite(AZmotorPin2, LOW);
  digitalWrite(AZmotorPin3, LOW);
  digitalWrite(AZmotorPin4, LOW);

  digitalWrite(ELmotorPin1, LOW);
  digitalWrite(ELmotorPin2, LOW);
  digitalWrite(ELmotorPin3, LOW);
  digitalWrite(ELmotorPin4, LOW);
}

void inPass()
{

  // Handle zero crossings
  if (AZstart < 0)
  {
    satAZsteps = satAZsteps - oneTurn;
  }
  if (satAZsteps - stepperAZ.currentPosition() > 100)
  {
    stepperAZ.setCurrentPosition(stepperAZ.currentPosition() + oneTurn);
    turns--;
  }
  if (satAZsteps - stepperAZ.currentPosition() < -100)
  {
    stepperAZ.setCurrentPosition(stepperAZ.currentPosition() - oneTurn);
    turns++;
  }

  // Update stepper position
  stepperAZ.runToNewPosition(satAZsteps);
  stepperEL.runToNewPosition(satELsteps);
  passEnd = timeNow;
  passStatus = 1;
}

void postpass()
{
#ifdef DEBUG
  Serial.println("Post pass time left: " + String(passEnd + 120 - timeNow));
#endif
  if (timeNow - passEnd > 90)
  {
    if (turns > 0)
    {
      stepperAZ.setCurrentPosition(stepperAZ.currentPosition() + oneTurn);
      turns--;
    }
    if (turns < 0)
    {
      stepperAZ.setCurrentPosition(stepperAZ.currentPosition() - oneTurn);
      turns++;
    }
  }
  if (passStatus == 1 && timeNow - passEnd > 100)
  {
    for (SAT = 0; SAT < numSats; SAT++)
    {
      sat.init(satname, TLE1[SAT], TLE2[SAT]);
      sat.findsat(timeNow);
      upcomingPasses[SAT] = Predict(1);
#ifdef DEBUG
      Serial.println("Next pass for Satellite #: " + String(SAT) + " in: " + String(upcomingPasses[SAT] - timeNow));
#endif
    }
    nextSat = nextSatPass(upcomingPasses);
    sat.init(satname, TLE1[nextSat], TLE2[nextSat]);
    Predict(1);
    passStatus = 0;
  }
}
