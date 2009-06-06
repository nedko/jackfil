/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2006,2007,2008,2009 Nedko Arnaudov <nedko@arnaudov.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *****************************************************************************/

#ifndef LV2FILTER_H__6EC1E456_7DD7_4536_B8D3_F23BE4583A23__INCLUDED
#define LV2FILTER_H__6EC1E456_7DD7_4536_B8D3_F23BE4583A23__INCLUDED

#define LV2FILTER_MONO_URI   "http://nedko.aranaudov.org/soft/filter/1/mono"
#define LV2FILTER_STEREO_URI "http://nedko.aranaudov.org/soft/filter/1/stereo"

LV2_Handle
lv2filter_instantiate(
  const LV2_Descriptor * descriptor,
  double sample_rate,
  const char * bundle_path,
  const LV2_Feature * const * features);

void
lv2filter_connect_port(
  LV2_Handle instance,
  uint32_t port,
  void * data_location);

void
lv2filter_run(
  LV2_Handle instance,
  uint32_t samples_count);

void
lv2filter_cleanup(
  LV2_Handle instance);

const void *
lv2filter_extension_data(
  const char * URI); 

#endif /* #ifndef LV2FILTER_H__6EC1E456_7DD7_4536_B8D3_F23BE4583A23__INCLUDED */
