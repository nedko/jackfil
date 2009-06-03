#! /usr/bin/env python
# encoding: utf-8

# TODO: check these flags and how to add them to waf
# LIBRARIES = -DPIC -Wall
# CFLAGS := -g -fPIC -DPIC -Wall -Werror

# the following two variables are used by the target "waf dist"
VERSION='1'
APPNAME='lv2fil'

# these variables are mandatory ('/' are converted automatically)
srcdir = '.'
blddir = 'build'

def set_options(opt):
    opt.parser.remove_option('--prefix') # prefix as commonly used concept has no use here, so we remove it to not add confusion
    opt.tool_options('compiler_cc')
    opt.tool_options('lv2plugin', tooldir='.')

def configure(conf):
    conf.check_tool('compiler_cc')
    conf.check_tool('lv2plugin', tooldir='.')

    conf.check_pkg('lv2core', mandatory=True)

def build(bld):
    filter = bld.create_obj('lv2plugin', type='cc')
    filter.uselib = 'LV2CORE'
    filter.target = 'filter'
    filter.ttl = ['filter.ttl', 'manifest.ttl']
    filter.source = ['filter.c', 'lv2filter.c', 'lv2plugin.c', 'log.c']
