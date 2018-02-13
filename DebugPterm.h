#ifndef __DebugPterm_H__
#define __DebugPterm_H__ 1

#include "PtermTrace.h"

#ifdef DEBUG
extern Trace debugF;
#define debug debugF.Log
#define traceF debugF
#define tracex debug
#else
#define debug(x,...) /* Nothing */
extern Trace traceF;
#define tracex traceF.Log
#endif

#endif  // __DebugPterm_H__
