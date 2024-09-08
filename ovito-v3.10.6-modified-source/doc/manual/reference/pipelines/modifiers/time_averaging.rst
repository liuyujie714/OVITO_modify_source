.. _particles.modifiers.time_averaging:

Time averaging |ovito-pro|
--------------------------

.. image:: /images/modifiers/time_averaging_panel.png
  :width: 30%
  :align: right

This modifier can compute the average value of one or more input quantities, averaged over the frames of the simulation trajectory.
Three kinds of input quantities are supported:

Attributes
  :ref:`Global attributes <usage.global_attributes>` are scalar variables associated with the dataset,
  which may vary with time. The modifier will compute the global mean value 
  of the selected input attribute and output it as a new attribute, which does not change with time. 

Data tables
  The modifier can compute the time-averaged versions of :ref:`data tables <scene_objects.data_table>`.
  Data tables typically contain dynamically computed structural or statistical information such as a :ref:`radial distribution function <particles.modifiers.coordination_analysis>`
  or :ref:`histograms <particles.modifiers.histogram>` of some particle property.
  With the help of the time averaging modifier you can average such time-varying tables over the entire trajectory.

Properties
  Finally, the modifier supports computing time averages of properties (e.g. :ref:`particle <scene_objects.particles>` or :ref:`voxel grid <scene_objects.voxel_grid>` properties).
  The average values of the selected input property are output as a new property with the appended name suffix "Average".
  Note that, for the computation to work, the number of data elements (e.g. particles) must not change with time. 
  Thus, make sure you place the time averaging modifier in the data pipeline before any filter operations
  that dynamically remove particles from the system.

Note that the modifier has to step through all frames of the simulation trajectory to compute the time average of the 
selected quantity. This can be a lengthy process depending on the extent of the trajectory and the dataset size. However, the averaging will happen 
in the background, and you can continue working with the program while the modifiers is performing the calculation.
Once the averaging calculation is completed, you can press the button :guilabel:`Show in data inspector` button 
to reveal the computed average quantity in the :ref:`data inspector <data_inspector>` of OVITO.

.. seealso::

  :py:class:`ovito.modifiers.TimeAveragingModifier` (Python API)
