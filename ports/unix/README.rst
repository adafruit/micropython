Unix
====

This port brings MicroPython to Unix (Linux)


Setup
-----

.. code-block:: shell

    sudo apt install build-essential libffi-dev gettext


Building (common)
-----------------
Before building the firmware for a given board, there are two additional steps.
These commands should be executed from the root directory of the repository
(``circuitpython/``).

1. There are various submodules that reside in different repositories. In order
   to have these submodules locally, you must pull them into your clone, using:

.. code-block:: shell

   git submodule update --init --recursive

2. The MicroPython cross-compiler must be built; it will be used to pre-compile
   some of the built-in scripts to bytecode.  The cross-compiler is built and
   run on the host machine, using:

.. code-block:: shell

    make -C mpy-cross


Building
--------

Build commands are run from the ``circuitpython/ports/unix`` directory.

To build for Unix:

.. code-block:: shell

    make deplibs
    make


Deploying
---------

 ./micropython
 MicroPython 4.0.0-alpha.5-28-gca60a034c on 2018-12-19; linux version
 Use Ctrl-D to exit, Ctrl-E for paste mode
 >>> sys.implementation
 (name='circuitpython', version=(4, 0, 0))


