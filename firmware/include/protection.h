#pragma once
#include "types.h"

FaultCode evaluateFaults(const Meas& m);
float voltageImbalancePct(const Meas& m);
