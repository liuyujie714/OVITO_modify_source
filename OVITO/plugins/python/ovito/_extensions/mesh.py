# Load dependencies.
import ovito._extensions.pyscript
import ovito._extensions.stdobj

# Load the C extension module.
import ovito.plugins.MeshPython

# Register import formats.
ovito.nonpublic.FileImporter._format_table["stl"] = ovito.nonpublic.STLImporter
ovito.nonpublic.FileImporter._format_table["obj"] = ovito.nonpublic.WavefrontOBJImporter
ovito.nonpublic.FileImporter._format_table["vtk/legacy/mesh"] = ovito.nonpublic.VTKFileImporter
ovito.nonpublic.FileImporter._format_table["vtk/pvd"] = ovito.nonpublic.ParaViewPVDImporter
ovito.nonpublic.FileImporter._format_table["vtk/vtm"] = ovito.nonpublic.ParaViewVTMImporter
ovito.nonpublic.FileImporter._format_table["vtk/vtp/mesh"] = ovito.nonpublic.ParaViewVTPMeshImporter

# Register export formats.
ovito.io.export_file._formatTable["vtk/trimesh"] = ovito.nonpublic.VTKTriangleMeshExporter

# Publish classes.
ovito.vis.__all__ += ['SurfaceMeshVis']
ovito.data.__all__ += ['SurfaceMesh', 'SurfaceMeshTopology']

from ovito.data import DataCollection, SurfaceMesh
from ovito.data._data_objects_dict import DataObjectsDict

# Implementation of the DataCollection.surfaces attribute.
def _DataCollection_surfaces(self):
    """
    Returns a dictionary view providing key-based access to all :py:class:`SurfaceMesh` objects in
    this data collection. Each :py:class:`SurfaceMesh` has a unique :py:attr:`~ovito.data.DataObject.identifier` key,
    which can be used to look it up in the dictionary.
    See the documentation of the modifier producing the surface mesh to find out what the right key is, or use

    .. literalinclude:: ../example_snippets/data_collection_surfaces.py
        :lines: 9-9

    to see which identifier keys exist. Then retrieve the desired :py:class:`SurfaceMesh` object from the collection using its identifier
    key, e.g.

    .. literalinclude:: ../example_snippets/data_collection_surfaces.py
        :lines: 14-15

    The view provides the convenience method :py:meth:`!surfaces.create`, which
    inserts a newly created :py:class:`SurfaceMesh` into the data collection. The method expects the unique :py:attr:`~ovito.data.DataObject.identifier`
    of the new surface mesh as first argument. All other keyword arguments are forwarded to the constructor
    to initialize the member fields of the :py:class:`SurfaceMesh` class:

    .. literalinclude:: ../example_snippets/data_collection_surfaces.py
        :lines: 21-24

    If there is already an existing mesh with the same :py:attr:`~ovito.data.DataObject.identifier` in the collection, the :py:meth:`!create`
    method modifies and returns that existing mesh instead of creating another one.
    """
    return DataObjectsDict(self, SurfaceMesh)
DataCollection.surfaces = property(_DataCollection_surfaces)

# For backward compatibility with OVITO 3.7.5:
SurfaceMesh.get_cutting_planes = lambda self: self.get_clipping_planes()
SurfaceMesh.set_cutting_planes = lambda self, planes: self.set_clipping_planes(planes)
SurfaceMesh.get_faces = lambda self: self.get_face_vertices()
