/* mma8452.h
 * Code for reading from the MMA8452 over I2C.
 */

#define WRITE_ADDR 0x38 // left-shifted address of MMA8452 (0x1C << 1)
#define READ_ADDR  0x39 // 8-bit address of MMA8452 with read bit set

#define CTRL_REG1    0x2A
#define XYZ_DATA_CFG 0x03
#define OUT_X_MSB    0x01
#define OUT_Y_MSB    0x03
#define OUT_Z_MSB    0x05

#define SCALE_2G 2
#define SCALE_4G 4
#define SCALE_8G 8

#define ODR_800 0
#define ODR_400 1
#define ODR_200 2
#define ODR_100 3
#define ODR_50  4
#define ODR_12  5
#define ODR_6   6
#define ODR_1   7

// Initialize the I2C interface
void i2c_init() {
  // Set up 400KHz I2C clock
  TWSR = 0x00;
  TWBR = 0x0C;
  TWCR |= (1 << TWEN); // Enable TWI
}

// Send I2C start signal
void i2c_start() {
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
  while ((TWCR & (1 << TWINT)) == 0);
}

// Send I2C stop signal
void i2c_stop() {
  TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

// Write data to I2C bus
void i2c_write(uint8_t data) {
  TWDR = data;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while ((TWCR & (1 << TWINT)) == 0);
}

// Read from the I2C bus and send an ACK signal
uint8_t i2c_read_ack() {
  TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
  while ((TWCR & (1 << TWINT)) == 0);
  return TWDR;
}

// Read from the I2C bus without an ACK signal
uint8_t i2c_read_no_ack() {
  TWCR = (1 << TWINT) | (1 << TWEN);
  while ((TWCR & (1 << TWINT)) == 0);
  return TWDR;
}

// Get status of the I2c
uint8_t i2c_get_status() {
  return TWSR & 0xF8; // return upper 5 bits of the status register
}

// Write to an MMA8452 register
void accel_write_register(uint8_t regaddr, uint8_t data) {
  i2c_start();
  i2c_write(WRITE_ADDR);
  i2c_write(regaddr);
  i2c_write(data);
  i2c_stop();
}

// Read from an MM8452 register
uint8_t accel_read_register(uint8_t regaddr) {
  i2c_start();
  i2c_write(WRITE_ADDR);
  i2c_write(regaddr);

  i2c_start();
  i2c_write(READ_ADDR);
  uint8_t data = i2c_read_no_ack();
  i2c_stop();

  return data;
}

// Initialize the MMA8452
void accel_init() {
  uint8_t data;
  i2c_init();

  PORTC |= (1 << PC5); // pull up SCL
  PORTC |= (1 << PC4); // pull up SDA

  // Put MMA8452 into standby mode so we can change registers
  data = accel_read_register(CTRL_REG1);
  accel_write_register(CTRL_REG1, data & ~(0x01)); // clear active bit to go standby

  // Set the accelerometer scale
  data = accel_read_register(XYZ_DATA_CFG);
  data &= 0xFC; // Mask out scale bits
  data |= (SCALE_2G >> 2); // set scale to +/- 2g (smallest range)
  accel_write_register(XYZ_DATA_CFG, data);

  // Set the output data rate
  data = accel_read_register(CTRL_REG1);
  data &= 0xCF; // mask out data rate bits
  data |= (ODR_50 << 3); // set data rate to 50Hz
  accel_write_register(CTRL_REG1, data);

  // Set fast read mode
  data = accel_read_register(CTRL_REG1);
  accel_write_register(CTRL_REG1, data | 0x02); // set f_read bit

  // Set active mode to start readings
  data = accel_read_register(CTRL_REG1);
  accel_write_register(CTRL_REG1, data | 0x01); // set active bit
}

// Read the x-axis
int8_t accel_read_x() {
  return (int8_t) accel_read_register(OUT_X_MSB);
}

// Read the y-axis
int8_t accel_read_y() {
  return (int8_t) accel_read_register(OUT_Y_MSB);
}

// Read the z-axis
int8_t accel_read_z() {
  return (int8_t) accel_read_register(OUT_Z_MSB);
}
