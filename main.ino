//#include <IRremote.h> // ir trasnmit
#include <Wire.h>

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRrecv.h>

#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3, 16, 2); 

Adafruit_MPU6050 mpu;
const int RECV_PIN = 4;
const int TRANS_PIN = 16;
IRsend irsend(TRANS_PIN);
IRrecv irrecv(RECV_PIN);

float const FREE_FALL_THRESHOLD_LOW = 3.0; // Example low g threshold (adjust as needed)
float const FREE_FALL_THRESHOLD_HIGH = 20.0; // Example high g threshold for impact

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin(21, 22); 
  byte count = 0;
    for (byte i = 1; i < 127; i++ ) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      Serial.println(i, HEX);
      count++;
      delay(1);
    }
  }

  if (!mpu.begin()) {
    Serial.println("MPU6050 not detected!");
  }
  irsend.begin();
  irrecv.enableIRIn();

    // Initialize the LCD
  lcd.init();
  // Turn on the backlight
  lcd.backlight();
  lcd.print("hi!!!");
  Serial.print("Sent hello");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void loop() {
  
  float total_acceleration = get_accel();

  if (total_acceleration < FREE_FALL_THRESHOLD_LOW) {
    Serial.println("possible free falL!");
    // A potential free fall has started, wait for impact
    delay(50); // Small delay to allow free fall to continue briefly
    total_acceleration = get_accel();
    if (total_acceleration > FREE_FALL_THRESHOLD_HIGH) {
      Serial.println("FALLL!!!");
      irsend.sendNEC(0x20DF10EF); 
      // free fall detected
    }
  }
  
  decode_results results;
  if (irrecv.decode(&results)) {
    Serial.println("CRASH!");
    lcd.print("CRASH!!!");
    irrecv.resume(); 
  }
}
float get_accel()
{
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float accel = sqrt(
    a.acceleration.x * a.acceleration.x +
    a.acceleration.y * a.acceleration.y +
    a.acceleration.z * a.acceleration.z
  );
  Serial.println(accel);

  return accel;
}
