#! /usr/bin/env python
# encoding: utf-8

# the following two variables are used by the target "waf dist"
VERSION='2.0'
APPNAME='jackfil'

# these variables are mandatory ('/' are converted automatically)
srcdir = '.'
blddir = 'build'

def options(opt):
    # options provided by the modules
    opt.load('compiler_c')
    #opt.load('wafautooptions')

def configure(conf):
    conf.load('compiler_c')

    conf.env.append_unique('LINKFLAGS', '-lm')
    #conf.env['PYTHON'] = getattr(Params.g_options, 'PYTHON')
    #conf.define('PYTHON', conf.env['PYTHON'])
    conf.write_config_header('config.h')
    #lv2plugin.display_msg('Python executable', conf.env['PYTHON'], 'GREEN')

def build(bld):
    # config.h, gitverson.h include path; public headers include path
    includes = [bld.path.get_bld(), "../include"]

    prog = bld(features=['c', 'cprogram'], includes=includes)
    prog.source = [
        'filter.c',
        'lv2filter.c',
        'jackplugin.c',
        'log.c',
        'lv2_ui.c',
        ]
    prog.target = 'jackfil'
    prog.defines = ["HAVE_CONFIG_H"]
