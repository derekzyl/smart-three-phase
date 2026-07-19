#pragma once
#include "types.h"

void netBegin();
void netLoop();  // reconnect + mqtt.loop
void netPublishTelemetry(const Meas& m, FaultCode f, bool relayOpen);
void netPublishFault(const Meas& m, FaultCode f);
bool netConsumeResetCommand();
