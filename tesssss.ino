/*
  This example creates a BLE peripheral with a service that contains a
  couple of characteristics to test BLE connection.
  The yellow LED shows the BLE module is initialized.
  The green LED shows RSSI of zero. The more it blinks the worse the connection.

  The circuit:
  - Arduino Nano 33 BLE Sense board.

  You can use a generic BLE central app, like LightBlue (iOS and Android) or
  nRF Connect (Android), to interact with the services and characteristics
  created in this sketch.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>


//----------------------------------------------------------------------------------------------------------------------
// BLE UUIDs
//----------------------------------------------------------------------------------------------------------------------

#define BLE_UUID_TEST_SERVICE               "9A48ECBA-2E92-082F-C079-9E75AAE428B1"
#define BLE_UUID_ACCELERATION               "2713"
#define BLE_UUID_COUNTER                    "1A3AC130-31EE-758A-BC50-54A61958EF81"
#define BLE_UUID_RESET_COUNTER              "FE4E19FF-B132-0099-5E94-3FFB2CF07940"

//----------------------------------------------------------------------------------------------------------------------
// BLE
//----------------------------------------------------------------------------------------------------------------------

BLEService testService( BLE_UUID_TEST_SERVICE );
BLEFloatCharacteristic accelerationCharacteristic( BLE_UUID_ACCELERATION, BLERead | BLENotify );
BLEUnsignedLongCharacteristic counterCharacteristic( BLE_UUID_COUNTER, BLERead | BLENotify );
BLEBoolCharacteristic resetCounterCharacteristic( BLE_UUID_RESET_COUNTER, BLEWriteWithoutResponse );


const int BLE_LED_PIN = LED_BUILTIN;
const int RSSI_LED_PIN = LED_PWR;


void setup()
{
  Serial.begin( 9600 );
  //  while ( !Serial );

  pinMode( BLE_LED_PIN, OUTPUT );
  pinMode( RSSI_LED_PIN, OUTPUT );

  if ( !IMU.begin() )
  {
    Serial.println( "Failed to initialize IMU!" );
    while ( 1 );
  }
  Serial.print( "Accelerometer sample rate = " );
  Serial.print( IMU.accelerationSampleRate() );
  Serial.println( " Hz" );

  if( setupBleMode() )
  {
    digitalWrite( BLE_LED_PIN, HIGH );
  }
} // setup


void loop()
{
  static unsigned long counter = 0;
  static long previousMillis = 0;

  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  if ( central )
  {
    Serial.print( "Connected to central: " );
    Serial.println( central.address() );

    while ( central.connected() )
    {
      if( resetCounterCharacteristic.written() )
      {
        counter = 0;
      }

      long interval = 20;
      unsigned long currentMillis = millis();
      if( currentMillis - previousMillis > interval )
      {
        previousMillis = currentMillis;

        Serial.print( "Central RSSI: " );
        Serial.println( central.rssi() );

        if( central.rssi() != 0 )
        {
          digitalWrite( RSSI_LED_PIN, LOW );
          float accelerationX, accelerationY, accelerationZ;
          if ( IMU.accelerationAvailable() )
          {
            IMU.readAcceleration( accelerationX, accelerationY, accelerationZ );
            accelerationCharacteristic.writeValue( accelerationX );
          }

          counter++;
          counterCharacteristic.writeValue( counter );
        }
        else
        {
          digitalWrite( RSSI_LED_PIN, HIGH );
        }
      } // intervall
    } // while connected

    Serial.print( F( "Disconnected from central: " ) );
    Serial.println( central.address() );
  } // if central
} // loop



bool setupBleMode()
{
  if ( !BLE.begin() )
  {
    return false;
  }

  // set advertised local name and service UUID:
  BLE.setDeviceName( "Arduino Nano 33 BLE" );
  BLE.setLocalName( "Arduino Nano 33 BLE" );
  BLE.setAdvertisedService( testService );

  // BLE add characteristics
  testService.addCharacteristic( accelerationCharacteristic );
  testService.addCharacteristic( counterCharacteristic );
  testService.addCharacteristic( resetCounterCharacteristic );

  // add service
  BLE.addService( testService );

  // set the initial value for the characeristic:
  accelerationCharacteristic.writeValue( 0.0 );
  counterCharacteristic.writeValue( 0 );

  // start advertising
  BLE.advertise();

  return true;
}
