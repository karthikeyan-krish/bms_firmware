#include "bms_fsm.hpp"

#include <gtest/gtest.h>

#include "fault_status.hpp"

namespace {

TEST(BmsFsmTest, StartsInIdleDischarge) {
  bms::domain::BmsFsmContext fsm;

  fsm.Start();

  EXPECT_EQ(fsm.GetState(), bms::domain::BmsState::kIdleDischarge);
  EXPECT_EQ(fsm.GetCurrentState().Name(), "IdleDischarge");
}

TEST(BmsFsmTest, IdleDischargeHandlesChargerConnected) {
  bms::domain::BmsFsmContext fsm;

  fsm.Start();
  fsm.HandleChargerConnected();

  EXPECT_TRUE(fsm.IsChargerConnected());
  EXPECT_EQ(fsm.GetState(), bms::domain::BmsState::kCharge);
  ASSERT_NE(fsm.GetPreviousState(), nullptr);
  EXPECT_EQ(fsm.GetPreviousState()->Id(),
            bms::domain::BmsState::kIdleDischarge);
}

TEST(BmsFsmTest, ChargeHandlesChargerDisconnected) {
  bms::domain::BmsFsmContext fsm;

  fsm.Start();
  fsm.HandleChargerConnected();
  fsm.HandleChargerDisconnected();

  EXPECT_FALSE(fsm.IsChargerConnected());
  EXPECT_EQ(fsm.GetState(), bms::domain::BmsState::kIdleDischarge);
}

TEST(BmsFsmTest, OperatingStatesHandleFaultsByEnteringSecure) {
  bms::domain::BmsFsmContext fsm;

  fsm.Start();
  fsm.HandleCurrentTooHigh();

  EXPECT_FALSE(fsm.IsCurrentNominal());
  EXPECT_EQ(fsm.GetState(), bms::domain::BmsState::kSecure);
}

TEST(BmsFsmTest, ChargeToleratesUndervoltage) {
  bms::domain::BmsFsmContext fsm;

  fsm.Start();
  fsm.HandleChargerConnected();
  fsm.HandleVoltageTooLow();

  EXPECT_FALSE(fsm.IsVoltageNominal());
  EXPECT_EQ(fsm.GetState(), bms::domain::BmsState::kCharge);
}

TEST(BmsFsmTest, SecureStaysSecureUntilAllInputsAreNominal) {
  bms::domain::BmsFsmContext fsm;

  fsm.Start();
  fsm.HandleVoltageTooHigh();
  EXPECT_EQ(fsm.GetState(), bms::domain::BmsState::kSecure);

  fsm.HandleTemperatureTooHigh();
  fsm.HandleVoltageNominal();

  EXPECT_TRUE(fsm.IsVoltageNominal());
  EXPECT_FALSE(fsm.IsTemperatureNominal());
  EXPECT_EQ(fsm.GetState(), bms::domain::BmsState::kSecure);
}

TEST(BmsFsmTest, SecureReturnsToIdleWhenFaultsClearAndChargerDisconnected) {
  bms::domain::BmsFsmContext fsm;

  fsm.Start();
  fsm.HandleCurrentTooHigh();
  fsm.HandleCurrentNominal();

  EXPECT_EQ(fsm.GetState(), bms::domain::BmsState::kIdleDischarge);
}

TEST(BmsFsmTest, SecureReturnsToChargeWhenFaultsClearAndChargerConnected) {
  bms::domain::BmsFsmContext fsm;

  fsm.Start();
  fsm.HandleChargerConnected();
  fsm.HandleSensorFault();
  fsm.HandleSensorNominal();

  EXPECT_EQ(fsm.GetState(), bms::domain::BmsState::kCharge);
}

TEST(BmsFsmTest, UpdateFaultStatusRoutesEventsThroughCurrentState) {
  bms::domain::BmsFsmContext fsm;
  bms::domain::FaultStatus faults{};

  fsm.Start();
  faults.overvoltage = true;
  fsm.UpdateFaultStatus(faults);

  EXPECT_EQ(fsm.GetState(), bms::domain::BmsState::kSecure);

  faults.overvoltage = false;
  faults.undervoltage = true;
  fsm.UpdateFaultStatus(faults);

  EXPECT_EQ(fsm.GetState(), bms::domain::BmsState::kSecure);

  faults.undervoltage = false;
  fsm.UpdateFaultStatus(faults);

  EXPECT_EQ(fsm.GetState(), bms::domain::BmsState::kIdleDischarge);
}

}  // namespace
