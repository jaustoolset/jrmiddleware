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
baseEnv['CPPPATH'] = ['#include']
baseEnv['LIBPATH'] = ['#lib']
baseEnv['INSTALL_LIBDIR'] = '#lib'
baseEnv['INSTALL_BINDIR'] = '#bin'

# Special flags needed for TinyXML
baseEnv.Append( CCFLAGS = ['-DTIXML_USE_STL'])

# Set-up a cross-compile environment, if selected
if ARGUMENTS.get('cross', 0) == 'arm':
    print "scons: Building for ARM..."  
    baseEnv['CXX'] = '/opt/crosstool/gcc-3.3.4-glibc-2.3.2/arm-unknown-linux-gnu/bin/arm-unknown-linux-gnu-g++'
    baseEnv['RANLIB'] = '/opt/crosstool/gcc-3.3.4-glibc-2.3.2/arm-unknown-linux-gnu/bin/arm-unknown-linux-gnu-ranlib'
    baseEnv['AR'] = '/opt/crosstool/gcc-3.3.4-glibc-2.3.2/arm-unknown-linux-gnu/bin/arm-unknown-linux-gnu-ar'
    baseEnv.Append(CPPPATH = ['/opt/crosstool/gcc-3.3.4-glibc-2.3.2/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include'])
    baseEnv.Append(LIBPATH = ['/opt/crosstool/gcc-3.3.4-glibc-2.3.2/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib'])
    baseEnv.Append(LIBPATH = ['#lib'])
    baseEnv.Append( LINKFLAGS = ['-static'] )
    baseEnv['PROGSUFFIX'] =''

# special handing for cygwin
elif baseEnv['PLATFORM'] == 'cygwin':
    print "scons: Building for CYGWIN..."
    baseEnv.Append( CCFLAGS = ['-D__CYGWIN__'] )
    
# Some additional stuff to do if we're building for windows
elif os.name == "nt":                     
   print "scons: Building for Windows..."
   baseEnv.Append( CCFLAGS = ['-DWINDOWS', '-EHsc','-D_CRT_SECURE_NO_DEPRECATE'])
   #baseEnv.Append( CPPPATH = [baseEnv['ENV']['SDKPATH']+"/Include"] )
   #baseEnv.Append( LIBPATH = [baseEnv['ENV']['SDKPATH']+"/Lib"] )
   baseEnv.Append( LINKFLAGS = ['/DEFAULTLIB:"WSock32.Lib"'] )

# When building for posix-compliant systems, we need the pthread_create library
if os.name == "posix":
   baseEnv.Append( LINKFLAGS = '-lpthread' )

# Export the environment to children
Export('baseEnv')

# Build the source tree
SConscript('src/SConscript', build_dir='src/obj')
SConscript('test/SConscript', build_dir='test/obj')
