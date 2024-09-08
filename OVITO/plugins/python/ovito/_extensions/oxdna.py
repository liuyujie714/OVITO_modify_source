# Load dependencies.
import ovito._extensions.pyscript
import ovito._extensions.particles

# Load the C extension module.
import ovito.plugins.oxDNAPython

# Register import formats.
ovito.nonpublic.FileImporter._format_table["oxdna"] = ovito.nonpublic.OXDNAImporter