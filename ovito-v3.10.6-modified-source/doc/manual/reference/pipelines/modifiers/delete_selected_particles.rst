.. _particles.modifiers.delete_selected_particles:

Delete selected
---------------

.. image:: /images/modifiers/delete_selected_panel.png
  :width: 35%
  :align: right

This modifier deletes all currently selected data elements (i.e. particles, bonds).

The checkbox list in the modifier's parameter panel lets you control which classes of data elements (particles, bonds, etc.) the modifier
should act on. Element classes that are not present in the modifier's pipeline input, are grayed out.
The modifier will delete those data elements from the selected classes whose ``Selection`` property is non-zero.

.. seealso::

  :py:class:`ovito.modifiers.DeleteSelectedModifier` (Python API)
