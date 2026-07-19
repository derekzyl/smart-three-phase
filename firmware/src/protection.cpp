#include "protection.h"
#include "config.h"
#include <math.h>

static uint8_t cPL = 0, cOV = 0, cUV = 0, cOC = 0, cIMB = 0;

static void bump(bool pred, uint8_t& cnt) {
  if (pred) {
    if (cnt < 255) ++cnt;
  } else {
    cnt = 0;
  }
}

static bool conf(uint8_t cnt) { return cnt >= FAULT_CONFIRM_N; }

float voltageImbalancePct(const Meas& m) {
  float avg = (m.Va + m.Vb + m.Vc) / 3.0f;
  if (avg < 1.0f) return 0.0f;
  float d = fabsf(m.Va - avg);
  d = fmaxf(d, fabsf(m.Vb - avg));
  d = fmaxf(d, fabsf(m.Vc - avg));
  return (d / avg) * 100.0f;
}

FaultCode evaluateFaults(const Meas& m) {
  const float vmax = fmaxf(m.Va, fmaxf(m.Vb, m.Vc));
  const bool pl =
      ((m.Va < V_PL) || (m.Vb < V_PL) || (m.Vc < V_PL)) && (vmax > V_PRESENT);
  const bool ov = (m.Va > V_OV) || (m.Vb > V_OV) || (m.Vc > V_OV);
  const bool uv =
      ((m.Va < V_UV) || (m.Vb < V_UV) || (m.Vc < V_UV)) && !pl;
  const bool oc = (m.Ia > I_OC) || (m.Ib > I_OC) || (m.Ic > I_OC);
  const bool imb = voltageImbalancePct(m) > IMB_LIMIT_PCT;

  bump(pl, cPL);
  bump(ov, cOV);
  bump(uv, cUV);
  bump(oc, cOC);
  bump(imb, cIMB);

  if (conf(cPL)) return FaultCode::PL;
  if (conf(cOV)) return FaultCode::OV;
  if (conf(cUV)) return FaultCode::UV;
  if (conf(cOC)) return FaultCode::OC;
  if (conf(cIMB)) return FaultCode::IMB;
  return FaultCode::NONE;
}
