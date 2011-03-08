# ***********************************************************************
# * @file      SConstruct
# * @author    Dave Martin, DeVivo AST, Inc.  
# * @date      2008/03/03
# *
# * @attention Copyright (C) 2008
#
#*  This file is part of Jr Middleware.
#*
#*  Jr Middleware is free software: you can redistribute it and/or modify
#*  it under the terms of the GNU Lesser General Public License as published by
#*  the Free Software Foundation, either version 3 of the License, or
#*  (at your option) any later version.
#*
#*  Jr Middleware is distributed in the hope that it will be useful,
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#*  GNU Lesser General Public License for more details.
#*
#*  You should have received a copy of the GNU Lesser General Public License
#*  along with Jr Middleware.  If not, see <http://www.gnu.org/licenses/>.
#*
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
   baseEnv.Append( LINKFLAGS = ['/DEFAULTLIB:"WSock32.Lib"'] )

# MAC doesn't support timers, so we have a special build flag
elif baseEnv['PLATFORM'] == 'darwin':
	baseEnv.Append( LINKFLAGS = ['-lpthread'] )
	baseEnv.Append( CCFLAGS = ['-D__MAC__'] )   

# When building for posix-compliant systems, we need the pthread and rt libraries
elif os.name == "posix":
   baseEnv.Append( LINKFLAGS = ['-lpthread', '-lrt'] )
   
# Allow the builder to statically link libraries
static_linking = int(0)
AddOption('--static', dest='static_linking', action='store_true') 
if GetOption('static_linking'):
   baseEnv.Append( LINKFLAGS = '-static' )
   print "Static linking selected from command line..."
   
# Export the environment to children
Export('baseEnv')   
   
# Define option to build JNI interface
build_jni = int(0)
AddOption('--jni', dest='build_jni', action='store_true') 
   
# Build the source tree
SConscript('src/SConscript', build_dir='src/obj')
#SConscript('test/SConscript', build_dir='test/obj')
SConscript('jni/SConscript', build_dir='jni/obj')


