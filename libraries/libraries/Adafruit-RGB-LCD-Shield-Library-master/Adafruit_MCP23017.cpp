/*************************************************** 
  This is a library for the MCP23017 i2c port expander

  These displays use I2C to communicate, 2 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#include <avr/pgmspace.h>
#include "Adafruit_MCP23017.h"
#ifdef __AVR__
 #define WIRE Wire
#else // Arduino Due
 #define WIRE Wire1
#endif

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

// minihelper
static inline void wiresend(uint8_t x) {
#if ARDUINO >= 100
  WIRE.write((uint8_t)x);
#else
  WIRE.send(x);
#endif
}

static inline uint8_t wirerecv(void) {
#if ARDUINO >= 100
  return WIRE.read();
#else
  return WIRE.receive();
#endif
}

////////////////////////////////////////////////////////////////////////////////

void Adafruit_MCP23017::begin(uint8_t addr) {
  if (addr > 7) {
    addr = 7;
  }
  i2caddr = addr;

  WIRE.begin();

  
  // set defaults!
  WIRE.beginTransmission(MCP23017_ADDRESS | i2caddr);
  wiresend(MCP23017_IODIRA);
  wiresend(0xFF);  // all inputs on port A
  WIRE.endTransmission();

  WIRE.beginTransmission(MCP23017_ADDRESS | i2caddr);
  wiresend(MCP23017_IODIRB);
  wiresend(0xFF);  // all inputs on port B
  WIRE.endTransmission();
}


void Adafruit_MCP23017::begin(void) {
  begin(0);
}

void Adafruit_MCP23017::pinMode(uint8_t p, uint8_t d) {
  uint8_t iodir;
  uint8_t iodiraddr;

  // only 16 bits!
  if (p > 15)
    return;

  if (p < 8)
    iodiraddr = MCP23017_IODIRA;
  else {
    iodiraddr = MCP23017_IODIRB;
    p -= 8;
  }

  // read the current IODIR
  WIRE.beginTransmission(MCP23017_ADDRESS | i2caddr);
  wiresend(iodiraddr);	
  WIRE.endTransmission();
  
  WIRE.requestFrom(MCP23017_ADDRESS | i2caddr, 1);
  iodir = wirerecv();

  // set the pin and direction
  if (d == INPUT) {
    iodir |= 1 << p; 
  } else {
    iodir &= ~(1 << p);
  }

  // write the new IODIR
  WIRE.beginTransmission(MCP23017_ADDRESS | i2caddr);
  wiresend(iodiraddr);
  wiresend(iodir);	
  WIRE.endTransmission();
}

uint16_t Adafruit_MCP23017::readGPIOAB() {
  uint16_t ba = 0;
  uint8_t a;

  // read the current GPIO output latches
  WIRE.beginTransmission(MCP23017_ADDRESS | i2caddr);
  wiresend(MCP23017_GPIOA);	
  WIRE.endTransmission();
  
  WIRE.requestFrom(MCP23017_ADDRESS | i2caddr, 2);
  a = wirerecv();
  ba = wirerecv();
  ba <<= 8;
  ba |= a;

  return ba;
}

void Adafruit_MCP23017::writeGPIOAB(uint16_t ba) {
  WIRE.beginTransmission(MCP23017_ADDRESS | i2caddr);
  wiresend(MCP23017_GPIOA);	
  wiresend(ba & 0xFF);
  wiresend(ba >> 8);
  WIRE.endTransmission();
}

void Adafruit_MCP23017::digitalWrite(uint8_t p, uint8_t d) {
  uint8_t gpio;
  uint8_t gpioaddr, olataddr;

  // only 16 bits!
  if (p > 15)
    return;

  if (p < 8) {
    olataddr = MCP23017_OLATA;
    gpioaddr = MCP23017_GPIOA;
  } else {
    olataddr = MCP23017_OLATB;
    gpioaddr = MCP23017_GPIOB;
    p -= 8;
  }

  // read the current GPIO output latches
  WIRE.beginTransmission(MCP23017_ADDRESS | i2caddr);
  wiresend(olataddr);	
  WIRE.endTransmission();
  
  WIRE.requestFrom(MCP23017_ADDRESS | i2caddr, 1);
   gpio = wirerecv();

  // set the pin and direction
  if (d == HIGH) {
    gpio |= 1 << p; 
  } else {
    gpio &= ~(1 << p);
  }

  // write the new GPIO
  WIRE.beginTransmission(MCP23017_ADDRESS | i2caddr);
  wiresend(gpioaddr);
  wiresend(gpio);	
  WIRE.endTransmission();
}

void Adafruit_MCP23017::pullUp(uint8_t p, uint8_t d) {
  uint8_t gppu;
  uint8_t gppuaddr;

  // only 16 bits!
  if (p > 15)
    return;

  if (p < 8)
    gppuaddr = MCP23017_GPPUA;
  else {
    gppuaddr = MCP23017_GPPUB;
    p -= 8;
  }


  // read the current pullup resistor set
  WIRE.beginTransmission(MCP23017_ADDRESS | i2caddr);
  wiresend(gppuaddr);	
  WIRE.endTransmission();
  
  WIRE.requestFrom(MCP23017_ADDRESS | i2caddr, 1);
  gppu = wirerecv();

  // set the pin and direction
  if (d == HIGH) {
    gppu |= 1 << p; 
  } else {
    gppu &= ~(1 << p);
  }

  // write the new GPIO
  WIRE.beginTransmission(MCP23017_ADDRESS | i2caddr);
  wiresend(gppuaddr);
  wiresend(gppu);	
  WIRE.endTransmission();
}

uint8_t Adafruit_MCP23017::digitalRead(uint8_t p) {
  uint8_t gpioaddr;

  // only 16 bits!
  if (p > 15)
    return 0;

  if (p < 8)
    gpioaddr = MCP23017_GPIOA;
  else {
    gpioaddr = MCP23017_GPIOB;
    p -= 8;
  }

  // read the current GPIO
  WIRE.beginTransmission(MCP23017_ADDRESS | i2caddr);
  wiresend(gpioaddr);	
  WIRE.endTransmission();
  
  WIRE.requestFrom(MCP23017_ADDRESS | i2caddr, 1);
  return (wirerecv() >> p) & 0x1;
}

//Added to default library
//From the Adafruit_RGBLCDShield library we learn that the
//buttons are on bits 0..4, which are on GPIOA,
//so we use GPINTENA, INTCONA and IOCONA.
void Adafruit_MCP23017::enableButtonInterrupt() 
{
	uint8_t data;
	//Enable the interrupts on the button pins 0..4
	Wire.beginTransmission(MCP23017_ADDRESS | i2caddr);
	wiresend(MCP23017_GPINTENA);
	Wire.endTransmission();
	Wire.requestFrom(MCP23017_ADDRESS | i2caddr, 1);
	data = wirerecv();
	data |= 0x1F; //Bits 0..4 high, to enable IOC
	//Write the new value back
	Wire.beginTransmission(MCP23017_ADDRESS | i2caddr);
	wiresend(MCP23017_GPINTENA);
	wiresend(data);
	Wire.endTransmission();
	//We set INTCONA bits 0..4 to 0 = State change interrupt
	//(instead of compare to state)
	Wire.beginTransmission(MCP23017_ADDRESS | i2caddr);
	wiresend(MCP23017_INTCONA);
	Wire.endTransmission();
	Wire.requestFrom(MCP23017_ADDRESS | i2caddr, 1);
	data = wirerecv();
	data &= ~0x1F; //Force bits 0..4 to low
	//Write the new value back
	Wire.beginTransmission(MCP23017_ADDRESS | i2caddr);
	wiresend(MCP23017_INTCONA);
	wiresend(data);
	Wire.endTransmission();
	//We set the INTA pin to Active-Low
	//first read current register value
	Wire.beginTransmission(MCP23017_ADDRESS | i2caddr);
	wiresend(MCP23017_IOCONA);
	Wire.endTransmission();
	Wire.requestFrom(MCP23017_ADDRESS | i2caddr, 1);
	data = wirerecv();
	//Bit 1 = INTPOL = Low = Active-Low
	//(When disabled or no interrupt, signal is high)
	data &= ~0x02;
	//Bit 2 = ODR = Low = Open Drain disabled
	data &= ~0x04;
	//Bit 6 = MIRROR = Low = INTA/INTB seperate
	data &= ~0x40;
	//Write the new value back
	Wire.beginTransmission(MCP23017_ADDRESS | i2caddr);
	wiresend(MCP23017_IOCONA);
	wiresend(data);
	Wire.endTransmission();
}
//Added to default library
void Adafruit_MCP23017::disableButtonInterrupt() 
{
	uint8_t data;
	//Disable the interrupts on the button pins 0..4
	Wire.beginTransmission(MCP23017_ADDRESS | i2caddr);
	wiresend(MCP23017_GPINTENA);
	Wire.endTransmission();
	Wire.requestFrom(MCP23017_ADDRESS | i2caddr, 1);
	data = wirerecv();
	data &= ~0x1F; //Bits 0..4 low, to disable IOC
	//Write the new value back
	Wire.beginTransmission(MCP23017_ADDRESS | i2caddr);
	wiresend(MCP23017_GPINTENA);
	wiresend(data);
	Wire.endTransmission();
}