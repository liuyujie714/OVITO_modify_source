# Load dependencies.
import ovito._extensions.pyscript
import ovito._extensions.stdobj

# Load the C extension module.
import ovito.plugins.TimeAveragingPython

# Publish classes.
ovito.modifiers.__all__ += ['TimeAveragingModifier', 'TimeSeriesModifier']
