/* -----------------------------------------------------------------------
/
/   omaregs.h
/
/ Copyright (c) 1989,  EG&G Princeton Applied Research, Inc.
/
/ Written by: TLB Version 1.00 13 December  1988
/
/ ----------------------------------------------------------------------- */

#ifndef OMAREGS_INCLUDED
#define OMAREGS_INCLUDED

/* -- register offsets from base I/O port address ----------------------- */

#define   OMA_CONTROL_0             0
#define   OMA_CONTROL_1             1

/* -- bits and bit patterns ---------------------------------------------- */

/* these are bit patterns to be used with the register typedefs */

#define   TWO_MEGABYTES           0x01   /* ControlReg0.memory_map */
#define   FOUR_MEGABYTES          0x03   /* ControlReg0.memory_map */
#define   SIX_MEGABYTES           0x07   /* ControlReg0.memory_map */
#define   EIGHT_MEGABYTES         0x0F   /* ControlReg0.memory_map */

#define   LO_TRUE                 0      /* Any 'lo true' bit with _lo */
#define   LO_FALSE                1      /* Any 'lo true' bit with _lo */

/* -- ASIC Registers ----------------------------------------------------- */

/* -- Control Register 0 ------------------------------------------------- */

typedef union {
  struct {
    BSHORT unused_bit         : 1;
    BSHORT memory_map         : 6; /* allows addr 2M 4M 6M 8M 10M 12M */
    BSHORT cooler_irq_enabled : 1;
  } bits;
  UCHAR byte;
} OMAControlReg0;

/* -- Control Register 1 ------------------------------------------------- */

typedef union {
  struct {
    BSHORT prog_mem_lo       : 1; /* $01 */
    BSHORT int_level         : 2; /* $02 $04 */
    BSHORT OMA_reset_lo      : 1; /* $08 */
    BSHORT ALU_reset_lo      : 1; /* $10 */
    BSHORT cooler_status     : 1; /* $20 */
    BSHORT host_attn         : 1; /* $40 */
    BSHORT cooler_irq_lo     : 1; /* $80 */
  } rbits;
  struct {
    BSHORT prog_mem_lo       : 1; /* $01 */
    BSHORT int_level         : 2; /* $02 $04 */
    BSHORT OMA_reset_lo      : 1; /* $08 */
    BSHORT ALU_reset_lo      : 1; /* $10 */
    BSHORT da_attn           : 1; /* $20 */
    BSHORT host_attn_ack     : 1; /* $40 */
    BSHORT cooler_attn_ack   : 1; /* $80 */
  } wbits;
  UCHAR byte;
} OMAControlReg1;

#endif  /*  OMAREGS_INCLUDED */
