# Load dependencies.
import ovito._extensions.pyscript
import ovito._extensions.grid
import ovito._extensions.particles

# Load the C extension module.
import ovito.plugins.SpatialBinningPython

# Publish classes.
ovito.modifiers.__all__ += ['SpatialBinningModifier']
