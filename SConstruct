#***********           LICENSE HEADER   *******************************
# JR Middleware
# Copyright (c)  2008-2019, DeVivo AST, Inc
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions are met:
# 
#        Redistributions of source code must retain the above copyright notice, 
# this list of conditions and the following disclaimer.
# 
#        Redistributions in binary form must reproduce the above copyright 
# notice, this list of conditions and the following disclaimer in the 
# documentation and/or other materials provided with the distribution.
# 
#        Neither the name of the copyright holder nor the names of 
# its contributors may be used to endorse or promote products derived from 
# this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
# POSSIBILITY OF SUCH DAMAGE.
# *********************  END OF LICENSE ***********************************



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
baseEnv.Append( extra_libs = [] )
if ARGUMENTS.get('cross', 0) == 'arm':
    print( "scons: Building for ARM..."  )
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
    print(  "scons: Building for CYGWIN...")
    baseEnv.Append( CCFLAGS = ['-D__CYGWIN__'] )
    
# Some additional stuff to do if we're building for windows
elif os.name == "nt":                     
   print(  "scons: Building for Windows...")
   baseEnv.Append( CCFLAGS = ['-DWINDOWS', '-EHsc','-D_CRT_SECURE_NO_DEPRECATE'])
   baseEnv.Append( LINKFLAGS = ['/DEFAULTLIB:"WSock32.Lib"'] )

# MAC doesn't support timers, so we have a special build flag
elif baseEnv['PLATFORM'] == 'darwin':
	baseEnv.Append( LINKFLAGS = ['-lpthread'] )
	baseEnv.Append( CCFLAGS = ['-D__MAC__'] )   

# When building for posix-compliant systems, we need the pthread and rt libraries
elif os.name == "posix":
   baseEnv.Append( LINKFLAGS = ['-lpthread', '-lrt'] )
   baseEnv.Append( extra_libs = ['pthread', 'rt'] )
   
# Allow the builder to statically link libraries
static_linking = int(0)
AddOption('--static', dest='static_linking', action='store_true') 
if GetOption('static_linking'):
   baseEnv.Append( LINKFLAGS = '-static' )
   print(  "Static linking selected from command line...")
   
# Export the environment to children
Export('baseEnv')   
   
# Define option to build JNI interface
build_jni = int(0)
AddOption('--jni', dest='build_jni', action='store_true') 
   
# Build the source tree
SConscript('src/SConscript', build_dir='src/obj')
#SConscript('test/SConscript', build_dir='test/obj')
SConscript('jni/SConscript', build_dir='jni/obj')


