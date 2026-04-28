#pragma once

#include "fault_status.hpp"

namespace bms::domain {

enum class BmsState {
  kIdleDischarge,
  kCharge,
  kSecure,
};

class BmsFsmContext;

class State {
 public:
  virtual ~State() = default;

  virtual BmsState Id() const = 0;
  virtual const char* Name() const = 0;

  virtual void HandleChargerConnected(BmsFsmContext& context);
  virtual void HandleChargerDisconnected(BmsFsmContext& context);
  virtual void HandleVoltageTooHigh(BmsFsmContext& context);
  virtual void HandleVoltageTooLow(BmsFsmContext& context);
  virtual void HandleVoltageNominal(BmsFsmContext& context);
  virtual void HandleTemperatureTooHigh(BmsFsmContext& context);
  virtual void HandleTemperatureTooLow(BmsFsmContext& context);
  virtual void HandleTemperatureNominal(BmsFsmContext& context);
  virtual void HandleCurrentTooHigh(BmsFsmContext& context);
  virtual void HandleCurrentNominal(BmsFsmContext& context);
  virtual void HandleSensorFault(BmsFsmContext& context);
  virtual void HandleSensorNominal(BmsFsmContext& context);

  virtual void Entry(BmsFsmContext& context);
  virtual void Exit(BmsFsmContext& context);
};

class StateIdleDischarge final : public State {
 public:
  static StateIdleDischarge& Instance();

  BmsState Id() const override { return BmsState::kIdleDischarge; }
  const char* Name() const override { return "IdleDischarge"; }

  void HandleChargerConnected(BmsFsmContext& context) override;
  void HandleVoltageTooHigh(BmsFsmContext& context) override;
  void HandleVoltageTooLow(BmsFsmContext& context) override;
  void HandleTemperatureTooHigh(BmsFsmContext& context) override;
  void HandleTemperatureTooLow(BmsFsmContext& context) override;
  void HandleCurrentTooHigh(BmsFsmContext& context) override;
  void HandleSensorFault(BmsFsmContext& context) override;
  void Entry(BmsFsmContext& context) override;
  void Exit(BmsFsmContext& context) override;
};

class StateCharge final : public State {
 public:
  static StateCharge& Instance();

  BmsState Id() const override { return BmsState::kCharge; }
  const char* Name() const override { return "Charge"; }

  void HandleChargerDisconnected(BmsFsmContext& context) override;
  void HandleVoltageTooHigh(BmsFsmContext& context) override;
  void HandleTemperatureTooHigh(BmsFsmContext& context) override;
  void HandleTemperatureTooLow(BmsFsmContext& context) override;
  void HandleCurrentTooHigh(BmsFsmContext& context) override;
  void HandleSensorFault(BmsFsmContext& context) override;
  void Entry(BmsFsmContext& context) override;
  void Exit(BmsFsmContext& context) override;
};

class StateSecure final : public State {
 public:
  static StateSecure& Instance();

  BmsState Id() const override { return BmsState::kSecure; }
  const char* Name() const override { return "Secure"; }

  void HandleVoltageNominal(BmsFsmContext& context) override;
  void HandleTemperatureNominal(BmsFsmContext& context) override;
  void HandleCurrentNominal(BmsFsmContext& context) override;
  void HandleSensorNominal(BmsFsmContext& context) override;
  void Entry(BmsFsmContext& context) override;
  void Exit(BmsFsmContext& context) override;
};

class BmsFsmContext {
 public:
  BmsFsmContext();

  void Start();
  void UpdateFaultStatus(const FaultStatus& fault_status);

  void HandleChargerConnected();
  void HandleChargerDisconnected();
  void HandleVoltageTooHigh();
  void HandleVoltageTooLow();
  void HandleVoltageNominal();
  void HandleTemperatureTooHigh();
  void HandleTemperatureTooLow();
  void HandleTemperatureNominal();
  void HandleCurrentTooHigh();
  void HandleCurrentNominal();
  void HandleSensorFault();
  void HandleSensorNominal();

  void SetState(State& new_state);
  BmsState GetState() const { return current_state_->Id(); }
  const State& GetCurrentState() const { return *current_state_; }
  const State* GetPreviousState() const { return previous_state_; }

  void SetChargerConnected(bool connected) { charger_connected_ = connected; }
  bool IsChargerConnected() const { return charger_connected_; }

  void SetVoltageNominal(bool nominal) { voltage_nominal_ = nominal; }
  bool IsVoltageNominal() const { return voltage_nominal_; }

  void SetTemperatureNominal(bool nominal) { temperature_nominal_ = nominal; }
  bool IsTemperatureNominal() const { return temperature_nominal_; }

  void SetCurrentNominal(bool nominal) { current_nominal_ = nominal; }
  bool IsCurrentNominal() const { return current_nominal_; }

  void SetSensorNominal(bool nominal) { sensor_nominal_ = nominal; }
  bool IsSensorNominal() const { return sensor_nominal_; }

  bool AllInputsNominal() const;
  void ReturnToOperatingState();

 private:
  State* previous_state_{nullptr};
  State* current_state_;
  bool started_{false};
  bool charger_connected_{false};
  bool voltage_nominal_{true};
  bool temperature_nominal_{true};
  bool current_nominal_{true};
  bool sensor_nominal_{true};
};

}  // namespace bms::domain
