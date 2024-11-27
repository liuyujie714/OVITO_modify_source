.. _tutorials.marker_particles:
.. _howto.marker_particles:

Motion visualization with marker particles
==========================================

.. image:: /images/howtos/shear_marker.gif
   :width: 40%
   :align: right

In this step-by-step tutorial, you will create an animation from an MD simulation of a simple shearing experiment
as shown on the right. You will learn how to highlight a group of atoms,
initially located in a narrow region, with a marker color to visualize the atomic motion 
in the interior of the crystal during the course of the simulation.

In particular, you will learn more about the purpose of the :ref:`particles.modifiers.freeze_property` modifier in OVITO,
which helps you preserve the initial selection state of a group of particles. 

Step 1: Load simulation trajectory
""""""""""""""""""""""""""""""""""

Start by downloading the simulation trajectory file 
`shear.dump <https://gitlab.com/stuko/ovito/-/blob/master/examples/data/shear.dump>`__
for this tutorial to your computer. It was generated from the ``shear`` simulation script 
found in the `LAMMPS examples folder <https://docs.lammps.org/Examples.html>`__. Use the :menuselection:`File --> Load File` function
to open the file :file:`shear.dump` in OVITO.

Step 2: Adjust animation speed
""""""""""""""""""""""""""""""

.. |play-button| image:: /images/animation_toolbar/play_animation.png
  :width: 22px
  :alt: Play button

.. |anim-settings-button| image:: /images/animation_toolbar/animation_settings.png
  :width: 22px
  :alt: Animation settings button

.. image:: /images/tutorials/marker_particles/animation_settings_dialog.jpg
   :width: 26%
   :align: right

The simulation trajectory comprises 41 frames as indicated by the numbers in the timeline of 
OVITO located below the viewports. Drag the time slider with the mouse to skip through the 
frames of the trajectory, or press the :guilabel:`Play` |play-button| button in the animation toolbar to play back 
the animation in the interactive viewports in a loop.

The playback speed, i.e. the number of animation frames per second, can be adjusted in the 
:ref:`Animation settings dialog <animation.animation_settings_dialog>`. Open the dialog using
the |anim-settings-button| button in the animation toolbar and make sure the :guilabel:`Frames per second` value is set to **15**.
This parameter not only affects the playback in the interactive viewports of OVITO but also 
the frame rate of movie files written by the program.

Step 3: Create particle selection
"""""""""""""""""""""""""""""""""

.. image:: /images/tutorials/marker_particles/slice_modifier_panel.jpg
   :width: 26%
   :align: right

We now insert the :ref:`particles.modifiers.slice` modifier into the :ref:`pipeline <usage.modification_pipeline>` to select all particles within a 
narrow slab of the crystal. Open the :guilabel:`Add modification...` drop-down list and select `Slice` from the `Modification` section.
The newly inserted modifier appears as a new item in the :ref:`pipeline editor <usage.modification_pipeline.pipeline_listbox>`.

While the :ref:`particles.modifiers.slice` modifier's normal operation is to actually delete all particles on one side of the slicing plane, 
we can tell the modifier to only select the particles by activating the option :guilabel:`Create selection (do not delete)`. 
Furthermore, set the :guilabel:`Slab width` to **5.0** to make the slab five angstroms wide and check :guilabel:`Reverse orientation` to select the particles
located *in between* the two parallel planes.

Step 4: Color the marker particles
""""""""""""""""""""""""""""""""""

OVITO highlights the selected particles using a bright red color. However, the actual color of these particles did not change
yet. The red color is only visible in the interactive viewports of the program to indicate which particles are currently selected,
but if you would render an output image or a movie of the system now (see step 6), these particles would still appear 
in their original gray color like the rest of the crystal.

You have to actively change the color of the selected particles by inserting another modifier into the pipeline.
Open again the :guilabel:`Add modification...` drop-down list and select `Assign color` from the `Coloring` section.
The :ref:`particles.modifiers.assign_color` modifier assigns a new uniform color of your choice to the currently selected particles. Let's use a green color:
 
.. image:: /images/tutorials/marker_particles/intermediate_frame0.jpg
   :width: 28%

.. image:: /images/tutorials/marker_particles/intermediate_frame20.jpg
   :width: 28%

.. image:: /images/tutorials/marker_particles/intermediate_frame40.jpg
   :width: 28%

Step 5: Freeze the particle colors
""""""""""""""""""""""""""""""""""

.. image:: /images/tutorials/marker_particles/freeze_property_color.jpg
   :width: 26%
   :align: right

When looking at the time sequence above, you will notice that the set of green marker particles is not quite right yet: The green slab remains exactly straight even though
the crystal is deforming. Different particles turn green as they enter the selection region and, after leaving the region, 
they become white again.

The reason for this is that the `Slice` and `Assign color` operations are (re-)performed dynamically on each frame of the simulation trajectory.
OVITO updates the results of these modifiers automatically whenever their input state changes, i.e., when particles move during 
the course of the simulation.

Often times this is exactly the behavior one needs, but here in this tutorial it is not: We'd rather like to create a static set of green marker 
particles, which remains unaffected by the particle motion. In other words, once the particle selection has been defined at the beginning of the simulation, it needs to be *frozen* 
to preserve it across the entire timeline. For this purpose, OVITO provides the :ref:`particles.modifiers.freeze_property` modifier.

Add this modifier to the pipeline as usual and change the :guilabel:`Property to freeze` to ``Color``. This tells the modifier to 
take the original colors of the particles from animation frame 0 and override the current colors with them in all other frames of the trajectory.
Thus, the effectively assigned particle colors will now remain static, replacing the otherwise dynamic coloring produced by the combination of modifiers `Slice` and `Assign color`:

.. image:: /images/tutorials/marker_particles/final_frame0.jpg
   :width: 28%

.. image:: /images/tutorials/marker_particles/final_frame20.jpg
   :width: 28%

.. image:: /images/tutorials/marker_particles/final_frame40.jpg
   :width: 28%

.. note::

  Note that we have placed the :ref:`particles.modifiers.freeze_property` modifier at the top of the modifier stack in the pipeline editor, which means 
  it will be executed last - after the two other modifiers have performed their actions. This ordering is important for two reasons: The :ref:`particles.modifiers.freeze_property` modifier
  is only able to preserve the particle state produced by modifiers preceding it in the pipeline. The effect of subsequent modifiers, in contrast, will not be visible to `Freeze property`.
  Furthermore, we want the :ref:`particles.modifiers.freeze_property` modifier to be the last one changing the colors of the particles. Placing additional modifiers
  behind it in the pipeline, which introduce again some dynamic coloring, might undo the step of freezing the particle colors.

.. image:: /images/tutorials/marker_particles/freeze_property_selection.jpg
  :width: 26%
  :align: right

An alternative approach, leading to virtually the same results, is to let the :ref:`particles.modifiers.freeze_property` modifier freeze the *selection* state of the particles instead of their *color* state. 
To do this, reorder the modifier sequence as shown in the second screenshot and change the :guilabel:`Property to freeze` to ``Selection``. 
Now `Freeze property` will preserve the particle selection created by `Slice` in frame 0 of the trajectory, and `Assign color` will use that frozen 
selection state as input to always color the same set of particles.

Step 6: Render a movie
""""""""""""""""""""""

To complete this tutorial you will now render a movie of the simulation and save it as a video file. 

Switch to the `Render` tab of the command panel and set the rendering range to :guilabel:`Complete animation`.
Click :guilabel:`Choose...` and specify the name and format of the video file to be written by OVITO, e.g. :file:`shear_marker.mp4`. 
The option :guilabel:`Save to file` should now automatically be turned on.

.. image:: /images/tutorials/marker_particles/render_settings.jpg
  :width: 26%

Make sure the `Top` viewport is currently active. If there is no `Top` viewport, switch the current viewport
to top view using the :ref:`viewport menu <usage.viewports.menu>`. A `Top` viewport shows the current scene
from above, along the negative z-axis, using a parallel projection.

Finally, press the button :guilabel:`Render active viewport` to start the rendering process.

.. tip::

  To further refine the visualization you may want to perform a few additional actions:

  - Turn off the display of the :ref:`visual_elements.simulation_cell` visual element in the pipeline editor.
  - Adjust the display radius of the particles in the :ref:`visual_elements.particles` visual element to a value of **1.0**.
  - Activate :menuselection:`Preview Mode` in the :ref:`viewport menu <usage.viewports.menu>` to check the visible viewport region before rendering the video.

Download tutorial solution
""""""""""""""""""""""""""

In case you would like to skip right to the end of this tutorial or verify your own solution, an OVITO session state file is available.
Download the state file `shear.ovito <https://gitlab.com/stuko/ovito/-/blob/master/examples/data/shear.ovito>`__
and save it in the same folder as the trajectory file :file:`shear.dump`. Use the :menuselection:`File --> Load Session State` 
menu function to load it in OVITO. 

If you encounter any problems with this tutorial, please drop us an email at support@ovito.org to help us improve 
the instructions.
