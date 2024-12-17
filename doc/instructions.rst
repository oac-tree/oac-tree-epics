Instructions
------------

ChannelAccessRead
^^^^^^^^^^^^^^^^^

The ``ChannelAccessRead`` instruction tries to read an EPICS ChannelAccess process variable on the network and writes its value to a workspace variable if successful. When the process variable cannot be found on the network, or copying the value to a workspace variable fails, e.g. due to incompatible types, the instruction will return ``FAILURE``.

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

This procedure will try to connect to an EPICS ChannelAccess process variable with a maximum timeout of 5 seconds. Then it will read its value and copy it to the local workspace variable. If successful, the local variable with name ``to-write`` will contain the value of the ChannelAccess process variable. Only on successful completion of the reading and write operation will the instruction return ``SUCCESS``. In all other cases, the instruction returns ``FAILURE``.

.. code-block:: xml

    <ChannelAccessRead channel="seq::test::variable" timeout="5.0" outputVar="to-write"/>
    <Workspace>
        <Local name="to-write"/>
    </Workspace>

ChannelAccessWrite
^^^^^^^^^^^^^^^^^^

The ``ChannelAccessWrite`` instruction tries to write a value to an EPICS ChannelAccess process variable on the network. The value is either fetched from a workspace variable or is encoded in JSON format as two attributes (``type`` and ``value``). When the value cannot be fetched or parsed, the process variable cannot be found on the network, or writing the value to the process variable fails, e.g. due to incompatible types, the instruction will return ``FAILURE``.

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
   * - varName
     - StringType
     - no
     - name of the workspace variable (or field thereof) from where to read the value
   * - type
     - StringType
     - no
     - JSON representation of the type of the value to write
   * - value
     - StringType
     - no
     - JSON representation of the value to write
   * - timeout
     - Float64Type
     - no
     - timeout in seconds to wait for a successful channel connection (default: 2.0)

.. note::

   The user must provide either the ``varName`` attribute or both the ``type`` and ``value`` attributes.

.. _ca_write_example:

**Example**

This procedure will try to write the floating point value ``3.14`` to an EPICS ChannelAccess process variable with a maximum timeout of 5 seconds. On successful completion of the operation, the instruction will return ``SUCCESS``.

.. code-block:: xml

    <ChannelAccessWrite channel="seq::test::variable" timeout="5.0"
                        type='{"type":"float64"}' value='3.14'/>

PvAccessRead
^^^^^^^^^^^^

The ``PvAccessRead`` instruction tries to read an EPICS PvAccess process variable on the network and writes its value to a workspace variable if successful. When the process variable cannot be found on the network, or copying the value to a workspace variable fails, e.g. due to incompatible types, the instruction will return ``FAILURE``.

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
     - name of the EPICS PvAccess channel of the process variable
   * - outputVar
     - StringType
     - yes
     - name of the workspace variable (or field thereof) where to write the read value
   * - timeout
     - Float64Type
     - no
     - timeout in seconds to wait for a successful channel connection (default: 2.0)

.. _pva_read_example:

**Example**

This procedure will try to connect to an EPICS PvAccess process variable with a maximum timeout of 5 seconds. Then it will read its value and copy it to the local workspace variable. If successful, the local variable with name ``to-write`` will contain the value of the PvAccess process variable. Only on successful completion of the reading and write operation will the instruction return ``SUCCESS``. In all other cases, the instruction returns ``FAILURE``.

.. code-block:: xml

    <PvAccessRead channel="seq::test::variable" timeout="5.0" outputVar="to-write"/>
    <Workspace>
        <Local name="to-write"/>
    </Workspace>

PvAccessWrite
^^^^^^^^^^^^^

The ``PvAccessWrite`` instruction tries to write a value to an EPICS PvAccess process variable on the network. The value is either fetched from a workspace variable or is encoded in JSON format as two attributes (``type`` and ``value``). When the value cannot be fetched or parsed, the process variable cannot be found on the network, or writing the value to the process variable fails, e.g. due to incompatible types, the instruction will return ``FAILURE``.

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
     - name of the EPICS PvAccess channel of the process variable
   * - varName
     - StringType
     - no
     - name of the workspace variable (or field thereof) from where to read the value
   * - type
     - StringType
     - no
     - JSON representation of the type of the value to write
   * - value
     - StringType
     - no
     - JSON representation of the value to write
   * - timeout
     - Float64Type
     - no
     - timeout in seconds to wait for a successful channel connection (default: 2.0)

.. note::

   The user must provide either the ``varName`` attribute or both the ``type`` and ``value`` attributes.

.. _pva_write_example:

**Example**

This procedure will try to write the struct with floating point member value ``3.14`` to an EPICS PvAccess process variable with a maximum timeout of 5 seconds. On successful completion of the operation, the instruction will return ``SUCCESS``.

.. code-block:: xml

    <PvAccessWrite channel="seq::test::variable" timeout="5.0"
                        type='{"type":"myFloat","attributes":[{"value":{"type":"float32"}}]}'
                        value='{"value":3.14}'/>

RPCClient
^^^^^^^^^

The ``RPCClient`` executes a Remote Procedure Call on an EPICS PvAccess RPC server. As input, it uses either a literal value, encoded in JSON format as two attributes (``type`` and ``value``) or the value of a workspace variable. The reply from the RPC server is optionally put into a workspace variable.

.. list-table::
   :widths: 25 25 15 50
   :header-rows: 1

   * - Attribute name
     - Attribute type
     - Mandatory
     - Description
   * - service
     - StringType
     - yes
     - name of EPICS PvAccess RPC server
   * - requestVar
     - StringType
     - no
     - name of the workspace variable (or field thereof) from where to read the input value
   * - type
     - StringType
     - no
     - JSON representation of the type of the input value
   * - value
     - StringType
     - no
     - JSON representation of the input value
   * - outputVar
     - StringType
     - no
     - name of the workspace variable (or field thereof) where to write the reply value
   * - timeout
     - Float64Type
     - no
     - timeout in seconds to wait for a successful connection (default: 5.0)

.. note::

   The user must provide either the ``requestVar`` attribute or both the ``type`` and ``value`` attributes.

.. _rpc_client_example:

**Example**

This procedure will try to execute an Remote Procedure Call on the service with name ``rpc@service``. As an input to the RPC call it will use the structure with a zero field, encoded in the two attributes ``type`` and ``value``. The reply of the RPC call, if successful, is written to a workspace variable. The instruction will return ``SUCCESS`` when all steps in this operation were successful.

.. code-block:: xml

    <RPCClient service="rpc@service"
               type='{"type":"rpcStruct","attributes":[{"value":{"type":"uint64"}}]}'
               value='{"value":0}' reply="reply" timeout="3.0"/>
    <Workspace>
        <Local name="reply"/>
    </Workspace>
