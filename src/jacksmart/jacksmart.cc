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
#include <string>
#include <sysexits.h>

#include <base/bind.h>
#include <base/command_line.h>
#include <base/macros.h>
#include <base/memory/weak_ptr.h>
#include <base/message_loop/message_loop.h>
#include <brillo/daemons/dbus_daemon.h>
#include <brillo/syslog_logging.h>
#include <libweaved/device.h>

// Used for LED / Relay light control over GPIO.
#include "include/peripherals/gpio/gpio.h"

// Used for the I2C face LEDs
#include "include/peripherals/i2c/facehelper.h"

// Used for sound playback
#include "include/peripherals/sound/sound.h"

class Daemon final : public brillo::DBusDaemon {
 public:
  Daemon() = default;
  void Update();

 protected:
  int OnInit() override;

 private:
  // Starts with the light on if true.
  bool bootLight_ = false;

  // Starts with the light on if true.
  bool bootSound_ = true;

  // Use I2C expander for additional LEDs if true.
  bool useExpLeds_ = false;

  // Starts the message loop.
  bool useMessageLoop_ = false;

  // Callbacks for weave commands sent to Brillo.
  void OnPlaySound(const std::weak_ptr<weaved::Command>& command);
  void OnLightSet(const std::weak_ptr<weaved::Command>& command);

  // Helper methods to propagate device state changes to Buffet and hence to
  // the cloud server or local clients.
  void UpdateDeviceState();

  std::unique_ptr<weaved::Device> device_;
  base::WeakPtrFactory<Daemon> weak_ptr_factory_{this};

  // Initialize sound playing status state to idle.
  std::string status_{"idle"};
  std::vector<bool> lights { true, true, true, true };

  // Poll duration, in milliseconds, e.g. 1000 = call update every second.
  const double poll_duration_ms = 3000;

  DISALLOW_COPY_AND_ASSIGN(Daemon);
};


// Daemon Initialization
int Daemon::OnInit() {
  int return_code = brillo::DBusDaemon::OnInit();
  if (return_code != EX_OK)
    return return_code;

  // Add command callbacks
  device_ = weaved::Device::CreateInstance(
      bus_, base::Bind(&Daemon::UpdateDeviceState, base::Unretained(this)));
  device_->AddCommandHandler(
      "_playsound._set",
      base::Bind(&Daemon::OnPlaySound, base::Unretained(this)));
  device_->AddCommandHandler(
      "_lights._set",
      base::Bind(&Daemon::OnLightSet, base::Unretained(this)));

  // Configures GPIO and sound playback.
  SetupGPIO();
  Sound sound;
  sound.SetupMixer();

  if (bootLight_) {
    LOG(INFO) << "Turning on the lights";
    WriteGPIO(GPIO::PIN_A, true);
  }

  if (bootSound_) {
    LOG(INFO) << "Playing boot sound...";
    sound.PlaySound("/data/boot.mp3");
  }

  if (useExpLeds_) {
    BlueMouth();
    usleep(10000);
    GreenMouth();
  }

  if (useMessageLoop_) {
    LOG(INFO) << "Starting message loop.";
    Update();
  }

  LOG(INFO) << "Jack Smart booted and waiting for commands...";
  return EX_OK;
}


/**
 * Command handler for when a sound command is received.
 */
void Daemon::OnPlaySound(const std::weak_ptr<weaved::Command>& cmd) {
  LOG(INFO) << "Play Sound";
  status_ = "playing";
  UpdateDeviceState();

  auto command = cmd.lock();
  if (!command)
    return;

  bool on = command->GetParameter<bool>("_on");
  if (on) {
    Sound sound;
    sound.SetupMixer();
    sound.PlaySound("/data/out.wav");
  } else {
    // TODO(class): Stop playback
  }

  // TODO(class): update service state
  status_ = "idle";
  CHECK(command->Complete({}, nullptr));
  UpdateDeviceState();
}


/**
 * Command handler for when a command to change a light state is received.
 */
void Daemon::OnLightSet(const std::weak_ptr<weaved::Command>& cmd) {
  LOG(INFO) << "Play Sound";
  auto command = cmd.lock();
  if (!command)
    return;

  LOG(INFO) << "Command: " << command->GetName();

  bool on = command->GetParameter<bool>("_on");
  int light = command->GetParameter<int>("_light");

  LOG(INFO) << "Setting Light: " << light << " to" << (on ? " on" : " off");

  // Map the light value to the GPIO pins connected to the relay.
  if (light == 1) {
    lights[0] = on;
    WriteGPIO(GPIO::PIN_A, on);
  } else if (light == 2) {
    lights[1] = on;
    WriteGPIO(GPIO::PIN_B, on);
  } else if (light == 3) {
    lights[2] = on;
    WriteGPIO(GPIO::PIN_C, on);
  } else if (light == 4) {
    lights[3] = on;
    WriteGPIO(GPIO::PIN_D, on);
  }

  // Updates command state on success
  CHECK(command->Complete({}, nullptr));

  // TODO(class): Update device state
}

/**
 * Updates the device state to reflect playback state and so on.
 */
void Daemon::UpdateDeviceState() {
  brillo::VariantDictionary state_change{
    {"_jacksmart._playingState", status_},
    {"_jacksmart._lights", lights},
  };
  CHECK(device_->SetStateProperties(state_change, nullptr));
}

/**
 * Very basic loop for any polling operations.
 */
void Daemon::Update() {
  // TODO(class): Any periodic tasks (e.g. check proximity sensors, animate
  // LEDs) can be done in this loop.
  LOG(INFO) << "INFO: Update loop called.";

  base::MessageLoop::current()->PostDelayedTask(
      FROM_HERE,
      base::Bind(&Daemon::Update, weak_ptr_factory_.GetWeakPtr()),
      base::TimeDelta::FromSecondsD(poll_duration_ms / 1000));
}


// Main loop for daemon.
int main(int argc, char* argv[]) {
  base::CommandLine::Init(argc, argv);

  brillo::InitLog(brillo::kLogToSyslog | brillo::kLogHeader);
  Daemon daemon;

  // Stop execution here and run the daemon.
  return daemon.Run();
}
