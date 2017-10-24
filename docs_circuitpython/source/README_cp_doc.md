Contributing to CircuitPython's Documentation
=============================================

This section describes how to build the CircuitPython documentation
from the source files in the repo.

The latest documentation build can be found at:
http://circuitpython.readthedocs.io/en/latest/

The documentation you see there is generated from the source files in
the whole repo tree:
https://github.com/adafruit/circuitpython/tree/master

Building the documentation locally
----------------------------------

If you're making changes to the documentation, you should build the
documentation locally so that you can preview your changes.

Create and activate a virtual environment:

```bash
    cd docs_circuitpython
    python3 venv mydocenv
    source mydocenv/bin/activate
```

From the `docs_circuitpython` directory, install requirements:

```bash
   python3 -m pip install -r requirements-doc.txt
```

In `circuitpython/docs_circuitpython` directory, build the docs:

```
   make clean
   make html
```

You'll find the index page at `build/html/index.html`. You have several
options to view the built docs (listed in order of difficulty):

1. Double click the `index.html` file to open in the browser.

2. From the terminal in the `circuitpython/docs_circuitpython` directory,
   enter `open build/html/index.html`.

3. Create a web server in python: `python3 -m http.server`. Then, go
   to the URL given in the web server message output and add
   `build/html/index.html` to the end of the URL.

Maintainer's Reference to ReadTheDocs Build
-------------------------------------------

ReadTheDocs is used to build and serve the Sphinx documentation to the
end user.

At the repo's base, there is a `readthedocs.yml` file that enables
many of the ReadTheDoc build configuration settings to be
programmatically set. This is handy as some of RTD's advanced settings
can be a bit tricky to debug.

The docs are built on RTD using Python 3 and the requirements listed in
the requirements file in the `docs_circuitpython` directory. The
`conf.py` file used is also found in the `docs_circuitpython` directory.
