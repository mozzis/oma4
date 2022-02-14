#include <stdlib.h>

#include "oma4driv.h"

#pragma argsused

int SetParam(int cmd, float param)
{
  return 0;
}

#pragma argsused
int GetParam(int cmd, float *param)
{
  float retval;
  static long lcount = 0;

  switch (cmd)
    {
    case THERE:
    case DMODEL:
    case X0:
    case Y0:
    case DELTAX:
    case TRACK:
    case TRACKS:
    case TRKMODE:
    case DAPROG:
    case I:
    case J:
      retval =  1.0F;
    break;
    case L:
      retval =  (float)++lcount;
    break;
    case DELTAY:
      retval =  10.0F;
    break;
    case DERROR:
    case SHUTMODE:
    case COOLONOFF:
    case RUN:
    case STOP:
    default:
      retval =  0.0F;
    break;
    case POINTS:
    case ACTIVEX:
      retval = 512.0F;
    break;
    case ANTIBLOOM:
      retval = 100.0F;
    break;
    case WFTO:
    case ET:
    case SAMEET:
    case FTIME:
      retval = .040F;
    break;
    case PTIME:
      retval = 10e-6F;
    break;
    case BYTES:
      retval = 2048.0F;
    break;
    }
  *param = retval;
  return 0;
}

#pragma argsused
int setup_detector_interface(USHORT port, ULONG address, ULONG size)
{
  return 0;
}

#pragma argsused
SHORT ReadCurveFromMem(PCHAR Track, SHORT len, USHORT curve)
{
  int i;
  long * pTrack = (long *)Track;

  len /= 4;
  for (i = 0; i < len; i++)
    {
    pTrack[i] = (long)rand();
    }
  return 0;
}
