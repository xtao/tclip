Python bindings for tclip (C++ Wrapper).
========================================
Just a python bindings for [tclip](https://github.com/exinnet/tclip)

OpenCV Install
--------------
Follow the guide:

- https://help.ubuntu.com/community/OpenCV
- https://github.com/jayrambhia/Install-OpenCV

Tclip install
-------------
`python setup.py install`

Quickstart
----------
```python
from tclip import tclip
tclip("file1.jpg", "file2.jpg", 400, 200)
```

