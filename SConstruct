# ***********************************************************************
# * @file      SConstruct
# * @author    Dave Martin, DeVivo AST, Inc.  
# * @date      2008/03/03
# *
# * @attention Copyright (C) 2008
# * @attention DeVivo AST, Inc.
# * @attention All rights reserved
# ************************************************************************



import os
baseEnv = Environment(ENV=os.environ)

# Add the include, library, and bin paths
baseEnv.Append(CPPPATH = ['#include'])
baseEnv['LIBPATH'] = ['#lib']
baseEnv['INSTALL_LIBDIR'] = '#lib'
baseEnv['INSTALL_BINDIR'] = '#bin'

# Some additional stuff to do for CYGWIN
if baseEnv['PLATFORM'] == 'cygwin':
    print "scons: Building for CYGWIN..."
    baseEnv.Append( CCFLAGS = ['-D__CYGWIN__'] )

# Some additional stuff to do if we're building for windows
if os.name == "nt":                     
   print "scons: Building for Windows..."
   baseEnv.Append( CCFLAGS = ['-DWINDOWS', '-EHsc'] )
   baseEnv.Append( CPPPATH = [baseEnv['ENV']['SDKPATH']+"/Include"] )
   baseEnv.Append( LIBPATH = [baseEnv['ENV']['SDKPATH']+"/Lib"] )
   baseEnv.Append( LINKFLAGS = ['/DEFAULTLIB:"WSock32.Lib"'] )
   baseEnv['INSTALL_BINDIR'] = '#winbin'

# Export the environment to children
Export('baseEnv')

# Build the source tree
SConscript('src/SConscript', build_dir='src/obj')
SConscript('test/SConscript', build_dir='test/obj')
