/* cmdtbl.h - OMA4 Driver Command Table                                   */
/* declare functions for use by main interface module                     */

#ifndef CMDTBL_DEFINED
#define CMDTBL_DEFINED

#include "eggtype.h"

typedef union
    {
    SHORT intval;
    float fltval;
    } rvalue;

/* the command table will be an array of these structs (def in detdriv) */

typedef struct
  {
  enum det_command cmd_tag;
  SHORT (*set_routine)(SHORT);
  SHORT (*get_routine)(SHORT *);
  } int_cmd_entry;

typedef struct
  {
  enum det_command cmd_tag;
  SHORT (*set_routine)(float);
  SHORT (*get_routine)(float *);
  } flt_cmd_entry;

typedef SHORT (*pfsint)(SHORT);   /* ptr to func setting an SHORT value  */
typedef SHORT (*pfgint)(SHORT *); /* ptr to func getting an SHORT value  */
typedef SHORT (*pfsflt)(float);   /* ptr to func setting an flt value  */
typedef SHORT (*pfgflt)(float *); /* ptr to func getting an flt value  */

#ifndef __WATCOMC__
SHORT set_int_detect_param(enum int_command cmd, SHORT prm);
SHORT set_flt_detect_param(enum flt_command cmd, FLOAT prm);
SHORT get_int_detect_param(enum int_command cmd, SHORT *prm);
SHORT get_flt_detect_param(enum int_command cmd, FLOAT *prm);

pfgint find_get_cmd(enum int_command cmd);
pfgflt find_flt_get_cmd(enum flt_command cmd);
pfsint find_set_cmd(enum int_command cmd);
pfsflt find_flt_set_cmd(enum flt_command cmd);
pfsflt find_flt_set_cmd(enum flt_command cmd);
SHORT set_int_detect_param(enum int_command cmd, SHORT prm);
SHORT set_flt_detect_param(enum flt_command cmd, FLOAT prm);

#else

SHORT set_int_detect_param(int cmd, SHORT prm);
SHORT set_flt_detect_param(int cmd, FLOAT prm);
SHORT get_int_detect_param(int cmd, SHORT *prm);
SHORT get_flt_detect_param(int cmd, FLOAT *prm);

pfgint find_get_cmd(int cmd);
pfgflt find_flt_get_cmd(int cmd);
pfsint find_set_cmd(int cmd);
pfsflt find_flt_set_cmd(int cmd);
pfsflt find_flt_set_cmd(int cmd);
SHORT set_int_detect_param(int cmd, SHORT prm);
SHORT set_flt_detect_param(int cmd, FLOAT prm);

#endif

#endif
