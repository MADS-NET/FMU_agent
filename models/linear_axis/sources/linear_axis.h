#ifndef linear_axis_h_
#define linear_axis_h_
#ifndef linear_axis_COMMON_INCLUDES_
#define linear_axis_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "solver_zc.h"
#include "math.h"
#endif

#include "linear_axis_types.h"
#include <string.h>
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
  real_T s_dot;
  real_T omega;
  real_T maxT;
  real_T FilterCoefficient;
  real_T Sum;
  real_T Gain5;
  real_T minT;
  real_T IntegralGain;
  real_T s_ddot;
} B_linear_axis_T;

typedef struct {
  int_T Abs_MODE;
  int_T Saturation_MODE;
  boolean_T LowerRelop1_Mode;
  boolean_T UpperRelop_Mode;
} DW_linear_axis_T;

typedef struct {
  real_T Integrator2_CSTATE;
  real_T Integrator3_CSTATE;
  real_T Integrator_CSTATE;
  real_T Filter_CSTATE;
} X_linear_axis_T;

typedef struct {
  real_T Integrator2_CSTATE;
  real_T Integrator3_CSTATE;
  real_T Integrator_CSTATE;
  real_T Filter_CSTATE;
} XDot_linear_axis_T;

typedef struct {
  boolean_T Integrator2_CSTATE;
  boolean_T Integrator3_CSTATE;
  boolean_T Integrator_CSTATE;
  boolean_T Filter_CSTATE;
} XDis_linear_axis_T;

typedef struct {
  real_T Abs_AbsZc_ZC;
  real_T Saturation_UprLim_ZC;
  real_T Saturation_LwrLim_ZC;
  real_T LowerRelop1_RelopInput_ZC;
  real_T UpperRelop_RelopInput_ZC;
} ZCV_linear_axis_T;

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
  real_T setpoint;
} ExtU_linear_axis_T;

typedef struct {
  real_T v;
  real_T s;
  real_T T;
} ExtY_linear_axis_T;

struct P_linear_axis_T_ {
  real_T R[3];
  real_T g_gb[3];
  real_T mass[3];
  real_T r_v[3];
  real_T PIDController_D;
  real_T PIDController_I;
  real_T PIDController_InitialConditionForFilter;
  real_T PIDController_InitialConditionForIntegrator;
  real_T PIDController_N;
  real_T PIDController_P;
  real_T Integrator2_IC;
  real_T Integrator3_IC;
  real_T Gain_Gain;
  real_T uDLookupTable_tableData[7];
  real_T uDLookupTable_bp01Data[7];
  real_T Saturation_UpperSat;
  real_T Saturation_LowerSat;
  real_T Gain5_Gain;
  real_T Gain2_Gain;
};

struct tag_RTM_linear_axis_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;
  X_linear_axis_T *contStates;
  int_T *periodicContStateIndices;
  real_T *periodicContStateRanges;
  real_T *derivs;
  XDis_linear_axis_T *contStateDisabled;
  boolean_T zCCacheNeedsReset;
  boolean_T derivCacheNeedsReset;
  boolean_T CTOutputIncnstWithState;
  real_T odeY[4];
  real_T odeF[3][4];
  ODE3_IntgData intgData;
  real_T *zcSignalVector;
  uint8_T zcSignalAttributes[5];
  ZcDetectorData zcDetectorData;
  real_T zcVL[5];
  real_T zcVR[5];
  real_T zcVZ[5];
  real_T zct0;
  real_T zctR;
  real_T zcx0[4];
  real_T zcxR[4];
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

extern P_linear_axis_T linear_axis_P;
extern B_linear_axis_T linear_axis_B;
extern X_linear_axis_T linear_axis_X;
extern XDis_linear_axis_T linear_axis_XDis;
extern DW_linear_axis_T linear_axis_DW;
extern ExtU_linear_axis_T linear_axis_U;
extern ExtY_linear_axis_T linear_axis_Y;
extern RT_MODEL_linear_axis_T *linear_axis(void);
extern void linear_axis_initialize(void);
extern void linear_axis_output(void);
extern void linear_axis_update(void);
extern void linear_axis_derivatives(void);
extern void linear_axis_terminate(void);
extern RT_MODEL_linear_axis_T *const linear_axis_M;

#endif
