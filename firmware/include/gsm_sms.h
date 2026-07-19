#pragma once
#include "types.h"

void gsmBegin();
void gsmEnqueueFaultSms(FaultCode f, const Meas& m);
void gsmTaskPoll();  // call from GSM task loop
