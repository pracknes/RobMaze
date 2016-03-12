///////EL ATTACH PARA LOS LACK OF PROGRESS ESTAN EN EL PIN #3//////////
//Cambios

#include <Encoder.h> //para que crees...
#include <NewPing.h>
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include <SharpIR.h> //sharps
#include <Servo.h>
#include "I2Cdev.h" //MPU
#include "MPU6050_6Axis_MotionApps20.h" //MPU
#include <SparkFunMLX90614.h>
#define MAX_DISTANCE 12
#define model 1080 //modelo del sharp GP2Y0A21Y
#define OUTPUT_READABLE_YAWPITCHROLL //Yaw Pitch Roll del MPU

/////////////////
////ALGROTIMO////
/////////////////

//Paso
int iPass[400];
//Posicion
int iPos[400];
//posibles movimientos
int iPossibility[400];
//coordenadas posibles
int iRun[400][4];
//counter de los pasos
int iCounter = -1;
//coordenada
int iCoord = 2020;
//direcciones
bool bDireccion[4] = {false, false, false, false};
//Direccion
int iDireccion = 1;
int iMovimiento = 0;

//////////////////
///////MPU////////
//////////////////

MPU6050 mpu;
#define OUTPUT_READABLE_YAWPITCHROLL

bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

uint8_t teapotPacket[14] = { '$', 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0x00, 0x00, '\r', '\n' };

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
  mpuInterrupt = true;
}

//////////////////
/////MOTORES//////
//////////////////

byte motDerE1 = 8;
byte motDerE2 = 9;

byte motDerA1 = 11;
byte motDerA2 = 10;

byte motIzqE1 = 7;
byte motIzqE2 = 6;

byte motIzqA1 = 5;
byte motIzqA2 = 4;

const byte MotD = 255;
const byte MotI = 255;
const byte MotDM = 150;
const byte MotIM = 90;

//////////////////
/////ENCODERS/////
//////////////////

Encoder EncDerE(18, 19);
Encoder Enc2(17, 27);

long oldPosition  = -999;

int const90 = 3600;

const int const30 = 6000;

//////////////////
//////CALOR///////
//////////////////

IRTherm therm1; //IZQUIERDA ADELANTE
IRTherm therm2; //DERECHA ATRAS
IRTherm therm3; //IZQUIERDA ATRAS
IRTherm therm4; //DERECHA ENFRENTE

//////////////////
//////SHARPS//////
//////////////////

byte Enf = A0;
byte Izq = A1;
byte Der =  A2;

SharpIR SharpEn(Enf, 25, 93, model);
SharpIR SharpIz(Izq, 25, 93, model);
SharpIR SharpDe(Der, 25, 93, model);

/////////////////
//////SERVO//////
/////////////////

Servo Dispensador;
const int pinservo = 47;
const int PulsoMinimo = 650;
const int PulsoMaximo = 2550;

//////////////////
//////COLOR//////
//////////////////
const byte s0 = 13;
const byte s1 = 12;
const byte s2 = 29;
const byte s3 = 1;
const byte out = 14;

//////////////////
////////LCD///////
//////////////////

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

//////////////////
///ULTRASONICOS///
//////////////////

byte Trigger1 = 28;
byte Echo1 = 26;

NewPing sonar1(Trigger1, Echo1, MAX_DISTANCE);  //llamar a la funcion para saber la distancia con sonar1.ping_cm();

byte Trigger2 = 38;
byte Echo2 = 36;

NewPing sonar2(Trigger2, Echo2, MAX_DISTANCE);  //llamar a la funcion para saber la distancia con sonar1.ping_cm();

byte Trigger3 = 42;
byte Echo3 = 40;


NewPing sonar3(Trigger3, Echo3, MAX_DISTANCE);  //llamar a la funcion para saber la distancia con sonar8.ping_cm();

byte Trigger4 = 51;
byte Echo4 = 53;

NewPing sonar4(Trigger4, Echo4, MAX_DISTANCE);  //llamar a la funcion para saber la distancia con sonar8.ping_cm();


byte Trigger7 = 23;
byte Echo7 = 25;

NewPing sonar7(Trigger7, Echo7, MAX_DISTANCE);  //llamar a la funcion para saber la distancia con sonar8.ping_cm();

byte Trigger8 = 24;
byte Echo8 = 22;

NewPing sonar8(Trigger8, Echo8, MAX_DISTANCE);  //llamar a la funcion para saber la distancia con sonar8.ping_cm();

void setup() {
  Serial.begin(115200);

  //MPU
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
  TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz). Comment this line if having compilation difficulties with TWBR.
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif
  while (!Serial); // wait for Leonardo enumeration, others continue immediately

  Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();

  // verify connection
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  // load and configure the DMP
  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  mpu.setXGyroOffset(4);
  mpu.setYGyroOffset(-6);
  mpu.setZGyroOffset(101);
  mpu.setZAccelOffset(1357); // 1688 factory default for my test chip

  if (devStatus == 0) {
    // turn on the DMP, now that it's ready
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
    attachInterrupt(0, dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    dmpReady = true;

    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {

    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }

  //CALOR
  therm1.begin(0x1C);
  therm1.setUnit(TEMP_C);

  therm2.begin(0x2C); //derecha atras
  therm2.setUnit(TEMP_C);

  therm3.begin(0x3C);
  therm3.setUnit(TEMP_C);

  therm4.begin(0x4C);
  therm4.setUnit(TEMP_C);

  //MOTOR
  pinMode(motDerE1, OUTPUT);
  pinMode(motDerE2, OUTPUT);

  pinMode(motDerA1, OUTPUT);
  pinMode(motDerA2, OUTPUT);

  pinMode(motIzqE1, OUTPUT);
  pinMode(motIzqE2, OUTPUT);

  pinMode(motIzqA1, OUTPUT);
  pinMode(motIzqA2, OUTPUT);

  analogWrite(motDerE1, LOW);
  analogWrite(motDerE2, LOW);

  analogWrite(motDerA1, LOW);
  analogWrite(motDerA2, LOW);

  analogWrite(motIzqE1, LOW);
  analogWrite(motIzqE2, LOW);

  analogWrite(motIzqA1, LOW);
  analogWrite(motIzqA2, LOW);

  //LCD
  lcd.begin(16, 2);  // initialize the lcd for 16 chars 2 lines, turn on backlight
  lcd.noBacklight(); // finish with backlight on
  lcd.setCursor(0, 0); //Start at character 4 on line 0
  lcd.print(";)");

  //SERVO
  Dispensador.attach(pinservo, PulsoMinimo, PulsoMaximo);

  //COLOR
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(out , INPUT);

  digitalWrite(s0, LOW); //HIGH, HIGH dif de 20
  digitalWrite(s1, HIGH);

  digitalWrite(s2, HIGH);
  digitalWrite(s3, LOW);
}


double MPUY()
{
  double iReturn;
  do {

    // wait for MPU interrupt or extra packet(s) available
    while (!mpuInterrupt && fifoCount < packetSize) {
      // other program behavior stuff here
      // .
      // .
      // .
      // if you are really paranoid you can frequently test in between other
      // stuff to see if mpuInterrupt is true, and if so, "break;" from the
      // while() loop to immediately process the MPU data
      // .
      // .
      // .
    }

    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
      // reset so we can continue cleanly
      mpu.resetFIFO();
      //Serial.println(F("FIFO overflow!"));
      iReturn = 99999;

      // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (mpuIntStatus & 0x02) {
      // wait for correct available data length, should be a VERY short wait
      while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

      // read a packet from FIFO
      mpu.getFIFOBytes(fifoBuffer, packetSize);

      // track FIFO count here in case there is > 1 packet available
      // (this lets us immediately read more without waiting for an interrupt)
      fifoCount -= packetSize;

#ifdef OUTPUT_READABLE_QUATERNION
      // display quaternion values in easy matrix form: w x y z
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      Serial.print("quat\t");
      Serial.print(q.w);
      Serial.print("\t");
      Serial.print(q.x);
      Serial.print("\t");
      Serial.print(q.y);
      Serial.print("\t");
      Serial.println(q.z);
#endif

#ifdef OUTPUT_READABLE_EULER
      // display Euler angles in degrees
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      mpu.dmpGetEuler(euler, &q);
      Serial.print("euler\t");
      Serial.print(euler[0] * 180 / M_PI);
      Serial.print("\t");
      Serial.print(euler[1] * 180 / M_PI);
      Serial.print("\t");
      Serial.println(euler[2] * 180 / M_PI);
#endif

#ifdef OUTPUT_READABLE_YAWPITCHROLL
      // display Euler angles in degrees
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      mpu.dmpGetGravity(&gravity, &q);
      mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
      Serial.print("ypr\t");
      Serial.print(ypr[0] * 180 / M_PI);
      Serial.print("\t");
      Serial.print(ypr[1] * 180 / M_PI);
      Serial.print("\t");
      Serial.println(ypr[2] * 180 / M_PI);
      iReturn = ypr[0] * 180 / M_PI;
#endif
    }
  } while (iReturn == 99999 || iReturn == -31073);
  return iReturn;
}

double MPUP()
{
  double iReturn;
  do {

    // wait for MPU interrupt or extra packet(s) available
    while (!mpuInterrupt && fifoCount < packetSize) {
      // other program behavior stuff here
      // .
      // .
      // .
      // if you are really paranoid you can frequently test in between other
      // stuff to see if mpuInterrupt is true, and if so, "break;" from the
      // while() loop to immediately process the MPU data
      // .
      // .
      // .
    }

    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
      // reset so we can continue cleanly
      mpu.resetFIFO();
      //Serial.println(F("FIFO overflow!"));
      iReturn = 99999;

      // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (mpuIntStatus & 0x02) {
      // wait for correct available data length, should be a VERY short wait
      while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

      // read a packet from FIFO
      mpu.getFIFOBytes(fifoBuffer, packetSize);

      // track FIFO count here in case there is > 1 packet available
      // (this lets us immediately read more without waiting for an interrupt)
      fifoCount -= packetSize;

#ifdef OUTPUT_READABLE_QUATERNION
      // display quaternion values in easy matrix form: w x y z
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      Serial.print("quat\t");
      Serial.print(q.w);
      Serial.print("\t");
      Serial.print(q.x);
      Serial.print("\t");
      Serial.print(q.y);
      Serial.print("\t");
      Serial.println(q.z);
#endif

#ifdef OUTPUT_READABLE_EULER
      // display Euler angles in degrees
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      mpu.dmpGetEuler(euler, &q);
      Serial.print("euler\t");
      Serial.print(euler[0] * 180 / M_PI);
      Serial.print("\t");
      Serial.print(euler[1] * 180 / M_PI);
      Serial.print("\t");
      Serial.println(euler[2] * 180 / M_PI);
#endif

#ifdef OUTPUT_READABLE_YAWPITCHROLL
      // display Euler angles in degrees
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      mpu.dmpGetGravity(&gravity, &q);
      mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
      Serial.print("ypr\t");
      Serial.print(ypr[0] * 180 / M_PI);
      Serial.print("\t");
      Serial.print(ypr[1] * 180 / M_PI);
      Serial.print("\t");
      Serial.println(ypr[2] * 180 / M_PI);
      iReturn = ypr[1] * 180 / M_PI;
#endif
    }
  } while (iReturn == 99999 || iReturn < 0);
  return iReturn;
}

bool Negro()
{
  bool iReturn = false;
  int color = pulseIn(out, LOW);
  if (color > 2800)
  {
    iReturn = true;
  }
  color = pulseIn(out, LOW);
  if (color > 2800)
  {
    iReturn = true;
  }
  return iReturn;
}

double MPUR()
{
  double iReturn;
  do {

    // wait for MPU interrupt or extra packet(s) available
    while (!mpuInterrupt && fifoCount < packetSize) {
      // other program behavior stuff here
      // .
      // .
      // .
      // if you are really paranoid you can frequently test in between other
      // stuff to see if mpuInterrupt is true, and if so, "break;" from the
      // while() loop to immediately process the MPU data
      // .
      // .
      // .
    }

    // reset interrupt flag and get INT_STATUS byte
    mpuInterrupt = false;
    mpuIntStatus = mpu.getIntStatus();

    // get current FIFO count
    fifoCount = mpu.getFIFOCount();

    // check for overflow (this should never happen unless our code is too inefficient)
    if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
      // reset so we can continue cleanly
      mpu.resetFIFO();
      //Serial.println(F("FIFO overflow!"));
      iReturn = 99999;

      // otherwise, check for DMP data ready interrupt (this should happen frequently)
    } else if (mpuIntStatus & 0x02) {
      // wait for correct available data length, should be a VERY short wait
      while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

      // read a packet from FIFO
      mpu.getFIFOBytes(fifoBuffer, packetSize);

      // track FIFO count here in case there is > 1 packet available
      // (this lets us immediately read more without waiting for an interrupt)
      fifoCount -= packetSize;

#ifdef OUTPUT_READABLE_QUATERNION
      // display quaternion values in easy matrix form: w x y z
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      Serial.print("quat\t");
      Serial.print(q.w);
      Serial.print("\t");
      Serial.print(q.x);
      Serial.print("\t");
      Serial.print(q.y);
      Serial.print("\t");
      Serial.println(q.z);
#endif

#ifdef OUTPUT_READABLE_EULER
      // display Euler angles in degrees
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      mpu.dmpGetEuler(euler, &q);
      Serial.print("euler\t");
      Serial.print(euler[0] * 180 / M_PI);
      Serial.print("\t");
      Serial.print(euler[1] * 180 / M_PI);
      Serial.print("\t");
      Serial.println(euler[2] * 180 / M_PI);
#endif

#ifdef OUTPUT_READABLE_YAWPITCHROLL
      // display Euler angles in degrees
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      mpu.dmpGetGravity(&gravity, &q);
      mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
      Serial.print("ypr\t");
      Serial.print(ypr[0] * 180 / M_PI);
      Serial.print("\t");
      Serial.print(ypr[1] * 180 / M_PI);
      Serial.print("\t");
      Serial.println(ypr[2] * 180 / M_PI);
      iReturn = ypr[2] * 180 / M_PI;
#endif
    }
  } while (iReturn == 99999 || iReturn == -31073 || iReturn > 1000);
  return iReturn;
}

//MOTORES
void Detenerse()
{
  analogWrite(motDerE1, 0);
  analogWrite(motDerE2, 0);

  analogWrite(motDerA1, 0);
  analogWrite(motDerA2, 0);

  analogWrite(motIzqE1, 0);
  analogWrite(motIzqE2, 0);

  analogWrite(motIzqA1, 0);
  analogWrite(motIzqA2, 0);
}

void Adelante()
{
  analogWrite(motDerE1, 160); //190
  analogWrite(motDerE2, 0);

  analogWrite(motDerA1, 220); //255  //205
  analogWrite(motDerA2, 0);

  analogWrite(motIzqE1, 148); //178  //132
  analogWrite(motIzqE2, 0);

  analogWrite(motIzqA1, 148); //178  //132
  analogWrite(motIzqA2, 0);
}

void Izquierda()
{
  analogWrite(motDerE1, 160);
  analogWrite(motDerE2, 0);

  analogWrite(motDerA1, 220);
  analogWrite(motDerA2, 0);

  analogWrite(motIzqE1, 0);
  analogWrite(motIzqE2, 148);

  analogWrite(motIzqA1, 0);
  analogWrite(motIzqA2, 148);
}

void Derecha()
{
  analogWrite(motDerE1, 0);
  analogWrite(motDerE2, 160);

  analogWrite(motDerA1, 0);
  analogWrite(motDerA2, 220);

  analogWrite(motIzqE1, 148);
  analogWrite(motIzqE2, 0);

  analogWrite(motIzqA1, 148);
  analogWrite(motIzqA2, 0);
}

void Atras()
{
  analogWrite(motDerE1, 0); //190
  analogWrite(motDerE2, 160);

  analogWrite(motDerA1, 0); //255  //205
  analogWrite(motDerA2, 220);

  analogWrite(motIzqE1, 0); //178  //132
  analogWrite(motIzqE2, 148);

  analogWrite(motIzqA1, 0); //178  //132
  analogWrite(motIzqA2, 148);
}

void DerechaM()
{
  analogWrite(motDerE1, 0);
  analogWrite(motDerE2, 180);

  analogWrite(motDerA1, 200);
  analogWrite(motDerA2, 0);

  analogWrite(motIzqE1, 150);
  analogWrite(motIzqE2, 0);

  analogWrite(motIzqA1, 0);
  analogWrite(motIzqA2, 160);
}

void IzquierdaM()
{
  analogWrite(motDerE1, 180);
  analogWrite(motDerE2, 0);

  analogWrite(motDerA1, 0);
  analogWrite(motDerA2, 200);

  analogWrite(motIzqE1, 0);
  analogWrite(motIzqE2, 150);

  analogWrite(motIzqA1, 160);
  analogWrite(motIzqA2, 0);
}

void IzquierdaM30()
{
  EncDerE.write(0);
  IzquierdaM();
  int Enc = EncDerE.read();
  while (Enc < 8600)
  {
    Serial.println(Enc);
    Enc = EncDerE.read();
  }
  Detenerse();
}

void GiroDer90()
{
  lcd.clear();
  lcd.print("GiroDer90");
  delay(1000);
  EncDerE.write(0);
  int Enc = EncDerE.read();
  //Serial.println(EncDerE.read());
  while (Enc < const90 )
  {
    Derecha();
    Enc = EncDerE.read();
  }
  Detenerse();
}

void Adelante10()
{
  EncDerE.write(0);
  int Enc = EncDerE.read();
  int const10 = const30 / 3;

  while (Enc < const10)
  {
    Adelante();
    Enc = EncDerE.read();
  }
  Detenerse();
}

void Adelante30()
{
  lcd.clear();
  lcd.print("Adelante30");
  Adelante10();
  Detectar();
  Adelante10();
  Detectar();
  Adelante10();
  Detectar();
  Detenerse();
}

void Atras30()
{
  lcd.clear();
  lcd.print("Atras30");
  EncDerE.write(0);
  int Enc = EncDerE.read();
  while (Enc > const30 * -1)
  {
    Atras();
    Enc = EncDerE.read();
    Serial.println(Enc);
  }
  Detenerse();
}

void GiroIzq90()
{
  lcd.clear();
  lcd.print("GiroIzq90");
  delay(1000);
  EncDerE.write(0);
  int Enc = EncDerE.read();
  //Serial.println(EncDerE.read());
  while (Enc > const90 * -1 )
  {
    Izquierda();
    Enc = EncDerE.read();
  }
  Detenerse();
}

bool ParedDer()
{
  bool Pared = true;
  int Sharp = SharpDe.distance();
  Serial.println(Sharp);
  if (Sharp >= 14)
  {
    Pared = false;
  }
  return Pared;
}

bool ParedIzq()
{
  bool Pared = true;
  int Sharp = SharpIz.distance();
  Serial.println(Sharp);
  if (Sharp >= 14)
  {
    Pared = false;
  }
  return Pared;
}

bool ParedEnf()
{
  bool Pared = true;
  int Sharp = SharpEn.distance();
  if (Sharp > 14)
  {
    Pared = false;
  }
  return Pared;
}

bool RampaArriba()
{
  bool Rampa = false;
  lcd.clear();
  lcd.print("RampaArriba");
  int U3 = sonar3.ping_cm();
  delay(30);
  int U4 = sonar4.ping_cm();
  delay(29);
  int Sharp = SharpDe.distance();
  int SharpEnf = SharpEn.distance();
  int SharpIzq = SharpIz.distance();
  Serial.println(U3);
  Serial.println(U4);
  Serial.println(Sharp);
  int Revision = 0;

  if (SharpEnf < 9 && SharpIzq < 9)
  {
    if (Sharp < 18 && (U3 == 0 || U4 == 0))
    {
      Rampa = true;
    }
    Serial.println(Rampa);
    if (Rampa == true)
    {
      GiroDer90();
      delay(200);
      Adelante30();
      delay(150);
      Revision = MPUP();
      lcd.print(Revision);
      delay(10);
      Revision = MPUP();
      lcd.print(Revision);
      delay(500);
      if (Revision > 18)
      {
        Rampa = true;
        Adelante();
        delay(10000);
        Detenerse();
        delay(600);
        Acejarse();
      }
      else
      {
        Rampa = false;
        Acejarse();
      }
    }
  }
  return Rampa;
}

void RampaAbajoIzq()
{
  lcd.clear();
  lcd.print("RampaAbajoIzq");
  int SharpDer = SharpDe.distance();
  int SharpEnf = SharpEn.distance();
  int SharpIzq = SharpIz.distance();
  int Roll = 0;
  if (SharpEnf < 9 && SharpDer < 9)
  {
    IzquierdaM30();
    delay(50);
    Roll = MPUR();
    lcd.setCursor(0, 1);
    lcd.print(Roll);
    if (Roll < -8)
    {
      IzquierdaM();
      delay(5000);
    }
    else
    {
      delay(100);
      GiroIzq90();
      delay(100);
    }
    Detenerse();
  }
}

bool HoyoNegro()
{
  bool Hoyo = false;
  lcd.clear();
  lcd.print("HoyoNegro");
  int DistIzq = SharpIz.distance();

  if (Negro())
  {
    Hoyo = true;
    Atras30();
    delay(500);
    DistIzq = SharpIz.distance();
    if (DistIzq > 20)
    {
      GiroIzq90();
      delay(500);
      Adelante30();
    }
    else
    {
      GiroIzq90();
      delay(500);
      GiroIzq90();
    }
  }
  return Hoyo;
}

void Detectar()
{
  therm1.read();
  therm2.read();
  therm3.read();
  therm4.read();
  int Therm1 = therm1.object();
  int Therm2 = therm2.object(); //Derecha
  int Therm3 = therm3.object();
  int Therm4 = therm4.object(); //Derecha
  int Temp = 23;
  //Serial.println(therm2.object());
  if (Therm1 > Temp || Therm2 > Temp || Therm3 > Temp || Therm4 > Temp)
  {
    Detenerse();
    lcd.clear();
    lcd.print("VICTIMA");
    lcd.setCursor(0, 1);
    lcd.print("DETECTADA");
    for (int iI = 75; iI < 113; iI++)
    {
      Dispensador.write(iI);
      delay(1);
    }
    delay(500);
    for (int iI = 113; iI > 75; iI--)
    {
      Dispensador.write(iI);
      delay(1);
    }
    for (int iI = 0; iI < 10; iI++)
    {
      lcd.noBacklight();
      delay(250);
      lcd.backlight();
      delay(250);
    }
  }
}


void Acomodo()
{
  lcd.clear();
  lcd.print("Acomodo");
  delay(29);
  int U1 = sonar7.ping_cm();
  delay(29);
  int Sharp = SharpIz.distance();
  int U2 = sonar8.ping_cm();
  delay(29);
  int DifU = U1 - U2;
  int Dif1 = Sharp - U1;
  int Dif2 = Sharp - U2;

  if (DifU >= 2 || DifU <= -2)
  {
    do {
      if (DifU >= 2)
      {
        Derecha();
        delay(80);
      }
      if (DifU <= -2)
      {
        Izquierda();
        delay(80);
      }
      Detenerse();
      delay(100);
      U1 = sonar7.ping_cm();
      delay(29);
      Sharp = SharpIz.distance();
      U2 = sonar8.ping_cm();
      delay(29);
      DifU = U1 - U2;
      Dif1 = Sharp - U1;
      Dif2 = Sharp - U2;
    } while (DifU >= 2 || DifU <= -2);
  }
}

void AcejarseDerecha()
{
  int Dist = SharpDe.distance();

  //Serial.println("entro 1 if");
  Dist = SharpDe.distance();

  while (Dist != 8 || Dist != 7 || Dist != 9) {
    if (Dist < 7)
    {
      IzquierdaM();
      while (Dist < 7)
      {

        Dist = SharpDe.distance();
        Serial.println(SharpDe.distance());
      }
    }
    else if (Dist > 9)
    {
      DerechaM();
      while (Dist > 9)
      {

        Dist = SharpDe.distance();
        Serial.println(SharpDe.distance());
      }
    }
    else if (Dist == 8 || Dist == 7 || Dist == 9)
      break;
    Detenerse();
  }

}

//izquierda
void AcejarseIzquierda()
{
  int Dist2 = SharpIz.distance();

  //Serial.println("entro 1 if");
  Dist2 = SharpIz.distance();

  while (Dist2 != 8 || Dist2 != 7 || Dist2 != 9) {
    if (Dist2 < 7)
    {
      DerechaM();
      while (Dist2 < 7)
      {

        Dist2 = SharpIz.distance();
        Serial.println(SharpIz.distance());
      }
    }
    else if (Dist2 > 9)
    {
      IzquierdaM();
      while (Dist2 > 9)
      {

        Dist2 = SharpIz.distance();
        Serial.println(SharpIz.distance());
      }
    }
    else if (Dist2 == 8 || Dist2 == 7 || Dist2 == 9)
      break;
    Detenerse();
  }


}

//enfrente
void AcejarseEnfrente()
{
  int SharpEnf = SharpEn.distance();
  int U = sonar1.ping_cm();
  Serial.print(SharpEnf); Serial.print("\t"); Serial.println(U);
  if (SharpEnf < 5 || (U < 4 && U != 0))
  {
    Atras();
    while (SharpEnf < 5 || U < 4)
    {
      SharpEnf = SharpEn.distance();
      U = sonar1.ping_cm();
      if (U >= 4 || U == 0)
      {
        break;
      }
      delay(10);
    }
    Detenerse();
  }

  else
  {

    Adelante();
    while (SharpEnf > 6 && SharpEnf < 14)
    {
      SharpEnf = SharpEn.distance();
      int U = sonar1.ping_cm();
      if (U < 4 && U != 0)
      {
        break;
      }
    }
    Detenerse();
  }
}

void Acejarse()
{
  lcd.clear();
  lcd.print("acejarse");
  bool Der = ParedDer();
  delay(100);
  bool Enf = ParedEnf();
  delay(100);
  bool Izq = ParedIzq();
  delay (100);

  if (Der && Enf)
  {
    AcejarseDerecha();
    delay(200);
    AcejarseEnfrente();
  }

  else if (Izq && Enf)
  {
    AcejarseIzquierda();
    delay(200);
    AcejarseEnfrente();
  }

  else if (Der)
  {
    AcejarseDerecha();
  }
  else if (Izq)
  {
    AcejarseIzquierda();
  }
  else if (Enf)
  {
    AcejarseEnfrente();
  }
}
void Revisiones()
{
  Acejarse();
  delay(300);
  Acomodo();
  delay(50);
  bool Hoyo = HoyoNegro();
  delay(50);
  if (Hoyo == false)
  {
    bool Rampa = RampaArriba();
    delay(29);
    if (Rampa == false)
    {
      RampaAbajoIzq();
    }
  }
}

void SeguirDerecha()
{
  bool ParedD = ParedDer();
  bool ParedE = ParedEnf();
  delay(100);
  ParedD = ParedDer();
  ParedE = ParedEnf();
  delay(100);
  ParedD = ParedDer();
  ParedE = ParedEnf();

  if (ParedD == false)
  {
    GiroDer90();
    delay(200);
    Adelante30();
    delay(1000);
    Detenerse();
    delay(500);
  }
  else if (ParedE == false)
  {
    Adelante30();
    delay(100);
    Detenerse();
    delay(1000);

  }
  else if (ParedD == true && ParedE == true)
  {
    GiroIzq90();
    delay(100);
  }
  Revisiones();
}

////////////////
///ALGORTIMO////
////////////////

//Actualiza el valor de coord
void setCoord(int iDirecc, int iMov)
{
  if (iDirecc == 1)
  {
    if (iMov == 1)
    {
      iCoord += 100;
    }
    else if (iMov == 2)
    {
      iCoord += 1;
    }
    else if (iMov == 3)
    {
      iCoord -= 100;
    }
    else if (iMov == 4)
    {
      iCoord -= 1;
    }
  }

  else if (iDirecc == 2)
  {
    if (iMov == 1)
    {
      iCoord += 1;
    }
    else if (iMov == 2)
    {
      iCoord -= 100;
    }
    else if (iMov == 3)
    {
      iCoord -= 1;
    }
    else if (iMov == 4)
    {
      iCoord += 100;
    }
  }

  else if (iDirecc == 3)
  {
    if (iMov == 1)
    {
      iCoord -= 100;
    }

    else if (iMov == 2)
    {
      iCoord -= 1;
    }

    else if (iMov == 3)
    {
      iCoord += 100;
    }

    else if (iMov == 4)
    {
      iCoord += 1;
    }
  }

  else if (iDirecc == 4)
  {
    if (iMov == 1)
    {
      iCoord -= 1;
    }

    else if (iMov == 2)
    {
      iCoord += 100;
    }

    else if (iMov == 3)
    {
      iCoord += 1;
    }

    else if (iMov == 4)
    {
      iCoord -= 100;
    }
  }
}

//Da el numero de posibilidades
int GetPossibility()
{
  //todo el arreglo a falso.
  bDireccion[0] = false;
  bDireccion[1] = false;
  bDireccion[2] = false;
  bDireccion[3] = false;

  //variable a regresar
  int iReturn = 0;

  //comprueba la Der
  int DistDerA = sonar3.ping_cm();
  int DistDerB = sonar4.ping_cm();
  if (DistDerA > 30 || DistDerB > 30)
  {
    bDireccion[0] = true;
    iReturn++;
  }

  //comprueba Enf
  int iDistEA = sonar1.ping_cm();
  int iDistEB = sonar2.ping_cm();
  if (iDistEA > 30 || iDistEB > 30)
  {
    bDireccion[1] = true;
    iReturn++;
  }

  //comprueba la Izq
  int iDistIzqA = sonar7.ping_cm();
  int iDistIzqB = sonar8.ping_cm();
  if (iDistIzqB > 30 || iDistIzqB > 30)
  {
    bDireccion[2] = true;
    iReturn++;
  }

  return iReturn;
}

//da los datos de la posicion actual
void GetData()
{
  iCounter++;
  //llena iPass
  iPass[iCounter] = iCounter;
  //llena iPos
  iPos[iCounter] = iCoord;
  //llena iPossibility
  iPossibility[iCounter] = GetPossibility();
  //llena iRun
  for (int iI = 0; iI < iPossibility[iCounter]; iI++)
  {
    if (bDireccion[0] == true)
    {
      iRun[iCounter][iI] = iCoord + 100;
      bDireccion[0] = false;
    }

    else if (bDireccion[1] == true)
    {
      iRun[iCounter][iI] = iCoord + 1;
      bDireccion[1] = false;
    }

    else if (bDireccion[2] == true)
    {
      iRun[iCounter][iI] = iCoord - 100;
      bDireccion[2] = false;
    }

    else if (bDireccion[3] == true)
    {
      iRun[iCounter][iI] = iCoord - 1;
      bDireccion[3] = false;
    }
  }
}

//comprueva si ya se ha estado en la posicion mandada como paramtetro
bool BeenHere(int Pos)
{
  bool bReturn = false;
  for (int iI = iCounter; iI >= 0; iI--)
  {
    if (Pos == iPos[iI])
    {
      bReturn = true;
    }
  }
  return bReturn;
}

//se mueve al irun preferencial o regresa un true si es necesario buscar alguno que no este adyecente
bool Moverse()
{
  bool Next = false;
  bool Go = false;
  int GoHere = 0;
  for (int iI = 0; iI < 4; iI++)
  {
    Go = BeenHere(iRun[iCounter][iI]);
    if (Go == false)
    {
      GoHere = iRun[iCounter][iI];
      break;
    }
  }
  if (GoHere == 0)
  {
    Next = true;
  }
  else
  {
    if (iPos[iCounter] + 100 == GoHere)
    {
      GiroDer90();
      delay(500);
      Adelante30();
      delay(500);
      iMovimiento = 1;
      iDireccion -= 1;
      if (iDireccion == 0)
      {
        iDireccion = 4;
      }
      setCoord(iDireccion, iMovimiento);
    }

    else if (iPos[iCounter] + 1 == GoHere)
    {
      Adelante30();
      iMovimiento = 2;
      setCoord(iDireccion, iMovimiento);
    }

    else if (iPos[iCounter] - 100 == GoHere)
    {
      GiroIzq90();
      delay(500);
      Adelante30();
      delay(500);
      iMovimiento = 3;
      iDireccion += 1;
      if (iDireccion == 5)
      {
        iDireccion = 0;
      }
      setCoord(iDireccion, iMovimiento);
    }

    else if (iPos[iCounter] - 1 == GoHere)
    {
      GiroDer90();
      delay(500);
      GiroDer90();
      delay(500);
      Adelante30();
      delay(500);
      iMovimiento = 4;
      iDireccion += 2;
      if (iDireccion == 5)
      {
        iDireccion = 1;
      }
      else if (iDireccion == 6)
      {
        iDireccion = 2;
      }
      setCoord(iDireccion, iMovimiento);
    }
  }
  return Next;
}

//busca y regresa el paso al cual quieres ir para moverte a una localizacion desconocida
int SearchWhereToGo(int &iReturn)
{
  int iPassToGo;
  for (int iI = iCounter; iI >= 0; iI--)
  {
    for (int iJ = 0; iJ < iPossibility[iI]; iJ++)
    {
      int iHelper = 0;
      for (int iK = iI; iK >= 0; iK--)
      {
        if (iRun[iI][iJ] == iPos[iK])
        {
          iHelper++;
        }
      }
      if (iHelper = 0)
      {
        iReturn = iRun[iI][iJ];
        iPassToGo = iI;
        break;
      }
    }
    if (iReturn != 999999)
    {
      break;
    }
  }
  return iPassToGo;
}

//consigue el iPass de el iRun que le des como parametro
int GetPass(int iHere)
{
  int iReturn = 0;
  int iCopyCounter = iCounter;
  while (iHere != iPos[iCopyCounter])
  {
    iCopyCounter--;
    if (iHere == iPos[iCopyCounter])
    {
      iReturn = iPass[iCopyCounter];
    }
  }
  return iReturn;
}

int GetCoord(int iThesePass)
{
  return iPos[iThesePass];
}

void MoverseShido(int iActual, int iDestination)
{
  if (iActual + 1 == iDestination)
  {
    Adelante30();
    iMovimiento = 2;
    setCoord(iDireccion, iMovimiento);
  }

  else if  (iActual - 1 == iDestination)
  {
    GiroDer90();
    delay(500);
    GiroDer90();
    delay(500);
    Adelante30();
    delay(500);
    iMovimiento = 4;
    iDireccion += 2;
    if (iDireccion == 5)
    {
      iDireccion = 1;
    }
    else if (iDireccion == 6)
    {
      iDireccion = 2;
    }
    setCoord(iDireccion, iMovimiento);
  }

  else if (iActual + 100 == iDestination)
  {
    GiroDer90();
    delay(500);
    Adelante30();
    delay(500);
    iMovimiento = 1;
    iDireccion -= 1;
    if (iDireccion == 0)
    {
      iDireccion = 4;
    }
    setCoord(iDireccion, iMovimiento);
  }

  else if (iActual - 100 == iDestination)
  {
    GiroIzq90();
    delay(500);
    Adelante30();
    delay(500);
    iMovimiento = 3;
    iDireccion += 1;
    if (iDireccion == 5)
    {
      iDireccion = 0;
    }
    setCoord(iDireccion, iMovimiento);
  }
}

//Ir a la posicion que necesitas ir
int Go(int &iParameter)
{
  int iCopyCounter = iCounter;
  int iYouAreHere = iPass[iCopyCounter];
  int iGetHere = SearchWhereToGo(iParameter);
  int iMinor[4];
  int iProx;
  int iHelper;
  int iTemporal = 9999;

  do
  {
    for (int iI = 0; iI < iPossibility[iCopyCounter]; iI++)
    {
      iMinor[iI] = GetPass(iRun[iCounter][iI]);
      if (iMinor[iI] < iTemporal)
      {
        iTemporal = iMinor[iI];
        iHelper = iI;
      }
    }
    //contiene el ipass menor al cual se debe de mover
    iProx = iTemporal;//iMinor[iI];
    int i1 = GetCoord(iYouAreHere);
    int i2 = GetCoord(iProx);
    MoverseShido(i1, i2);
    iYouAreHere = iProx;
    iCopyCounter = iProx;
  } while (iYouAreHere != iGetHere);
  return iYouAreHere;
}

void Laberinto()
{

  GetData();
  bool Next = Moverse();
  if (Next == true)
  {
    int iReturn = 999999;
    int iDestination = Go(iReturn);
    MoverseShido(iPos[iDestination], iReturn);
  }
}

void loop()
{
  Laberinto();
}
