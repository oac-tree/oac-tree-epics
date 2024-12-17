Variables
---------

ChannelAccessClient
^^^^^^^^^^^^^^^^^^^

``ChannelAccessClient`` is a workspace variable type that connects as a client to an EPICS ChannelAccess process variables. Read and write operations to this variables will result in reading and writing the process variables on the network.

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
     - name of EPICS ChannelAccess channel
   * - type
     - StringType
     - yes
     - JSON representation of the type of the input value

.. note::

   The ``type`` attribute is used to define the type of the process variable's value. For example, this allows to obtain an EPICS enumeration record as either the string value of the enumerator or as the integer index. Furthermore, by defining this type to be a structured type with specific member fields, the user can obtain some metadata about the process variables. The following special member fields can be used:

   * ``value``: Field to hold the process variable's value. Its type is interpreted exactly the same as for a scalar ``type`` attribute.
   * ``connected``: Field to indicate if the channel is connected. Must be of a type to which a boolean can be converted, e.g. boolean or integer type.
   * ``timestamp``: Timestamp of the process variable. Must be of a type to which an unsigned 64 bit integer can be converted.
   * ``status``: Status field. Must be of a type to which an unsigned 16 bit integer can be converted.
   * ``severity``: Severity of the alarm field. Must be of a type to which an unsigned 16 bit integer can be converted.

