.. _scene_objects.simulation_cell:

Simulation cell
---------------

.. image:: /images/scene_objects/simulation_cell_panel.png
  :width: 30%
  :align: right

This :ref:`data object <scene_objects>` represents the geometry of the two- or three-dimensional simulation domain
and the boundary conditions (PBC flags). You can find the cell object that was loaded from the imported simulation file in the 
`Data source` section of the :ref:`pipeline editor <usage.modification_pipeline.pipeline_listbox>` (see screenshot).
Note that the pipeline editor usually shows another item also called `Simulation cell`.
This is the corresponding :ref:`visual element <visual_elements.simulation_cell>` controlling the appearance of the simulation cell in rendered images.

Some simulation file formats do not store the dimensions of the simulation cell that was used in the original simulation.
If OVITO cannot find the cell dimensions in the imported file, it automatically generated an ad-hoc simulation box that corresponds to the axis-aligned 
bounding box of all particle coordinates.

Dimensionality
""""""""""""""

OVITO supports 2- and 3-dimensional datasets. The *dimensionality* detected by OVITO during file import can be overridden by the user in the simulation cell panel.
In mode :guilabel:`2D`, the z-coordinates of particles and the third simulation cell vector will be ignored by most computations performed by OVITO.

Boundary conditions
"""""""""""""""""""

The periodic boundary condition (PBC) flags of the simulation cell determine whether OVITO performs calculations within
a periodic domain or not. If possible, OVITO tries to read or guess these flags from the imported simulation file, 
but you can manually override them here if needed.

Geometry
""""""""

The shape of the three-dimensional simulation cell is defined by three vectors spanning a parallelepiped and the coordinates of 
one corner of the parallelepiped (the cell's origin).
The panel displays the original shape information loaded from the imported simulation file. 
To modify the simulation cell vectors, you can insert the :ref:`particles.modifiers.affine_transformation` modifier into the data pipeline.

.. seealso::

  :py:class:`ovito.data.SimulationCell` (Python API)