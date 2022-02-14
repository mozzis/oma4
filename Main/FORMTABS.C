/* -----------------------------------------------------------------------
/
/  formtabs.c
/
/  Copyright (c) 1990,  EG&G Instruments Inc.
/
/  $Header:   J:/logfiles/oma4000/main/formtabs.c_v   1.3   13 Jan 1992 13:03:34   cole  $
/  $Log:   J:/logfiles/oma4000/main/formtabs.c_v  $
 * 
*/

#include "formtabs.h"
#include "omaform.h"
#include "runsetup.h"
#include "spgraph.h"
#include "baslnsub.h"
#include "calib.h"
#include "ycalib.h"
#include "macruntm.h"
#include "tagcurve.h"
#include "baslnsub.h"
#include "config.h"
#include "scanset.h"
#include "rapset.h"
#include "runforms.h"
#include "pltsetup.h"
#include "fileform.h"
#include "mathform.h"
#include "statform.h"
#include "backgrnd.h"
#include "macrofrm.h"

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// table used by MacPlayForm() in macrecor.c
FORM * FormTable[] =
{
   NULL,       // Dummy entry for no entry
 
   NULL,       // &DA_Form;

   NULL,       // &PlotSetupForm;

   NULL,       // &chformat_form;
   NULL,       // &getmethodhdrForm;

   NULL,       // &LIVE_TO_DISK_FORM;
   NULL,       // &LiveForm;
   NULL,       // &AccumForm;

   NULL,       // &KeyStrokeForm;

   NULL,       // &BackGroundForm;

   NULL,       // &MathForm
   NULL,       // &StatForm

   NULL,       // &ControlForm;
   NULL,       // &CurveScrollForm;
   NULL,       // &FileScrollForm;
   NULL,       // &UserForm;

   NULL,       // &X_CalibrationForm;
   NULL,       // &X_CalScrollForm;
   NULL,       // &GetCalibPointForm;

   NULL,       // &Y_CalibrationForm;
   NULL,       // &Y_KnotsForm;

   NULL,       // &ScanSetupForm;
   NULL,       // &SpecialsForm;

// add the followi&ng only for TABULAR scan setup
   NULL,       // &SliceForm;
   NULL,       // &SliceScrollForm;
   NULL,       // &TrackForm;
   NULL,       // &TrackScrollForm;
   
   NULL,       // &ConfigForm;

   NULL,       // &baseline_sub_form;
   NULL,       // &knots_form;

   NULL,       // &CursorGoToForm;

   NULL,       // &TagCurveGroupForm;
   NULL,       // &SaveTaggedForm;

   NULL,       // &M1235Form;
   NULL,       // &GetOffsetPointForm;
  
   NULL,       // &RapdaForm;
   NULL        // &RegionScrollForm;
};

// form table for the forms not in FormTable[]. not recorded during keystroke
// record, not used by MacPlayForm(), MacFormIndex is KSI_NO_INDEX
FORM * NorecFormTable[] =
{
   NULL,       // &LiveBackForm;

   NULL,       // &AccumBackForm;

   NULL,       // &MacroForm;
};

// initialize FormTable[] by calling all the registration functions of
// all the modules with forms.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void initFormTable(void)
{
   registerDA_Form();
   registerPlotSetupForm();
   registerConfigForm();
   registerMathForm();
   registerStatForm();
   registerRUNFORMS();
   registerBaslnsubForms();
   registerTagcurveForms();
   registerScansetForms();
   registerRapdaForms();
   registerXCalibForms();
   registerYCalibForms();
   registerScrlltstForms();
   registerOmaform1Forms();
   registerSPGraphForm();
   registerBackgroundForm();
   registerMacroForm();
}
