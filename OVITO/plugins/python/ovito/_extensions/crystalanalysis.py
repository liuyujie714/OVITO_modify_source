# Load dependencies.
import ovito._extensions.pyscript
import ovito._extensions.particles
import ovito._extensions.mesh

# Load the C extension module.
import ovito.plugins.CrystalAnalysisPython

# Load class add-ons.
import ovito.modifiers._grain_segmentation_modifier
import ovito.modifiers._elastic_strain_modifier
import ovito.vis._dislocation_vis

# Publish classes.
ovito.vis.__all__ += ['DislocationVis']
ovito.modifiers.__all__ += ['DislocationAnalysisModifier', 'ElasticStrainModifier', 'GrainSegmentationModifier']
ovito.data.__all__ += ['DislocationNetwork']
# For backward compatibility with OVITO 3.10.1:
ovito.data.__all__ += ['DislocationSegment']

# Register import formats.
ovito.nonpublic.FileImporter._format_table["ca"] = ovito.nonpublic.CAImporter

# Register export formats.
ovito.io.export_file._formatTable["ca"] = ovito.nonpublic.CAExporter
ovito.io.export_file._formatTable["vtk/disloc"] = ovito.nonpublic.VTKDislocationsExporter

from ovito.data import DataCollection, DislocationNetwork

# Implementation of the DataCollection.dislocations attribute.
def _DataCollection_dislocations(self):
    """
    Returns the :py:class:`DislocationNetwork` data object; or ``None`` if there
    is no object of this type in the collection. Typically, the :py:class:`DislocationNetwork` is created by a :ref:`pipeline <modifiers_overview>`
    containing the :py:class:`~ovito.modifiers.DislocationAnalysisModifier`.
    """
    return self._find_object_type(DislocationNetwork)
DataCollection.dislocations = property(_DataCollection_dislocations)

# Returns a mutable version of the DislocationNetwork object.
DataCollection.dislocations_ = property(lambda self: self.make_mutable(self.dislocations))

# For backward compatibility with OVITO 3.10.1:
ovito.data.DislocationSegment = DislocationNetwork.Line
DislocationNetwork.Line.set_segment = lambda self, **args: self.set_line(**args)