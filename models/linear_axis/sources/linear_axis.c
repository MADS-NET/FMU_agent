#include "linear_axis_macros.h"
#include "linear_axis.h"
#include "linear_axis_private.h"
#include "rtwtypes.h"
#include <string.h>
#include "zero_crossing_types.h"
#include <math.h>

B_linear_axis_T linear_axis_B;
X_linear_axis_T linear_axis_X;
XDis_linear_axis_T linear_axis_XDis;
DW_linear_axis_T linear_axis_DW;
PrevZCX_linear_axis_T linear_axis_PrevZCX;
ExtU_linear_axis_T linear_axis_U;
ExtY_linear_axis_T linear_axis_Y;
static RT_MODEL_linear_axis_T linear_axis_M_;
RT_MODEL_linear_axis_T *const linear_axis_M = &linear_axis_M_;
real_T rt_roundd_snf(real_T u)
{
  real_T y;
  if (fabs(u) < 4.503599627370496E+15) {
    if (u >= 0.5) {
      y = floor(u + 0.5);
    } else if (u > -0.5) {
      y = u * 0.0;
    } else {
      y = ceil(u - 0.5);
    }
  } else {
    y = u;
  }

  return y;
}

void linear_axis_output(void)
{
  real_T rtb_error;
  boolean_T didZcEventOccur;
  boolean_T tmp;
  tmp = rtsiIsModeUpdateTimeStep(&linear_axis_M->solverInfo);
  if (tmp) {
    didZcEventOccur = (linear_axis_U.reset &&
                       (linear_axis_PrevZCX.Integrator1_Reset_ZCE != POS_ZCSIG));
    linear_axis_PrevZCX.Integrator1_Reset_ZCE = linear_axis_U.reset;
    if (didZcEventOccur) {
      linear_axis_X.position = linear_axis_P.X0;
    }

    didZcEventOccur = (linear_axis_U.reset &&
                       (linear_axis_PrevZCX.Integrator_Reset_ZCE != POS_ZCSIG));
    linear_axis_PrevZCX.Integrator_Reset_ZCE = linear_axis_U.reset;
    if (didZcEventOccur) {
      linear_axis_X.speed = linear_axis_P.Xaxis_v0;
    }

    didZcEventOccur = (linear_axis_U.reset &&
                       (linear_axis_PrevZCX.Integrator_Reset_ZCE_i != POS_ZCSIG));
    linear_axis_PrevZCX.Integrator_Reset_ZCE_i = linear_axis_U.reset;
    if (didZcEventOccur) {
      linear_axis_X.Integrator_CSTATE =
        linear_axis_P.PIDController_InitialConditionForIntegrator;
    }

    didZcEventOccur = (linear_axis_U.reset &&
                       (linear_axis_PrevZCX.Filter_Reset_ZCE != POS_ZCSIG));
    linear_axis_PrevZCX.Filter_Reset_ZCE = linear_axis_U.reset;
    if (didZcEventOccur) {
      linear_axis_X.Filter_CSTATE =
        linear_axis_P.PIDController_InitialConditionForFilter;
    }
  }

  linear_axis_Y.X = linear_axis_P.Gain_Gain * linear_axis_X.position;
  linear_axis_B.v = linear_axis_X.speed;
  linear_axis_Y.Vx = linear_axis_P.Gain1_Gain * linear_axis_B.v;
  rtb_error = linear_axis_P.tom_Gain * linear_axis_U.setpoint - rt_roundd_snf
    (linear_axis_X.position / linear_axis_P.res) * linear_axis_P.res;
  linear_axis_B.FilterCoefficient = (linear_axis_P.XD * rtb_error -
    linear_axis_X.Filter_CSTATE) * linear_axis_P.XN;
  linear_axis_B.Sum = (linear_axis_P.XP * rtb_error +
                       linear_axis_X.Integrator_CSTATE) +
    linear_axis_B.FilterCoefficient;
  linear_axis_B.ZeroGain = linear_axis_P.ZeroGain_Gain * linear_axis_B.Sum;
  if (tmp) {
    if (linear_axis_B.Sum > linear_axis_P.Xsat) {
      linear_axis_DW.DeadZone_MODE = 1;
    } else if (linear_axis_B.Sum >= -linear_axis_P.Xsat) {
      linear_axis_DW.DeadZone_MODE = 0;
    } else {
      linear_axis_DW.DeadZone_MODE = -1;
    }
  }

  if (linear_axis_DW.DeadZone_MODE == 1) {
    linear_axis_B.DeadZone = linear_axis_B.Sum - linear_axis_P.Xsat;
  } else if (linear_axis_DW.DeadZone_MODE == -1) {
    linear_axis_B.DeadZone = linear_axis_B.Sum - (-linear_axis_P.Xsat);
  } else {
    linear_axis_B.DeadZone = 0.0;
  }

  linear_axis_B.IntegralGain = linear_axis_P.XI * rtb_error;
  if (tmp) {
    linear_axis_DW.NotEqual_Mode = (linear_axis_B.ZeroGain !=
      linear_axis_B.DeadZone);
    if (linear_axis_B.DeadZone > 0.0) {
      linear_axis_DW.SignPreSat_MODE = 1;
    } else if (linear_axis_B.DeadZone < 0.0) {
      linear_axis_DW.SignPreSat_MODE = -1;
    } else {
      linear_axis_DW.SignPreSat_MODE = 0;
    }

    if (linear_axis_B.IntegralGain > 0.0) {
      linear_axis_DW.SignPreIntegrator_MODE = 1;
    } else if (linear_axis_B.IntegralGain < 0.0) {
      linear_axis_DW.SignPreIntegrator_MODE = -1;
    } else {
      linear_axis_DW.SignPreIntegrator_MODE = 0;
    }
  }

  linear_axis_B.AND3 = (linear_axis_DW.NotEqual_Mode && ((int8_T)
    linear_axis_DW.SignPreSat_MODE == (int8_T)
    linear_axis_DW.SignPreIntegrator_MODE));
  if (rtmIsMajorTimeStep(linear_axis_M)) {
    linear_axis_B.Memory = linear_axis_DW.Memory_PreviousInput;
  }

  if (linear_axis_B.Memory) {
    linear_axis_B.Switch = linear_axis_P.Constant1_Value;
  } else {
    linear_axis_B.Switch = linear_axis_B.IntegralGain;
  }

  if (tmp) {
    linear_axis_DW.Saturation_MODE = linear_axis_B.Sum >= linear_axis_P.Xsat ? 1
      : linear_axis_B.Sum > -linear_axis_P.Xsat ? 0 : -1;
  }

  linear_axis_B.a = (((linear_axis_DW.Saturation_MODE == 1 ? linear_axis_P.Xsat :
                       linear_axis_DW.Saturation_MODE == -1 ?
                       -linear_axis_P.Xsat : linear_axis_B.Sum) +
                      linear_axis_U.noiseX) - linear_axis_P.Xdc *
                     linear_axis_B.v) * (1.0 / linear_axis_P.Xm);
}

void linear_axis_update(void)
{
  if (rtmIsMajorTimeStep(linear_axis_M)) {
    linear_axis_DW.Memory_PreviousInput = linear_axis_B.AND3;
  }

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
  _rtXdot->position = linear_axis_B.v;
  _rtXdot->speed = linear_axis_B.a;
  _rtXdot->Integrator_CSTATE = linear_axis_B.Switch;
  _rtXdot->Filter_CSTATE = linear_axis_B.FilterCoefficient;
}

void linear_axis_zeroCrossings(void)
{
  ZCV_linear_axis_T *_rtZCSV;
  real_T DeadZone_LwrReg_ZC_tmp;
  real_T DeadZone_UprReg_ZC_tmp;
  _rtZCSV = ((ZCV_linear_axis_T *) linear_axis_M->zcSignalVector);
  DeadZone_LwrReg_ZC_tmp = linear_axis_B.Sum - (-linear_axis_P.Xsat);
  _rtZCSV->DeadZone_LwrReg_ZC = DeadZone_LwrReg_ZC_tmp;
  DeadZone_UprReg_ZC_tmp = linear_axis_B.Sum - linear_axis_P.Xsat;
  _rtZCSV->DeadZone_UprReg_ZC = DeadZone_UprReg_ZC_tmp;
  _rtZCSV->NotEqual_RelopInput_ZC = linear_axis_B.ZeroGain -
    linear_axis_B.DeadZone;
  _rtZCSV->SignPreSat_Input_ZC = linear_axis_B.DeadZone;
  _rtZCSV->SignPreIntegrator_Input_ZC = linear_axis_B.IntegralGain;
  _rtZCSV->Saturation_UprLim_ZC = DeadZone_UprReg_ZC_tmp;
  _rtZCSV->Saturation_LwrLim_ZC = DeadZone_LwrReg_ZC_tmp;
}

void linear_axis_initialize(void)
{
  linear_axis_PrevZCX.Integrator1_Reset_ZCE = UNINITIALIZED_ZCSIG;
  linear_axis_PrevZCX.Integrator_Reset_ZCE = UNINITIALIZED_ZCSIG;
  linear_axis_PrevZCX.Integrator_Reset_ZCE_i = UNINITIALIZED_ZCSIG;
  linear_axis_PrevZCX.Filter_Reset_ZCE = UNINITIALIZED_ZCSIG;
  linear_axis_X.position = linear_axis_P.X0;
  linear_axis_X.speed = linear_axis_P.Xaxis_v0;
  linear_axis_X.Integrator_CSTATE =
    linear_axis_P.PIDController_InitialConditionForIntegrator;
  linear_axis_X.Filter_CSTATE =
    linear_axis_P.PIDController_InitialConditionForFilter;
  linear_axis_DW.Memory_PreviousInput = linear_axis_P.Memory_InitialCondition;
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
    static uint8_T zcAttributes[11] = { (0xc0|SL_ZCS_EVENT_ALL_UP), (0xc0|
      SL_ZCS_EVENT_ALL_UP), (0xc0|SL_ZCS_EVENT_ALL_UP), (0xc0|
      SL_ZCS_EVENT_ALL_UP), (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL),
      (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL),
      (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL) };

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
  linear_axis_M->Timing.stepSize0 = 1.0;
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
  (void)memset(&linear_axis_U, 0, sizeof(ExtU_linear_axis_T));
  (void)memset(&linear_axis_Y, 0, sizeof(ExtY_linear_axis_T));
  return linear_axis_M;
}
