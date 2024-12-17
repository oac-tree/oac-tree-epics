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

All these instructions will establish a network connection each time they are called and will close that connection immediately afterwards. For process variables, whose value is continuously being monitored or just repeatedly used, this involves a considerable overhead. For that reason, the plugin offers specific types of workspace variables that will keep their connection alive and can be read or written like any other workspace variable:

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
