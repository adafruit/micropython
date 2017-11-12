nrfutil
==========
nrfutil is a Python package that includes the nrfutil command line utility and the nordicsemi library.

About
-----
The library is written for Python 2.7.

Prerequisites
-------------
To install nrfutil the following prerequisites must be satisfied:

* Python 2.7 (2.7.6 or newer, not Python 3)
* pip (https://pip.pypa.io/en/stable/installing.html)
* setuptools (upgrade to latest version: pip install -U setuptools)
* install required modules: pip install -r requirements.txt

py2exe prerequisites (Windows only):  

* py2exe (Windows only) (v0.6.9) (pip install http://sourceforge.net/projects/py2exe/files/latest/download?source=files)
* VC compiler for Python (Windows only) (http://www.microsoft.com/en-us/download/confirmation.aspx?id=44266)

Installation
------------
To install the library to the local Python site-packages and script folder:  
```
python setup.py install
```

To generate a self-contained Windows exe version of the utility (Windows only):  
```
python setup.py py2exe
```
NOTE: Some anti-virus programs will stop py2exe from executing correctly when it modifies the .exe file.

Usage
-----
To get info on usage of nrfutil:  
```
nrfutil --help
```
