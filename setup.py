 # -*- coding: utf-8 -*-
from subprocess import Popen, PIPE
from distutils.core import setup, Extension

popen = Popen(["pkg-config", "--cflags", "opencv"], stdout=PIPE, stderr=PIPE)
stdoutdata, stderrdata = popen.communicate()
if popen.returncode != 0:
    print(stderrdata)
    sys.exit()
opencv_include = [i[2:] for i in stdoutdata.split()]

popen = Popen(["pkg-config", "--libs", "opencv"], stdout=PIPE, stderr=PIPE)
stdoutdata, stderrdata = popen.communicate()
if popen.returncode != 0:
    print(stderrdata)
    sys.exit()
opencv_library = stdoutdata.split()

tclip_module = Extension("tclip",
                         sources=['tclip.cpp'],
                         include_dirs=opencv_include,
                         library_dirs=[],
                         libraries=[],
                         extra_link_args=opencv_library)

setup(name='tclip',
      description='tclip.',
      keywords='opencv',
      license='BSD License',
      maintainer='xutao',
      maintainer_email='xutao@douban.com',
      ext_modules=[tclip_module])
