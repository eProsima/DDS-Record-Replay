.. include:: ../../exports/alias.include

.. _recorder_remote_control:

###################
Remote control
###################

Recorder exposes a DDS command topic and a DDS status topic when
``remote-controller.enable`` is true, which is the default. A custom DDS
application or the optional graphical Remote Controller can use this interface.

Recorder states
===============

.. list-table::
   :header-rows: 1
   :widths: 18 27 28 27

   * - State
     - DDS entities
     - Incoming samples
     - Output file
   * - ``RUNNING``
     - Active
     - Written in batches
     - Open and growing
   * - ``PAUSED``
     - Active
     - Kept in a rolling ``event-window`` buffer
     - Written only by ``event``
   * - ``SUSPENDED``
     - Active
     - Received and discarded
     - Finalized
   * - ``STOPPED``
     - Recorder entities destroyed; controller remains
     - Not received
     - Finalized
   * - ``CLOSED``
     - Destroyed
     - Not received
     - Finalized; process exits

Starting from STOPPED recreates Recorder and reloads every non-controller YAML
setting. This is the way to apply output, transport, QoS, logging, or other
settings that cannot be hot-reloaded.

.. figure:: /rst/figures/recorder-states.svg
   :align: center
   :alt: Recorder can move among running, paused, suspended, and stopped states; close ends the process and event captures the paused window.

   Recorder states. The transition table below is authoritative.

Commands and transitions
========================

Commands are case-insensitive. ``args`` is either empty or a JSON object
serialized as a string.

.. list-table::
   :header-rows: 1
   :widths: 17 25 23 35

   * - Command
     - Valid source
     - Result
     - Arguments and notes
   * - ``start``
     - RUNNING, PAUSED, SUSPENDED, STOPPED
     - RUNNING
     - Creates Recorder first when source is STOPPED.
   * - ``pause``
     - RUNNING, PAUSED, SUSPENDED, STOPPED
     - PAUSED
     - Begins or continues the rolling event buffer.
   * - ``event``
     - PAUSED only
     - PAUSED by default
     - Flushes the current event window. Optional ``next_state`` is ``RUNNING``,
       ``SUSPENDED``, or ``STOPPED``.
   * - ``suspend``
     - RUNNING, PAUSED, SUSPENDED, STOPPED
     - SUSPENDED
     - Keeps DDS entities ready but discards received samples.
   * - ``stop``
     - RUNNING, PAUSED, SUSPENDED
     - STOPPED
     - Optional ``avoid_overwriting_output: true`` detaches finalized files
       from the next Recorder instance's rotation tracking.
   * - ``close``
     - Any state
     - CLOSED
     - Finalizes output and terminates the process.

Examples of serialized ``args`` values:

.. code-block:: json

   {"next_state": "STOPPED"}

.. code-block:: json

   {"avoid_overwriting_output": true}

An unsupported command, an ``event`` outside PAUSED, or an invalid argument is
logged and does not change the current state.

.. _recorder_remote_controller_data_types:

DDS interface
=============

The controller domain defaults to the recording domain. Topic names can be
changed under ``remote-controller``.

.. list-table::
   :header-rows: 1
   :widths: 25 35 40

   * - Direction
     - Default topic
     - Type
   * - Controller → Recorder
     - ``/ddsrecorder/command``
     - ``DdsRecorderCommand``; reliable, volatile, depth 1
   * - Recorder → Controller
     - ``/ddsrecorder/status``
     - ``DdsRecorderStatus``; reliable, transient-local, depth 1

.. code-block:: idl

   struct DdsRecorderCommand
   {
       string command;
       string args;
   };

   struct DdsRecorderStatus
   {
       string previous;
       string current;
       string info;
   };

``previous`` and ``current`` contain the uppercase state names. ``info`` is
reserved and currently unused.

If the controller uses the same domain as Recorder, its command and status
topics are ordinary discovered topics and may be recorded. Exclude them when
that traffic is not useful:

.. literalinclude:: ../../examples/recorder_controller_blocklist.yaml
   :language: yaml
   :linenos:

.. _recorder_remote_controller:

Graphical Remote Controller
===========================

Build with ``-DBUILD_DDSRECORDER_CONTROLLER=ON`` and install Python 3 and
PyQt6, then run:

.. code-block:: console

   $ ddsrecorder_controller

Choose the controller DDS domain and, when needed, custom command/status topic
names from the **File** menu. Once a Recorder is discovered, the state display
and buttons reflect commands and status updates.

.. figure:: /rst/figures/controller_interact.png
   :align: center
   :alt: DDS Recorder Remote Controller displaying a discovered recorder and state control buttons.

   The graphical controller after discovering a Recorder.

After ``close``, the Recorder process no longer exists and cannot receive more
commands; start a new ``ddsrecorder`` process to continue.
