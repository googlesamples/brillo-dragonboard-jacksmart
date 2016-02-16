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
#ifndef _INCLUDE_FACEHELPER_H
#define _INCLUDE_FACEHELPER_H
#include <stdint.h>

int OpenMpc();
int SetRegister(uint8_t register_address, uint8_t value);
int BlueMouth();
int GreenMouth();
int RedMouth();
int RedEyes();
int GreenEyes();
int BlueEyes();
#endif  // _INCLUDE_FACEHELPER_H
