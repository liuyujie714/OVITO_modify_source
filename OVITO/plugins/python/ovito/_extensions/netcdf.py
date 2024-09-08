# Load dependencies.
import ovito._extensions.pyscript
import ovito._extensions.particles

# Load the C extension module.
import ovito.plugins.NetCDFPluginPython

# Register import formats.
ovito.nonpublic.FileImporter._format_table["netcdf/amber"] = ovito.nonpublic.AMBERNetCDFImporter

# Register export formats.
ovito.io.export_file._formatTable["netcdf/amber"] = ovito.nonpublic.AMBERNetCDFExporter
