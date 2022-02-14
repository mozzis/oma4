/********************************************************************
*                                                                   *
*                              OMA4.H                               *
*                  Contains data for various modules                *
*                      of the OMA4 ASIC compiler                    *
*                                                                   *
*                                                                   *
*********************************************************************/


#define TRUE  1           /* added for C skeleton */
#define FALSE 0
#define MAXINT 32767

#define STACKSIZE 200     /* maximum size of LR(1) parser stack */
#define EOS '\0'          /* marks end of string */
#define EOLCH '\n'        /* end of line character */
#define LINELEN 132       /* maximum length of a line */
#define FILENAME_LEN 65
#define STRTABLEN 28000   /* maximum number of chars in string table */
#define STRING_QUOTE '\'' /* character delimiting quoted strings */
#define MAXERRORS 20      /* maximum errors before aborting */
#define ERROR 1           /* return code for terminate on error */
#define GOOD 0            /* return code for terminate without error */
#define HASHSIZE 1013     /* hash table size -- prime number! */
#define HLIMIT 1012       /* limit in hash table (hashsize minus one) */
#define MAXTOKLEN 15      /* length of a token or symbol */
#define ERRSYMLEN 7       /* length of err#xx plus a terminator */
#define RARGLIMIT 5       /* Max number of replaceable arguments in macro */
#define MAXMACROS 100     /* Maximum number of macros defined */
#define COUNTER_LIMIT 512 /* Maximum number of counters or tags */
#define FIXUP_LIMIT 1000  /* Maximum number of fixup records */

typedef int boolean;
typedef short int small_int;
typedef char symbol[MAXTOKLEN+1];
typedef short int symtype;

/* Enumerated values for SYMTYPE */

#define RESERVED 0
#define SYMERR   1
#define USER     2
#define INT_VARIABLE 3
#define LABEL_TYPE 4
#define EXT_KEY 5
#define LAST_SYMTYPE 5

extern int symcount;
extern char *symtypename[];

/* structure for <identifier>s and keywords */

struct symtabtype {
   struct symtabtype far * next;
   int level;
   symbol sym;                        /* symbol name */
   symtype symt;                      /* values: reserved..user */
   unsigned sympc;                    /* program counter sym defined at */
   int pc_type;                       /* which program counter, dma or fc */
      union {
      int tokval;                     /* if symt = reserved */
      unsigned long int_variable;     /* if symt = int_variable */
      } usym;                         /* name of union is usym */
   };

typedef struct symtabtype *symtabp;

/* Enumerated values for SEMTYPE */

#define OTHER   0
#define IDENT   1
#define FIXED   2
#define FLT     3
#define STRNG   4
#define INT_VAR 5
#define LOCATOR 6
#define LAST_SEMTYPE 6

extern char *semtypename[];
typedef short int semtype;

/**********************************/
/* semantic stack structure       */
/**********************************/

struct semrec
  {
  semtype semt; /* values: other..strng  */
  union
    {
    symtabp symp;     /* semt = ident, point to symtab entry  */
    unsigned long numval;      /* semt = fixed, carry value */
    float rval;       /* semt = flt    ""        */
    int stx;          /* semt = strng; (pos in strtab) */
    symtabp int_var;  /* semt = int_var */
    } usem;           /* union name is usem */
  };

typedef struct semrec *semrecp;


/* Dynamic parser data structures */

extern int far stack[];         /* the LR(1) state stack */
extern semrecp far semstack[];  /* semantics stack */
extern int cstate;
extern int stackx;              /* index of top of stack */

/* Lexical and token data */

extern int bs;
extern int line_cnt;   /* number of source lines (for error report) */
extern char far line[];    /* source line */
extern int lx;         /* index of next character in LINE */
extern int errpos;     /* current token index in LINE */
extern char ch; /* next character from input file */
extern int token;      /* Next token in input list */
extern semrecp lsemp;  /* current semantics assoc. with token */
extern int tokenx;     /* index into TOKARY, LSEMPARY */
extern int tokary[];   /* token queue */
extern semrecp lsempary[];
extern char errsym[];  /* special symbol reserved for errors */

/* Symbol table data */

extern symtabp far symtab[];
extern char far strtab[];
extern int strtabx;

struct macro_rec_type {
    struct macro_rec_type far *next;  /* Point to next macro */
    symbol name;                      /* Store name of macro here */
    int text_strt;                    /* Store ptr to body of macro here */
    int text_end;                     /* Store ptr to end of macro here */
    int numrargs;                     /* Legal number of replaceable args */
    };                                /* (just index into strtab) */

typedef struct macro_rec_type *macro_recp;

extern macro_recp far macros[];
extern semrecp far rargs[];
extern boolean macro_mode;
extern boolean macro_def;
extern int macrox;
extern int macroptr;
extern int macro_next;
extern int rargctr;
extern int gencntr;
extern boolean rlabel_seen;

#define FC_MODE  0
#define DMA_MODE 1
#define DAC_MODE 2

extern FILE *sfile;                    /* source file */
extern char sfilename[];               /* source file name */
extern FILE *object_file;              /* dest file */
extern char object_filename[];         /* dest file name */
extern char savename[];                /* for include files handling */
extern FILE * savefile;                /* old pointer */
extern int line_tmp;

extern int errors;
extern int debug;                      /* >0 turns on some tracing */
extern int first_pass;
extern int listmode;
extern int xrefmode;
extern int include_mode;
extern int verbose;

extern int pc_mode;
extern unsigned int PC[3]; 
extern unsigned int block_count;
extern long object_file_offset;

extern char far * key_array[];

extern  int hashit(char *fsym);
extern  struct symtabtype *new_sym(short tsymt,int lev);
extern  struct symtabtype *find_lev_sym(char *fsym,int lev);
extern  struct symtabtype *findsym(char *fsym,int lev);
extern  struct symtabtype *makesym(char *fsym,short syt,int lev);
extern  struct symtabtype *forcesym(char *fsym,short syt,int lev);
extern  void clearsym(int clevel);
extern  void dump_all(FILE *dest);
extern  void get_val(void );
void eval_binop(int op,struct semrec *opnd1,struct semrec *opnd2,struct semrec *result);
extern  int create_object_file(char *filename);
extern  void close_object_file(void );
extern  long write_record_header(int rec_type,unsigned int start_addr, char *name);
extern  void go_back_to_set_record_length(long file_offset,unsigned int record_length);
extern  void write_EOF_record(void );
extern  struct fixup_rec *new_fix(unsigned int pc,unsigned int addr);
extern  char upshift(int c);
extern  void inc_cntr(int i);
extern  char *choose_p(void );
extern  void listline_out(int c,short w);

void  wconst_out(int c);
void  wdata_out(int w);
void  ldata_out(unsigned long w);
void  code_wdata_out(int c, int w);
void  code_ldata_out(int c, unsigned long w);
void  scanfile(void);
extern void code_out(int c);
extern void code1_out(int c1, int c2);
extern void code_and_24bits_out(int c1, long c2);
extern void code_and_long_out(int c1, unsigned long c2);
extern void data_out(short w);
extern void code_and_data_out(int c,short w);
extern void lconst_out(unsigned long c);
extern void include(void);
extern void apply(int pflag,struct semrec *tsemp);
extern void pre_apply(int pflag,struct semrec *tsemp);
extern void parse_abort(char *msg);
extern void error(char *msg);
extern int wrtok(FILE *dest,int tx);
extern void getline(void );
extern void nextch(void );
extern void putstr(char *str);
extern void get_token(void );
extern void copy_semrec(struct semrec *dst,struct semrec *src);
extern int trial_parse(int *cstate,int *stack,int stackx);
extern int getargs(int argc,char * *argv);
extern void parser(void);
void inittables(void);
void init_sem(void);
void end_sem(void);
