int percent(int full,int percentage);
int limit(int value,int min,int max);
void scale_axis(AXISDATA * axis); 
void format_clean_tick_value(double value,char *string);
void select_displayed_magnitude(AXISDATA *axis,int *stepped_exponent,float *tick_scale_factor); 
void exponent_step_text(char *str,int exponent_step);
void draw_x_tick_line_text_and_grid(PLOTBOX *plot, float tick_value,char *text,int first,int last); 
void draw_x_axis(PLOTBOX * plot); 
void draw_y_tick_line_text_and_grid(PLOTBOX * plot, float tick_value,char *text,int first,int last); 
void draw_y_axis(PLOTBOX *plot); 
void draw_plotbox(PLOTBOX * plot); 
void set_plotbox_size(PLOTBOX * plot); 
void create_plotbox(PLOTBOX *plot,int device_handle); 
int position_on_axis(AXISDATA *axis, float value); 
float axis_position_to_value(AXISDATA *axis,int coordinate); 
void set_origin_point(PLOTBOX *plot,float xval,float yval); 
void plot_point(PLOTBOX * plot, float xval, float yval); 
void plot_line(PLOTBOX  *plot,float xval1,float yval1,float xval2,float yval2); 
int array_plot(PLOTBOX *plot,float *xvalues,float *yvalues,int count); 
int ramped_x_array_plot(PLOTBOX * plot,float x_start,float x_step,float *yvalues,int count); 
