import os
baseEnv = Environment(ENV=os.environ)

# Add the include, library, and bin paths
baseEnv.Append(CPPPATH = ['#include'])
baseEnv['LIBPATH'] = ['#lib']
baseEnv['INSTALL_LIBDIR'] = '#lib'
baseEnv['INSTALL_BINDIR'] = '#bin'

# Some additional stuff to do if we're building for windows
if os.name == "nt":                     
   print "scons: Building for Windows..."
   baseEnv.Append( CCFLAGS = ['-DWINDOWS'] )
   baseEnv.Append( CCFLAGS = ['-EHsc'] )
   #baseEnv.Append( CPPPATH = ["c://Program Files//Microsoft Visual Studio 8//VC//include"] )
   baseEnv.Append( CPPPATH = [baseEnv['ENV']['SDKPATH']+"/Include"] )
   baseEnv.Append( LIBPATH = [baseEnv['ENV']['SDKPATH']+"/Lib"] )
   baseEnv['INSTALL_BINDIR'] = '#winbin'


else:
   print "scons: Building for UNIX..."


# Export the environment to children
Export('baseEnv')

# Build the source tree
SConscript('src/SConscript', build_dir='src/obj')
