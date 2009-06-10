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

#include <stdlib.h>
#include <lv2.h>

#include "lv2filter.h"
//#define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"

static LV2_Descriptor g_lv2_plugins[] =
{
  {
    .URI = LV2FILTER_MONO_URI,
    .instantiate = lv2filter_instantiate,
    .connect_port = lv2filter_connect_port,
    .run = lv2filter_run,
    .cleanup = lv2filter_cleanup,
    .extension_data = lv2filter_extension_data
  },
  {
    .URI = LV2FILTER_STEREO_URI,
    .instantiate = lv2filter_instantiate,
    .connect_port = lv2filter_connect_port,
    .run = lv2filter_run,
    .cleanup = lv2filter_cleanup,
    .extension_data = lv2filter_extension_data
  },
  {
    .URI = NULL
  }
};

static int g_lv2_plugins_count;

void lv2_initialise() __attribute__((constructor));
void lv2_initialise()
{
  const LV2_Descriptor * descr_ptr;

  LOG_DEBUG("lv2_initialise() called.");

  descr_ptr = g_lv2_plugins;

  while (descr_ptr->URI != NULL)
  {
    g_lv2_plugins_count++;
    descr_ptr++;
  }
}

const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
  LOG_DEBUG("lv2_descriptor(%u) called.", (unsigned int)index);

  if (index >= g_lv2_plugins_count)
  {
    LOG_DEBUG("plugin at index %u not found.", (unsigned int)index);
    return NULL;
  }

  LOG_DEBUG("<%s> found.", g_lv2_plugins[index].URI);
  return g_lv2_plugins + index;
}
