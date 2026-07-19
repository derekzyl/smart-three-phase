#pragma once
#include <Arduino.h>

enum class FaultCode : uint8_t {
  NONE = 0,
  OV,
  UV,
  OC,
  PL,
  IMB
};

inline const char* faultName(FaultCode f) {
  switch (f) {
    case FaultCode::OV: return "OV";
    case FaultCode::UV: return "UV";
    case FaultCode::OC: return "OC";
    case FaultCode::PL: return "PL";
    case FaultCode::IMB: return "IMB";
    default: return "NONE";
  }
}

struct Meas {
  float Va = 0, Vb = 0, Vc = 0;
  float Ia = 0, Ib = 0, Ic = 0;
  uint32_t ms = 0;
};

struct SharedState {
  Meas meas;
  FaultCode fault = FaultCode::NONE;
  bool relayOpen = false;
  bool latched = false;
  bool resetRequest = false;
  portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
};

extern SharedState gState;
