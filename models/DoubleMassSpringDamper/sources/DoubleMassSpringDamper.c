#include "DoubleMassSpringDamper_macros.h"
#include "DoubleMassSpringDamper.h"
#include "rtwtypes.h"
#include "DoubleMassSpringDamper_private.h"
#include <string.h>

B_DoubleMassSpringDamper_T DoubleMassSpringDamper_B;
X_DoubleMassSpringDamper_T DoubleMassSpringDamper_X;
XDis_DoubleMassSpringDamper_T DoubleMassSpringDamper_XDis;
ExtY_DoubleMassSpringDamper_T DoubleMassSpringDamper_Y;
static RT_MODEL_DoubleMassSpringDamper_T DoubleMassSpringDamper_M_;
RT_MODEL_DoubleMassSpringDamper_T *const DoubleMassSpringDamper_M =
  &DoubleMassSpringDamper_M_;
void DoubleMassSpringDamper_output(void)
{
  real_T rtb_Damping2;
  real_T rtb_Stiffness2;
  real_T uMass1_tmp;
  DoubleMassSpringDamper_B.s1 = DoubleMassSpringDamper_X.Integrator1_CSTATE;
  DoubleMassSpringDamper_Y.M1 = DoubleMassSpringDamper_B.s1;
  DoubleMassSpringDamper_B.s2 = DoubleMassSpringDamper_X.Integrator3_CSTATE;
  DoubleMassSpringDamper_Y.M2 = DoubleMassSpringDamper_B.s2;
  rtb_Damping2 = (DoubleMassSpringDamper_B.s2 - DoubleMassSpringDamper_B.s1) *
    DoubleMassSpringDamper_P.b;
  rtb_Stiffness2 = (DoubleMassSpringDamper_X.Integrator4_CSTATE -
                    DoubleMassSpringDamper_X.Integrator2_CSTATE) *
    DoubleMassSpringDamper_P.k;
  uMass1_tmp = 1.0 / DoubleMassSpringDamper_P.m;
  DoubleMassSpringDamper_B.uMass1 = (((rtb_Damping2 + rtb_Stiffness2) -
    DoubleMassSpringDamper_P.k * DoubleMassSpringDamper_X.Integrator2_CSTATE) -
    DoubleMassSpringDamper_P.b * DoubleMassSpringDamper_B.s1) * uMass1_tmp;
  DoubleMassSpringDamper_B.uMass2 = ((0.0 - rtb_Stiffness2) - rtb_Damping2) *
    uMass1_tmp;
}

void DoubleMassSpringDamper_update(void)
{
  if (!(++DoubleMassSpringDamper_M->Timing.clockTick0)) {
    ++DoubleMassSpringDamper_M->Timing.clockTickH0;
  }

  DoubleMassSpringDamper_M->Timing.t[0] = rtsiGetSolverStopTime
    (&DoubleMassSpringDamper_M->solverInfo);
}

void DoubleMassSpringDamper_derivatives(void)
{
  XDot_DoubleMassSpringDamper_T *_rtXdot;
  _rtXdot = ((XDot_DoubleMassSpringDamper_T *) DoubleMassSpringDamper_M->derivs);
  _rtXdot->Integrator1_CSTATE = DoubleMassSpringDamper_B.uMass1;
  _rtXdot->Integrator3_CSTATE = DoubleMassSpringDamper_B.uMass2;
  _rtXdot->Integrator4_CSTATE = DoubleMassSpringDamper_B.s2;
  _rtXdot->Integrator2_CSTATE = DoubleMassSpringDamper_B.s1;
}

void DoubleMassSpringDamper_initialize(void)
{
  DoubleMassSpringDamper_X.Integrator1_CSTATE =
    DoubleMassSpringDamper_P.Integrator1_IC;
  DoubleMassSpringDamper_X.Integrator3_CSTATE =
    DoubleMassSpringDamper_P.Integrator3_IC;
  DoubleMassSpringDamper_X.Integrator4_CSTATE =
    DoubleMassSpringDamper_P.Integrator4_IC;
  DoubleMassSpringDamper_X.Integrator2_CSTATE =
    DoubleMassSpringDamper_P.Integrator2_IC;
}

void DoubleMassSpringDamper_terminate(void)
{
}

RT_MODEL_DoubleMassSpringDamper_T *DoubleMassSpringDamper(void)
{
  (void) memset((void *)DoubleMassSpringDamper_M, 0,
                sizeof(RT_MODEL_DoubleMassSpringDamper_T));

  {
    rtsiSetSimTimeStepPtr(&DoubleMassSpringDamper_M->solverInfo,
                          &DoubleMassSpringDamper_M->Timing.simTimeStep);
    rtsiSetTPtr(&DoubleMassSpringDamper_M->solverInfo, &rtmGetTPtr
                (DoubleMassSpringDamper_M));
    rtsiSetStepSizePtr(&DoubleMassSpringDamper_M->solverInfo,
                       &DoubleMassSpringDamper_M->Timing.stepSize0);
    rtsiSetdXPtr(&DoubleMassSpringDamper_M->solverInfo,
                 &DoubleMassSpringDamper_M->derivs);
    rtsiSetContStatesPtr(&DoubleMassSpringDamper_M->solverInfo, (real_T **)
                         &DoubleMassSpringDamper_M->contStates);
    rtsiSetNumContStatesPtr(&DoubleMassSpringDamper_M->solverInfo,
      &DoubleMassSpringDamper_M->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&DoubleMassSpringDamper_M->solverInfo,
      &DoubleMassSpringDamper_M->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&DoubleMassSpringDamper_M->solverInfo,
      &DoubleMassSpringDamper_M->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&DoubleMassSpringDamper_M->solverInfo,
      &DoubleMassSpringDamper_M->periodicContStateRanges);
    rtsiSetContStateDisabledPtr(&DoubleMassSpringDamper_M->solverInfo,
      (boolean_T**) &DoubleMassSpringDamper_M->contStateDisabled);
    rtsiSetErrorStatusPtr(&DoubleMassSpringDamper_M->solverInfo,
                          (&rtmGetErrorStatus(DoubleMassSpringDamper_M)));
    rtsiSetRTModelPtr(&DoubleMassSpringDamper_M->solverInfo,
                      DoubleMassSpringDamper_M);
  }

  rtsiSetSimTimeStep(&DoubleMassSpringDamper_M->solverInfo, MAJOR_TIME_STEP);
  rtsiSetIsMinorTimeStepWithModeChange(&DoubleMassSpringDamper_M->solverInfo,
    false);
  rtsiSetIsContModeFrozen(&DoubleMassSpringDamper_M->solverInfo, false);
  DoubleMassSpringDamper_M->intgData.y = DoubleMassSpringDamper_M->odeY;
  DoubleMassSpringDamper_M->intgData.f[0] = DoubleMassSpringDamper_M->odeF[0];
  DoubleMassSpringDamper_M->intgData.f[1] = DoubleMassSpringDamper_M->odeF[1];
  DoubleMassSpringDamper_M->intgData.f[2] = DoubleMassSpringDamper_M->odeF[2];
  DoubleMassSpringDamper_M->contStates = ((X_DoubleMassSpringDamper_T *)
    &DoubleMassSpringDamper_X);
  DoubleMassSpringDamper_M->contStateDisabled = ((XDis_DoubleMassSpringDamper_T *)
    &DoubleMassSpringDamper_XDis);
  DoubleMassSpringDamper_M->Timing.tStart = (0.0);
  rtsiSetSolverData(&DoubleMassSpringDamper_M->solverInfo, (void *)
                    &DoubleMassSpringDamper_M->intgData);
  rtsiSetSolverName(&DoubleMassSpringDamper_M->solverInfo,"ode3");
  rtmSetTPtr(DoubleMassSpringDamper_M, &DoubleMassSpringDamper_M->Timing.tArray
             [0]);
  DoubleMassSpringDamper_M->Timing.stepSize0 = 0.2;
  (void) memset(((void *) &DoubleMassSpringDamper_B), 0,
                sizeof(B_DoubleMassSpringDamper_T));

  {
    (void) memset((void *)&DoubleMassSpringDamper_X, 0,
                  sizeof(X_DoubleMassSpringDamper_T));
  }

  {
    (void) memset((void *)&DoubleMassSpringDamper_XDis, 0,
                  sizeof(XDis_DoubleMassSpringDamper_T));
  }

  (void)memset(&DoubleMassSpringDamper_Y, 0, sizeof
               (ExtY_DoubleMassSpringDamper_T));
  return DoubleMassSpringDamper_M;
}
