#include "linear_axis_macros.h"
#include "linear_axis.h"
#include "rtwtypes.h"
#include "linear_axis_private.h"
#include <string.h>

B_linear_axis_T linear_axis_B;
X_linear_axis_T linear_axis_X;
XDis_linear_axis_T linear_axis_XDis;
DW_linear_axis_T linear_axis_DW;
ExtU_linear_axis_T linear_axis_U;
ExtY_linear_axis_T linear_axis_Y;
static RT_MODEL_linear_axis_T linear_axis_M_;
RT_MODEL_linear_axis_T *const linear_axis_M = &linear_axis_M_;
real_T look1_binlxpw(real_T u0, const real_T bp0[], const real_T table[],
                     uint32_T maxIndex)
{
  real_T frac;
  real_T yL_0d0;
  uint32_T iLeft;
  if (u0 <= bp0[0U]) {
    iLeft = 0U;
    frac = (u0 - bp0[0U]) / (bp0[1U] - bp0[0U]);
  } else if (u0 < bp0[maxIndex]) {
    uint32_T bpIdx;
    uint32_T iRght;
    bpIdx = maxIndex >> 1U;
    iLeft = 0U;
    iRght = maxIndex;
    while (iRght - iLeft > 1U) {
      if (u0 < bp0[bpIdx]) {
        iRght = bpIdx;
      } else {
        iLeft = bpIdx;
      }

      bpIdx = (iRght + iLeft) >> 1U;
    }

    frac = (u0 - bp0[iLeft]) / (bp0[iLeft + 1U] - bp0[iLeft]);
  } else {
    iLeft = maxIndex - 1U;
    frac = (u0 - bp0[maxIndex - 1U]) / (bp0[maxIndex] - bp0[maxIndex - 1U]);
  }

  yL_0d0 = table[iLeft];
  return (table[iLeft + 1U] - yL_0d0) * frac + yL_0d0;
}

void linear_axis_output(void)
{
  real_T rtb_Sum;
  real_T rtb_Switch2;
  boolean_T tmp;
  linear_axis_B.s_dot = linear_axis_X.Integrator2_CSTATE;
  linear_axis_Y.v = linear_axis_B.s_dot;
  linear_axis_Y.s = linear_axis_X.Integrator3_CSTATE;
  linear_axis_B.omega = linear_axis_P.g_gb[0] * linear_axis_B.s_dot;
  tmp = rtsiIsModeUpdateTimeStep(&linear_axis_M->solverInfo);
  if (tmp) {
    linear_axis_DW.Abs_MODE = (linear_axis_B.omega >= 0.0);
  }

  linear_axis_B.maxT = look1_binlxpw((linear_axis_DW.Abs_MODE > 0 ?
    linear_axis_B.omega : -linear_axis_B.omega) * linear_axis_P.Gain_Gain,
    linear_axis_P.uDLookupTable_bp01Data, linear_axis_P.uDLookupTable_tableData,
    6U);
  rtb_Sum = linear_axis_U.setpoint - linear_axis_X.Integrator3_CSTATE;
  linear_axis_B.FilterCoefficient = (linear_axis_P.PIDController_D * rtb_Sum -
    linear_axis_X.Filter_CSTATE) * linear_axis_P.PIDController_N;
  linear_axis_B.Sum = (linear_axis_P.PIDController_P * rtb_Sum +
                       linear_axis_X.Integrator_CSTATE) +
    linear_axis_B.FilterCoefficient;
  if (tmp) {
    linear_axis_DW.Saturation_MODE = linear_axis_B.Sum >=
      linear_axis_P.Saturation_UpperSat ? 1 : linear_axis_B.Sum >
      linear_axis_P.Saturation_LowerSat ? 0 : -1;
  }

  linear_axis_B.Gain5 = (linear_axis_DW.Saturation_MODE == 1 ?
    linear_axis_P.Saturation_UpperSat : linear_axis_DW.Saturation_MODE == -1 ?
    linear_axis_P.Saturation_LowerSat : linear_axis_B.Sum) *
    linear_axis_P.Gain5_Gain;
  if (tmp) {
    linear_axis_DW.LowerRelop1_Mode = (linear_axis_B.Gain5 > linear_axis_B.maxT);
  }

  linear_axis_B.minT = linear_axis_P.Gain2_Gain * linear_axis_B.maxT;
  if (tmp) {
    linear_axis_DW.UpperRelop_Mode = (linear_axis_B.Gain5 < linear_axis_B.minT);
  }

  if (linear_axis_DW.LowerRelop1_Mode) {
    rtb_Switch2 = linear_axis_B.maxT;
  } else if (linear_axis_DW.UpperRelop_Mode) {
    rtb_Switch2 = linear_axis_B.minT;
  } else {
    rtb_Switch2 = linear_axis_B.Gain5;
  }

  linear_axis_Y.T = rtb_Switch2;
  linear_axis_B.IntegralGain = linear_axis_P.PIDController_I * rtb_Sum;
  linear_axis_B.s_ddot = 1.0 / linear_axis_P.R[0] * rtb_Switch2 * (1.0 /
    linear_axis_P.mass[0]) - linear_axis_P.r_v[0] / linear_axis_P.mass[0] *
    linear_axis_B.s_dot;
}

void linear_axis_update(void)
{
  if (rtmIsMajorTimeStep(linear_axis_M)) {
    if (!(++linear_axis_M->Timing.clockTick0)) {
      ++linear_axis_M->Timing.clockTickH0;
    }

    linear_axis_M->Timing.t[0] = rtsiGetSolverStopTime
      (&linear_axis_M->solverInfo);

    {
      linear_axis_M->Timing.clockTick1++;
      if (!linear_axis_M->Timing.clockTick1) {
        linear_axis_M->Timing.clockTickH1++;
      }
    }
  }
}

void linear_axis_derivatives(void)
{
  XDot_linear_axis_T *_rtXdot;
  _rtXdot = ((XDot_linear_axis_T *) linear_axis_M->derivs);
  _rtXdot->Integrator2_CSTATE = linear_axis_B.s_ddot;
  _rtXdot->Integrator3_CSTATE = linear_axis_B.s_dot;
  _rtXdot->Integrator_CSTATE = linear_axis_B.IntegralGain;
  _rtXdot->Filter_CSTATE = linear_axis_B.FilterCoefficient;
}

void linear_axis_zeroCrossings(void)
{
  ZCV_linear_axis_T *_rtZCSV;
  _rtZCSV = ((ZCV_linear_axis_T *) linear_axis_M->zcSignalVector);
  _rtZCSV->Abs_AbsZc_ZC = linear_axis_B.omega;
  _rtZCSV->Saturation_UprLim_ZC = linear_axis_B.Sum -
    linear_axis_P.Saturation_UpperSat;
  _rtZCSV->Saturation_LwrLim_ZC = linear_axis_B.Sum -
    linear_axis_P.Saturation_LowerSat;
  _rtZCSV->LowerRelop1_RelopInput_ZC = linear_axis_B.Gain5 - linear_axis_B.maxT;
  _rtZCSV->UpperRelop_RelopInput_ZC = linear_axis_B.Gain5 - linear_axis_B.minT;
}

void linear_axis_initialize(void)
{
  linear_axis_X.Integrator2_CSTATE = linear_axis_P.Integrator2_IC;
  linear_axis_X.Integrator3_CSTATE = linear_axis_P.Integrator3_IC;
  linear_axis_X.Integrator_CSTATE =
    linear_axis_P.PIDController_InitialConditionForIntegrator;
  linear_axis_X.Filter_CSTATE =
    linear_axis_P.PIDController_InitialConditionForFilter;
}

void linear_axis_terminate(void)
{
}

RT_MODEL_linear_axis_T *linear_axis(void)
{
  (void) memset((void *)linear_axis_M, 0,
                sizeof(RT_MODEL_linear_axis_T));

  {
    rtsiSetSimTimeStepPtr(&linear_axis_M->solverInfo,
                          &linear_axis_M->Timing.simTimeStep);
    rtsiSetTPtr(&linear_axis_M->solverInfo, &rtmGetTPtr(linear_axis_M));
    rtsiSetStepSizePtr(&linear_axis_M->solverInfo,
                       &linear_axis_M->Timing.stepSize0);
    rtsiSetdXPtr(&linear_axis_M->solverInfo, &linear_axis_M->derivs);
    rtsiSetContStatesPtr(&linear_axis_M->solverInfo, (real_T **)
                         &linear_axis_M->contStates);
    rtsiSetNumContStatesPtr(&linear_axis_M->solverInfo,
      &linear_axis_M->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&linear_axis_M->solverInfo,
      &linear_axis_M->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&linear_axis_M->solverInfo,
      &linear_axis_M->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&linear_axis_M->solverInfo,
      &linear_axis_M->periodicContStateRanges);
    rtsiSetContStateDisabledPtr(&linear_axis_M->solverInfo, (boolean_T**)
      &linear_axis_M->contStateDisabled);
    rtsiSetSolverZcSignalPtr(&linear_axis_M->solverInfo,
      &linear_axis_M->zcSignalVector);
    rtsiSetErrorStatusPtr(&linear_axis_M->solverInfo, (&rtmGetErrorStatus
      (linear_axis_M)));
    rtsiSetRTModelPtr(&linear_axis_M->solverInfo, linear_axis_M);
  }

  rtsiSetSimTimeStep(&linear_axis_M->solverInfo, MAJOR_TIME_STEP);
  rtsiSetIsMinorTimeStepWithModeChange(&linear_axis_M->solverInfo, false);
  rtsiSetIsContModeFrozen(&linear_axis_M->solverInfo, false);
  linear_axis_M->intgData.y = linear_axis_M->odeY;
  linear_axis_M->intgData.f[0] = linear_axis_M->odeF[0];
  linear_axis_M->intgData.f[1] = linear_axis_M->odeF[1];
  linear_axis_M->intgData.f[2] = linear_axis_M->odeF[2];
  linear_axis_M->contStates = ((X_linear_axis_T *) &linear_axis_X);
  linear_axis_M->contStateDisabled = ((XDis_linear_axis_T *) &linear_axis_XDis);
  linear_axis_M->Timing.tStart = (0.0);
  rtsiSetSolverData(&linear_axis_M->solverInfo, (void *)&linear_axis_M->intgData);

  {
    static uint8_T zcAttributes[5] = { (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL),
      (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL) };

    rtsiSetSolverZcSignalAttrib(&linear_axis_M->solverInfo,
      zcAttributes);
    linear_axis_M->zcDetectorData.vL = linear_axis_M->zcVL;
    linear_axis_M->zcDetectorData.vR = linear_axis_M->zcVR;
    linear_axis_M->zcDetectorData.vZ = linear_axis_M->zcVZ;
    linear_axis_M->zcDetectorData.t0 = linear_axis_M->zct0;
    linear_axis_M->zcDetectorData.tR = linear_axis_M->zctR;
    linear_axis_M->zcDetectorData.x0 = linear_axis_M->zcx0;
    linear_axis_M->zcDetectorData.xR = linear_axis_M->zcxR;
  }

  rtsiSetSolverName(&linear_axis_M->solverInfo,"ode3");
  rtmSetTPtr(linear_axis_M, &linear_axis_M->Timing.tArray[0]);
  linear_axis_M->Timing.stepSize0 = 0.001;
  (void) memset(((void *) &linear_axis_B), 0,
                sizeof(B_linear_axis_T));

  {
    (void) memset((void *)&linear_axis_X, 0,
                  sizeof(X_linear_axis_T));
  }

  {
    (void) memset((void *)&linear_axis_XDis, 0,
                  sizeof(XDis_linear_axis_T));
  }

  (void) memset((void *)&linear_axis_DW, 0,
                sizeof(DW_linear_axis_T));
  linear_axis_U.setpoint = 0.0;
  (void)memset(&linear_axis_Y, 0, sizeof(ExtY_linear_axis_T));
  return linear_axis_M;
}
