#ifndef OMA4DRV_INCLUDED
#define OMA4DRV_INCLUDED

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
#define QUERYPHYSADDR 0x28    /* Return the physical address of a detector */
                              /* board's memory space */

   /* Send Codes */
#define ACTIVATEDETECTOR 0x00   /* Activate the detector to use the next */
                                /* available address block.  Return error */
                                /* if no memory space left. */
#define DEACTIVATEDETECTOR 0x01 /* free memory and turn off board */

#define BLOCKFORMSG     0x02    /* Block and wait for the interface board to
                                    ask for service */

#define STOPACQUISITION 0x03    /* Unblock detector's blocked acquisition  */ 
                                /* thread and keep any more from blocking. */
                                /* BLOCKFORMSG will return EXPSTOP         */

// 1/10/91 DAI added for OMA4000 development
// Additional codes for debugging
#define GET_SEG_ABSOLUTE 0x10   // create a protected mode selector for a
                                // given address
#define DEL_SEG_ABSOLUTE 0x11   // delete the created protected mode selector


/***************************************************************************/
/*                        device driver messages                           */
/***************************************************************************/

#define EXPSTOP 1                /* stop the data acquisition process */
#define CURVESREADY 2            /* Curves are ready to be transferred off */
                                 /* of the interface board */
#define TIMEDOUT 3               /* timed out of block */
/***************************************************************************/

#define ACQSTKSZ 0x800

#endif
