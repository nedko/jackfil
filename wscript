#! /usr/bin/env python
# encoding: utf-8

# the following two variables are used by the target "waf dist"
VERSION='2.0'
APPNAME='jackfil'

# these variables are mandatory ('/' are converted automatically)
srcdir = '.'
blddir = 'build'

from waftoolchainflags import WafToolchainFlags

def options(opt):
    opt.load('compiler_c')
    opt.load('wafautooptions')

    opt.add_auto_option(
        'devmode',
        help='Enable devmode', # enable warnings and treat them as errors
        conf_dest='BUILD_DEVMODE',
        default=False,
    )

    opt.add_auto_option(
        'debug',
        help='Enable debug symbols',
        conf_dest='BUILD_DEBUG',
        default=False,
    )

def configure(conf):
    conf.load('compiler_c')
    conf.load('wafautooptions')

    flags = WafToolchainFlags(conf)

    flags.add_c('-std=gnu99')
    if conf.env['BUILD_DEVMODE']:
        flags.add_c(['-Wall', '-Wextra'])
        #flags.add_c('-Wpedantic')
        flags.add_c('-Werror')
        flags.add_c(['-Wno-variadic-macros', '-Wno-gnu-zero-variadic-macro-arguments'])

        # https://wiki.gentoo.org/wiki/Modern_C_porting
        if conf.env['CC'] == 'clang':
            flags.add_c('-Wno-unknown-argumemt')
            flags.add_c('-Werror=implicit-function-declaration')
            flags.add_c('-Werror=incompatible-function-pointer-types')
            flags.add_c('-Werror=deprecated-non-prototype')
            flags.add_c('-Werror=strict-prototypes')
            if int(conf.env['CC_VERSION'][0]) < 16:
                flags.add_c('-Werror=implicit-int')
        else:
            flags.add_c('-Wno-unknown-warning-option')
            flags.add_c('-Werror=implicit-function-declaration')
            flags.add_c('-Werror=implicit-int')
            flags.add_c('-Werror=incompatible-pointer-types')
            flags.add_c('-Werror=strict-prototypes')
    if conf.env['BUILD_DEBUG']:
        flags.add_c(['-O0', '-g', '-fno-omit-frame-pointer'])
        flags.add_link('-g')

    flags.flush()

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
