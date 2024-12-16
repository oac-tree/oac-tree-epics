Welcome to sequencer-plugin-epics's documentation!
==================================================

This is a work in progress.

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   `sequencer-plugin-epics` is a sequencer plugin that provides instructions and variables to interface with EPICSv7 process variables. It supports both ChannelAccess and PvAccess.

Overview
--------

This plugin provides instructions to read and write from/to EPICS process variables and a single instruction to call functions that are hosted by an EPICS RPC server.

.. list-table:: EPICS Instructions
   :widths: 25 25
   :header-rows: 1

   * - Instruction
     - Description
   * - ChannelAccessRead
     - Read a ChannelAccess process variable and store the result in a workspace variable
   * - ChannelAccessWrite
     - Read a workspace variable and write its value to a ChannelAccess process variable
   * - PvAccessRead
     - Read a PvAccess process variable and store the result in a workspace variable
   * - PvAccessWrite
     - Read a workspace variable and write its value to a PvAccess process variable
   * - RPCClient
     - Execute a Remote Procedure Call to an EPICS RPC server

All these instructions will establish a network connection each time they are called and will close that connection immediately afterwards. For process variables, whose value is continuously being monitored or just repeatedly used, this involves a considerable overhead. For that reason, the plugin offers specific types of workspace variables that will keep their connected alive and can be read or written like any other workspace variable:

.. list-table:: EPICS variables
   :widths: 25 25
   :header-rows: 1

   * - Variable
     - Description
   * - ChannelAccessClient
     - This workspace variable connects to an EPICS ChannelAccess process variable as a client
   * - PvAccessClient
     - This workspace variable connects to an EPICS PvAccess process variable as a client
   * - PvAccessServer
     - This workspace variable publishes its value on the network as an EPICS PvAccess process variable

Usage
-----

Before being able to use this plugin in a procedure, the user have it installed in a known shared library path (e.g. LD_LIBRARY_PATH). The instructions provided by this plugin are available to the user after loading the plugin by adding the following line to the procedure XML file:

.. code-block:: xml

  <Plugin>libsequencer-epics.so</Plugin>

The user is then able to use all instructions and variables provided by this plugin.

Instructions
------------

ChannelAccessRead
^^^^^^^^^^^^^^^^^

The `ChannelAccessRead` instruction tries to read an EPICS ChannelAccess process variable on the network and writes its value to a workspace variable if successful. When the process variable cannot be found on the network, or copying the value to a workspace variable fails, e.g. due to incompatible types, the instruction will return `FAILURE`.

.. list-table::
   :widths: 25 25 15 50
   :header-rows: 1

   * - Attribute name
     - Attribute type
     - Mandatory
     - Description
   * - channel
     - StringType
     - yes
     - name of the EPICS ChannelAccess channel of the process variable
   * - outputVar
     - StringType
     - yes
     - name of the workspace variable (or field thereof) where to write the read value
   * - timeout
     - Float64Type
     - no
     - timeout in seconds to wait for a successful channel connection (default: 2.0)

.. _ca_read_example:

**Example**

This procedure will try to connect to an EPICS ChannelAccess process variable with a maximum timeout of 5 seconds. Then it will read its value and copy it to the local workspace variable. If successful, the local variable with name `to-write` will contain the value of the ChannelAccess process variable. Only on successful completion of the reading and write operation will the instruction return `SUCCESS`. In all other cases, the instruction returns `FAILURE`.

.. code-block:: xml

    <PvAccessRead channel="seq::test::variable" timeout="5.0" outputVar="to-write"/>
    <Workspace>
        <Local name="to-write"/>
    </Workspace>

Indices and tables
==================

* :ref:`genindex`
* :ref:`search`
