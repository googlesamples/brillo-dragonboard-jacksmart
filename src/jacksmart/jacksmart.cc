/*
 * Copyright 2016 Google Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
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
#include <brillo/binder_watcher.h>
#include <brillo/daemons/daemon.h>
#include <brillo/syslog_logging.h>
#include <libweaved/service.h>

// Used for LED / Relay light control over GPIO.
#include "include/peripherals/gpio/gpio.h"

// Used for the I2C face LEDs
#include "include/peripherals/i2c/facehelper.h"

// Used for sound playback
#include "include/peripherals/sound/sound.h"

namespace {
  const char kWeaveComponent[] = "jacksmart";
  const char kWeaveTrait[] = "_jacksmart";
  const char kBaseComponent[] = "base";
  const char kBaseTrait[] = "base";
}  // anonymous namespace

class Daemon final : public brillo::Daemon {
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
  void OnPlaySound(std::unique_ptr<weaved::Command> cmd);
  void OnLightSet(std::unique_ptr<weaved::Command> cmd);

  // Configured method for when the service is connected
  void OnWeaveServiceConnected(const std::weak_ptr<weaved::Service>& service);
  void OnPairingInfoChanged(const weaved::Service::PairingInfo* info);

  // Helper methods to propagate device state changes to Buffet and hence to
  // the cloud server or local clients.
  void UpdateDeviceState();

  std::weak_ptr<weaved::Service> service_;

  // Initialize sound playing status state to idle.
  std::string status_{"idle"};
  std::vector<bool> gpios_{ false, false, false, false};

  // Poll duration, in milliseconds, e.g. 1000 = call update every second.
  const double poll_duration_ms = 3000;

  brillo::BinderWatcher binder_watcher_;
  std::unique_ptr<weaved::Service::Subscription> weave_service_subscription_;

  base::WeakPtrFactory<Daemon> weak_ptr_factory_{this};
  DISALLOW_COPY_AND_ASSIGN(Daemon);
};


// Daemon Initialization
int Daemon::OnInit() {
  int return_code = brillo::Daemon::OnInit();
  if (return_code != EX_OK)
    return return_code;

  if (!binder_watcher_.Init())
    return EX_OSERR;
  weave_service_subscription_ = weaved::Service::Connect(
      brillo::MessageLoop::current(),
      base::Bind(&Daemon::OnWeaveServiceConnected,
                 weak_ptr_factory_.GetWeakPtr()));

  // Configures GPIO and sound playback.
  SetupGPIO();
  Sound sound;
  sound.SetupMixer();

  if (bootLight_) {
    LOG(INFO) << "Turning on the lights";
    WriteGPIO(GPIO::PIN_A, true);
    gpios_[0] = true;
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

void Daemon::OnWeaveServiceConnected(
    const std::weak_ptr<weaved::Service>& service) {
  LOG(INFO) << "Jack Smart adding traits::OnWeaveServiceConnected";
  service_ = service;

  auto weave_service = service_.lock();
  if (!weave_service)
    return;

  // Add base components / traits
  weave_service->AddComponent(kWeaveComponent, {kWeaveTrait}, nullptr);

  // Add command callbacks
  weave_service->AddCommandHandler(
      kWeaveComponent, kWeaveTrait, "playsound",
      base::Bind(&Daemon::OnPlaySound, weak_ptr_factory_.GetWeakPtr()));
  weave_service->AddCommandHandler(
      kWeaveComponent, kWeaveTrait, "gpioToggle",
      base::Bind(&Daemon::OnLightSet, base::Unretained(this)));
  weave_service->SetPairingInfoListener(
      base::Bind(&Daemon::OnPairingInfoChanged,
                 weak_ptr_factory_.GetWeakPtr()));

  // Update the device state for any local changes.
  UpdateDeviceState();
}


/**
 * Used while pairing information changes.
 */
void Daemon::OnPairingInfoChanged(const weaved::Service::PairingInfo* info) {
  LOG(INFO) << "Pairing info changed: " << info;
  // TODO: Add any code for pairing UI here
}

/**
 * Command handler for when a sound command is received.
 */
void Daemon::OnPlaySound(std::unique_ptr<weaved::Command> cmd) {
  LOG(INFO) << "Play Sound";
  status_ = "playing";
  UpdateDeviceState();

  bool on = cmd->GetParameter<bool>("on");
  if (on) {
    Sound sound;
    sound.SetupMixer();
    sound.PlaySound("/data/out.wav");
  } else {
    // TODO(class): Stop playback
  }

  status_ = "idle";
  CHECK(cmd->Complete({}, nullptr));
  UpdateDeviceState();
}


/**
 * Command handler for when a command to change a light state is received.
 */
void Daemon::OnLightSet(std::unique_ptr<weaved::Command> cmd) {
  int light = cmd->GetParameter<int>("pin");
  bool on = cmd->GetParameter<bool>("on");

  LOG(INFO) << "Setting Light: " << light << " to" << (on ? " on" : " off");

  // Map the light value to the GPIO pins connected to the relay.
  if (light == 1) {
    gpios_[0] = on;
    WriteGPIO(GPIO::PIN_A, on);
  } else if (light == 2) {
    gpios_[1] = on;
    WriteGPIO(GPIO::PIN_B, on);
  } else if (light == 3) {
    gpios_[2] = on;
    WriteGPIO(GPIO::PIN_C, on);
  } else if (light == 4) {
    gpios_[3] = on;
    WriteGPIO(GPIO::PIN_D, on);
  }

  // Updates command state on success
  CHECK(cmd->Complete({}, nullptr));

  UpdateDeviceState();
}

/**
 * Updates the device state to reflect playback state and so on.
 */
void Daemon::UpdateDeviceState() {
  base::DictionaryValue state_change;

  auto weave_service = service_.lock();
  state_change.SetString("_jacksmart.playStatus", status_);
  state_change.Set("_jacksmart.gpioEnabled", brillo::ToValue(gpios_).release());
  weave_service->SetStateProperties(kWeaveComponent, state_change, nullptr);
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
