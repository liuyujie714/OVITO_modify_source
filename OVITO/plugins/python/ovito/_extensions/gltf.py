# Load dependencies.
import ovito._extensions.pyscript

# Load the C extension module.
import ovito.plugins.GLTFPython

# Register export formats.
ovito.io.export_file._formatTable["gltf"] = ovito.nonpublic.GLTFExporter
