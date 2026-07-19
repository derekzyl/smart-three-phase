#pragma once
#include "types.h"

void oledBegin();
void oledRender(const Meas& m, FaultCode f, bool relayOpen);
