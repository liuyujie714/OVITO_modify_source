.. _visual_elements.bonds:

Bonds
-----

.. image:: /images/visual_elements/bonds_panel.jpg
  :width: 30%
  :align: right

This :ref:`visual element <visual_elements>` renders bonds between pairs of particles.
:ref:`Bonds <scene_objects.bonds>` are either loaded from the simulation file as part of the model or they can be created within OVITO by adding the
:ref:`particles.modifiers.create_bonds` modifier to the data pipeline. Alternatively, the :ref:`particles.modifiers.voronoi_analysis` modifier
is able to generate bonds between nearest neighbor particles without a distance criterion.

Parameters
""""""""""

Bond width
  Controls the width of bonds cylinders or lines (in simulation units of length). Ignored if per-bond diameter values have been assigned to the ``Width`` bond property.

Flat shading
  Switches to a flat line representation instead of three-dimensional cylinders.

Coloring mode
  Selects how the colors of the bonds are determined. 
  You can choose between (i) a uniform color used for all bonds, (ii) a colors reflecting each :ref:`bond's type <scene_objects.bonds>`,
  and (iii) adopting the colors from the particles connected by the bonds.

  .. hint:: 
  
    A fourth method of coloring the bonds is to assign explicit RGB color values to the ``Color`` property of the bonds.
    This approach gives you full control over the color of each individual bond. You can set the ``Color`` bond property
    using the :ref:`particles.modifiers.compute_property` or :ref:`particles.modifiers.color_coding` modifiers, for example.

.. seealso::

  :py:class:`ovito.vis.BondsVis` (Python API)