# ops.awk - awk program to convert from intermetrics symbol table
# to OMA4 compiler include file
# so addresses of monitor routines will be known to compiler
# to use, run awk like this:
# awk -f ops.awk monitor.sym >ops.add
# (The awk I use is a Unix compatible awk from Polytron called Polyawk)

# These are handy equates (usually not needed in an OMA4 program)

$1 == "'FCTBLPTR"   ||
$1 == "'FCTBLINX"   ||
$1 == "'FCCODPTR"   ||
$1 == "'DCTBLPTR"   ||
$1 == "'DCTBLINX"   ||
$1 == "'DCCODPTR"   ||
$1 == "'POINTERS"   ||
$1 == "'DACTBLPTR"  ||
$1 == "'DACTBLINX"  ||
$1 == "'DACCODPTR"  ||
$1 == "'LCTBLPTR"   ||
$1 == "'LCTBLINX"   ||
$1 == "'DATA"       ||
$1 == "'DCTBL"      ||
$1 == "'FCTBL"      ||
$1 == "'DACTBL"     ||
$1 == "'LCTBL"      ||
$1 == "'IN_DA"      ||
$1 == "'PC_INPROG"  ||
$1 == "'PC_MAILBOX" ||
$1 == "'LOOPSTACK"  ||
$1 == "'LOOPTR"     ||
$1 == "'RESFLG"     ||
$1 == "'DA_START"   ||
$1 == "'GO"         ||
$1 == "'SAVREGS"    ||
$1 == "'EMSGTBL"    ||
$1 == "'EMSGBUF"    ||
$1 == "'INITPATT"   ||
$1 == "'PATT"       ||
$1 == "'QUIKTEST"   ||
$1 == "'SUCCESS"   { printf("%s\tEQU %s\n", substr($1,2), $3) }

# These are the labels of the data acquisition routines, declared
# with the OMA4 'DACCMD' directive

$1 == "'_ADDIB"    ||
$1 == "'_ADDIW"    ||
$1 == "'_ADDIL"    ||
$1 == "'_ADDIBN"   ||
$1 == "'_ADDIWN"   ||
$1 == "'_ADDILN"   ||
$1 == "'_ADDAB"    ||
$1 == "'_ADDAW"    ||
$1 == "'_ADDAL"    ||
$1 == "'_ADDABTN"  ||
$1 == "'_ADDAWTN"  ||
$1 == "'_ADDALTN"  ||
$1 == "'_ADDABFN"  ||
$1 == "'_ADDAWFN"  ||
$1 == "'_ADDALFN"  ||
$1 == "'_ADDABDN"  ||
$1 == "'_ADDAWDN"  ||
$1 == "'_ADDALDN"  ||
$1 == "'_BEQ"      ||
$1 == "'_BGT"      ||
$1 == "'_BLT"      ||
$1 == "'_CMPRIL"   ||
$1 == "'_CMPRIW"   ||
$1 == "'_CMPRIB"   ||
$1 == "'_CMPRL"    ||
$1 == "'_CMPRW"    ||
$1 == "'_CMPRB"    ||
$1 == "'_CLRMEM"   ||
$1 == "'_DOI"      ||
$1 == "'_DOAT"     ||
$1 == "'_DOATN"    ||
$1 == "'_DONE"     ||
$1 == "'_DONOT"    ||
$1 == "'_GOTO"     ||
$1 == "'_INC.MEM"  ||
$1 == "'_LOOP"     ||
$1 == "'_MOVELI"   ||
$1 == "'_MOVEWI"   ||
$1 == "'_MOVEBI"   ||
$1 == "'_MOVELIN"  ||
$1 == "'_MOVEWIN"  ||
$1 == "'_MOVEBIN"  ||
$1 == "'_MOVELIA"  ||
$1 == "'_MOVEWIA"  ||
$1 == "'_MOVEBIA"  ||
$1 == "'_MOVEL"    ||
$1 == "'_MOVEW"    ||
$1 == "'_MOVEB"    ||
$1 == "'_MOVELA"   ||
$1 == "'_MOVEWA"   ||
$1 == "'_MOVEBA"   ||
$1 == "'_MOVELFN"  ||
$1 == "'_MOVEWFN"  ||
$1 == "'_MOVEBFN"  ||
$1 == "'_MOVELNA"  ||
$1 == "'_MOVEWNA"  ||
$1 == "'_MOVEBNA"  ||
$1 == "'_MOVELTN"  ||
$1 == "'_MOVEWTN"  ||
$1 == "'_MOVEBTN"  ||
$1 == "'_MOVELDN"  ||
$1 == "'_MOVEWDN"  ||
$1 == "'_MOVEBDN"  ||
$1 == "'_NEXT_DA"  ||
$1 == "'_ORIB"     ||
$1 == "'_ORIW"     ||
$1 == "'_ORIL"     ||
$1 == "'_ORAB"     ||
$1 == "'_ORAW"     ||
$1 == "'_ORAL"     ||
$1 == "'_RAPSCAN"  ||
$1 == "'_SORTTRK"  ||
$1 == "'_SUBIB"    ||
$1 == "'_SUBIW"    ||
$1 == "'_SUBIL"    ||
$1 == "'_SUBIBN"   ||
$1 == "'_SUBIWN"   ||
$1 == "'_SUBILN"   ||
$1 == "'_SUBAB"    ||
$1 == "'_SUBAW"    ||
$1 == "'_SUBAL"    ||
$1 == "'_SUBABTN"  ||
$1 == "'_SUBAWTN"  ||
$1 == "'_SUBALTN"  ||
$1 == "'_SUBABFN"  ||
$1 == "'_SUBAWFN"  ||
$1 == "'_SUBALFN"  ||
$1 == "'_SUBABDN"  ||
$1 == "'_SUBAWDN"  ||
$1 == "'_SUBALDN"  ||
$1 == "'_TESTMEM"  ||
$1 == "'_WAIT_FC"  ||
$1 == "'_WAIT_DMA" ||
$1 == "'_ANDIB"    ||
$1 == "'_ANDIW"    ||
$1 == "'_ANDIL"    ||
$1 == "'_ANDAB"    ||
$1 == "'_ANDAW"    ||
$1 == "'_ANDAL"    ||
$1 == "'_TEST"     { printf("DACCMD '%s'\t= %s\n", substr($1, 3), $3) }
