Variables
---------

ChannelAccessClient
^^^^^^^^^^^^^^^^^^^

``ChannelAccessClient`` is a workspace variable type that connects as a client to an EPICS ChannelAccess process variable. Read and write operations to these variables will result in reading and writing the process variables on the network.

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
     - JSON representation of the type of the variable

.. note::

   The ``type`` attribute is used to define the type of the process variable's value. For example, this allows to obtain an EPICS enumeration record as either the string value of the enumerator or as the integer index. Furthermore, by defining this type to be a structured type with specific member fields, the user can obtain some metadata about the process variables. The following special member fields can be used:

   * ``value``: Field to hold the process variable's value. Its type is interpreted exactly the same as for a scalar ``type`` attribute.
   * ``connected``: Field to indicate if the channel is connected. Must be of a type to which a boolean can be converted, e.g. boolean or integer type.
   * ``timestamp``: Timestamp of the process variable. Must be of a type to which an unsigned 64 bit integer can be converted.
   * ``status``: Status field. Must be of a type to which an unsigned 16 bit integer can be converted.
   * ``severity``: Severity of the alarm field. Must be of a type to which an unsigned 16 bit integer can be converted.

.. _ca_client_example:

**Example**

This procedure contains a ``ChannelAccessClient`` workspace variable that will connect to an EPICS ChannelAccess process variable and make its value and connected status available to procedure instructions. The first instruction will copy the whole structure of this variable to a local variable. Then it will compare the ``connected`` field of this locally cached value to ``true``. The procedure will succeed if the ``ChannelAccessClient`` workspace variable was properly connected to the EPICS channel and its value could be read as a 64 bit floating point value.

.. code-block:: xml

    <Sequence>
        <Copy inputVar="ca_pv" outputVar="cache">
        <Equals leftVar="cache.connected" rightVar="true">
    </Sequence>
    <Workspace>
        <ChannelAccessClient name="ca_pv" channel='EXAMPLE:SETPOINT'
            type='{"type":"mySetPointType","attributes":[{"value":{"type":"float64"}},{"connected":{"type":"bool"}}]}'/>
        <Local name="cache">
        <Local name="true" type='{"type":"bool"}' value='true'>
    </Workspace>

PvAccessClient
^^^^^^^^^^^^^^

``PvAccessClient`` is a workspace variable type that connects as a client to an EPICS PvAccess process variable. Read and write operations to these variables will result in reading and writing the process variables on the network.

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
     - name of EPICS PvAccess channel
   * - type
     - StringType
     - no
     - JSON representation of the type of the variable

.. note::

   The ``type`` attribute is used to define the type of the process variable's value. If it is a scalar type, the EPICS PvAccess process variable has to be a structured value with a scalar ``value`` member field, whose value will be cached in the workspace variable. If it is a structured type, the type of the process variable has to be convertible to it. This implies the exact same fields in the structure and the convertibility of all of its leaf values. For structured types, it is more convenient to not define this ``type`` attribute at all, so that the type will be an exact copy of the process variable's type.

.. _pva_client_example:

**Example**

This procedure contains a ``PvAccessClient`` workspace variable that will connect to an EPICS PvAccess process variable and make its value available to procedure instructions. The first instruction will copy this scalar value to a local variable. Then it will verify that the locally cached value is less than a predefined threshold. The procedure will succeed if it could succesfully read the ``PvAccessClient`` variable, i.e. it was connected, and the obtained value was less than the threshold.

.. code-block:: xml

    <Sequence>
        <Copy inputVar="pva_pv" outputVar="cache">
        <IsLessThan leftVar="cache" rightVar="threshold">
    </Sequence>
    <Workspace>
        <PvAccessClient name="pva_pv" channel='EXAMPLE:SETPOINT' type='{"type":"float64"}'/>
        <Local name="cache">
        <Local name="threshold" type='{"type":"float64"}' value='4000.0'>
    </Workspace>

PvAccessServer
^^^^^^^^^^^^^^

``PvAccessServer`` is a workspace variable type that acts as a server for a EPICS PvAccess process variable. Its value be published to PvAccess clients on the network.

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
     - name of EPICS PvAccess channel
   * - type
     - StringType
     - yes
     - JSON representation of the type of the variable
   * - value
     - StringType
     - no
     - JSON representation of the initial value of the variable

.. note::

   The ``type`` attribute is used to define the type of the process variable's value. If it is a scalar type, the underlying EPICS PvAccess process variable will be a structured value with a single scalar ``value`` member field of that type. If it is a structured type, it will be used directly as the type of the underlying process variable.

.. warning::

   The implementation is based on the ``PVXS`` library, which provides some restrictions to the possible types that can be used. In particular, the following restriction apply:

   * Scalar nodes (inside a structure or array) of the type ``char8`` will be converted to ``uint8``;
   * Arrays of structures are not supported;
   * Arrays, both when top level or as members of a struct, cannot be named.

.. _pva_server_example:

**Example**

This procedure contains a ``PvAccessServer`` workspace variable that will publish an EPICS PvAccess process variable with a structured type that has a single integer field with name ``value``. The procedure will perform a loop of 10 iterations, each iteration consisting of the increment of the published value and a one second waiting instruction. At the end of the procedure, the EPICS PvAccess process variable will contain the value 10.

.. code-block:: xml

    <Repeat maxCount="10">
        <Sequence>
            <Increment varName="pva_pv"/>
            <Wait name="wait" timeout="1.0"/>
        </Sequence>
    </Repeat>
    <Workspace>
        <PvAccessServer name="pva_pv" channel="EXAMPLE:COUNTER"
                        type='{"type":"uint64"}' value='0'/>
    </Workspace>
