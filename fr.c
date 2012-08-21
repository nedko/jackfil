/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 * Copyright (C) 2012 Nedko Arnaudov <nedko@arnaudov.name>
 * Filter response code by Fons Adriaensen
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
/*****************************************************************************
 *
 * DESCRIPTION:
 *  Frequency response widget - implementaiton
 *
 *****************************************************************************/

#if 0

#include "fr.h"

class filter_band:
    def __init__(self):
        self.fsamp = 48e3

    def set_params(self, freq, bandw, gain):
        freq_ratio = freq / self.fsamp
        gain2 = pow(10.0, 0.05 * gain)
        b = 7 * bandw * freq_ratio / sqrt(gain2)
        self.gn = 0.5 * (gain2 - 1)
        self.v1 = -cos(2 * pi * freq_ratio)
        self.v2 = (1 - b) / (1 + b)
        self.v1 *= (1 + self.v2)
        self.gn *= (1 - self.v2)

    def get_response(self, freq):
        w = 2 * pi * (freq / self.fsamp)
        c1 = cos(w)
        s1 = sin(w)
        c2 = cos(2 * w)
        s2 = sin(2 * w)

        x = c2 + self.v1 * c1 + self.v2
        y = s2 + self.v1 * s1
        t1 = hypot(x, y)
        x += self.gn * (c2 - 1)
        y += self.gn * s2
        t2 = hypot(x, y)

        #return t2 / t1
        return 20 * log10(t2 / t1)

class frequency_response(gtk.DrawingArea):
    def __init__(self):
        gtk.DrawingArea.__init__(self)

        self.connect("expose-event", self.on_expose)
        self.connect("size-request", self.on_size_request)
        self.connect("size_allocate", self.on_size_allocate)

        self.color_bg = gtk.gdk.Color(0,0,0)
        self.color_value = gtk.gdk.Color(int(65535 * 0.8), int(65535 * 0.7), 0)
        self.color_mark = gtk.gdk.Color(int(65535 * 0.3), int(65535 * 0.3), int(65535 * 0.3))
        self.color_sum = gtk.gdk.Color(int(65535 * 1.0), int(65535 * 1.0), int(65535 * 1.0))
        self.width = 0
        self.height = 0
        self.margin = 10
        self.db_range = 30
        self.master_gain = 0.0
        self.master_enabled = False

        self.filters = {}

    def on_expose(self, widget, event):
        cairo_ctx = widget.window.cairo_create()

        # set a clip region for the expose event
        cairo_ctx.rectangle(event.area.x, event.area.y, event.area.width, event.area.height)
        cairo_ctx.clip()

        self.draw(cairo_ctx)

        return False

    def on_size_allocate(self, widget, allocation):
        #print allocation.x, allocation.y, allocation.width, allocation.height
        self.width = float(allocation.width)
        self.height = float(allocation.height)
        self.font_size = 10

    def on_size_request(self, widget, requisition):
        #print "size-request, %u x %u" % (requisition.width, requisition.height)
        requisition.width = 150
        requisition.height = 150
        return

    def invalidate_all(self):
        self.queue_draw_area(0, 0, int(self.width), int(self.height))

    def get_x(self, hz):
        width = self.width - 3.5 * self.margin
        #x = self.margin + width * (hz - 20) / (20000 - 20)
        x = 2.5 * self.margin + width * log(hz / 20.0, 1000.0)
        #print x
        return x

    def get_freq(self, x):
        width = self.width - 3.5 * self.margin
        return 20 * pow(1000, (x - 2.5 * self.margin) / width)

    def get_y(self, db):
        height = self.height - 2.5 * self.margin
        y = self.margin + height * (self.db_range - db) / (self.db_range * 2)
        #print y
        return y

    def draw_db_grid(self, cairo_ctx, db):
        x = self.get_x(20)
        y = self.get_y(db)
        cairo_ctx.move_to(x, y)
        cairo_ctx.line_to(self.get_x(20000), y)

        if db % 10 == 0:
            x -= 20
            y += 3
            cairo_ctx.move_to(x, y)
            label = "%+d" % db
            if db == 0:
                label = " " + label
            cairo_ctx.show_text(label)

        cairo_ctx.stroke()

    def invalidate_all(self):
        self.queue_draw_area(0, 0, int(self.width), int(self.height))

    def draw(self, cairo_ctx):
        cairo_ctx.select_font_face("Fixed")

        cairo_ctx.set_source_color(self.color_bg)
        cairo_ctx.rectangle(0, 0, self.width, self.height)
        cairo_ctx.fill()

        cairo_ctx.set_source_color(self.color_mark)
        cairo_ctx.set_line_width(1);

        for hz in range(20, 101, 10) + range(100, 1001, 100) + range(1000, 10001, 1000) + range(10000, 20001, 10000):
            if hz >= 10000:
                label = "%dk" % int(hz / 1000)
            elif hz >= 1000:
                label = "%dk" % int(hz / 1000)
            else:
                label = "%d" % int(hz)
            first_digit = int(label[0])
            if first_digit > 5 or (first_digit > 3 and (len(label) == 3)):
                label = None

            x = self.get_x(hz)
            cairo_ctx.move_to(x, self.get_y(self.db_range))
            y = self.get_y(-self.db_range)
            cairo_ctx.line_to(x, y)
            if label:
                y += 10
                if hz == 20000:
                    x -= 15
                elif hz != 20:
                    x -= 3
                cairo_ctx.move_to(x, y)
                cairo_ctx.show_text(label)
            cairo_ctx.stroke()

        for db in range(0, self.db_range + 1, 5):
            self.draw_db_grid(cairo_ctx, db)

            if db != 0:
                self.draw_db_grid(cairo_ctx, -db)

        curves = [[x, {}, self.master_gain, self.get_freq(x)] for x in range(int(self.get_x(20)), int(self.get_x(20e3)))]
        #print repr(curves)

        # calculate filter responses
        for label, filter in self.filters.items():
            if not filter.enabled:
                continue

            for point in curves:
                db = filter.get_response(point[3])
                point[1][label] = [self.get_y(db), db]

        # calculate sum curve
        for point in curves:
            for label, filter_point in point[1].items():
                point[2] += filter_point[1]
            #print point

        # draw filter curves
        for label, filter in self.filters.items():
            if not filter.enabled:
                continue

            cairo_ctx.set_source_color(filter.color)
            cairo_ctx.move_to(curves[0][0], curves[0][1][label][0])
            for point in curves:
                cairo_ctx.line_to(point[0], point[1][label][0])
            cairo_ctx.stroke()

        if self.master_enabled:
            # draw sum curve
            cairo_ctx.set_source_color(self.color_sum)
            cairo_ctx.set_line_width(2);
            cairo_ctx.move_to(curves[0][0], self.get_y(curves[0][2]))
            for point in curves:
                cairo_ctx.line_to(point[0], self.get_y(point[2]))
            cairo_ctx.stroke()

        # draw base point markers
        for label, filter in self.filters.items():
            if not filter.enabled:
                continue

            cairo_ctx.set_source_color(self.color_value)
            x = self.get_x(filter.adj_hz.value)
            y = self.get_y(filter.adj_db.value)

            cairo_ctx.move_to(x, y)
            cairo_ctx.show_text(label)
            cairo_ctx.stroke()

    def add_filter(self, label, adj_hz, adj_db, adj_bw, color):
        #print "filter %s added (%.2f Hz, %.2f dB, %.2f bw)" % (label, adj_hz.value, adj_db.value, adj_bw.value)
        filter = filter_band()
        filter.enabled = False
        filter.label = label
        filter.color = color
        filter.set_params(adj_hz.value, adj_bw.value, adj_db.value)
        adj_hz.filter = filter
        adj_db.filter = filter
        adj_bw.filter = filter
        filter.adj_hz = adj_hz
        filter.adj_db = adj_db
        filter.adj_bw = adj_bw
        adj_hz.connect("value-changed", self.on_value_change_request)
        adj_db.connect("value-changed", self.on_value_change_request)
        adj_bw.connect("value-changed", self.on_value_change_request)
        self.filters[label] = filter

    def enable_filter(self, label):
        filter = self.filters[label]
        #print "filter %s enabled (%.2f Hz, %.2f dB, %.2f bw)" % (label, filter.adj_hz.value, filter.adj_db.value, filter.adj_bw.value)
        filter.enabled = True
        self.invalidate_all()

    def disable_filter(self, label):
        filter = self.filters[label]
        #print "filter %s disabled (%.2f Hz, %.2f dB, %.2f bw)" % (label, filter.adj_hz.value, filter.adj_db.value, filter.adj_bw.value)
        filter.enabled = False
        self.invalidate_all()

    def on_value_change_request(self, adj):
        #print "adj changed"
        adj.filter.set_params(adj.filter.adj_hz.value, adj.filter.adj_bw.value, adj.filter.adj_db.value)
        self.invalidate_all()

    def master_enable(self):
        self.master_enabled = True;
        self.invalidate_all()

    def master_disable(self):
        self.master_enabled = False;
        self.invalidate_all()

    def set_master_gain(self, gain):
        self.master_gain = gain;
        self.invalidate_all()

#endif
