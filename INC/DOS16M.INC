D16INFO STRUC
        version                 DW      ?  ; DOS/16M version * 100
        passdn0                 DW      ?  ; offset to int 0 passdown in 
                                           ; kernel seg 
        passdn255               DW      ?  ; offset of last passdown 
        PassupIntP              DD      ?  ; pointer to passup interrupt index 
        xfer_buff               DD      ?  ; area where data transfers are 
                                           ; buffered 
        xfer_bsize              DW      ?  ; transfer buffer size 
        delay_286               DW      ?  ; switch delay for 286 cpus 
        delay_386               DW      ?  ; switch delay for 386 cpus 
        switch_control          DW      ?  ; switch technique control 
                                           ; flags (see SwCtrl) 
        D16MStkP                DD      ?  ; pointer to D16MoveStack function 
        D16ToRMP                DD      ?  ; pointer to D16ToReal function 
        D16ToPMP                DD      ?  ; rm! pointer to D16ToProtected 
                                           ; function 
        D16rmInP                DD      ?  ; pointer to D16rmInterrupt 
                                           ; function 
        bannerp                 DW      ?  ; true if display standard startup message 
        D16rdata                DW      ?  ; real mode DOS/16M data segment 
        versionp                DD      ?  ; pointer to DOS/16M version 
                                           ; string in kernel data 
        swmode                  DB      ?  ; switch mode number 
        cpumod                  DB      ?  ; cpu type (2 = 286, 3 = 386) 
        copyright               DW      ?  ; offset of DOS/16M copyright 
                                           ; string 
        dbginfop                DW      ?
        D16_intno               DB      ?  ; inter-DOS/16M communications 
        A20                     DB      ?
        A20_rigor               DB      ?
        A20_orig                DB      ?
        last_delseg             DW      ?  ; last deleted segment (for fault 
                                           ; suppression) 
        tsp_ptr                 DD      ?  ; ptr to TSF pointer 
        has_87                  DB      ?  ; true if coprocessor present and 
                                           ; not emulating 
        slow_A20                DB      ?  ; A20 resets not synchronized 
        MSW_bits                DW      ?  ; bits OR-ed into MSW going to 
                                           ; protected mode 
                                           ; !! Note:  you should always 
                                           ; preserve the bits in this word 
                                           ; that you are not using. 
        package_info_p          DD      ?  ; package information 
D16INFO ENDS


DOS16M_ADDR STRUC
        lo      DW      ?
        hi      DW      ?
DOS16M_ADDR ENDS
