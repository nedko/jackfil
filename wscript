#! /usr/bin/env python
# encoding: utf-8

# TODO: check these flags and how to add them to waf
# LIBRARIES = -DPIC -Wall
# CFLAGS := -g -fPIC -DPIC -Wall -Werror

import sys
import Params
import lv2plugin

# the following two variables are used by the target "waf dist"
VERSION='2.0'
APPNAME='lv2fil'

# these variables are mandatory ('/' are converted automatically)
srcdir = '.'
blddir = 'build'

def set_options(opt):
    opt.parser.remove_option('--prefix') # prefix as commonly used concept has no use here, so we remove it to not add confusion
    opt.add_option('--python', type='string', default=sys.executable, dest='PYTHON', help='Path to python executable.')
    opt.tool_options('compiler_cc')
    opt.tool_options('lv2plugin', tooldir='.')

def configure(conf):
    conf.check_tool('compiler_cc')
    conf.check_tool('lv2plugin', tooldir='.')

    if not conf.check_pkg('lv2core', mandatory=False):
        conf.check_pkg('lv2', mandatory=True, destvar='LV2CORE')

    conf.env.append_unique('LINKFLAGS', '-lm')
    conf.env['PYTHON'] = getattr(Params.g_options, 'PYTHON')
    conf.define('PYTHON', conf.env['PYTHON'])
    conf.write_config_header('config.h')
    lv2plugin.display_msg('Python executable', conf.env['PYTHON'], 'GREEN')

def build(bld):
    filter = bld.create_obj('lv2plugin', type='cc')
    filter.uselib = 'LV2CORE'
    filter.target = 'filter'
    filter.ttl = ['filter.ttl', 'manifest.ttl', 'ui', 'lv2logo.png']
    filter.source = ['filter.c', 'lv2filter.c', 'lv2plugin.c', 'log.c', 'lv2_ui.c']
