#ifndef linear_axis_private_h_
#define linear_axis_private_h_
#include "rtwtypes.h"
#include "multiword_types.h"
#include "linear_axis_types.h"
#include "linear_axis.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "solver_zc.h"
#ifndef rtmIsMajorTimeStep
#define rtmIsMajorTimeStep(rtm)        (((rtm)->Timing.simTimeStep) == MAJOR_TIME_STEP)
#endif

#ifndef rtmIsMinorTimeStep
#define rtmIsMinorTimeStep(rtm)        (((rtm)->Timing.simTimeStep) == MINOR_TIME_STEP)
#endif

#ifndef rtmSetTPtr
#define rtmSetTPtr(rtm, val)           ((rtm)->Timing.t = (val))
#endif

extern real_T look1_binlxpw(real_T u0, const real_T bp0[], const real_T table[],
  uint32_T maxIndex);
extern void linear_axis_derivatives(void);
extern void linear_axis_zeroCrossings(void);

#endif
