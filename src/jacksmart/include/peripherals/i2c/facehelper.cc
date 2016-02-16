/*
 * Copyright 2016 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "facehelper.h"
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <base/logging.h>

#define mcpaddress   0x20
#define gpioregister 0x12
#define i2cdevice    "/dev/i2c-6"

int OpenMpc() {
  // Opening the device is just a R/W file open.
  int fd = open("/dev/i2c-6", O_RDWR);
  if (-1 == fd) {
    LOG(ERROR) << "Could not open i2c device.\n";
    return -1;
  }
  LOG(INFO) << "i2c device opened. ";
  // The I2C_SLAVE ioctl is sent to the device at the given address.
  int rc = ioctl(fd, I2C_SLAVE, 0x20);
  if (rc < 0) {
    LOG(ERROR) << "i2c device failed to become a slave.";
    return rc;
  }
  LOG(INFO) << "i2c device is a slave.";
  return fd;
}


int SetRegister(const uint8_t register_address,
                const uint8_t value) {
  // Setting a register involves sending a single message with a 2-byte payload.
  // The first byte is the register address, and the second is the value.
  int fd = OpenMpc();
  if (fd < 0) {
    return fd;
  }
  uint8_t outbuf[] = {register_address, value};
  struct i2c_msg messages[1];
  messages[0].addr  = mcpaddress;
  messages[0].flags = 0;
  messages[0].len   = sizeof(outbuf);
  messages[0].buf   = outbuf;
  outbuf[0] = register_address;
  outbuf[1] = value;
  struct i2c_rdwr_ioctl_data data;
  data.msgs = messages;
  data.nmsgs = 1;
  int rc = ioctl(fd, I2C_RDWR, &data);
  if (rc < 0) {
    LOG(ERROR) << "Unable to send data.";
    return rc;
  }
  return close(fd);
}


int BlueMouth() {
  return SetRegister(0x12, 0b11111111);
  return SetRegister(0x14, 0b11111111);
}


int GreenMouth() {
  return SetRegister(gpioregister, 0b00000000);
  return SetRegister(0x14, 0b00000000);
}


int RedMouth() {
  return SetRegister(gpioregister, 0b11111111);
}


int RedEyes() {
  return SetRegister(gpioregister, 0b00000000);
}


int GreenEyes() {
  return SetRegister(gpioregister, 0b00000000);
}


int BlueEyes() {
  return SetRegister(gpioregister, 0b00000000);
}
