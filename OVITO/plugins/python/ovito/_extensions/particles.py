# Load dependencies.
import ovito._extensions.pyscript
import ovito._extensions.stdobj
import ovito._extensions.stdmod
import ovito._extensions.mesh
import ovito._extensions.grid

# Load the C extension module.
import ovito.plugins.ParticlesPython

# Load class add-ons.
import ovito.modifiers._structure_identification_modifier
import ovito.modifiers._compute_property_modifier
import ovito.data._bonds_class
import ovito.data._particles_class
import ovito.data._cutoff_neighbor_finder
import ovito.data._nearest_neighbor_finder
import ovito.data._ptm_neighbor_finder
import ovito.nonpublic._lammps_data_io

# Publish classes.
ovito.vis.__all__ += ['ParticlesVis', 'VectorVis', 'BondsVis', 'TrajectoryVis']
ovito.pipeline.__all__ += ['ReferenceConfigurationModifier']
ovito.modifiers.__all__ += [
            'AmbientOcclusionModifier',
            'WrapPeriodicImagesModifier',
            'ExpandSelectionModifier',
            'StructureIdentificationModifier',
            'CommonNeighborAnalysisModifier',
            'AcklandJonesModifier',
            'CreateBondsModifier',
            'CentroSymmetryModifier',
            'ClusterAnalysisModifier',
            'CoordinationAnalysisModifier',
            'CalculateDisplacementsModifier',
            'AtomicStrainModifier',
            'WignerSeitzAnalysisModifier',
            'VoronoiAnalysisModifier',
            'IdentifyDiamondModifier',
            'LoadTrajectoryModifier',
            'PolyhedralTemplateMatchingModifier',
            'CoordinationPolyhedraModifier',
            'SmoothTrajectoryModifier',
            'GenerateTrajectoryLinesModifier',
            'UnwrapTrajectoriesModifier',
            'ChillPlusModifier',
            'ConstructSurfaceModifier']
ovito.data.__all__ += ['ParticleType', 'BondType', 'BondsEnumerator',
            'CutoffNeighborFinder', 'NearestNeighborFinder', 'PTMNeighborFinder',
            'Particles', 'Bonds', 'TrajectoryLines']

# Register import formats.
ovito.nonpublic.FileImporter._format_table["lammps/dump"] = ovito.nonpublic.LAMMPSTextDumpImporter
ovito.nonpublic.FileImporter._format_table["lammps/dump/bin"] = ovito.nonpublic.LAMMPSBinaryDumpImporter
ovito.nonpublic.FileImporter._format_table["lammps/dump/local"] = ovito.nonpublic.LAMMPSDumpLocalImporter
ovito.nonpublic.FileImporter._format_table["lammps/dump/yaml"] = ovito.nonpublic.LAMMPSDumpYAMLImporter
ovito.nonpublic.FileImporter._format_table["lammps/data"] = ovito.nonpublic.LAMMPSDataImporter
ovito.nonpublic.FileImporter._format_table["imd"] = ovito.nonpublic.IMDImporter
ovito.nonpublic.FileImporter._format_table["vasp"] = ovito.nonpublic.POSCARImporter
ovito.nonpublic.FileImporter._format_table["xyz"] = ovito.nonpublic.XYZImporter
ovito.nonpublic.FileImporter._format_table["fhi-aims"] = ovito.nonpublic.FHIAimsImporter
ovito.nonpublic.FileImporter._format_table["fhi-aims/log"] = ovito.nonpublic.FHIAimsLogFileImporter
ovito.nonpublic.FileImporter._format_table["gsd/hoomd"] = ovito.nonpublic.GSDImporter
ovito.nonpublic.FileImporter._format_table["castep/cell"] = ovito.nonpublic.CastepCellImporter
ovito.nonpublic.FileImporter._format_table["castep/md"] = ovito.nonpublic.CastepMDImporter
ovito.nonpublic.FileImporter._format_table["cfg"] = ovito.nonpublic.CFGImporter
ovito.nonpublic.FileImporter._format_table["cif"] = ovito.nonpublic.CIFImporter
ovito.nonpublic.FileImporter._format_table["mmcif"] = ovito.nonpublic.mmCIFImporter
ovito.nonpublic.FileImporter._format_table["gaussian/cube"] = ovito.nonpublic.GaussianCubeImporter
ovito.nonpublic.FileImporter._format_table["dlpoly"] = ovito.nonpublic.DLPOLYImporter
ovito.nonpublic.FileImporter._format_table["gro"] = ovito.nonpublic.GroImporter
ovito.nonpublic.FileImporter._format_table["xtc"] = ovito.nonpublic.XTCImporter
ovito.nonpublic.FileImporter._format_table["reaxff/bonds"] = ovito.nonpublic.ReaxFFBondImporter
ovito.nonpublic.FileImporter._format_table["parcas"] = ovito.nonpublic.ParcasFileImporter
ovito.nonpublic.FileImporter._format_table["pdb"] = ovito.nonpublic.PDBImporter
ovito.nonpublic.FileImporter._format_table["quantumespresso"] = ovito.nonpublic.QuantumEspressoImporter
ovito.nonpublic.FileImporter._format_table["vtk/vtp/particles"] = ovito.nonpublic.ParaViewVTPParticleImporter
ovito.nonpublic.FileImporter._format_table["xsf"] = ovito.nonpublic.XSFImporter
ovito.nonpublic.FileImporter._format_table["dcd"] = ovito.nonpublic.DCDImporter

# Register export formats.
ovito.io.export_file._formatTable["lammps/dump"] = ovito.nonpublic.LAMMPSDumpExporter
ovito.io.export_file._formatTable["lammps/data"] = ovito.nonpublic.LAMMPSDataExporter
ovito.io.export_file._formatTable["imd"] = ovito.nonpublic.IMDExporter
ovito.io.export_file._formatTable["vasp"] = ovito.nonpublic.POSCARExporter
ovito.io.export_file._formatTable["xyz"] = ovito.nonpublic.XYZExporter
ovito.io.export_file._formatTable["fhi-aims"] = ovito.nonpublic.FHIAimsExporter
ovito.io.export_file._formatTable["gsd/hoomd"] = ovito.nonpublic.GSDExporter

# For backward compatibility with OVITO 2.9.0:
ovito.io.export_file._formatTable["lammps_dump"] = ovito.nonpublic.LAMMPSDumpExporter
ovito.io.export_file._formatTable["lammps_data"] = ovito.nonpublic.LAMMPSDataExporter

# For backward compatibility with OVITO 2.9.0:
ovito.modifiers.CoordinationNumberModifier = ovito.modifiers.CoordinationAnalysisModifier
ovito.modifiers.InterpolateTrajectoryModifier = ovito.modifiers.SmoothTrajectoryModifier
ovito.modifiers.__all__ += ['CoordinationNumberModifier', 'InterpolateTrajectoryModifier']

from ovito.data import DataCollection, Particles, TrajectoryLines

# Implementation of the DataCollection.particles attribute.
def _DataCollection_particles(self):
    """
    Returns the :py:class:`Particles` object, which manages all :ref:`per-particle properties <particle_properties_intro>`.
    It may be ``None`` if the data collection contains no particle model at all.

    .. important::

        The :py:class:`Particles` data object returned by this attribute may be marked as read-only,
        which means attempts to modify its contents will raise a Python error.
        This is typically the case if the data collection was produced by a pipeline and all data objects are owned by the system.

    If you intend to modify the contents of the :py:class:`Particles` object in some way, use the :py:attr:`!particles_`
    attribute instead to explicitly request a mutable version of the particles object. See topic :ref:`underscore_notation` for more information.
    Use :py:attr:`!particles` for read access and :py:attr:`!particles_` for write access, e.g. ::

        print(data.particles.positions[0])
        data.particles_.positions_[0] += (0.0, 0.0, 2.0)

    To create a new :py:class:`Particles` object in a data collection that might not have particles yet, use the
    :py:meth:`create_particles` method or simply assign a new instance of the :py:class:`Particles` class to the :py:attr:`!particles` attribute.
    """
    return self._find_object_type(Particles)
# Implement the assignment of a Particles object to the DataCollection.particles field.
def _DataCollection_set_particles(self, obj):
    assert(obj is None or isinstance(obj, Particles)) # Must assign a Particles data object to this field.
    # Check if there already is an existing Particles object in the DataCollection.
    # If yes, first remove it from the collection before adding the new one.
    existing = self._find_object_type(Particles)
    if existing is not obj:
        if not existing is None: self.objects.remove(existing)
        if not obj is None: self.objects.append(obj)
DataCollection.particles = property(_DataCollection_particles, _DataCollection_set_particles)

# Implementation of the DataCollection.particles_ attribute.
DataCollection.particles_ = property(lambda self: self.make_mutable(self.particles), _DataCollection_set_particles)

# Implementation of the DataCollection.trajectories attribute.
# For backward compatibility with OVITO 3.9.2:
def _DataCollection_trajectories(self):
    return self._find_object_type(TrajectoryLines)
DataCollection.trajectories = property(_DataCollection_trajectories)

# Implementation of the DataCollection.trajectories_ attribute.
DataCollection.trajectories_ = property(lambda self: self.make_mutable(self.trajectories))
