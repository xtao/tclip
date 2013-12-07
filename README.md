Python bindings for tclip (C++ Wrapper).
========================================
Just a python bindings for [tclip](https://github.com/exinnet/tclip)

OpenCV Install
--------------
Follow the guide:
1. https://help.ubuntu.com/community/OpenCV
2. https://github.com/jayrambhia/Install-OpenCV

Tclip install
-------------
1. `python setup.py install`

Quickstart
----------
```python
from tclip import tclip
tclip("file1.jpg", "file2.jpg", 400, 200)
```

