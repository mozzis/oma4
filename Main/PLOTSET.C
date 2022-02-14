#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "plotbox.h"


struct plot_box THISBOX;
char sbuf[80];

void ploc(char * str, int offval, int szval)
{
  strcpy(sbuf, str);
  strcat(sbuf,"%4u  %4u  %4x\n");
  printf(sbuf, szval, offval, offval);

}
  
int main(void)
{
  printf("  field name       size  dec   hex\n\n");
  printf("Plotbox:\n");
  ploc("plotbox..........", offsetof(PLOTBOX, plotboxSpareOne), sizeof(THISBOX.plotboxSpareOne));
  ploc("title............", offsetof(PLOTBOX, title),   sizeof(THISBOX.title));
  ploc("xscale...........", offsetof(PLOTBOX, xscale),  sizeof(THISBOX.xscale));
  ploc("yscale...........", offsetof(PLOTBOX, yscale),  sizeof(THISBOX.yscale));
  ploc("fullarea.........", offsetof(PLOTBOX, fullarea),sizeof(THISBOX.fullarea));
  ploc("plotarea.........", offsetof(PLOTBOX, plotarea),sizeof(THISBOX.plotarea));
  ploc("x AXIS...........", offsetof(PLOTBOX, x),       sizeof(THISBOX.x));

  ploc("  x legend.......", offsetof(PLOTBOX, x.legend),sizeof(THISBOX.x.legend));
  ploc("  axis_end_offset",  offsetof(PLOTBOX, x.axis_end_offset), sizeof(THISBOX.x.axis_end_offset));
  ploc("  axis_zero......",  offsetof(PLOTBOX, x.axis_zero), sizeof(THISBOX.x.axis_zero));
  ploc("  max_value......",  offsetof(PLOTBOX, x.max_value), sizeof(THISBOX.x.max_value));
  ploc("  min_value......",  offsetof(PLOTBOX, x.min_value), sizeof(THISBOX.x.min_value));
  ploc("  orig_max_value.",  offsetof(PLOTBOX, x.original_max_value), sizeof(THISBOX.x.original_max_value));
  ploc("  orig_min_value.",  offsetof(PLOTBOX, x.original_min_value), sizeof(THISBOX.x.original_min_value));
  ploc("  inv_range......",  offsetof(PLOTBOX, x.inv_range), sizeof(THISBOX.x.inv_range));
  ploc("  ascending......",  offsetof(PLOTBOX, x.ascending), sizeof(THISBOX.x.ascending));
  ploc("  units..........",  offsetof(PLOTBOX, x.units), sizeof(THISBOX.x.units));

  ploc("y AXIS...........", offsetof(PLOTBOX, y), sizeof(THISBOX.y));

  ploc("  y legend.......", offsetof(PLOTBOX, y.legend), sizeof(THISBOX.y.legend));
  ploc("  axis_end_offset",  offsetof(PLOTBOX, y.axis_end_offset), sizeof(THISBOX.y.axis_end_offset));
  ploc("  axis_zero......",  offsetof(PLOTBOX, y.axis_zero), sizeof(THISBOX.y.axis_zero));
  ploc("  max_value......",  offsetof(PLOTBOX, y.max_value), sizeof(THISBOX.y.max_value));
  ploc("  min_value......",  offsetof(PLOTBOX, y.min_value), sizeof(THISBOX.y.min_value));
  ploc("  orig_max_value.",  offsetof(PLOTBOX, y.original_max_value), sizeof(THISBOX.y.original_max_value));
  ploc("  orig_min_value.",  offsetof(PLOTBOX, y.original_min_value), sizeof(THISBOX.y.original_min_value));
  ploc("  inv_range......",  offsetof(PLOTBOX, y.inv_range), sizeof(THISBOX.y.inv_range));
  ploc("  ascending......",  offsetof(PLOTBOX, y.ascending), sizeof(THISBOX.y.ascending));
  ploc("  units..........",  offsetof(PLOTBOX, y.units), sizeof(THISBOX.y.units));


  ploc("z AXIS...........", offsetof(PLOTBOX, z),       sizeof(THISBOX.z));

  ploc("  z legend.......", offsetof(PLOTBOX, z.legend),sizeof(THISBOX.z.legend));
  ploc("  axis_end_offset",  offsetof(PLOTBOX, z.axis_end_offset), sizeof(THISBOX.z.axis_end_offset));
  ploc("  axis_zero......",  offsetof(PLOTBOX, z.axis_zero), sizeof(THISBOX.z.axis_zero));
  ploc("  max_value......",  offsetof(PLOTBOX, z.max_value), sizeof(THISBOX.z.max_value));
  ploc("  min_value......",  offsetof(PLOTBOX, z.min_value), sizeof(THISBOX.z.min_value));
  ploc("  orig_max_value.",  offsetof(PLOTBOX, z.original_max_value), sizeof(THISBOX.z.original_max_value));
  ploc("  orig_min_value.",  offsetof(PLOTBOX, z.original_min_value), sizeof(THISBOX.z.original_min_value));
  ploc("  inv_range......",  offsetof(PLOTBOX, z.inv_range), sizeof(THISBOX.z.inv_range));
  ploc("  ascending......",  offsetof(PLOTBOX, z.ascending), sizeof(THISBOX.z.ascending));
  ploc("  units..........",  offsetof(PLOTBOX, z.units), sizeof(THISBOX.z.units));


  ploc("xz_percent.......", offsetof(PLOTBOX, xz_percent), sizeof(THISBOX.xz_percent));
  ploc("yz_percent.......", offsetof(PLOTBOX, yz_percent), sizeof(THISBOX.yz_percent));
  ploc("z_position.......", offsetof(PLOTBOX, z_position),  sizeof(THISBOX.z_position));
  ploc("background color.", offsetof(PLOTBOX, background_color), sizeof(THISBOX.background_color));
  ploc("box_color........", offsetof(PLOTBOX, box_color),  sizeof(THISBOX.box_color));
  ploc("text_color.......", offsetof(PLOTBOX, text_color), sizeof(THISBOX.text_color));
  ploc("grid_color.......", offsetof(PLOTBOX, grid_color), sizeof(THISBOX.grid_color));
  ploc("grid line type...", offsetof(PLOTBOX, grid_line_type), sizeof(THISBOX.grid_line_type));
  ploc("plot color.......", offsetof(PLOTBOX, plot_color), sizeof(THISBOX.plot_color));
  ploc("plot line type...", offsetof(PLOTBOX, plot_line_type), sizeof(THISBOX.plot_line_type));
  ploc("style............", offsetof(PLOTBOX, style), sizeof(THISBOX.style));
  ploc("plot_peak_labels.", offsetof(PLOTBOX, plot_peak_labels), sizeof(THISBOX.plot_peak_labels));
  ploc("color key width..", offsetof(PLOTBOX, colorKeyWidth), sizeof(THISBOX.colorKeyWidth));

  return(0);

}
