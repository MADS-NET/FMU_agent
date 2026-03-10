#ifndef machine_tool_h_
#define machine_tool_h_
#ifndef machine_tool_COMMON_INCLUDES_
#define machine_tool_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "solver_zc.h"
#include "math.h"
#endif

#include "machine_tool_types.h"
#include <string.h>
#include "zero_crossing_types.h"
#ifndef rtmGetContStateDisabled
#define rtmGetContStateDisabled(rtm)   ((rtm)->contStateDisabled)
#endif

#ifndef rtmSetContStateDisabled
#define rtmSetContStateDisabled(rtm, val) ((rtm)->contStateDisabled = (val))
#endif

#ifndef rtmGetContStates
#define rtmGetContStates(rtm)          ((rtm)->contStates)
#endif

#ifndef rtmSetContStates
#define rtmSetContStates(rtm, val)     ((rtm)->contStates = (val))
#endif

#ifndef rtmGetContTimeOutputInconsistentWithStateAtMajorStepFlag
#define rtmGetContTimeOutputInconsistentWithStateAtMajorStepFlag(rtm) ((rtm)->CTOutputIncnstWithState)
#endif

#ifndef rtmSetContTimeOutputInconsistentWithStateAtMajorStepFlag
#define rtmSetContTimeOutputInconsistentWithStateAtMajorStepFlag(rtm, val) ((rtm)->CTOutputIncnstWithState = (val))
#endif

#ifndef rtmGetDerivCacheNeedsReset
#define rtmGetDerivCacheNeedsReset(rtm) ((rtm)->derivCacheNeedsReset)
#endif

#ifndef rtmSetDerivCacheNeedsReset
#define rtmSetDerivCacheNeedsReset(rtm, val) ((rtm)->derivCacheNeedsReset = (val))
#endif

#ifndef rtmGetIntgData
#define rtmGetIntgData(rtm)            ((rtm)->intgData)
#endif

#ifndef rtmSetIntgData
#define rtmSetIntgData(rtm, val)       ((rtm)->intgData = (val))
#endif

#ifndef rtmGetOdeF
#define rtmGetOdeF(rtm)                ((rtm)->odeF)
#endif

#ifndef rtmSetOdeF
#define rtmSetOdeF(rtm, val)           ((rtm)->odeF = (val))
#endif

#ifndef rtmGetOdeY
#define rtmGetOdeY(rtm)                ((rtm)->odeY)
#endif

#ifndef rtmSetOdeY
#define rtmSetOdeY(rtm, val)           ((rtm)->odeY = (val))
#endif

#ifndef rtmGetPeriodicContStateIndices
#define rtmGetPeriodicContStateIndices(rtm) ((rtm)->periodicContStateIndices)
#endif

#ifndef rtmSetPeriodicContStateIndices
#define rtmSetPeriodicContStateIndices(rtm, val) ((rtm)->periodicContStateIndices = (val))
#endif

#ifndef rtmGetPeriodicContStateRanges
#define rtmGetPeriodicContStateRanges(rtm) ((rtm)->periodicContStateRanges)
#endif

#ifndef rtmSetPeriodicContStateRanges
#define rtmSetPeriodicContStateRanges(rtm, val) ((rtm)->periodicContStateRanges = (val))
#endif

#ifndef rtmGetSolverZcSignalVector
#define rtmGetSolverZcSignalVector(rtm) ((rtm)->zcSignalVector)
#endif

#ifndef rtmSetSolverZcSignalVector
#define rtmSetSolverZcSignalVector(rtm, val) ((rtm)->zcSignalVector = (val))
#endif

#ifndef rtmGetZCCacheNeedsReset
#define rtmGetZCCacheNeedsReset(rtm)   ((rtm)->zCCacheNeedsReset)
#endif

#ifndef rtmSetZCCacheNeedsReset
#define rtmSetZCCacheNeedsReset(rtm, val) ((rtm)->zCCacheNeedsReset = (val))
#endif

#ifndef rtmGetZCVL
#define rtmGetZCVL(rtm)                ((rtm)->zcVL)
#endif

#ifndef rtmSetZCVL
#define rtmSetZCVL(rtm, val)           ((rtm)->zcVL = (val))
#endif

#ifndef rtmGetZCVR
#define rtmGetZCVR(rtm)                ((rtm)->zcVR)
#endif

#ifndef rtmSetZCVR
#define rtmSetZCVR(rtm, val)           ((rtm)->zcVR = (val))
#endif

#ifndef rtmGetZCVZ
#define rtmGetZCVZ(rtm)                ((rtm)->zcVZ)
#endif

#ifndef rtmSetZCVZ
#define rtmSetZCVZ(rtm, val)           ((rtm)->zcVZ = (val))
#endif

#ifndef rtmGetZCt0
#define rtmGetZCt0(rtm)                ((rtm)->zct0)
#endif

#ifndef rtmSetZCt0
#define rtmSetZCt0(rtm, val)           ((rtm)->zct0 = (val))
#endif

#ifndef rtmGetZCtR
#define rtmGetZCtR(rtm)                ((rtm)->zctR)
#endif

#ifndef rtmSetZCtR
#define rtmSetZCtR(rtm, val)           ((rtm)->zctR = (val))
#endif

#ifndef rtmGetZCx0
#define rtmGetZCx0(rtm)                ((rtm)->zcx0)
#endif

#ifndef rtmSetZCx0
#define rtmSetZCx0(rtm, val)           ((rtm)->zcx0 = (val))
#endif

#ifndef rtmGetZCxR
#define rtmGetZCxR(rtm)                ((rtm)->zcxR)
#endif

#ifndef rtmSetZCxR
#define rtmSetZCxR(rtm, val)           ((rtm)->zcxR = (val))
#endif

#ifndef rtmGetZcDetectorData
#define rtmGetZcDetectorData(rtm)      ((rtm)->zcDetectorData)
#endif

#ifndef rtmSetZcDetectorData
#define rtmSetZcDetectorData(rtm, val) ((rtm)->zcDetectorData = (val))
#endif

#ifndef rtmGetZcSignalAttibutes
#define rtmGetZcSignalAttibutes(rtm)   ((rtm)->zcSignalAttributes)
#endif

#ifndef rtmSetZcSignalAttibutes
#define rtmSetZcSignalAttibutes(rtm, val) ((rtm)->zcSignalAttributes = (val))
#endif

#ifndef rtmGetdX
#define rtmGetdX(rtm)                  ((rtm)->derivs)
#endif

#ifndef rtmSetdX
#define rtmSetdX(rtm, val)             ((rtm)->derivs = (val))
#endif

#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

#ifndef rtmGetStopRequested
#define rtmGetStopRequested(rtm)       ((rtm)->Timing.stopRequestedFlag)
#endif

#ifndef rtmSetStopRequested
#define rtmSetStopRequested(rtm, val)  ((rtm)->Timing.stopRequestedFlag = (val))
#endif

#ifndef rtmGetStopRequestedPtr
#define rtmGetStopRequestedPtr(rtm)    (&((rtm)->Timing.stopRequestedFlag))
#endif

#ifndef rtmGetT
#define rtmGetT(rtm)                   (rtmGetTPtr((rtm))[0])
#endif

#ifndef rtmGetTPtr
#define rtmGetTPtr(rtm)                ((rtm)->Timing.t)
#endif

#ifndef rtmGetTStart
#define rtmGetTStart(rtm)              ((rtm)->Timing.tStart)
#endif

typedef struct {
  real_T v;
  real_T v_p;
  real_T v_k;
  real_T FilterCoefficient;
  real_T Sum;
  real_T ZeroGain;
  real_T DeadZone;
  real_T IntegralGain;
  real_T Switch;
  real_T a;
  real_T FilterCoefficient_n;
  real_T Sum_b;
  real_T ZeroGain_a;
  real_T DeadZone_n;
  real_T IntegralGain_n;
  real_T Switch_i;
  real_T a_a;
  real_T FilterCoefficient_p;
  real_T Sum_d;
  real_T ZeroGain_o;
  real_T DeadZone_g;
  real_T IntegralGain_m;
  real_T Switch_g;
  real_T a_j;
  boolean_T AND3;
  boolean_T Memory;
  boolean_T AND3_b;
  boolean_T Memory_a;
  boolean_T AND3_d;
  boolean_T Memory_f;
} B_machine_tool_T;

typedef struct {
  int_T DeadZone_MODE;
  int_T SignPreSat_MODE;
  int_T SignPreIntegrator_MODE;
  int_T Saturation_MODE;
  int_T DeadZone_MODE_o;
  int_T SignPreSat_MODE_n;
  int_T SignPreIntegrator_MODE_h;
  int_T Saturation_MODE_a;
  int_T DeadZone_MODE_a;
  int_T SignPreSat_MODE_a;
  int_T SignPreIntegrator_MODE_hi;
  int_T Saturation_MODE_d;
  boolean_T NotEqual_Mode;
  boolean_T Memory_PreviousInput;
  boolean_T NotEqual_Mode_c;
  boolean_T Memory_PreviousInput_j;
  boolean_T NotEqual_Mode_p;
  boolean_T Memory_PreviousInput_a;
} DW_machine_tool_T;

typedef struct {
  real_T position;
  real_T position_i;
  real_T position_c;
  real_T speed;
  real_T speed_i;
  real_T speed_h;
  real_T Integrator_CSTATE;
  real_T Filter_CSTATE;
  real_T Integrator_CSTATE_d;
  real_T Filter_CSTATE_p;
  real_T Integrator_CSTATE_i;
  real_T Filter_CSTATE_o;
} X_machine_tool_T;

typedef struct {
  real_T position;
  real_T position_i;
  real_T position_c;
  real_T speed;
  real_T speed_i;
  real_T speed_h;
  real_T Integrator_CSTATE;
  real_T Filter_CSTATE;
  real_T Integrator_CSTATE_d;
  real_T Filter_CSTATE_p;
  real_T Integrator_CSTATE_i;
  real_T Filter_CSTATE_o;
} XDot_machine_tool_T;

typedef struct {
  boolean_T position;
  boolean_T position_i;
  boolean_T position_c;
  boolean_T speed;
  boolean_T speed_i;
  boolean_T speed_h;
  boolean_T Integrator_CSTATE;
  boolean_T Filter_CSTATE;
  boolean_T Integrator_CSTATE_d;
  boolean_T Filter_CSTATE_p;
  boolean_T Integrator_CSTATE_i;
  boolean_T Filter_CSTATE_o;
} XDis_machine_tool_T;

typedef struct {
  real_T Integrator1_Reset_ZC;
  real_T Integrator1_Reset_ZC_f;
  real_T Integrator1_Reset_ZC_k;
  real_T Integrator_Reset_ZC;
  real_T Integrator_Reset_ZC_p;
  real_T Integrator_Reset_ZC_d;
  real_T Integrator_Reset_ZC_f;
  real_T Filter_Reset_ZC;
  real_T DeadZone_LwrReg_ZC;
  real_T DeadZone_UprReg_ZC;
  real_T NotEqual_RelopInput_ZC;
  real_T SignPreSat_Input_ZC;
  real_T SignPreIntegrator_Input_ZC;
  real_T Saturation_UprLim_ZC;
  real_T Saturation_LwrLim_ZC;
  real_T Integrator_Reset_ZC_n;
  real_T Filter_Reset_ZC_p;
  real_T DeadZone_LwrReg_ZC_h;
  real_T DeadZone_UprReg_ZC_m;
  real_T NotEqual_RelopInput_ZC_p;
  real_T SignPreSat_Input_ZC_g;
  real_T SignPreIntegrator_Input_ZC_g;
  real_T Saturation_UprLim_ZC_f;
  real_T Saturation_LwrLim_ZC_a;
  real_T Integrator_Reset_ZC_h;
  real_T Filter_Reset_ZC_n;
  real_T DeadZone_LwrReg_ZC_b;
  real_T DeadZone_UprReg_ZC_n;
  real_T NotEqual_RelopInput_ZC_i;
  real_T SignPreSat_Input_ZC_n;
  real_T SignPreIntegrator_Input_ZC_p;
  real_T Saturation_UprLim_ZC_k;
  real_T Saturation_LwrLim_ZC_e;
} ZCV_machine_tool_T;

typedef struct {
  ZCSigState Integrator1_Reset_ZCE;
  ZCSigState Integrator1_Reset_ZCE_k;
  ZCSigState Integrator1_Reset_ZCE_a;
  ZCSigState Integrator_Reset_ZCE;
  ZCSigState Integrator_Reset_ZCE_o;
  ZCSigState Integrator_Reset_ZCE_i;
  ZCSigState Integrator_Reset_ZCE_it;
  ZCSigState Filter_Reset_ZCE;
  ZCSigState Integrator_Reset_ZCE_h;
  ZCSigState Filter_Reset_ZCE_l;
  ZCSigState Integrator_Reset_ZCE_ic;
  ZCSigState Filter_Reset_ZCE_j;
} PrevZCX_machine_tool_T;

#ifndef ODE3_INTG
#define ODE3_INTG

typedef struct {
  real_T *y;
  real_T *f[3];
} ODE3_IntgData;

#endif

typedef struct {
  real_T *vL;
  real_T *vR;
  real_T *vZ;
  real_T t0;
  real_T tR;
  real_T *x0;
  real_T *xR;
} ZcDetectorData;

typedef struct {
  real_T setpoint[3];
  real_T noise[3];
  boolean_T reset;
} ExtU_machine_tool_T;

typedef struct {
  real_T position[3];
  real_T speed[3];
} ExtY_machine_tool_T;

struct P_machine_tool_T_ {
  real_T X0;
  real_T XD;
  real_T XI;
  real_T XN;
  real_T XP;
  real_T Xdc;
  real_T Xm;
  real_T Xsat;
  real_T Y0;
  real_T YD;
  real_T YI;
  real_T YN;
  real_T YP;
  real_T Ydc;
  real_T Ym;
  real_T Ysat;
  real_T Z0;
  real_T ZD;
  real_T ZI;
  real_T ZN;
  real_T ZP;
  real_T Zdc;
  real_T Zm;
  real_T Zsat;
  real_T res;
  real_T PIDController_InitialConditionForFilter;
  real_T PIDController_InitialConditionForFilter_c;
  real_T PIDController_InitialConditionForFilter_n;
  real_T PIDController_InitialConditionForIntegrator;
  real_T PIDController_InitialConditionForIntegrator_n;
  real_T PIDController_InitialConditionForIntegrator_h;
  real_T Xaxis_v0;
  real_T Yaxis_v0;
  real_T Zaxis_v0;
  real_T Constant1_Value;
  real_T Constant1_Value_g;
  real_T Constant1_Value_c;
  real_T Gain1_Gain;
  real_T Gain2_Gain;
  real_T Gain9_Gain;
  real_T ZeroGain_Gain;
  real_T ZeroGain_Gain_m;
  real_T ZeroGain_Gain_n;
  boolean_T Memory_InitialCondition;
  boolean_T Memory_InitialCondition_l;
  boolean_T Memory_InitialCondition_i;
};

struct tag_RTM_machine_tool_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;
  X_machine_tool_T *contStates;
  int_T *periodicContStateIndices;
  real_T *periodicContStateRanges;
  real_T *derivs;
  XDis_machine_tool_T *contStateDisabled;
  boolean_T zCCacheNeedsReset;
  boolean_T derivCacheNeedsReset;
  boolean_T CTOutputIncnstWithState;
  real_T odeY[12];
  real_T odeF[3][12];
  ODE3_IntgData intgData;
  real_T *zcSignalVector;
  uint8_T zcSignalAttributes[33];
  ZcDetectorData zcDetectorData;
  real_T zcVL[33];
  real_T zcVR[33];
  real_T zcVZ[33];
  real_T zct0;
  real_T zctR;
  real_T zcx0[12];
  real_T zcxR[12];
  struct {
    int_T numContStates;
    int_T numPeriodicContStates;
    int_T numSampTimes;
  } Sizes;

  struct {
    uint32_T clockTick0;
    uint32_T clockTickH0;
    time_T stepSize0;
    uint32_T clockTick1;
    uint32_T clockTickH1;
    time_T tStart;
    SimTimeStep simTimeStep;
    boolean_T stopRequestedFlag;
    time_T *t;
    time_T tArray[2];
  } Timing;
};

extern P_machine_tool_T machine_tool_P;
extern B_machine_tool_T machine_tool_B;
extern X_machine_tool_T machine_tool_X;
extern XDis_machine_tool_T machine_tool_XDis;
extern DW_machine_tool_T machine_tool_DW;
extern PrevZCX_machine_tool_T machine_tool_PrevZCX;
extern ExtU_machine_tool_T machine_tool_U;
extern ExtY_machine_tool_T machine_tool_Y;
extern RT_MODEL_machine_tool_T *machine_tool(void);
extern void machine_tool_initialize(void);
extern void machine_tool_output(void);
extern void machine_tool_update(void);
extern void machine_tool_derivatives(void);
extern void machine_tool_terminate(void);
extern RT_MODEL_machine_tool_T *const machine_tool_M;

#endif
