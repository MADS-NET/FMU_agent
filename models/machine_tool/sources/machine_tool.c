#include "machine_tool_macros.h"
#include "machine_tool.h"
#include "machine_tool_private.h"
#include "rtwtypes.h"
#include <string.h>
#include "zero_crossing_types.h"
#include <math.h>

B_machine_tool_T machine_tool_B;
X_machine_tool_T machine_tool_X;
XDis_machine_tool_T machine_tool_XDis;
DW_machine_tool_T machine_tool_DW;
PrevZCX_machine_tool_T machine_tool_PrevZCX;
ExtU_machine_tool_T machine_tool_U;
ExtY_machine_tool_T machine_tool_Y;
static RT_MODEL_machine_tool_T machine_tool_M_;
RT_MODEL_machine_tool_T *const machine_tool_M = &machine_tool_M_;
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

void machine_tool_output(void)
{
  real_T rtb_Integrator;
  boolean_T didZcEventOccur;
  boolean_T tmp;
  boolean_T tmp_0;
  tmp = rtsiIsModeUpdateTimeStep(&machine_tool_M->solverInfo);
  if (tmp) {
    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Integrator1_Reset_ZCE != POS_ZCSIG));
    machine_tool_PrevZCX.Integrator1_Reset_ZCE = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.position = machine_tool_P.X0;
    }

    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Integrator1_Reset_ZCE_k !=
                        POS_ZCSIG));
    machine_tool_PrevZCX.Integrator1_Reset_ZCE_k = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.position_i = machine_tool_P.Y0;
    }

    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Integrator1_Reset_ZCE_a !=
                        POS_ZCSIG));
    machine_tool_PrevZCX.Integrator1_Reset_ZCE_a = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.position_c = machine_tool_P.Z0;
    }

    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Integrator_Reset_ZCE != POS_ZCSIG));
    machine_tool_PrevZCX.Integrator_Reset_ZCE = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.speed = machine_tool_P.Xaxis_v0;
    }

    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Integrator_Reset_ZCE_o != POS_ZCSIG));
    machine_tool_PrevZCX.Integrator_Reset_ZCE_o = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.speed_i = machine_tool_P.Yaxis_v0;
    }

    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Integrator_Reset_ZCE_i != POS_ZCSIG));
    machine_tool_PrevZCX.Integrator_Reset_ZCE_i = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.speed_h = machine_tool_P.Zaxis_v0;
    }

    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Filter_Reset_ZCE != POS_ZCSIG));
    machine_tool_PrevZCX.Filter_Reset_ZCE = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.Filter_CSTATE =
        machine_tool_P.PIDController_InitialConditionForFilter;
    }

    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Integrator_Reset_ZCE_it !=
                        POS_ZCSIG));
    machine_tool_PrevZCX.Integrator_Reset_ZCE_it = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.Integrator_CSTATE =
        machine_tool_P.PIDController_InitialConditionForIntegrator;
    }

    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Integrator_Reset_ZCE_g != POS_ZCSIG));
    machine_tool_PrevZCX.Integrator_Reset_ZCE_g = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.Integrator_CSTATE_g =
        machine_tool_P.PIDController1_InitialConditionForIntegrator;
    }

    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Filter_Reset_ZCE_m != POS_ZCSIG));
    machine_tool_PrevZCX.Filter_Reset_ZCE_m = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.Filter_CSTATE_o =
        machine_tool_P.PIDController1_InitialConditionForFilter;
    }
  }

  machine_tool_Y.position[0] = machine_tool_P.mtomm_Gain *
    machine_tool_X.position;
  machine_tool_Y.position[1] = machine_tool_P.mtomm_Gain *
    machine_tool_X.position_i;
  machine_tool_Y.position[2] = machine_tool_P.mtomm_Gain *
    machine_tool_X.position_c;
  machine_tool_B.v = machine_tool_X.speed;
  machine_tool_B.v_p = machine_tool_X.speed_i;
  machine_tool_B.v_k = machine_tool_X.speed_h;
  machine_tool_Y.speed[0] = machine_tool_P.mstommmin_Gain * machine_tool_B.v;
  machine_tool_Y.speed[1] = machine_tool_P.mstommmin_Gain * machine_tool_B.v_p;
  machine_tool_Y.speed[2] = machine_tool_P.mstommmin_Gain * machine_tool_B.v_k;
  rtb_Integrator = machine_tool_P.mmtom_Gain * machine_tool_U.setpoint[0] -
    rt_roundd_snf(machine_tool_X.position / machine_tool_P.res) *
    machine_tool_P.res;
  machine_tool_B.IntegralGain = machine_tool_P.XI_pos * rtb_Integrator;
  machine_tool_B.FilterCoefficient = (machine_tool_P.XD_pos * rtb_Integrator -
    machine_tool_X.Filter_CSTATE) * machine_tool_P.XN_pos;
  rtb_Integrator = ((machine_tool_P.XP_pos * rtb_Integrator +
                     machine_tool_X.Integrator_CSTATE) +
                    machine_tool_B.FilterCoefficient) - rt_roundd_snf
    (machine_tool_B.v / machine_tool_P.res) * machine_tool_P.res;
  machine_tool_B.FilterCoefficient_p = (machine_tool_P.XD_vel * rtb_Integrator -
    machine_tool_X.Filter_CSTATE_o) * machine_tool_P.XN_vel;
  machine_tool_B.Sum = (machine_tool_P.XP_vel * rtb_Integrator +
                        machine_tool_X.Integrator_CSTATE_g) +
    machine_tool_B.FilterCoefficient_p;
  machine_tool_B.ZeroGain = machine_tool_P.ZeroGain_Gain * machine_tool_B.Sum;
  if (tmp) {
    if (machine_tool_B.Sum > machine_tool_P.Xsat) {
      machine_tool_DW.DeadZone_MODE = 1;
    } else if (machine_tool_B.Sum >= -machine_tool_P.Xsat) {
      machine_tool_DW.DeadZone_MODE = 0;
    } else {
      machine_tool_DW.DeadZone_MODE = -1;
    }
  }

  if (machine_tool_DW.DeadZone_MODE == 1) {
    machine_tool_B.DeadZone = machine_tool_B.Sum - machine_tool_P.Xsat;
  } else if (machine_tool_DW.DeadZone_MODE == -1) {
    machine_tool_B.DeadZone = machine_tool_B.Sum - (-machine_tool_P.Xsat);
  } else {
    machine_tool_B.DeadZone = 0.0;
  }

  machine_tool_B.IntegralGain_l = machine_tool_P.XI_vel * rtb_Integrator;
  if (tmp) {
    machine_tool_DW.NotEqual_Mode = (machine_tool_B.ZeroGain !=
      machine_tool_B.DeadZone);
    if (machine_tool_B.DeadZone > 0.0) {
      machine_tool_DW.SignPreSat_MODE = 1;
    } else if (machine_tool_B.DeadZone < 0.0) {
      machine_tool_DW.SignPreSat_MODE = -1;
    } else {
      machine_tool_DW.SignPreSat_MODE = 0;
    }

    if (machine_tool_B.IntegralGain_l > 0.0) {
      machine_tool_DW.SignPreIntegrator_MODE = 1;
    } else if (machine_tool_B.IntegralGain_l < 0.0) {
      machine_tool_DW.SignPreIntegrator_MODE = -1;
    } else {
      machine_tool_DW.SignPreIntegrator_MODE = 0;
    }
  }

  machine_tool_B.AND3 = (machine_tool_DW.NotEqual_Mode && ((int8_T)
    machine_tool_DW.SignPreSat_MODE == (int8_T)
    machine_tool_DW.SignPreIntegrator_MODE));
  tmp_0 = rtmIsMajorTimeStep(machine_tool_M);
  if (tmp_0) {
    machine_tool_B.Memory = machine_tool_DW.Memory_PreviousInput;
  }

  if (machine_tool_B.Memory) {
    machine_tool_B.Switch = machine_tool_P.Constant1_Value;
  } else {
    machine_tool_B.Switch = machine_tool_B.IntegralGain_l;
  }

  if (tmp) {
    if (machine_tool_B.Sum >= machine_tool_P.Xsat) {
      machine_tool_DW.Saturation_MODE = 1;
    } else if (machine_tool_B.Sum > -machine_tool_P.Xsat) {
      machine_tool_DW.Saturation_MODE = 0;
    } else {
      machine_tool_DW.Saturation_MODE = -1;
    }
  }

  if (machine_tool_DW.Saturation_MODE == 1) {
    rtb_Integrator = machine_tool_P.Xsat;
  } else if (machine_tool_DW.Saturation_MODE == -1) {
    rtb_Integrator = -machine_tool_P.Xsat;
  } else {
    rtb_Integrator = machine_tool_B.Sum;
  }

  machine_tool_B.a = ((rtb_Integrator + machine_tool_U.noise[0]) -
                      machine_tool_P.Xdc * machine_tool_B.v) * (1.0 /
    machine_tool_P.Xm);
  rtb_Integrator = machine_tool_P.mmtom_Gain * machine_tool_U.setpoint[1] -
    rt_roundd_snf(machine_tool_X.position_i / machine_tool_P.res) *
    machine_tool_P.res;
  if (tmp) {
    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Filter_Reset_ZCE_l != POS_ZCSIG));
    machine_tool_PrevZCX.Filter_Reset_ZCE_l = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.Filter_CSTATE_p =
        machine_tool_P.PIDController_InitialConditionForFilter_c;
    }

    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Integrator_Reset_ZCE_h != POS_ZCSIG));
    machine_tool_PrevZCX.Integrator_Reset_ZCE_h = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.Integrator_CSTATE_d =
        machine_tool_P.PIDController_InitialConditionForIntegrator_n;
    }

    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Integrator_Reset_ZCE_gd !=
                        POS_ZCSIG));
    machine_tool_PrevZCX.Integrator_Reset_ZCE_gd = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.Integrator_CSTATE_e =
        machine_tool_P.PIDController1_InitialConditionForIntegrator_h;
    }

    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Filter_Reset_ZCE_c != POS_ZCSIG));
    machine_tool_PrevZCX.Filter_Reset_ZCE_c = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.Filter_CSTATE_m =
        machine_tool_P.PIDController1_InitialConditionForFilter_p;
    }
  }

  machine_tool_B.IntegralGain_n = machine_tool_P.YI_pos * rtb_Integrator;
  machine_tool_B.FilterCoefficient_n = (machine_tool_P.YD_pos * rtb_Integrator -
    machine_tool_X.Filter_CSTATE_p) * machine_tool_P.YN_pos;
  rtb_Integrator = ((machine_tool_P.YP_pos * rtb_Integrator +
                     machine_tool_X.Integrator_CSTATE_d) +
                    machine_tool_B.FilterCoefficient_n) - rt_roundd_snf
    (machine_tool_B.v_p / machine_tool_P.res) * machine_tool_P.res;
  machine_tool_B.FilterCoefficient_d = (machine_tool_P.YD_vel * rtb_Integrator -
    machine_tool_X.Filter_CSTATE_m) * machine_tool_P.YN_vel;
  machine_tool_B.Sum_n = (machine_tool_P.YP_vel * rtb_Integrator +
    machine_tool_X.Integrator_CSTATE_e) + machine_tool_B.FilterCoefficient_d;
  machine_tool_B.ZeroGain_n = machine_tool_P.ZeroGain_Gain_i *
    machine_tool_B.Sum_n;
  if (tmp) {
    if (machine_tool_B.Sum_n > machine_tool_P.Ysat) {
      machine_tool_DW.DeadZone_MODE_b = 1;
    } else if (machine_tool_B.Sum_n >= -machine_tool_P.Ysat) {
      machine_tool_DW.DeadZone_MODE_b = 0;
    } else {
      machine_tool_DW.DeadZone_MODE_b = -1;
    }
  }

  if (machine_tool_DW.DeadZone_MODE_b == 1) {
    machine_tool_B.DeadZone_i = machine_tool_B.Sum_n - machine_tool_P.Ysat;
  } else if (machine_tool_DW.DeadZone_MODE_b == -1) {
    machine_tool_B.DeadZone_i = machine_tool_B.Sum_n - (-machine_tool_P.Ysat);
  } else {
    machine_tool_B.DeadZone_i = 0.0;
  }

  machine_tool_B.IntegralGain_c = machine_tool_P.YI_vel * rtb_Integrator;
  if (tmp) {
    machine_tool_DW.NotEqual_Mode_g = (machine_tool_B.ZeroGain_n !=
      machine_tool_B.DeadZone_i);
    if (machine_tool_B.DeadZone_i > 0.0) {
      machine_tool_DW.SignPreSat_MODE_b = 1;
    } else if (machine_tool_B.DeadZone_i < 0.0) {
      machine_tool_DW.SignPreSat_MODE_b = -1;
    } else {
      machine_tool_DW.SignPreSat_MODE_b = 0;
    }

    if (machine_tool_B.IntegralGain_c > 0.0) {
      machine_tool_DW.SignPreIntegrator_MODE_g = 1;
    } else if (machine_tool_B.IntegralGain_c < 0.0) {
      machine_tool_DW.SignPreIntegrator_MODE_g = -1;
    } else {
      machine_tool_DW.SignPreIntegrator_MODE_g = 0;
    }
  }

  machine_tool_B.AND3_o = (machine_tool_DW.NotEqual_Mode_g && ((int8_T)
    machine_tool_DW.SignPreSat_MODE_b == (int8_T)
    machine_tool_DW.SignPreIntegrator_MODE_g));
  if (tmp_0) {
    machine_tool_B.Memory_p = machine_tool_DW.Memory_PreviousInput_e;
  }

  if (machine_tool_B.Memory_p) {
    machine_tool_B.Switch_g = machine_tool_P.Constant1_Value_a;
  } else {
    machine_tool_B.Switch_g = machine_tool_B.IntegralGain_c;
  }

  if (tmp) {
    if (machine_tool_B.Sum_n >= machine_tool_P.Ysat) {
      machine_tool_DW.Saturation_MODE_p = 1;
    } else if (machine_tool_B.Sum_n > -machine_tool_P.Ysat) {
      machine_tool_DW.Saturation_MODE_p = 0;
    } else {
      machine_tool_DW.Saturation_MODE_p = -1;
    }
  }

  if (machine_tool_DW.Saturation_MODE_p == 1) {
    rtb_Integrator = machine_tool_P.Ysat;
  } else if (machine_tool_DW.Saturation_MODE_p == -1) {
    rtb_Integrator = -machine_tool_P.Ysat;
  } else {
    rtb_Integrator = machine_tool_B.Sum_n;
  }

  machine_tool_B.a_a = ((rtb_Integrator + machine_tool_U.noise[1]) -
                        machine_tool_P.Ydc * machine_tool_B.v_p) * (1.0 /
    machine_tool_P.Ym);
  rtb_Integrator = machine_tool_P.mmtom_Gain * machine_tool_U.setpoint[2] -
    rt_roundd_snf(machine_tool_X.position_c / machine_tool_P.res) *
    machine_tool_P.res;
  if (tmp) {
    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Filter_Reset_ZCE_j != POS_ZCSIG));
    machine_tool_PrevZCX.Filter_Reset_ZCE_j = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.Filter_CSTATE_oe =
        machine_tool_P.PIDController_InitialConditionForFilter_n;
    }

    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Integrator_Reset_ZCE_ic !=
                        POS_ZCSIG));
    machine_tool_PrevZCX.Integrator_Reset_ZCE_ic = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.Integrator_CSTATE_i =
        machine_tool_P.PIDController_InitialConditionForIntegrator_h;
    }

    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Integrator_Reset_ZCE_k != POS_ZCSIG));
    machine_tool_PrevZCX.Integrator_Reset_ZCE_k = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.Integrator_CSTATE_m =
        machine_tool_P.PIDController1_InitialConditionForIntegrator_m;
    }

    didZcEventOccur = (machine_tool_U.reset &&
                       (machine_tool_PrevZCX.Filter_Reset_ZCE_g != POS_ZCSIG));
    machine_tool_PrevZCX.Filter_Reset_ZCE_g = machine_tool_U.reset;
    if (didZcEventOccur) {
      machine_tool_X.Filter_CSTATE_e =
        machine_tool_P.PIDController1_InitialConditionForFilter_l;
    }
  }

  machine_tool_B.IntegralGain_m = machine_tool_P.ZI_pos * rtb_Integrator;
  machine_tool_B.FilterCoefficient_py = (machine_tool_P.ZD_pos * rtb_Integrator
    - machine_tool_X.Filter_CSTATE_oe) * machine_tool_P.ZN_pos;
  rtb_Integrator = ((machine_tool_P.ZP_pos * rtb_Integrator +
                     machine_tool_X.Integrator_CSTATE_i) +
                    machine_tool_B.FilterCoefficient_py) - rt_roundd_snf
    (machine_tool_B.v_k / machine_tool_P.res) * machine_tool_P.res;
  machine_tool_B.FilterCoefficient_g = (machine_tool_P.ZD_vel * rtb_Integrator -
    machine_tool_X.Filter_CSTATE_e) * machine_tool_P.ZN_vel;
  machine_tool_B.Sum_l = (machine_tool_P.ZP_vel * rtb_Integrator +
    machine_tool_X.Integrator_CSTATE_m) + machine_tool_B.FilterCoefficient_g;
  machine_tool_B.ZeroGain_i = machine_tool_P.ZeroGain_Gain_d *
    machine_tool_B.Sum_l;
  if (tmp) {
    if (machine_tool_B.Sum_l > machine_tool_P.Zsat) {
      machine_tool_DW.DeadZone_MODE_i = 1;
    } else if (machine_tool_B.Sum_l >= -machine_tool_P.Zsat) {
      machine_tool_DW.DeadZone_MODE_i = 0;
    } else {
      machine_tool_DW.DeadZone_MODE_i = -1;
    }
  }

  if (machine_tool_DW.DeadZone_MODE_i == 1) {
    machine_tool_B.DeadZone_g = machine_tool_B.Sum_l - machine_tool_P.Zsat;
  } else if (machine_tool_DW.DeadZone_MODE_i == -1) {
    machine_tool_B.DeadZone_g = machine_tool_B.Sum_l - (-machine_tool_P.Zsat);
  } else {
    machine_tool_B.DeadZone_g = 0.0;
  }

  machine_tool_B.IntegralGain_k = machine_tool_P.ZI_vel * rtb_Integrator;
  if (tmp) {
    machine_tool_DW.NotEqual_Mode_p = (machine_tool_B.ZeroGain_i !=
      machine_tool_B.DeadZone_g);
    if (machine_tool_B.DeadZone_g > 0.0) {
      machine_tool_DW.SignPreSat_MODE_h = 1;
    } else if (machine_tool_B.DeadZone_g < 0.0) {
      machine_tool_DW.SignPreSat_MODE_h = -1;
    } else {
      machine_tool_DW.SignPreSat_MODE_h = 0;
    }

    if (machine_tool_B.IntegralGain_k > 0.0) {
      machine_tool_DW.SignPreIntegrator_MODE_m = 1;
    } else if (machine_tool_B.IntegralGain_k < 0.0) {
      machine_tool_DW.SignPreIntegrator_MODE_m = -1;
    } else {
      machine_tool_DW.SignPreIntegrator_MODE_m = 0;
    }
  }

  machine_tool_B.AND3_c = (machine_tool_DW.NotEqual_Mode_p && ((int8_T)
    machine_tool_DW.SignPreSat_MODE_h == (int8_T)
    machine_tool_DW.SignPreIntegrator_MODE_m));
  if (tmp_0) {
    machine_tool_B.Memory_n = machine_tool_DW.Memory_PreviousInput_n;
  }

  if (machine_tool_B.Memory_n) {
    machine_tool_B.Switch_p = machine_tool_P.Constant1_Value_f;
  } else {
    machine_tool_B.Switch_p = machine_tool_B.IntegralGain_k;
  }

  if (tmp) {
    if (machine_tool_B.Sum_l >= machine_tool_P.Zsat) {
      machine_tool_DW.Saturation_MODE_k = 1;
    } else if (machine_tool_B.Sum_l > -machine_tool_P.Zsat) {
      machine_tool_DW.Saturation_MODE_k = 0;
    } else {
      machine_tool_DW.Saturation_MODE_k = -1;
    }
  }

  if (machine_tool_DW.Saturation_MODE_k == 1) {
    rtb_Integrator = machine_tool_P.Zsat;
  } else if (machine_tool_DW.Saturation_MODE_k == -1) {
    rtb_Integrator = -machine_tool_P.Zsat;
  } else {
    rtb_Integrator = machine_tool_B.Sum_l;
  }

  machine_tool_B.a_j = ((rtb_Integrator + machine_tool_U.noise[2]) -
                        machine_tool_P.Zdc * machine_tool_B.v_k) * (1.0 /
    machine_tool_P.Zm);
}

void machine_tool_update(void)
{
  if (rtmIsMajorTimeStep(machine_tool_M)) {
    machine_tool_DW.Memory_PreviousInput = machine_tool_B.AND3;
    machine_tool_DW.Memory_PreviousInput_e = machine_tool_B.AND3_o;
    machine_tool_DW.Memory_PreviousInput_n = machine_tool_B.AND3_c;
  }

  if (rtmIsMajorTimeStep(machine_tool_M)) {
    if (!(++machine_tool_M->Timing.clockTick0)) {
      ++machine_tool_M->Timing.clockTickH0;
    }

    machine_tool_M->Timing.t[0] = rtsiGetSolverStopTime
      (&machine_tool_M->solverInfo);

    {
      machine_tool_M->Timing.clockTick1++;
      if (!machine_tool_M->Timing.clockTick1) {
        machine_tool_M->Timing.clockTickH1++;
      }
    }
  }
}

void machine_tool_derivatives(void)
{
  XDot_machine_tool_T *_rtXdot;
  _rtXdot = ((XDot_machine_tool_T *) machine_tool_M->derivs);
  _rtXdot->position = machine_tool_B.v;
  _rtXdot->position_i = machine_tool_B.v_p;
  _rtXdot->position_c = machine_tool_B.v_k;
  _rtXdot->speed = machine_tool_B.a;
  _rtXdot->speed_i = machine_tool_B.a_a;
  _rtXdot->speed_h = machine_tool_B.a_j;
  _rtXdot->Filter_CSTATE = machine_tool_B.FilterCoefficient;
  _rtXdot->Integrator_CSTATE = machine_tool_B.IntegralGain;
  _rtXdot->Integrator_CSTATE_g = machine_tool_B.Switch;
  _rtXdot->Filter_CSTATE_o = machine_tool_B.FilterCoefficient_p;
  _rtXdot->Filter_CSTATE_p = machine_tool_B.FilterCoefficient_n;
  _rtXdot->Integrator_CSTATE_d = machine_tool_B.IntegralGain_n;
  _rtXdot->Integrator_CSTATE_e = machine_tool_B.Switch_g;
  _rtXdot->Filter_CSTATE_m = machine_tool_B.FilterCoefficient_d;
  _rtXdot->Filter_CSTATE_oe = machine_tool_B.FilterCoefficient_py;
  _rtXdot->Integrator_CSTATE_i = machine_tool_B.IntegralGain_m;
  _rtXdot->Integrator_CSTATE_m = machine_tool_B.Switch_p;
  _rtXdot->Filter_CSTATE_e = machine_tool_B.FilterCoefficient_g;
}

void machine_tool_zeroCrossings(void)
{
  ZCV_machine_tool_T *_rtZCSV;
  real_T DeadZone_LwrReg_ZC_tmp;
  real_T DeadZone_UprReg_ZC_tmp;
  _rtZCSV = ((ZCV_machine_tool_T *) machine_tool_M->zcSignalVector);
  DeadZone_LwrReg_ZC_tmp = machine_tool_B.Sum - (-machine_tool_P.Xsat);
  _rtZCSV->DeadZone_LwrReg_ZC = DeadZone_LwrReg_ZC_tmp;
  DeadZone_UprReg_ZC_tmp = machine_tool_B.Sum - machine_tool_P.Xsat;
  _rtZCSV->DeadZone_UprReg_ZC = DeadZone_UprReg_ZC_tmp;
  _rtZCSV->NotEqual_RelopInput_ZC = machine_tool_B.ZeroGain -
    machine_tool_B.DeadZone;
  _rtZCSV->SignPreSat_Input_ZC = machine_tool_B.DeadZone;
  _rtZCSV->SignPreIntegrator_Input_ZC = machine_tool_B.IntegralGain_l;
  _rtZCSV->Saturation_UprLim_ZC = DeadZone_UprReg_ZC_tmp;
  _rtZCSV->Saturation_LwrLim_ZC = DeadZone_LwrReg_ZC_tmp;
  DeadZone_LwrReg_ZC_tmp = machine_tool_B.Sum_n - (-machine_tool_P.Ysat);
  _rtZCSV->DeadZone_LwrReg_ZC_g = DeadZone_LwrReg_ZC_tmp;
  DeadZone_UprReg_ZC_tmp = machine_tool_B.Sum_n - machine_tool_P.Ysat;
  _rtZCSV->DeadZone_UprReg_ZC_j = DeadZone_UprReg_ZC_tmp;
  _rtZCSV->NotEqual_RelopInput_ZC_c = machine_tool_B.ZeroGain_n -
    machine_tool_B.DeadZone_i;
  _rtZCSV->SignPreSat_Input_ZC_o = machine_tool_B.DeadZone_i;
  _rtZCSV->SignPreIntegrator_Input_ZC_h = machine_tool_B.IntegralGain_c;
  _rtZCSV->Saturation_UprLim_ZC_e = DeadZone_UprReg_ZC_tmp;
  _rtZCSV->Saturation_LwrLim_ZC_l = DeadZone_LwrReg_ZC_tmp;
  DeadZone_LwrReg_ZC_tmp = machine_tool_B.Sum_l - (-machine_tool_P.Zsat);
  _rtZCSV->DeadZone_LwrReg_ZC_i = DeadZone_LwrReg_ZC_tmp;
  DeadZone_UprReg_ZC_tmp = machine_tool_B.Sum_l - machine_tool_P.Zsat;
  _rtZCSV->DeadZone_UprReg_ZC_i = DeadZone_UprReg_ZC_tmp;
  _rtZCSV->NotEqual_RelopInput_ZC_f = machine_tool_B.ZeroGain_i -
    machine_tool_B.DeadZone_g;
  _rtZCSV->SignPreSat_Input_ZC_e = machine_tool_B.DeadZone_g;
  _rtZCSV->SignPreIntegrator_Input_ZC_f = machine_tool_B.IntegralGain_k;
  _rtZCSV->Saturation_UprLim_ZC_i = DeadZone_UprReg_ZC_tmp;
  _rtZCSV->Saturation_LwrLim_ZC_h = DeadZone_LwrReg_ZC_tmp;
}

void machine_tool_initialize(void)
{
  machine_tool_PrevZCX.Integrator1_Reset_ZCE = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Integrator1_Reset_ZCE_k = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Integrator1_Reset_ZCE_a = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Integrator_Reset_ZCE = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Integrator_Reset_ZCE_o = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Integrator_Reset_ZCE_i = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Filter_Reset_ZCE = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Integrator_Reset_ZCE_it = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Integrator_Reset_ZCE_g = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Filter_Reset_ZCE_m = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Filter_Reset_ZCE_l = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Integrator_Reset_ZCE_h = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Integrator_Reset_ZCE_gd = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Filter_Reset_ZCE_c = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Filter_Reset_ZCE_j = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Integrator_Reset_ZCE_ic = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Integrator_Reset_ZCE_k = UNINITIALIZED_ZCSIG;
  machine_tool_PrevZCX.Filter_Reset_ZCE_g = UNINITIALIZED_ZCSIG;
  machine_tool_X.position = machine_tool_P.X0;
  machine_tool_X.position_i = machine_tool_P.Y0;
  machine_tool_X.position_c = machine_tool_P.Z0;
  machine_tool_X.speed = machine_tool_P.Xaxis_v0;
  machine_tool_X.speed_i = machine_tool_P.Yaxis_v0;
  machine_tool_X.speed_h = machine_tool_P.Zaxis_v0;
  machine_tool_X.Filter_CSTATE =
    machine_tool_P.PIDController_InitialConditionForFilter;
  machine_tool_X.Integrator_CSTATE =
    machine_tool_P.PIDController_InitialConditionForIntegrator;
  machine_tool_X.Integrator_CSTATE_g =
    machine_tool_P.PIDController1_InitialConditionForIntegrator;
  machine_tool_X.Filter_CSTATE_o =
    machine_tool_P.PIDController1_InitialConditionForFilter;
  machine_tool_DW.Memory_PreviousInput = machine_tool_P.Memory_InitialCondition;
  machine_tool_X.Filter_CSTATE_p =
    machine_tool_P.PIDController_InitialConditionForFilter_c;
  machine_tool_X.Integrator_CSTATE_d =
    machine_tool_P.PIDController_InitialConditionForIntegrator_n;
  machine_tool_X.Integrator_CSTATE_e =
    machine_tool_P.PIDController1_InitialConditionForIntegrator_h;
  machine_tool_X.Filter_CSTATE_m =
    machine_tool_P.PIDController1_InitialConditionForFilter_p;
  machine_tool_DW.Memory_PreviousInput_e =
    machine_tool_P.Memory_InitialCondition_k;
  machine_tool_X.Filter_CSTATE_oe =
    machine_tool_P.PIDController_InitialConditionForFilter_n;
  machine_tool_X.Integrator_CSTATE_i =
    machine_tool_P.PIDController_InitialConditionForIntegrator_h;
  machine_tool_X.Integrator_CSTATE_m =
    machine_tool_P.PIDController1_InitialConditionForIntegrator_m;
  machine_tool_X.Filter_CSTATE_e =
    machine_tool_P.PIDController1_InitialConditionForFilter_l;
  machine_tool_DW.Memory_PreviousInput_n =
    machine_tool_P.Memory_InitialCondition_d;
}

void machine_tool_terminate(void)
{
}

RT_MODEL_machine_tool_T *machine_tool(void)
{
  (void) memset((void *)machine_tool_M, 0,
                sizeof(RT_MODEL_machine_tool_T));

  {
    rtsiSetSimTimeStepPtr(&machine_tool_M->solverInfo,
                          &machine_tool_M->Timing.simTimeStep);
    rtsiSetTPtr(&machine_tool_M->solverInfo, &rtmGetTPtr(machine_tool_M));
    rtsiSetStepSizePtr(&machine_tool_M->solverInfo,
                       &machine_tool_M->Timing.stepSize0);
    rtsiSetdXPtr(&machine_tool_M->solverInfo, &machine_tool_M->derivs);
    rtsiSetContStatesPtr(&machine_tool_M->solverInfo, (real_T **)
                         &machine_tool_M->contStates);
    rtsiSetNumContStatesPtr(&machine_tool_M->solverInfo,
      &machine_tool_M->Sizes.numContStates);
    rtsiSetNumPeriodicContStatesPtr(&machine_tool_M->solverInfo,
      &machine_tool_M->Sizes.numPeriodicContStates);
    rtsiSetPeriodicContStateIndicesPtr(&machine_tool_M->solverInfo,
      &machine_tool_M->periodicContStateIndices);
    rtsiSetPeriodicContStateRangesPtr(&machine_tool_M->solverInfo,
      &machine_tool_M->periodicContStateRanges);
    rtsiSetContStateDisabledPtr(&machine_tool_M->solverInfo, (boolean_T**)
      &machine_tool_M->contStateDisabled);
    rtsiSetSolverZcSignalPtr(&machine_tool_M->solverInfo,
      &machine_tool_M->zcSignalVector);
    rtsiSetErrorStatusPtr(&machine_tool_M->solverInfo, (&rtmGetErrorStatus
      (machine_tool_M)));
    rtsiSetRTModelPtr(&machine_tool_M->solverInfo, machine_tool_M);
  }

  rtsiSetSimTimeStep(&machine_tool_M->solverInfo, MAJOR_TIME_STEP);
  rtsiSetIsMinorTimeStepWithModeChange(&machine_tool_M->solverInfo, false);
  rtsiSetIsContModeFrozen(&machine_tool_M->solverInfo, false);
  machine_tool_M->intgData.y = machine_tool_M->odeY;
  machine_tool_M->intgData.f[0] = machine_tool_M->odeF[0];
  machine_tool_M->intgData.f[1] = machine_tool_M->odeF[1];
  machine_tool_M->intgData.f[2] = machine_tool_M->odeF[2];
  machine_tool_M->contStates = ((X_machine_tool_T *) &machine_tool_X);
  machine_tool_M->contStateDisabled = ((XDis_machine_tool_T *)
    &machine_tool_XDis);
  machine_tool_M->Timing.tStart = (0.0);
  rtsiSetSolverData(&machine_tool_M->solverInfo, (void *)
                    &machine_tool_M->intgData);

  {
    static uint8_T zcAttributes[39] = { (0xc0|SL_ZCS_EVENT_ALL_UP), (0xc0|
      SL_ZCS_EVENT_ALL_UP), (0xc0|SL_ZCS_EVENT_ALL_UP), (0xc0|
      SL_ZCS_EVENT_ALL_UP), (0xc0|SL_ZCS_EVENT_ALL_UP), (0xc0|
      SL_ZCS_EVENT_ALL_UP), (0xc0|SL_ZCS_EVENT_ALL_UP), (0xc0|
      SL_ZCS_EVENT_ALL_UP), (0xc0|SL_ZCS_EVENT_ALL_UP), (0xc0|
      SL_ZCS_EVENT_ALL_UP), (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL),
      (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL),
      (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL), (0xc0|SL_ZCS_EVENT_ALL_UP), (0xc0|
      SL_ZCS_EVENT_ALL_UP), (0xc0|SL_ZCS_EVENT_ALL_UP), (0xc0|
      SL_ZCS_EVENT_ALL_UP), (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL),
      (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL),
      (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL), (0xc0|SL_ZCS_EVENT_ALL_UP), (0xc0|
      SL_ZCS_EVENT_ALL_UP), (0xc0|SL_ZCS_EVENT_ALL_UP), (0xc0|
      SL_ZCS_EVENT_ALL_UP), (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL),
      (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL),
      (SL_ZCS_EVENT_ALL), (SL_ZCS_EVENT_ALL) };

    rtsiSetSolverZcSignalAttrib(&machine_tool_M->solverInfo,
      zcAttributes);
    machine_tool_M->zcDetectorData.vL = machine_tool_M->zcVL;
    machine_tool_M->zcDetectorData.vR = machine_tool_M->zcVR;
    machine_tool_M->zcDetectorData.vZ = machine_tool_M->zcVZ;
    machine_tool_M->zcDetectorData.t0 = machine_tool_M->zct0;
    machine_tool_M->zcDetectorData.tR = machine_tool_M->zctR;
    machine_tool_M->zcDetectorData.x0 = machine_tool_M->zcx0;
    machine_tool_M->zcDetectorData.xR = machine_tool_M->zcxR;
  }

  rtsiSetSolverName(&machine_tool_M->solverInfo,"ode3");
  rtmSetTPtr(machine_tool_M, &machine_tool_M->Timing.tArray[0]);
  machine_tool_M->Timing.stepSize0 = 1.0;
  (void) memset(((void *) &machine_tool_B), 0,
                sizeof(B_machine_tool_T));

  {
    (void) memset((void *)&machine_tool_X, 0,
                  sizeof(X_machine_tool_T));
  }

  {
    (void) memset((void *)&machine_tool_XDis, 0,
                  sizeof(XDis_machine_tool_T));
  }

  (void) memset((void *)&machine_tool_DW, 0,
                sizeof(DW_machine_tool_T));
  (void)memset(&machine_tool_U, 0, sizeof(ExtU_machine_tool_T));
  (void)memset(&machine_tool_Y, 0, sizeof(ExtY_machine_tool_T));
  return machine_tool_M;
}
