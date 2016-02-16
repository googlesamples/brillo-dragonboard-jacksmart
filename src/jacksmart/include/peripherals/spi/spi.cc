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
#include "spi.h"
#include <fcntl.h>
#include <getopt.h>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <sys/ioctl.h>

using namespace std;


SPI::SPI() {
}


bool SPI::Init() {
  spifd_ = open(device_, O_RDWR);

  if (spifd_ < 0) {
    // LOG(ERROR) << "Can't open device";
    return false;
  }

  int ret = ioctl(spifd_, SPI_IOC_WR_MODE, &mode_);
  if (ret == -1) {
    // LOG(ERROR) << "can't set spi mode";
    return false;
  }

  ret = ioctl(spifd_, SPI_IOC_WR_BITS_PER_WORD, &bits_);
  if (ret == -1) {
    // LOG(ERROR) << "can't set bits per word";
    return false;
  }

  ret = ioctl(spifd_, SPI_IOC_WR_MAX_SPEED_HZ, &speed_);
  if (ret == -1) {
    // LOG(ERROR) << "can't set max speed hz";
    return false;
  }

  return true;
}


void SPI::Uninit() {
  close(spifd_);
}


int SPI::Write(const uint8_t tx[], int size) {
  uint8_t rx[size];
  if (size > 0) {
    rx[0] = 0;
  }

  // Setup xfer structure / data.
  struct spi_ioc_transfer tr;
  tr.tx_buf = reinterpret_cast<uint64_t>(tx);
  tr.rx_buf = reinterpret_cast<uint64_t>(rx);
  tr.len = size;
  tr.delay_usecs = delay_;
  tr.speed_hz = speed_;
  tr.bits_per_word = bits_;

  int ret = ioctl(spifd_, SPI_IOC_MESSAGE(1), &tr);

  if (ret < 1) {
    // TODO(class): LOG(ERROR) << "Can't send spi message";
  }
  for (ret = 0; ret < size; ret++) {
    // TODO(class): Return data.
    /*
    printf("%d ", rx[ret]);
    if ((ret+1) %4 == 0)
      printf("\n");
    */
  }

  return ret;
}
