/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 * Copyright (C) 2012 Nedko Arnaudov <nedko@arnaudov.name>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 *****************************************************************************/

#define UI_URI        "http://nedko.aranaudov.org/soft/filter/2/gtk2gui"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <locale.h>
#include <errno.h>
#include <gtk/gtk.h>

#include <lv2.h>
#include "lv2_ui.h"

#define KNOB    1
#define TOGGLE  2

struct port
{
  unsigned int index;
  const char * name;
  unsigned int type;
  float min;
  float max;
  const char * unit;
  bool log;
};

struct control
{
  //struct lv2_external_ui virt;  /* WARNING: code assumes this is the first struct member */

  LV2UI_Controller controller;
  LV2UI_Write_Function write_function;

  struct port ports[22];
};

static
void
add_param_box(
  GtkWidget * parent_box,
  GtkWidget * param_box)
{
  GtkWidget * align;

  align = gtk_alignment_new(0.5, 0.5, 1.0, 1.0);
  gtk_alignment_set_padding(GTK_ALIGNMENT(align), 10, 10, 10, 10);
  gtk_container_add(GTK_CONTAINER(align), param_box);

  gtk_box_pack_start(GTK_BOX(parent_box), align, TRUE, FALSE, 0);
}

static
GtkWidget *
create_knob_box(
  struct port * port_ptr)
{
  GtkWidget * param_box;
  //GtkWidget * knob;
  //GtkWidget * align;
  GtkWidget * label;

  param_box = gtk_vbox_new(FALSE, 0);
#if 0
  step = (port['max'] - port['min']) / 100;
  adj = SmartAdjustment(port['log'], port['min'], port['min'], port['max'], step, step * 20);
  adj.port = port;
  port['adj'] = adj;

  //adj.connect("value-changed", self.on_adj_value_changed)

  knob = Knob();
  knob.set_adjustment(adj);
  align = gtk.Alignment(0.5, 0.5, 0, 0);
  align.set_padding(0, 0, 20, 20);
  align.add(knob);
  param_box.pack_start(align, False);

  adj.label = gtk.Label(self.get_adj_value_text(adj)[0]);
  param_box.pack_start(adj.label, False);
  //#spin = gtk.SpinButton(adj, 0.0, 2)
  //#param_box.pack_start(spin, False)
#endif

  label = gtk_label_new(port_ptr->name);
  gtk_box_pack_start(GTK_BOX(param_box), label, FALSE, FALSE, 0);

  return param_box;
}

static
GtkWidget *
create_toggle_box(
  struct port * port_ptr)
{
  GtkWidget * param_box;
  GtkWidget * button;
  GtkWidget * align;

  param_box = gtk_vbox_new(FALSE, 0);
  button = gtk_check_button_new_with_label(port_ptr->name);
  //button.port = port
  //port['widget'] = button

  //button.connect("toggled", self.on_button_toggled)

  align = gtk_alignment_new(0.5, 0.5, 0, 0);
  gtk_container_add(GTK_CONTAINER(align), button);
  gtk_box_pack_start(GTK_BOX(param_box), align, FALSE, FALSE, 0);

  return param_box;
}

#if 0
    def on_about(self, widget):
        about = gtk.AboutDialog()
        about.set_transient_for(self.window)
        about.set_name("4-band parametric filter")
        #about.set_website(program_data['website'])
        about.set_authors(["Nedko Arnaudov - LV2 plugin and GUI", 'Fons Adriaensen - DSP code'])
        about.set_artists(["LV2 logo has been designed by Thorsten Wilms, based on a concept from Peter Shorthose."])
        about.set_logo(self.lv2logo)
        about.show()
        about.run()
        about.hide()

    def get_adj_value_text(self, adj):
        value = adj.get_value()
        if value >= 10000:
            format = "%.0f"
        elif value >= 1000:
            format = "%.1f"
        else:
            format = "%.2f"
        text = format % value
        unit = adj.port['unit']
        if unit:
            text += " " + unit

        return value, text

    def on_adj_value_changed(self, adj):
        value, text = self.get_adj_value_text(adj)
        adj.label.set_text(text)

        if adj.port['index'] == 1:
            #print "Master gain = %.2f dB" % adj.get_value()
            self.fr.set_master_gain(adj.get_value())

        if self.initator:
            #print adj.port, adj.get_value()
            self.send_port_value(adj.port['index'] + self.port_base, value)

    def on_button_toggled(self, widget):
        port_index = widget.port['index']
        band_no = (port_index - 2) / 4 + 1
        if widget.get_active():
            value = 1.0
            if band_no > 0:
                self.fr.enable_filter(str(band_no))
            else:
                self.fr.master_enable()
        else:
            value = 0.0
            if band_no > 0:
                self.fr.disable_filter(str(band_no))
            else:
                self.fr.master_disable()

        if self.initator:
            self.send_port_value(port_index + self.port_base, value)

    def on_port_value_changed(self, port_index, port_value):
        #print "port %d set to %f" % (port_index, port_value)
        port_index -= self.port_base
        port = self.ports[port_index]
        #print repr(port)
        port_type = port['type']
        if port_type == 'knob':
            self.initator = False
            port['adj'].set_value(port_value)
            self.initator = True
        elif port_type == 'toggle':
            if port_value > 0.0:
                toggled = True
            else:
                toggled = False

            self.initator = False
            port['widget'].set_active(toggled)
            self.initator = True
#endif

static
LV2UI_Handle
instantiate(
  const struct _LV2UI_Descriptor * descriptor,
  const char * plugin_uri,
  const char * bundle_path,
  LV2UI_Write_Function write_function,
  LV2UI_Controller controller,
  LV2UI_Widget * widget,
  const LV2_Feature * const * features)
{
  struct control * control_ptr;
  GtkWidget * align;
  GtkWidget * top_vbox;
  GtkWidget * fr;
  GtkWidget * frame;
  GtkWidget * param_hbox;
  GtkWidget * misc_box;
  GtkWidget * master_frame;
  GtkWidget * master_box;
  //GtkWidget * button;
  GtkWidget * button_box;
  GtkWidget * band_frame;
  GtkWidget * band_box;
  struct port * port_ptr;
  unsigned int port_index;
  int band_index, param_index;

  static const struct port band_parameters[4] = {
    {.name = "Active",    .type = TOGGLE},
    {.name = "Frequency", .type = KNOB, .unit = "Hz", .log = true},
    {.name = "Bandwidth", .type = KNOB,               .log = true,  .min = 0.125, .max = 8.0},
    {.name = "Gain",      .type = KNOB, .unit = "dB", .log = false, .min = -20.0, .max = 20.0}};

  static const float freq_min[4] = {  20.0,   40.0,   100.0,   200.0};
  static const float freq_max[4] = {2000.0, 4000.0, 10000.0, 20000.0};

#if 0
  filter_colors = [gtk.gdk.Color(int(65535 * 1.0), int(65535 * 0.6), int(65535 * 0.0)),
                   gtk.gdk.Color(int(65535 * 0.6), int(65535 * 1.0), int(65535 * 0.6)),
                   gtk.gdk.Color(int(65535 * 0.0), int(65535 * 0.6), int(65535 * 1.0)),
                   gtk.gdk.Color(int(65535 * 0.9), int(65535 * 0.0), int(65535 * 0.5))]
#endif

  printf("instantiate('%s', '%s') called\n", plugin_uri, bundle_path);

  control_ptr = malloc(sizeof(struct control));
  if (control_ptr == NULL)
  {
    goto fail;
  }

  control_ptr->controller = controller;
  control_ptr->write_function = write_function;

  top_vbox = gtk_vbox_new(FALSE, 10);

  /* align = gtk_alignment_new(0.5, 0.5, 1.0, 1.0); */
  /* gtk_alignment_set_padding(GTK_ALIGNMENT(align), 10, 10, 10, 10); */
  /* gtk_container_add(GTK_CONTAINER(align), top_vbox); */

  fr = gtk_drawing_area_new();
  gtk_widget_set_size_request(fr, 400, 200);

  frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_OUT);
  gtk_container_add(GTK_CONTAINER(frame), fr);

  gtk_box_pack_start(GTK_BOX(top_vbox), frame, TRUE, TRUE, 0);

  param_hbox = gtk_hbox_new(FALSE, 10);
  gtk_box_pack_start(GTK_BOX(top_vbox), param_hbox, FALSE, FALSE, 0);

  misc_box = gtk_vbox_new(FALSE, 5);

  master_frame = gtk_frame_new("Master");
  gtk_frame_set_label_align(GTK_FRAME(master_frame), 0.5, 0.5);

  master_box = gtk_vbox_new(FALSE, 5);

  port_ptr = control_ptr->ports;
  port_ptr->index = 0;
  port_ptr->name = "Active";
  port_ptr->type = TOGGLE;
  add_param_box(master_box, create_toggle_box(port_ptr));
  port_ptr++;

  port_ptr->index = 1;
  port_ptr->name = "Gain";
  port_ptr->type = KNOB;
  port_ptr->min = -20.0;
  port_ptr->max = 20.0;
  port_ptr->unit = "dB";
  port_ptr->log = false;
  add_param_box(master_box, create_knob_box(port_ptr));
  port_ptr++;

  gtk_container_add(GTK_CONTAINER(master_frame), master_box);
  gtk_box_pack_start(GTK_BOX(misc_box), master_frame, FALSE, FALSE, 0);

  button_box = gtk_vbox_new(FALSE, 0);

#if 0
  button = gtk_button_new_from_stock(GTK_STOCK_ABOUT);
  //button.connect("clicked", self.on_about)
  gtk_box_pack_start(GTK_BOX(button_box), button, FALSE, FALSE, 0);

  button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
  //button.connect("clicked", self.on_window_closed)
  gtk_box_pack_start(GTK_BOX(button_box), button, FALSE, FALSE, 0);
#endif

  align = gtk_alignment_new(0.5, 1.00, 1.0, 1.0);
  gtk_container_add(GTK_CONTAINER(align), button_box);
  gtk_box_pack_start(GTK_BOX(misc_box), align, TRUE, TRUE, 0);

  port_index = 2;

  for (band_index = 0; band_index < 4; band_index++)
  {
    char name[128];
    sprintf(name, "Band %d", band_index + 1);
    band_frame = gtk_frame_new(name);
    gtk_frame_set_label_align(GTK_FRAME(band_frame), 0.5, 0.5);

    band_box = gtk_vbox_new(FALSE, 5);

    for (param_index = 0; param_index < 4; param_index++)
    {
      *port_ptr = band_parameters[param_index];
      port_ptr->index = port_index++;

      if (param_index == 1)     /* frequency */
      {
        port_ptr->min = freq_min[band_index];
        port_ptr->max = freq_max[band_index];
      }

      //param_box.set_spacing(5)
      if (port_ptr->type == KNOB)
        add_param_box(band_box, create_knob_box(port_ptr));
      else if (port_ptr->type == TOGGLE)
        add_param_box(band_box, create_toggle_box(port_ptr));

      port_ptr++;
  }

#if 0
    self.fr.add_filter(
      str(i + 1),
      self.ports[port_index - 3]['adj'], # frequency
      self.ports[port_index - 1]['adj'], # gain
      self.ports[port_index - 2]['adj'], # bandwidth
      filter_colors[i]);
#endif

    gtk_container_add(GTK_CONTAINER(band_frame), band_box);
    gtk_box_pack_start(GTK_BOX(param_hbox), band_frame, TRUE, TRUE, 0);
  }

  gtk_box_pack_start(GTK_BOX(param_hbox), misc_box, TRUE, TRUE, 0);

  *widget = (LV2UI_Widget)top_vbox;//gtk_label_new("blablabla");

  return control_ptr;

//fail_free_control:
  free(control_ptr);

fail:
  fprintf(stderr, "lv2fil UI launch failed\n");
  return NULL;
}

#define control_ptr ((struct control *)ui)

static
void
cleanup(
  LV2UI_Handle ui)
{
  //printf("cleanup() called\n");
  free(control_ptr);
}

static
void
port_event(
  LV2UI_Handle ui,
  uint32_t port_index,
  uint32_t buffer_size,
  uint32_t format,
  const void * buffer)
{
  //printf("port_event(%u, %f) called\n", (unsigned int)port_index, *(float *)buffer);
}

#undef control_ptr

static LV2UI_Descriptor descriptors[] =
{
  {UI_URI, instantiate, cleanup, port_event, NULL}
};

const LV2UI_Descriptor *
lv2ui_descriptor(
  uint32_t index)
{
  //printf("lv2ui_descriptor(%u) called\n", (unsigned int)index);

  if (index >= sizeof(descriptors) / sizeof(descriptors[0]))
  {
    return NULL;
  }

  return descriptors + index;
}
