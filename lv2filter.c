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
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <lv2.h>

#include "lv2filter.h"
#include "filter.h"
#define LOG_LEVEL LOG_LEVEL_ERROR
#include "log.h"

#define BANDS_COUNT 4

#define LV2_PORT_MONO_AUDIO_IN       0
#define LV2_PORT_MONO_AUDIO_OUT      1
#define LV2_MONO_AUDIO_PORT_COUNT    2
#define LV2_PORTS_COUNT_MONO         (LV2_MONO_AUDIO_PORT_COUNT + GLOBAL_PARAMETERS_COUNT + BANDS_COUNT * BAND_PARAMETERS_COUNT)

#define LV2_PORT_LEFT_AUDIO_IN       0
#define LV2_PORT_RIGHT_AUDIO_IN      1
#define LV2_PORT_LEFT_AUDIO_OUT      2
#define LV2_PORT_RIGHT_AUDIO_OUT     3
#define LV2_STEREO_AUDIO_PORT_COUNT  4
#define LV2_PORTS_COUNT_STEREO       (LV2_STEREO_AUDIO_PORT_COUNT + GLOBAL_PARAMETERS_COUNT + BANDS_COUNT * BAND_PARAMETERS_COUNT)

struct lv2filter
{
  bool stereo;
  filter_handle filter;
  filter_handle filter_right;
  char * bundle_path;
  const float * audio_in;
  const float * audio_in_right;
  float * audio_out;
  float * audio_out_right;
  const LV2_Feature * const * host_features;
};

LV2_Handle
lv2filter_instantiate(
  const LV2_Descriptor * descriptor,
  double sample_rate,
  const char * bundle_path,
  const LV2_Feature * const * host_features)
{
  struct lv2filter * lv2filter_ptr;
  const LV2_Feature * const * feature_ptr_ptr;

  LOG_DEBUG("lv2filter_create_plugin_instance() called.");
  LOG_DEBUG("uri = \"%s\"", descriptor->URI);
  LOG_DEBUG("sample_rate = %f", sample_rate);
  LOG_DEBUG("bundle_path = \"%s\"", bundle_path);

  feature_ptr_ptr = host_features;
  while (*feature_ptr_ptr)
  {
    LOG_DEBUG("Host feature <%s> detected", (*feature_ptr_ptr)->URI);

    feature_ptr_ptr++;
  }

  lv2filter_ptr = malloc(sizeof(struct lv2filter));
  if (lv2filter_ptr == NULL)
  {
    goto fail;
  }

  if (strcmp(descriptor->URI, LV2FILTER_STEREO_URI) == 0)
  {
    lv2filter_ptr->stereo = true;
  }
  else if (strcmp(descriptor->URI, LV2FILTER_MONO_URI) == 0)
  {
    lv2filter_ptr->stereo = false;
  }
  else
  {
    assert(false);
    goto fail_free_instance;
  }

  lv2filter_ptr->host_features = host_features;

  lv2filter_ptr->bundle_path = strdup(bundle_path);
  if (lv2filter_ptr->bundle_path == NULL)
  {
    goto fail_free_instance;
  }

  if (!filter_create(sample_rate, BANDS_COUNT, &lv2filter_ptr->filter))
  {
    goto fail_free_bundle_path;
  }

  if (lv2filter_ptr->stereo)
  {
    if (!filter_create(sample_rate, BANDS_COUNT, &lv2filter_ptr->filter_right))
    {
      goto fail_destroy_filter;
    }
  }

  return (LV2_Handle)lv2filter_ptr;

fail_destroy_filter:
  filter_destroy(lv2filter_ptr->filter);

fail_free_bundle_path:
  free(lv2filter_ptr->bundle_path);

fail_free_instance:
  free(lv2filter_ptr);

fail:
  return NULL;
}

#define lv2filter_ptr ((struct lv2filter *)instance)

/* The run() callback. This is the function that gets called by the host
   when it wants to run the plugin. The parameter is the number of sample
   frames to process. */
void
lv2filter_run(
  LV2_Handle instance,
  uint32_t samples_count)
{
  LOG_DEBUG("lv2filter_run");
  filter_run(
    lv2filter_ptr->filter,
    lv2filter_ptr->audio_in,
    lv2filter_ptr->audio_out,
    samples_count);

  if (lv2filter_ptr->stereo)
  {
    filter_run(
      lv2filter_ptr->filter_right,
      lv2filter_ptr->audio_in_right,
      lv2filter_ptr->audio_out_right,
      samples_count);
  }
}

void
lv2filter_cleanup(
  LV2_Handle instance)
{
  filter_destroy(lv2filter_ptr->filter);

  if (lv2filter_ptr->stereo)
  {
    filter_destroy(lv2filter_ptr->filter_right);
  }

  free(lv2filter_ptr->bundle_path);
  free(lv2filter_ptr);
}

void
lv2filter_connect_port(
  LV2_Handle instance,
  uint32_t port,
  void * data_location)
{
  LOG_DEBUG("lv2filter_connect_port %u %p", (unsigned int)port, data_location);

  if (lv2filter_ptr->stereo)
  {
    if (port >= LV2_PORTS_COUNT_STEREO)
    {
      assert(0);
      return;
    }

    if (port == LV2_PORT_LEFT_AUDIO_IN)
    {
      lv2filter_ptr->audio_in = data_location;
    }
    else if (port == LV2_PORT_LEFT_AUDIO_OUT)
    {
      lv2filter_ptr->audio_out = data_location;
    }
    else if (port == LV2_PORT_RIGHT_AUDIO_IN)
    {
      lv2filter_ptr->audio_in_right = data_location;
    }
    else if (port == LV2_PORT_RIGHT_AUDIO_OUT)
    {
      lv2filter_ptr->audio_out_right = data_location;
    }
    else
    {
      assert(port >= LV2_STEREO_AUDIO_PORT_COUNT);
      port -= LV2_STEREO_AUDIO_PORT_COUNT;
      if (port < GLOBAL_PARAMETERS_COUNT)
      {
        filter_connect_global_parameter(lv2filter_ptr->filter, port, data_location);
        filter_connect_global_parameter(lv2filter_ptr->filter_right, port, data_location);
      }
      else
      {
        assert(port >= GLOBAL_PARAMETERS_COUNT);
        port -= GLOBAL_PARAMETERS_COUNT;

        filter_connect_band_parameter(lv2filter_ptr->filter, port / BANDS_COUNT, port % BANDS_COUNT, data_location);
        filter_connect_band_parameter(lv2filter_ptr->filter_right, port / BANDS_COUNT, port % BANDS_COUNT, data_location);
      }
    }
  }
  else
  {
    if (port >= LV2_PORTS_COUNT_MONO)
    {
      assert(0);
      return;
    }

    if (port == LV2_PORT_MONO_AUDIO_IN)
    {
      lv2filter_ptr->audio_in = data_location;
    }
    else if (port == LV2_PORT_MONO_AUDIO_OUT)
    {
      lv2filter_ptr->audio_out = data_location;
    }
    else
    {
      port -= LV2_MONO_AUDIO_PORT_COUNT;
      if (port < GLOBAL_PARAMETERS_COUNT)
      {
        filter_connect_global_parameter(lv2filter_ptr->filter, port, data_location);
      }
      else
      {
        port -= GLOBAL_PARAMETERS_COUNT;

        filter_connect_band_parameter(lv2filter_ptr->filter, port / BANDS_COUNT, port % BANDS_COUNT, data_location);
      }
    }
  }
}

const void *
lv2filter_extension_data(
  const char * URI)
{
  return NULL;
}
