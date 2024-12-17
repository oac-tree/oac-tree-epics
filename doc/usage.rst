Usage
-----

Before being able to use this plugin in a procedure, the user have it installed in a known shared library path (e.g. LD_LIBRARY_PATH). The instructions provided by this plugin are available to the user after loading the plugin by adding the following line to the procedure XML file:

.. code-block:: xml

  <Plugin>libsequencer-epics.so</Plugin>

The user is then able to use all instructions and variables provided by this plugin.
