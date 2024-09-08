# Load dependencies.
import ovito._extensions.pyscript

# Load the C extension module.
import ovito.plugins.ThreeJSPython

# Load class add-ons.
import ovito.vis._jupyter_widget

def _jupyter_labextension_paths():
    # Called by Jupyter Lab Server to detect if it is a valid labextension and
    # to install the widget.

    # Returns
    # =======
    # src: Source directory name to copy files from. Webpack outputs generated files
    #     into this directory and Jupyter Lab copies from this directory during
    #     widget installation
    # dest: Destination directory name to install widget files to. Jupyter Lab copies
    #     from `src` directory into <jupyter path>/labextensions/<dest> directory
    #     during widget installation
    return [{
        'src': 'labextension',
        'dest': 'jupyter-ovito',
    }]

def _jupyter_nbextension_paths():
    # Called by Jupyter Notebook Server to detect if it is a valid nbextension and
    # to install the widget.

    # Returns
    # =======
    # section: The section of the Jupyter Notebook Server to change.
    #     Must be 'notebook' for widget extensions
    # src: Source directory name to copy files from. Webpack outputs generated files
    #     into this directory and Jupyter Notebook copies from this directory during
    #     widget installation
    # dest: Destination directory name to install widget files to. Jupyter Notebook copies
    #     from `src` directory into <jupyter path>/nbextensions/<dest> directory
    #     during widget installation
    # require: Path to importable AMD Javascript module inside the
    #     <jupyter path>/nbextensions/<dest> directory
    return [{
        'section': 'notebook',
        'src': 'nbextension',
        'dest': 'jupyter-ovito',
        'require': 'jupyter-ovito/extension'
    }]

# Place the special functions, which register the Jupyter lab and notebook extensions, into the main ovito module.
ovito._jupyter_labextension_paths = _jupyter_labextension_paths
ovito._jupyter_nbextension_paths = _jupyter_nbextension_paths
