#ifndef DoubleMassSpringDamper_private_h_
#define DoubleMassSpringDamper_private_h_
#include "rtwtypes.h"
#include "multiword_types.h"
#include "DoubleMassSpringDamper_types.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#ifndef rtmIsMajorTimeStep
#define rtmIsMajorTimeStep(rtm)        (((rtm)->Timing.simTimeStep) == MAJOR_TIME_STEP)
#endif

#ifndef rtmIsMinorTimeStep
#define rtmIsMinorTimeStep(rtm)        (((rtm)->Timing.simTimeStep) == MINOR_TIME_STEP)
#endif

#ifndef rtmSetTPtr
#define rtmSetTPtr(rtm, val)           ((rtm)->Timing.t = (val))
#endif

extern void DoubleMassSpringDamper_derivatives(void);

#endif
