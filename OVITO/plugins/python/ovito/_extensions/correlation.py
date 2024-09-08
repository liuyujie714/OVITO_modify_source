# Load dependencies.
import ovito._extensions.pyscript
import ovito._extensions.particles

# Load the C extension module.
import ovito.plugins.CorrelationFunctionPluginPython

# Publish classes.
ovito.modifiers.__all__ += ['SpatialCorrelationFunctionModifier']
