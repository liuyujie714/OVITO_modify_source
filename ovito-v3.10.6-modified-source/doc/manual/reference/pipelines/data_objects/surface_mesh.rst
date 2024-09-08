.. _scene_objects.surface_mesh:

Surface meshes
--------------

A surface mesh is a type of :ref:`data object <scene_objects>` that represents two-dimensional, closed and orientable manifolds.
Typically, surface meshes are generated within OVITO by modifiers such as :ref:`particles.modifiers.construct_surface_mesh`,
:ref:`particles.modifiers.create_isosurface` or :ref:`particles.modifiers.coordination_polyhedra`.

The appearance of a surface mesh can be controlled through the parameter panel of the :ref:`Surface mesh visual element <visual_elements.surface_mesh>`,
which is located in the topmost section of the :ref:`pipeline editor <usage.modification_pipeline.pipeline_listbox>`.

Surfaces embedded in periodic domains
"""""""""""""""""""""""""""""""""""""

.. image:: /images/visual_elements/surface_mesh_example.png
  :width: 30%
  :align: right

Surface meshes may be embedded in a periodic domain, i.e. in a simulation cell with periodic boundary conditions.
That means the triangle faces of the surface mesh can connect vertices on opposite sides of a simulation box and wrap around correctly.
OVITO takes care of computing the intersections of the periodic mesh with the box boundaries and automatically produces a non-periodic representation of the triangle mesh
when it comes to rendering the surface.

Interior and exterior region
""""""""""""""""""""""""""""

As surface meshes are closed orientable manifolds, they define an *interior* and an *exterior* spatial region,
which are separated by the surface manifold. For example, if the surface mesh was constructed by the :ref:`particles.modifiers.construct_surface_mesh` modifier
around a set of particles, then the volume enclosed by the surface represents the "filled" region and the outside region is the empty region.

Sometimes there is no interior region and the exterior region is infinite and fills all space. In this case the surface mesh is degenerate and
consists of no triangles. The opposite extreme is also possible in periodic domains: The interior region extends over the entire periodic domain
and there is no outside region. Again, the surface mesh will consist of zero triangles in this case.

Data export
"""""""""""

OVITO can export the surface as a triangle mesh.
During export, a non-periodic version is produced by truncating triangles at the periodic domain boundaries and generating "cap polygons" to fill the holes that
occur at the intersection of the interior region with the domain boundaries. To export the mesh, use OVITO's :ref:`file export function <usage.export>`
and select the output format `VTK`.

.. seealso::

  :py:class:`ovito.data.SurfaceMesh` (Python API)