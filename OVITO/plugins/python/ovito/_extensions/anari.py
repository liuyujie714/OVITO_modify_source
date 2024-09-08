# Load dependencies.
import ovito._extensions.pyscript

# Load the C extension module.
import ovito.plugins.AnariRendererPython

# Publish classes.
ovito.vis.__all__ += ['AnariRenderer']