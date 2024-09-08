.. _custom_initial_session_state:

Customizing the initial program state
=====================================

Upon each program start, OVITO looks for a file named :file:`defaults.ovito` on your system and, if found, uses it 
to initialize a new program session. This optional mechanism allows you to override hardcoded factory defaults for various settings
and can save you from having to set up everything over and again. In the :file:`defaults.ovito` file you can specify

 * the initial layout of the viewport windows,
 * the projection type and camera orientation of each viewport,
 * the render settings.

Note that this mechanism is orthogonal to the way most other application settings are managed by the software, which 
can be edited in the :ref:`application_settings` dialog and which get automatically stored in a different location on your system.

The software will look in the following filesystem locations for the session state file:

.. list-table::
    :widths: auto

    * - Windows:
      - :file:`C:\\Users\\<USER>\\AppData\\Roaming\\Ovito\\Ovito\\defaults.ovito` 
    * - Linux:
      - :file:`~/.local/share/Ovito/Ovito/defaults.ovito` 
    * - macOS:
      - :file:`~/Library/Application Support/Ovito/Ovito/defaults.ovito`

You can create the file in one of these locations simply using the :menuselection:`File --> Save Session State As` function of OVITO. 
Make sure the saved session state contains an empty scene (no pipelines or objects) because it just serves as an initial starting point before 
you import an actual simulation dataset into the scene.