/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 * Copyright (C) 2012 Nedko Arnaudov <nedko@arnaudov.name>
 * Copyright (C) 2006 Leonard Ritter <contact@leonard-ritter.com>
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
 *  Knob widget - implementation
 *
 *****************************************************************************/

#include "knob.h"

#if 0
def map_coords_linear(x,y):
    return x,1.0-y

def map_coords_spheric(x,y):
    nx = cos(x * 2 * pi) * y
    ny = -sin(x * 2 * pi) * y
    return nx, ny

def get_peaks(f, tolerance=0.01, maxd=0.01, mapfunc=map_coords_linear):
    corners = 360
    yc = 1.0/corners
    peaks = []
    x0,y0 = 0.0,0.0
    t0 = -9999.0
    i0 = 0
    for i in xrange(int(corners)):
        p = i*yc
        a = f(p)
        x,y = mapfunc(p, a)
        if i == 0:
            x0,y0 = x,y
        t = atan2((y0 - y), (x0 - x)) / (2*pi)
        td = t - t0
        if (abs(td) >= tolerance):
            t0 = t
            peaks.append((x,y))
        x0,y0 = x,y
    return peaks

def make_knobshape(gaps, gapdepth):
    def knobshape_func(x):
        x = (x*gaps)%1.0
        w = 0.5
        g1 = 0.5 - w*0.5
        g2 = 0.5 + w*0.5
        if (x >= g1) and (x < 0.5):
            x = (x-g1)/(w*0.5)
            return 0.5 - gapdepth * x * 0.9
        elif (x >= 0.5) and (x < g2):
            x = (x-0.5)/(w*0.5)
            return 0.5 - gapdepth * (1-x) * 0.9
        else:
            return 0.5
    return get_peaks(knobshape_func, 0.03, 0.05, map_coords_spheric)

def hls_to_color(h,l,s):
    r,g,b = hls_to_rgb(h,l,s)
    return gtk.gdk.color_parse('#%04X%04X%04X' % (int(r*65535),int(g*65535),int(b*65535)))

def color_to_hls(color):
    string = color.to_string()
    r = int(string[1:5], 16) / 65535.0
    g = int(string[5:9], 16) / 65535.0
    b = int(string[9:13], 16) / 65535.0
    return rgb_to_hls(r, g, b)

MARKER_NONE = ''
MARKER_LINE = 'line'
MARKER_ARROW = 'arrow'
MARKER_DOT = 'dot'

LEGEND_NONE = ''
LEGEND_DOTS = 'dots' # painted dots
LEGEND_LINES = 'lines' # painted ray-like lines
LEGEND_RULER = 'ruler' # painted ray-like lines + a circular one
LEGEND_RULER_INWARDS = 'ruler-inwards' # same as ruler, but the circle is on the outside
LEGEND_LED_SCALE = 'led-scale' # an LCD scale
LEGEND_LED_DOTS = 'led-dots' # leds around the knob
#endif

#if 0
class KnobTooltip:
    def __init__(self):
        self.tooltip_window = gtk.Window(gtk.WINDOW_POPUP)
        self.tooltip = gtk.Label()
        #self.tooltip.modify_fg(gtk.STATE_NORMAL, hls_to_color(0.0, 1.0, 0.0))
        self.tooltip_timeout = None
        vbox = gtk.VBox()
        vbox2 = gtk.VBox()
        vbox2.add(self.tooltip)
        vbox2.set_border_width(2)
        vbox.add(vbox2)
        self.tooltip_window.add(vbox)
        vbox.connect('expose-event', self.on_tooltip_expose)

    def show_tooltip(self, knob):
        text = knob.format_value()
        rc = knob.get_allocation()
        x,y = knob.window.get_origin()
        self.tooltip_window.show_all()
        w,h = self.tooltip_window.get_size()
        wx,wy = x+rc.x-w, y+rc.y+rc.height/2-h/2
        self.tooltip_window.move(wx,wy)
        rc = self.tooltip_window.get_allocation()
        self.tooltip_window.window.invalidate_rect((0,0,rc.width,rc.height), False)
        self.tooltip.set_text(text)
        if self.tooltip_timeout:
            gobject.source_remove(self.tooltip_timeout)
        self.tooltip_timeout = gobject.timeout_add(500, self.hide_tooltip)

    def hide_tooltip(self):
        self.tooltip_window.hide_all()

    def on_tooltip_expose(self, widget, event):
        ctx = widget.window.cairo_create()
        rc = widget.get_allocation()
        #ctx.set_source_rgb(*hls_to_rgb(0.0, 0.0, 0.5))
        #ctx.paint()
        ctx.set_source_rgb(*hls_to_rgb(0.0, 0.0, 0.5))
        ctx.translate(0.5, 0.5)
        ctx.set_line_width(1)
        ctx.rectangle(rc.x, rc.y,rc.width-1,rc.height-1)
        ctx.stroke()
        return False



knob_tooltip = None
def get_knob_tooltip():
    global knob_tooltip
    if not knob_tooltip:
        knob_tooltip = KnobTooltip()
    return knob_tooltip
#endif

#if 0
class Knob(gtk.VBox):
    def __init__(self):
        gtk.VBox.__init__(self)
        self.gapdepth = 4
        self.gaps = 10
        self.value = 0.0
        self.min_value = 0.0
        self.max_value = 127.0
        self.fg_hls = 0.0, 0.7, 0.0
        self.legend_hls = None
        self.dragging = False
        self.start = 0.0
        self.digits = 2
        self.segments = 13
        self.label = ''
        self.marker = MARKER_LINE
        self.angle = (3.0/4.0) * 2 * pi
        self.knobshape = None
        self.legend = LEGEND_DOTS
        self.lsize = 2
        self.lscale = False
        self.set_double_buffered(True)
        self.connect('realize', self.on_realize)
        self.connect("size_allocate", self.on_size_allocate)
        self.connect('expose-event', self.on_expose)
        self.set_border_width(6)
        self.set_size_request(50, 50)
        self.tooltip_enabled = False
        self.adj = None

    def set_adjustment(self, adj):
        self.min_value = 0.0
        self.max_value = 1.0
        self.value = adj.get_normalized_value()
        if self.adj:
            self.adj.disconnect(self.adj_id)
        self.adj = adj
        self.adj_id = adj.connect("value-changed", self.on_adj_value_changed)

    def is_sensitive(self):
        return self.get_property("sensitive")

    def format_value(self):
        if self.adj:
            value = self.adj.value
        else:
            value = self.value
        return ("%%.%if" % self.digits) % value

    def show_tooltip(self):
        if self.tooltip_enabled:
            get_knob_tooltip().show_tooltip(self)

    def on_realize(self, widget):
        self.root = self.get_toplevel()
        self.root.add_events(gtk.gdk.ALL_EVENTS_MASK)
        self.root.connect('scroll-event', self.on_mousewheel)
        self.root.connect('button-press-event', self.on_left_down)
        self.root.connect('button-release-event', self.on_left_up)
        self.root.connect('motion-notify-event', self.on_motion)
        self.update_knobshape()

    def update_knobshape(self):
        rc = self.get_allocation()
        b = self.get_border_width()
        size = min(rc.width, rc.height) - 2*b
        gd = float(self.gapdepth*0.5) / size
        self.gd = gd
        self.knobshape = make_knobshape(self.gaps, gd)

    def set_legend_scale(self, scale):
        self.lscale = scale
        self.refresh()

    def set_legend_line_width(self, width):
        self.lsize = width
        self.refresh()

    def set_segments(self, segments):
        self.segments = segments
        self.refresh()

    def set_marker(self, marker):
        self.marker = marker
        self.refresh()

    def set_range(self, minvalue, maxvalue):
        self.min_value = minvalue
        self.max_value = maxvalue
        self.set_value(self.value)

    def quantize_value(self, value):
        scaler = 10**self.digits
        value = int((value*scaler)+0.5) / float(scaler)
        return value

    def on_adj_value_changed(self, adj):
        new_value = adj.get_normalized_value()
        if self.value != new_value:
            self.value = new_value
            self.refresh()

    def set_value(self, value):
        oldval = self.value
        self.value = min(max(self.quantize_value(value), self.min_value), self.max_value)
        if self.value != oldval:
            if self.adj:
                self.adj.set_normalized_value(value)
            self.refresh()

    def get_value(self):
        return self.value

    def set_top_color(self, h, l, s):
        self.fg_hls = h,l,s
        self.refresh()

    def set_legend_color(self, h, l, s):
        self.legend_hls = h,l,s
        self.refresh()

    def get_top_color(self):
        return self.fg_hls

    def set_gaps(self, gaps):
        self.gaps = gaps
        self.knobshape = None
        self.refresh()

    def get_gaps(self):
        return self.gaps

    def set_gap_depth(self, gapdepth):
        self.gapdepth = gapdepth
        self.knobshape = None
        self.refresh()

    def get_gap_depth(self):
        return self.gapdepth

    def set_angle(self, angle):
        self.angle = angle
        self.refresh()

    def get_angle(self):
        return self.angle

    def set_legend(self, legend):
        self.legend = legend
        self.refresh()

    def get_legend(self):
        return self.legend

    def on_left_down(self, widget, event):
        #print "on_left_down"

        # dont drag insensitive widgets
        if not self.is_sensitive():
            return False

        if not sum(self.get_allocation().intersect((int(event.x), int(event.y), 1, 1))):
            return False
        if event.button == 1:
            #print "start draggin"
            self.startvalue = self.value
            self.start = event.y
            self.dragging = True
            self.show_tooltip()
            self.grab_add()
            return True
        return False

    def on_left_up(self, widget, event):
        #print "on_left_up"
        if not self.dragging:
            return False
        if event.button == 1:
            #print "stop draggin"
            self.dragging = False
            self.grab_remove()
            return True
        return False

    def on_motion(self, widget, event):
        #print "on_motion"

        # dont drag insensitive widgets
        if not self.is_sensitive():
            return False

        if self.dragging:
            x,y,state = self.window.get_pointer()
            rc = self.get_allocation()
            range = self.max_value - self.min_value
            scale = rc.height
            if event.state & gtk.gdk.SHIFT_MASK:
                scale = rc.height*8
            value = self.startvalue - ((y - self.start)*range)/scale
            oldval = self.value
            self.set_value(value)
            self.show_tooltip()
            if oldval != self.value:
                self.start = y
                self.startvalue = self.value
            return True
        return False

    def on_mousewheel(self, widget, event):

        # dont move insensitive widgets
        if not self.is_sensitive():
            return False

        if not sum(self.get_allocation().intersect((int(event.x), int(event.y), 1, 1))):
            return
        range = self.max_value - self.min_value
        minstep = 1.0 / (10**self.digits)
        if event.state & (gtk.gdk.SHIFT_MASK | gtk.gdk.BUTTON1_MASK):
            step = minstep
        else:
            step = max(self.quantize_value(range/25.0), minstep)
        value = self.value
        if event.direction == gtk.gdk.SCROLL_UP:
            value += step
        elif event.direction == gtk.gdk.SCROLL_DOWN:
            value -= step
        self.set_value(value)
        self.show_tooltip()

    def on_size_allocate(self, widget, allocation):
        #print allocation.x, allocation.y, allocation.width, allocation.height
        self.update_knobshape()

    def draw_points(self, ctx, peaks):
        ctx.move_to(*peaks[0])
        for peak in peaks[1:]:
            ctx.line_to(*peak)

    def draw(self, ctx):
        if not self.legend_hls:
            self.legend_hls = color_to_hls(self.style.fg[gtk.STATE_NORMAL])

        if not self.knobshape:
            self.update_knobshape()
        startangle = pi*1.5 - self.angle*0.5
        angle = ((self.value - self.min_value) / (self.max_value - self.min_value)) * self.angle + startangle
        rc = self.get_allocation()
        size = min(rc.width, rc.height)

        kh = self.get_border_width() # knob height

        ps = 1.0/size # pixel size
        ps2 = 1.0 / (size-(2*kh)-1) # pixel size inside knob
        ss = ps * kh # shadow size
        lsize = ps2 * self.lsize # legend line width
        # draw spherical
        ctx.translate(rc.x, rc.y)
        ctx.translate(0.5,0.5)
        ctx.translate(size*0.5, size*0.5)
        ctx.scale(size-(2*kh)-1, size-(2*kh)-1)
        if self.legend == LEGEND_DOTS:
            ctx.save()
            ctx.set_source_rgb(*hls_to_rgb(*self.legend_hls))
            dots = self.segments
            for i in xrange(dots):
                s = float(i)/(dots-1)
                a = startangle + self.angle*s
                ctx.save()
                ctx.rotate(a)
                r = lsize*0.5
                if self.lscale:
                    r = max(r*s,ps2)
                ctx.arc(0.5+lsize, 0.0, r, 0.0, 2*pi)
                ctx.fill()
                ctx.restore()
            ctx.restore()
        elif self.legend in (LEGEND_LINES, LEGEND_RULER, LEGEND_RULER_INWARDS):
            ctx.save()
            ctx.set_source_rgb(*hls_to_rgb(*self.legend_hls))
            dots = self.segments
            n = ps2*(kh-1)
            for i in xrange(dots):
                s = float(i)/(dots-1)
                a = startangle + self.angle*s
                ctx.save()
                ctx.rotate(a)
                r = n*0.9
                if self.lscale:
                    r = max(r*s,ps2)
                ctx.move_to(0.5+ps2+n*0.1, 0.0)
                ctx.line_to(0.5+ps2+n*0.1+r, 0.0)
                ctx.set_line_width(lsize)
                ctx.stroke()
                ctx.restore()
            ctx.restore()
            if self.legend == LEGEND_RULER:
                ctx.save()
                ctx.set_source_rgb(*hls_to_rgb(*self.legend_hls))
                ctx.set_line_width(lsize)
                ctx.arc(0.0, 0.0, 0.5+ps2+n*0.1, startangle, startangle+self.angle)
                ctx.stroke()
                ctx.restore()
            elif self.legend == LEGEND_RULER_INWARDS:
                ctx.save()
                ctx.set_source_rgb(*hls_to_rgb(*self.legend_hls))
                ctx.set_line_width(lsize)
                ctx.arc(0.0, 0.0, 0.5+ps2+n, startangle, startangle+self.angle)
                ctx.stroke()

        # draw shadow only for sensitive widgets that have height
        if self.is_sensitive() and kh:
            ctx.save()
            ctx.translate(ss, ss)
            ctx.rotate(angle)
            self.draw_points(ctx, self.knobshape)
            ctx.close_path()
            ctx.restore()
            ctx.set_source_rgba(0,0,0,0.3)
            ctx.fill()

        if self.legend in (LEGEND_LED_SCALE, LEGEND_LED_DOTS):
            ch,cl,cs = self.legend_hls
            n = ps2*(kh-1)
            ctx.save()
            ctx.set_line_cap(cairo.LINE_CAP_ROUND)
            ctx.set_source_rgb(*hls_to_rgb(ch,cl*0.2,cs))
            ctx.set_line_width(lsize)
            ctx.arc(0.0, 0.0, 0.5+ps2+n*0.5, startangle, startangle+self.angle)
            ctx.stroke()
            ctx.set_source_rgb(*hls_to_rgb(ch,cl,cs))
            if self.legend == LEGEND_LED_SCALE:
                ctx.set_line_width(lsize-ps2*2)
                ctx.arc(0.0, 0.0, 0.5+ps2+n*0.5, startangle, angle)
                ctx.stroke()
            elif self.legend == LEGEND_LED_DOTS:
                dots = self.segments
                dsize = lsize-ps2*2
                seg = self.angle/dots
                endangle = startangle + self.angle
                for i in xrange(dots):
                    s = float(i)/(dots-1)
                    a = startangle + self.angle*s
                    if ((a-seg*0.5) > angle) or (angle == startangle):
                        break
                    ctx.save()
                    ctx.rotate(a)
                    r = dsize*0.5
                    if self.lscale:
                        r = max(r*s,ps2)
                    ctx.arc(0.5+ps2+n*0.5, 0.0, r, 0.0, 2*pi)
                    ctx.fill()
                    ctx.restore()
            ctx.restore()
        pat = cairo.LinearGradient(-0.5, -0.5, 0.5, 0.5)
        pat.add_color_stop_rgb(1.0, 0.2,0.2,0.2)
        pat.add_color_stop_rgb(0.0, 0.3,0.3,0.3)
        ctx.set_source(pat)
        ctx.rotate(angle)
        self.draw_points(ctx, self.knobshape)
        ctx.close_path()
        ctx.fill_preserve()
        ctx.set_source_rgba(0.1,0.1,0.1,1)
        ctx.save()
        ctx.identity_matrix()
        ctx.set_line_width(1.0)
        ctx.stroke()
        ctx.restore()

        ctx.arc(0.0, 0.0, 0.5-self.gd, 0.0, pi*2.0)
        ctx.set_source_rgb(*hls_to_rgb(self.fg_hls[0], max(self.fg_hls[1]*0.4,0.0), self.fg_hls[2]))
        ctx.fill()
        ctx.arc(0.0, 0.0, 0.5-self.gd-ps, 0.0, pi*2.0)
        ctx.set_source_rgb(*hls_to_rgb(self.fg_hls[0], min(self.fg_hls[1]*1.2,1.0), self.fg_hls[2]))
        ctx.fill()
        ctx.arc(0.0, 0.0, 0.5-self.gd-(2*ps), 0.0, pi*2.0)
        ctx.set_source_rgb(*hls_to_rgb(*self.fg_hls))
        ctx.fill()

        # dont draw cap for insensitive widgets
        if not self.is_sensitive():
            return

        #~ ctx.set_line_cap(cairo.LINE_CAP_ROUND)
        #~ ctx.move_to(0.5-0.3-self.gd-ps, 0.0)
        #~ ctx.line_to(0.5-self.gd-ps*5, 0.0)

        if self.marker == MARKER_LINE:
            ctx.set_line_cap(cairo.LINE_CAP_BUTT)
            ctx.move_to(0.5-0.3-self.gd-ps, 0.0)
            ctx.line_to(0.5-self.gd-ps, 0.0)
            ctx.save()
            ctx.identity_matrix()
            ctx.translate(0.5,0.5)
            ctx.set_line_width(5)
            ctx.set_source_rgb(*hls_to_rgb(self.fg_hls[0], min(self.fg_hls[1]*1.2,1.0), self.fg_hls[2]))
            ctx.stroke_preserve()
            ctx.set_line_width(3)
            ctx.set_source_rgb(*hls_to_rgb(self.fg_hls[0], max(self.fg_hls[1]*0.4,0.0), self.fg_hls[2]))
            ctx.stroke()
            ctx.restore()
        elif self.marker == MARKER_DOT:
            ctx.arc(0.5-0.05-self.gd-ps*5, 0.0, 0.05, 0.0, 2*pi)
            ctx.save()
            ctx.identity_matrix()
            ctx.set_source_rgb(*hls_to_rgb(self.fg_hls[0], min(self.fg_hls[1]*1.2,1.0), self.fg_hls[2]))
            ctx.stroke_preserve()
            ctx.set_line_width(1)
            ctx.set_source_rgb(*hls_to_rgb(self.fg_hls[0], max(self.fg_hls[1]*0.4,0.0), self.fg_hls[2]))
            ctx.fill()
            ctx.restore()
        elif self.marker == MARKER_ARROW:
            ctx.set_line_cap(cairo.LINE_CAP_BUTT)
            ctx.move_to(0.5-0.3-self.gd-ps, 0.1)
            ctx.line_to(0.5-0.1-self.gd-ps, 0.0)
            ctx.line_to(0.5-0.3-self.gd-ps, -0.1)
            ctx.close_path()
            ctx.save()
            ctx.identity_matrix()
            #~ ctx.set_source_rgb(*hls_to_rgb(self.fg_hls[0], min(self.fg_hls[1]*1.2,1.0), self.fg_hls[2]))
            #~ ctx.stroke_preserve()
            ctx.set_line_width(1)
            ctx.set_source_rgb(*hls_to_rgb(self.fg_hls[0], max(self.fg_hls[1]*0.4,0.0), self.fg_hls[2]))
            ctx.fill()
            ctx.restore()

    def refresh(self):
        rect = self.get_allocation()
        if self.window:
            self.window.invalidate_rect(rect, False)
        return True

    def on_expose(self, widget, event):
        self.context = self.window.cairo_create()
        self.draw(self.context)
        return False

#endif
