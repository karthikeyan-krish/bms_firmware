#define PW_LOG_MODULE_NAME "bms-fsm"
#define PW_LOG_LEVEL PW_LOG_LEVEL_INFO

#include "bms_fsm.hpp"

#include "pw_log/log.h"

namespace bms::domain {

namespace {

void EnterSecure(BmsFsmContext& context) {
  context.SetState(StateSecure::Instance());
}

}  // namespace

void State::HandleChargerConnected(BmsFsmContext& context) {
  context.SetChargerConnected(true);
  PW_LOG_DEBUG("Unhandled ChargerConnected event");
}

void State::HandleChargerDisconnected(BmsFsmContext& context) {
  context.SetChargerConnected(false);
  PW_LOG_DEBUG("Unhandled ChargerDisconnected event");
}

void State::HandleVoltageTooHigh(BmsFsmContext& context) {
  context.SetVoltageNominal(false);
  PW_LOG_DEBUG("Unhandled VoltageTooHigh event");
}

void State::HandleVoltageTooLow(BmsFsmContext& context) {
  context.SetVoltageNominal(false);
  PW_LOG_DEBUG("Unhandled VoltageTooLow event");
}

void State::HandleVoltageNominal(BmsFsmContext& context) {
  context.SetVoltageNominal(true);
  PW_LOG_DEBUG("Unhandled VoltageNominal event");
}

void State::HandleTemperatureTooHigh(BmsFsmContext& context) {
  context.SetTemperatureNominal(false);
  PW_LOG_DEBUG("Unhandled TemperatureTooHigh event");
}

void State::HandleTemperatureTooLow(BmsFsmContext& context) {
  context.SetTemperatureNominal(false);
  PW_LOG_DEBUG("Unhandled TemperatureTooLow event");
}

void State::HandleTemperatureNominal(BmsFsmContext& context) {
  context.SetTemperatureNominal(true);
  PW_LOG_DEBUG("Unhandled TemperatureNominal event");
}

void State::HandleCurrentTooHigh(BmsFsmContext& context) {
  context.SetCurrentNominal(false);
  PW_LOG_DEBUG("Unhandled CurrentTooHigh event");
}

void State::HandleCurrentNominal(BmsFsmContext& context) {
  context.SetCurrentNominal(true);
  PW_LOG_DEBUG("Unhandled CurrentNominal event");
}

void State::HandleSensorFault(BmsFsmContext& context) {
  context.SetSensorNominal(false);
  PW_LOG_DEBUG("Unhandled SensorFault event");
}

void State::HandleSensorNominal(BmsFsmContext& context) {
  context.SetSensorNominal(true);
  PW_LOG_DEBUG("Unhandled SensorNominal event");
}

void State::Entry(BmsFsmContext& context) { static_cast<void>(context); }

void State::Exit(BmsFsmContext& context) { static_cast<void>(context); }

StateIdleDischarge& StateIdleDischarge::Instance() {
  static StateIdleDischarge instance;
  return instance;
}

void StateIdleDischarge::HandleChargerConnected(BmsFsmContext& context) {
  context.SetChargerConnected(true);
  context.SetState(StateCharge::Instance());
}

void StateIdleDischarge::HandleVoltageTooHigh(BmsFsmContext& context) {
  context.SetVoltageNominal(false);
  EnterSecure(context);
}

void StateIdleDischarge::HandleVoltageTooLow(BmsFsmContext& context) {
  context.SetVoltageNominal(false);
  EnterSecure(context);
}

void StateIdleDischarge::HandleTemperatureTooHigh(BmsFsmContext& context) {
  context.SetTemperatureNominal(false);
  EnterSecure(context);
}

void StateIdleDischarge::HandleTemperatureTooLow(BmsFsmContext& context) {
  context.SetTemperatureNominal(false);
  EnterSecure(context);
}

void StateIdleDischarge::HandleCurrentTooHigh(BmsFsmContext& context) {
  context.SetCurrentNominal(false);
  EnterSecure(context);
}

void StateIdleDischarge::HandleSensorFault(BmsFsmContext& context) {
  context.SetSensorNominal(false);
  EnterSecure(context);
}

void StateIdleDischarge::Entry(BmsFsmContext& context) {
  static_cast<void>(context);
  PW_LOG_INFO("Entering IdleDischarge state");
  PW_LOG_INFO("Relay action: close discharge relay");
  PW_LOG_INFO("Relay action: open charge relay");
}

void StateIdleDischarge::Exit(BmsFsmContext& context) {
  static_cast<void>(context);
  PW_LOG_INFO("Exiting IdleDischarge state");
}

StateCharge& StateCharge::Instance() {
  static StateCharge instance;
  return instance;
}

void StateCharge::HandleChargerDisconnected(BmsFsmContext& context) {
  context.SetChargerConnected(false);
  context.SetState(StateIdleDischarge::Instance());
}

void StateCharge::HandleVoltageTooHigh(BmsFsmContext& context) {
  context.SetVoltageNominal(false);
  EnterSecure(context);
}

void StateCharge::HandleTemperatureTooHigh(BmsFsmContext& context) {
  context.SetTemperatureNominal(false);
  EnterSecure(context);
}

void StateCharge::HandleTemperatureTooLow(BmsFsmContext& context) {
  context.SetTemperatureNominal(false);
  EnterSecure(context);
}

void StateCharge::HandleCurrentTooHigh(BmsFsmContext& context) {
  context.SetCurrentNominal(false);
  EnterSecure(context);
}

void StateCharge::HandleSensorFault(BmsFsmContext& context) {
  context.SetSensorNominal(false);
  EnterSecure(context);
}

void StateCharge::Entry(BmsFsmContext& context) {
  static_cast<void>(context);
  PW_LOG_INFO("Entering Charge state");
  PW_LOG_INFO("Relay action: close charge relay");
  PW_LOG_INFO("Relay action: open discharge relay");
}

void StateCharge::Exit(BmsFsmContext& context) {
  static_cast<void>(context);
  PW_LOG_INFO("Exiting Charge state");
}

StateSecure& StateSecure::Instance() {
  static StateSecure instance;
  return instance;
}

void StateSecure::HandleVoltageNominal(BmsFsmContext& context) {
  context.SetVoltageNominal(true);
  if (context.AllInputsNominal()) {
    context.ReturnToOperatingState();
  }
}

void StateSecure::HandleTemperatureNominal(BmsFsmContext& context) {
  context.SetTemperatureNominal(true);
  if (context.AllInputsNominal()) {
    context.ReturnToOperatingState();
  }
}

void StateSecure::HandleCurrentNominal(BmsFsmContext& context) {
  context.SetCurrentNominal(true);
  if (context.AllInputsNominal()) {
    context.ReturnToOperatingState();
  }
}

void StateSecure::HandleSensorNominal(BmsFsmContext& context) {
  context.SetSensorNominal(true);
  if (context.AllInputsNominal()) {
    context.ReturnToOperatingState();
  }
}

void StateSecure::Entry(BmsFsmContext& context) {
  static_cast<void>(context);
  PW_LOG_INFO("Entering Secure state");
  PW_LOG_INFO("Relay action: open charge relay");
  PW_LOG_INFO("Relay action: open discharge relay");
}

void StateSecure::Exit(BmsFsmContext& context) {
  static_cast<void>(context);
  PW_LOG_INFO("Exiting Secure state");
}

BmsFsmContext::BmsFsmContext()
    : current_state_(&StateIdleDischarge::Instance()) {}

void BmsFsmContext::Start() {
  if (started_) {
    return;
  }

  started_ = true;
  current_state_->Entry(*this);
}

void BmsFsmContext::UpdateFaultStatus(const FaultStatus& fault_status) {
  if (!started_) {
    Start();
  }

  if (fault_status.sensor_fault) {
    HandleSensorFault();
  } else {
    HandleSensorNominal();
  }

  if (fault_status.overvoltage) {
    HandleVoltageTooHigh();
  } else if (fault_status.undervoltage) {
    HandleVoltageTooLow();
  } else {
    HandleVoltageNominal();
  }

  if (fault_status.overtemperature) {
    HandleTemperatureTooHigh();
  } else if (fault_status.undertemperature) {
    HandleTemperatureTooLow();
  } else {
    HandleTemperatureNominal();
  }

  if (fault_status.overcurrent) {
    HandleCurrentTooHigh();
  } else {
    HandleCurrentNominal();
  }
}

void BmsFsmContext::HandleChargerConnected() {
  current_state_->HandleChargerConnected(*this);
}

void BmsFsmContext::HandleChargerDisconnected() {
  current_state_->HandleChargerDisconnected(*this);
}

void BmsFsmContext::HandleVoltageTooHigh() {
  current_state_->HandleVoltageTooHigh(*this);
}

void BmsFsmContext::HandleVoltageTooLow() {
  current_state_->HandleVoltageTooLow(*this);
}

void BmsFsmContext::HandleVoltageNominal() {
  current_state_->HandleVoltageNominal(*this);
}

void BmsFsmContext::HandleTemperatureTooHigh() {
  current_state_->HandleTemperatureTooHigh(*this);
}

void BmsFsmContext::HandleTemperatureTooLow() {
  current_state_->HandleTemperatureTooLow(*this);
}

void BmsFsmContext::HandleTemperatureNominal() {
  current_state_->HandleTemperatureNominal(*this);
}

void BmsFsmContext::HandleCurrentTooHigh() {
  current_state_->HandleCurrentTooHigh(*this);
}

void BmsFsmContext::HandleCurrentNominal() {
  current_state_->HandleCurrentNominal(*this);
}

void BmsFsmContext::HandleSensorFault() {
  current_state_->HandleSensorFault(*this);
}

void BmsFsmContext::HandleSensorNominal() {
  current_state_->HandleSensorNominal(*this);
}

void BmsFsmContext::SetState(State& new_state) {
  if (current_state_ == &new_state) {
    return;
  }

  State* old_state = current_state_;
  old_state->Exit(*this);

  previous_state_ = old_state;
  current_state_ = &new_state;
  PW_LOG_INFO("BMS state transition: %s -> %s",
              previous_state_->Name(),
              current_state_->Name());
  current_state_->Entry(*this);
}

bool BmsFsmContext::AllInputsNominal() const {
  return voltage_nominal_ && temperature_nominal_ && current_nominal_ &&
         sensor_nominal_;
}

void BmsFsmContext::ReturnToOperatingState() {
  if (charger_connected_) {
    SetState(StateCharge::Instance());
  } else {
    SetState(StateIdleDischarge::Instance());
  }
}

}  // namespace bms::domain
