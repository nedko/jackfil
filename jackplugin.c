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
#include <unistd.h>
#include <lv2.h>

#include "lv2_ui.h"
#include "lv2_external_ui.h"

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

int
main(void)
{
  const LV2_Descriptor * descr_ptr;
  LV2_Handle h;

  descr_ptr = lv2_descriptor(1);

  if (descr_ptr == NULL) return 1;

  h = descr_ptr->instantiate(
    descr_ptr,
    /* FIXME: sample_rate */ 48000,
    /* FIXMEL: bundle path for UI */ "./",
    NULL);
  if (h == NULL)
  {
    LOG_ERROR("Instantiation failed.");
    return 1;
  }

  const LV2UI_Descriptor * ui_ptr;

  ui_ptr = lv2ui_descriptor(0);

  LV2UI_Handle ui;
  LV2UI_Widget widget;
  ui = ui_ptr->instantiate(
    ui_ptr,
    "plugin_uri",
    /* FIXME: bundle path */ "./",
    /* FIXME: write_function */ NULL,
    /* FIXME: controller */ NULL,
    &widget,
    /* FIXME: const LV2_Feature* const*       features */ NULL);

  struct lv2_external_ui * eui_ptr = (struct lv2_external_ui *)widget;

  eui_ptr->show(eui_ptr);

  while (1)
  {
    eui_ptr->run(eui_ptr);
    usleep(10 * 1000);
  }

  descr_ptr->cleanup(h);
}
