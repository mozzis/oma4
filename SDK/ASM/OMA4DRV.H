/* category codes */
#define OMA4CNTRL 0xC0

/* Function codes */
#define RET_ERR 0x80  /* return error if unknown function flag */
   /* Query Codes */
#define QUERYINIFILE 0x20     /* get filename of init file as put in */
                              /* device initialization params        */
#define QUERYDETNUM   0x21    /* get the number of detectors attached */
#define QUERYOMATYPE  0x22    /* see what kind of OMA interface is attached */
                              /* as set by device initialization flag */
#define QUERYDETTYPE  0x23    /* return detector model number */
#define QUERYMEMADDR  0x24    /* Return a pointer to an array of virtual */
                              /* addresses to a detector board's memory */
                              /* space */
#define QUERYMEMSIZE  0x25    /* Return the size in Kbytes of a detector */
                              /* board's memory                          */
#define QUERYPORTADDR 0x26   /* Return the base port address for a detector */
                             /* board */
#define QUERYDATASEG  0x27    /* copy the device driver's data seg to a */ 
                              /* buffer */

   /* Send Codes */
#define ACTIVATEDETECTOR 0x00   /* Activate the detector to use the next */
                                /* available address block.  Return error */
                                /* if no memory space left. */
#define DEACTIVATEDETECTOR 0x01 /* free memory and turn off board */
