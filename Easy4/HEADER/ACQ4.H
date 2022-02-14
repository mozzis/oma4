/*==========================================================================*/
/*                            ACQ4.H                                        */
/*==========================================================================*/
/* Header of the constants, variables and other headers shared by all the   */
/* files fo the new DILOR software. If you develop your own programs, don't */
/* forget to include it in your home-made files.                            */
/*==========================================================================*/

#include <stdio.h>

#define SECURITY 400.0          /* Security for the foremono.  */

#define LIVE   0
#define CLOSED 1
#define OPEN   2

#define CM_1     0    /* Working units.             */
#define NM       1
#define EV       2
#define DIO      3
#define KBAR     4

#define N_WINDOW 23   /* Number of windows to draw */

#define STD      0
#define UV       1

#define MSU      1
#define MS0      0

#define INSTALLED    1
#define UNINSTALLED   0

#define YES 1
#define NO 0

#define  SIGNAL  12
#define  DARK    10
#define  S_D     8

#define  DAC_12  0
#define  DAC_14  1
#define  DAC_16  2

#define  FORWARD     0x98
#define  BACKWARD    0x88

#define  FOREMONO    0
#define  SPECTRO     1
#define  TRIPLE      2
#define  TIME        2
#define  PHOT_COUNT  3
#define  PULSOR      4
#define  MICRO_P     5

#define  CCD         1
#define  PDA         0
#define  OMA3        0
#define  OMA4        1

#define  MOUSE       1
#define  KEYBOARD    0
#define  LEFT_BUT    1
#define  RIGHT_BUT   2

/* Keys definition */

#define  ESC         27
#define  BCKSPACE    8
#define  SPACE       32
#define  ENTER       13

#define  UP          72
#define  DOWN        80
#define  LEFT        75
#define  RIGHT       77

#define  END         79
#define  INS         82
#define  DEL         83
#define  HOME        71
#define  PG_UP       73
#define  PG_DOWN     81

#define  CTRL_UP      141
#define  CTRL_DOWN    145
#define  CTRL_LEFT    115
#define  CTRL_RIGHT   116
#define  CTRL_END     117
#define  CTRL_PG_UP   132
#define  CTRL_PG_DOWN 118

#define  f1          59
#define  f2          60
#define  f3          61
#define  f4          62
#define  f5          63
#define  f6          64
#define  f7          65
#define  f8          66
#define  f9          67
#define  f0          68


/*============*/
/* CCD-SET4.C */
/*============*/

unsigned char serial_poll_gpib(void);

int ccd_data_acc(int current_acc),
    enter_ccd_accumul(void),
    ccd_data_acc(int current_acc),
    get_cooler_status(void),
    get_dmodel(void),
    get_cooltype(void),
    get_activex(void),
    get_activey(void),
    get_antibloom(void),
    get_shiftmode(void),
    get_outreg(void),
    get_temp(void);

void background_accumul(int recmode),
     background_accumul(int recmode),
     ccd_accumul(int recmode),
     conv_second(char *hh,int second),
     cooler_status(void),
     clear_mem(int memnum),
     disp_time(double *val),
     disp_coolstat(int aa),
     read_ccd_data(),
     send_comnd(int device,char *command),
     set_deltax(void), 
     set_deltay(void), 
     set_da_mode(void),
     set_et(void),
     set_freq(void),
     set_frame(void),
     set_ignores(void), 
     set_slices(void),
     set_slcmode(void),
     set_shiftmode(int mode),
     set_scans(void),
     set_tracks(void),
     set_track_mode(void),
     set_x0(void), 
     set_y0(void),
     set_shutter_forced_mode(void);

/*==================*/
/*   OMA4-ACQ       */
/*==================*/

int oma4_accumul(int recmode);
void read_oma4_data();

/*==================*/
/*   CCD_USE4       */
/*==================*/

void activ_setting_up(void),
     close_error_window(void),
     ccd_init(void),
     displ_error_message(char *message),
     enter_ccd_frame_setup(void),
     enter_trkslc_mode(int x,int y,int trkslc), 
     frame_screen(void),
     frame_description(void),
     frame_verify(char *param,char *paramlim),
     read_frame(void),
     select_setting_up(int color),
     slice_setup(void),
     slice_screen(void),
     slice_description(void),
     track_screen(void),
     track_description(void),
     write_frame(void);
     
/*=============*/
/*  CONFIA.C   */
/*=============*/

int test_m(void);

void able_param(void),
     config_display(void),
     conv_config(int nb,int aff),
     edit_nbr(int length,int x,int y,int a),
     modif_conf(int nb),
     op_window(int nb,int up,int dow,int l,int ch),
     page2(void),
     page3(void),
     set_system_characteristics(void),
     wwindow(int nb,int *insert),
     write_t(int a,int b,int length,int c,char *s);


/*============*/
/*  DISKA.C   */
/*============*/

void read_characteristics(void),
     read_param(void),
     read_coeff(void),
     read_decalage(void),
     read_pulsor_param(void),
     save_characteristics(void),
     save_param(void),
     temp_msu(int num,float *maximum,float *minimum),
     save_m3d_data(),
     save_m3d(int sizex,int sizey),
     save_m3d_maxmin(long *max,long *min);

int save_ms0(int recmode),
    save_msu(int n_domain,int npts_ovlap,float *central_pos,int *cut_flag,float *maximum,float *minimum),
    save_window_msu(int n_domain,float *central_pos,int *zone);

/*============*/
/*  EDITA.C   */
/*============*/

int diode_position(float *nui),
    edit(char *strname,int strleng,int pointpos,int k,int sign),
    edit_param(void);

float central_length(int dionum,float *nurel),
      diode_wavelength(int i);

void edit_integ_time(void),
     edit_string(int length,int x,int y,char *s,int *ins),
     spectral_width(void);

/*===========*/
/*  FILA.C   */
/*===========*/

int dir_exist(char *drive,char *dir),
    free_space_message(void),
    file_exist(char *path),
    protect(int drive,int ecr),
    verify_path(void);
    
/*============*/
/*  FUNC0.C   */
/*============*/

void accumul(int disp,int recmode),
     ask_accu(int mode),
     enter_move(int dir),
     enter_stop(void),
     enter_plot(int type),
     enter_memo(void),
     enter_zoomx(void),
     enter_elevator(int selectmode),
     enter_user(void),
     mospeed(int dii);

int accu_mode(void),
    accu_param(void),
    enter_accu();

/*=============*/
/*  FUNC1.C    */
/*=============*/

void displ_box(int x0,int y0,int x1,int y1,int back_color),
     displ_text(int x,int y,int ind),
     ent_plat_scan();

/*================*/
/*  GPIB-USE.C    */
/*================*/

void clean_gpib(void),
     count_time(unsigned char time),
     gpib_init(void),
     phot_count(void),
     read_data(void),
     setpulsor(char *pulsparam),
     set_integ_time(char *integtime),
     set_microp_time(char *integtime,int naccu),
     set_sr_integ_time(char *integtime),
     stop(int *module);

float read_position(int *module);

char moveb(char vitesse,int *module),
     movef(char vitesse,int *module),
     read_S_D(int current_acc),
     read_mopro(int current_acc),
     speed(float delta);   /* calculates the speed   + -> -    */
     
int bpositionning(float *position),
    positionning(float *position,int *module),
    read_single();

/*============*/
/*  GRAPHA4.C  */
/*============*/

void close_window(int left,int top),
     cur_int(void),
     draw_box(int w_numb,int color),
     draw_screen(void),
     er_cur(),
     erase_spacc(void),
     erase_disp(void),
     init_graph(void),  /* This function must be called before any   */
     int_range(void),
     /*ms_disp(int siz,int maximum,int minimum);*/
     /*ms_disp(int siz,long maximum,long minimum),*/
     ms_disp(int over,int npts_ovlap,float *stp,long maximum,long minimum),
     new_cur(int color,int recmode),
     open_window(int left,int top,int right,int bottom,int color),
     outline_box(int x0,int y0,int len,int wid,int color_cadre,int color_bord),
     param_text(void),
     quick_disp(int disp,int recmode), 
     restore_cur_image(void),
     save_cur_image(void),
     save_cur_image(void),
     screen_ticks(void),
     spacc_disp(int current_acc,int fin_acc,int recmode),
     unzoom(),
     write_text(int color_ground,int x0,int y0,int length,int high,int color_text,char *text);

int next_param(int i),
    prev_param(int i);

/*============*/
/*  MENU.C    */
/*============*/

void adjust(char *c),
     enter_value(int i),
     end_adj(void),
     get_com_spec(char *cs),
     init_adj(void),
     os(void);  /* Appel de MS-DOS */
     
 /*=============*/
/*  MOUSA.C    */
/*=============*/

int cursor_counter(int dir),
    enable_mouse(void),
    test_press(int button),
    test_button(void);

void action(int i),
     cursor_shape(int i),
     init_mouse(void),
     mouse_sensivity(int xratio,int yratio),
     mouse_yclip(int ymin,int ymax),
     mouse_xclip(int xmin,int xmax),
     place_cursor(void),
     read_curs_pos(int *xcoord,int *ycoord),
     set_curs_pos(int xcoord,int ycoord),
     test_mouse(void);

/*=================*/
/*    PARAM.C      */
/*=================*/

void enter_posit(int scan_unit),
     enter_delay(void),
     enter_width(void),
     enter_x(void),
     enter_y(void),
     enter_mode(void),
     enter_units(void),
     enter_char(void),
     enter_time(void),
     init_param(void),
     init_length(void),
     init_var(void),
     set_dark(void),
     set_acq_mode(void),
     set_time(char *cch);
     
int safety_message (float *position);

char *select_par(int ii,int aa,int x,int y);

/*================*/
/*    HPGL.C      */
/*================*/

int config_RS232(void);

char *enter(char *value);

void accu_maxmin(float *accu_maxi,float *accu_mini),
     init_plt(void),
     label(char *texte),
     label_dir(float angle),
     maxmin(int *maxi,int *mini),
     movea(float x, float y),   /* move absolu */
     mover(float x, float y),   /* move relatif */
     output(char *value),
     pen_up(void),
     pen_down(void),
     plt_lineto(float x, float y, int penup),
     quick_plot(int para_flag,int type),
     select_pen(int n);
     
/*===============*/
/*   AUTOREC.C   */
/*===============*/

     
void agree_text(int x,int y,int ind),
     displ_auto_lim(float pos),
     enter_scan(void),
     save_text(int x,int y,int ind),
     yes_no_text(int x,int y,int ind);

int enter_ovlap(float *central_pos,int *n_domain,int *cut_flag,int *acc,int *zone),
    displ_auto_pos(float pos),
    param_agree(void),
    saving_mode(void),
    yes_no(void);
