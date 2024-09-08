# Load dependencies.
import ovito._extensions.pyscript

# Load the C extension module.
import ovito.plugins.StdObjPython

# Load class add-ons.
import ovito.data._data_table
import ovito.data._property_class
import ovito.data._property_container
import ovito.data._lines
import ovito.data._simulation_cell
import ovito.data._ovito_ndarray_adapter

# Publish classes.
ovito.data.__all__ += [
    "Lines",
    "TrajectoryLines",
    "SimulationCell",
    "Property",
    "PropertyContainer",
    "DataTable",
    "ElementType",
]
ovito.vis.__all__ += ["LinesVis", "SimulationCellVis", "TrajectoryLinesVis"]

# Register export formats.
ovito.io.export_file._formatTable["txt/table"] = ovito.nonpublic.DataTableExporter
# For backward compatibility with older development versions of OVITO:
ovito.io.export_file._formatTable["txt/series"] = ovito.nonpublic.DataTableExporter

from ovito.data import DataCollection, DataTable, Lines, SimulationCell
from ovito.data._data_objects_dict import DataObjectsDict


# Implementation of the DataCollection.tables attribute.
def _DataCollection_tables(self):
    """
    A dictionary view of all :py:class:`DataTable` objects in
    this data collection. Each :py:class:`DataTable` has a unique :py:attr:`~ovito.data.DataObject.identifier` key,
    which allows it to be looked up in this dictionary. Use

    .. literalinclude:: ../example_snippets/data_collection_tables.py
        :lines: 9-9

    to find out which table identifiers are present in the data collection.
    Then use the identifier to retrieve the desired :py:class:`DataTable` from the dictionary, e.g.

    .. literalinclude:: ../example_snippets/data_collection_tables.py
        :lines: 14-15

    The view provides the convenience method :py:meth:`!tables.create`, which
    inserts a newly created :py:class:`DataTable` into the data collection. The method expects the unique :py:attr:`~ovito.data.DataObject.identifier`
    of the new data table as first argument. All other keyword arguments are forwarded to the constructor
    to initialize the member fields of the :py:class:`DataTable` class:

    .. literalinclude:: ../example_snippets/data_collection_tables.py
        :lines: 21-34

    If there is already an existing table with the same :py:attr:`~ovito.data.DataObject.identifier` in the collection, the :py:meth:`!create`
    method modifies and returns that existing table instead of creating another one.
    """
    return DataObjectsDict(self, DataTable)


DataCollection.tables = property(_DataCollection_tables)


# Implementation of the DataCollection.cell attribute.
def _DataCollection_cell(self):
    """
    Returns the :py:class:`SimulationCell` data object describing the cell vectors and periodic boundary
    condition flags. It may be ``None``.

    .. important::

        The :py:class:`SimulationCell` data object returned by this attribute may be marked as read-only,
        which means your attempts to modify the cell object will raise a Python error.
        This is typically the case if the data collection was produced by a pipeline and its objects are owned by the system.

    If you intend to modify the :py:class:`SimulationCell` data object within this data collection, use the :py:attr:`!cell_`
    attribute instead to explicitly request a mutable version of the cell object. See topic :ref:`underscore_notation` for more information.
    Use :py:attr:`!cell` for read access and :py:attr:`!cell_` for write access, e.g. ::

        print(data.cell.volume)
        data.cell_.pbc = (True, True, False)

    To create a :py:class:`SimulationCell` in a data collection that might not have a simulation cell yet, use the
    :py:meth:`create_cell` method or simply assign a new instance of the :py:class:`SimulationCell` class to the :py:attr:`!cell` attribute.
    """
    return self._find_object_type(SimulationCell)


# Implement the assignment of a SimulationCell object to the DataCollection.cell field.
def _DataCollection_set_cell(self, obj):
    assert obj is None or isinstance(
        obj, SimulationCell
    )  # Must assign a SimulationCell data object to this field.
    # Check if there already is an existing SimulationCell object in the DataCollection.
    # If yes, first remove it from the collection before adding the new one.
    existing = self._find_object_type(SimulationCell)
    if existing is not obj:
        if not existing is None:
            self.objects.remove(existing)
        if not obj is None:
            self.objects.append(obj)


DataCollection.cell = property(_DataCollection_cell, _DataCollection_set_cell)

# Implementation of the DataCollection.cell_ attribute.
DataCollection.cell_ = property(
    lambda self: self.make_mutable(self.cell), _DataCollection_set_cell
)


# Implementation of the DataCollection.lines attribute.
def _DataCollection_lines(self):
    """
    A dictionary view providing key-based access to all :py:class:`Lines` objects in
    this data collection. Each :py:class:`Lines` object has a unique :py:attr:`~ovito.data.DataObject.identifier` key,
    which can be used to look it up in the dictionary. You can use

    .. literalinclude:: ../example_snippets/data_collection_lines.py
        :lines: 13-13

    to see which identifiers exist. Then retrieve the desired :py:class:`Lines` object from the collection using its identifier
    key, e.g.

    .. literalinclude:: ../example_snippets/data_collection_lines.py
        :lines: 18-19

    The :py:class:`Lines` object with the identifier ``"trajectories"``, for example, is the one that gets
    created by the :py:class:`~ovito.modifiers.GenerateTrajectoryLinesModifier`.

    If you would like to create a new :py:class:`Lines` object, in a user-defined modifier for instance,
    the dictionary view provides the method :py:meth:`!lines.create`, which
    creates a new :py:class:`Lines` and adds it to the data collection. The method expects the unique :py:attr:`~ovito.data.DataObject.identifier`
    of the new lines object as first argument. All other keyword arguments are forwarded to the class constructor
    to initialize the member fields of the :py:class:`Lines` object:

    .. literalinclude:: ../example_snippets/data_collection_lines.py
        :lines: 24-24

    If there is already an existing :py:class:`Lines` object with the same :py:attr:`~ovito.data.DataObject.identifier` in the collection, the :py:meth:`!create`
    method returns that object instead of creating another one and makes sure it can be safely modified.
    """
    return DataObjectsDict(self, Lines)
DataCollection.lines = property(_DataCollection_lines)

# For backward compatibility with OVITO 3.9.2:
ovito.data.TrajectoryLines = ovito.data.Lines
ovito.vis.TrajectoryVis = ovito.vis.LinesVis
ovito.vis.TrajectoryLinesVis = ovito.vis.LinesVis
