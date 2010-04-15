import sys
import glob
import os
import platform

env = Environment(ARCH=platform.machine());
print "Building for architecture %s"%(env['ARCH'])

SConscript([
'Libraries/TinyXML/SConstruct', 
'Libraries/OSCPack/SConstruct', 
'Core/SConstruct',
'EP/SConstruct',
]);
