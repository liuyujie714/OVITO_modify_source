.. _usage.data_model:
.. _usage.particle_properties:

Data model
==========

This page gives an introduction to the data model used by OVITO to represent molecular structures and other particle-based datasets.
Understanding the basic concepts of OVITO's data model is important for you to work efficiently with the data analysis and
visualization functions of the program, which operate on particle simulation data.

Particle properties
-------------------

:ref:`Particle properties <scene_objects.particles>` are numeric values associated with individual particles.
They play a central role in the data model of OVITO and the way molecular and other structures are represented. 
Typical particle properties are the particle position, chemical type, and velocity. The user can assign any number of additional properties to particles, 
either explicitly or as a result of computations performed by the program.

This general property concept is employed for other types of data elements as well, not only particles. 
For instance, the :ref:`interatomic bonds <scene_objects.bonds>` may be associated with *bond properties*, e.g. bond type or color. 
Keep in mind that, while the following introduction focuses mainly on particles and their properties,
the same principles apply also to other classes of data elements.

What's a particle property?
---------------------------

Technically, an OVITO particle property is a uniform data array containing a numeric value for each particle in the system. All values
are of the same data type, which may be scalar or vectorial. The length of the property array is always equal
to the number of particles in the system. 

Each property has a unique name, for example, ``Position`` or ``Potential Energy``. 
OVITO has a built-in list of :ref:`commonly-used property names <particle-properties-list>`, which have a meaning to the program and a prescribed data layout, but
you are free to define additional properties with user-defined names. 

The ``Position`` particle property is special because it is always present. Particles cannot exist without spatial coordinates. 
Other standard properties such as ``Color``, ``Radius``, or ``Selection`` are optional. They may or may not be present. 
If they are not already loaded from the imported simulation file, you can add them within the program using various functions. 
The mentioned standard properties :ref:`affect how OVITO renders the particles <usage.particle_properties.special>`. 
By assigning values to these properties, you can control the visual appearance of the particles.

In OVITO, per-particle property values can have different data types (real or integer) and dimensionality (e.g. scalar, vector, tensor). 
The ``Position`` property, for instance, is a vector property with three components per particle, referred to as 
``Position.X``, ``Position.Y`` and ``Position.Z`` within OVITO's user interface. 

Inspecting properties
---------------------

.. figure:: /images/usage/properties/particle_inspection_example.*
   :figwidth: 50%
   :align: right

   Data inspector displaying the table of particle properties

Standard properties such as ``Position``, ``Particle Type``, or ``Velocity`` are typically initialized from the 
imported simulation file. Some file formats such as *LAMMPS dump* or the `extended XYZ format <http://libatoms.github.io/QUIP/io.html#module-ase.io.extxyz>`_
can store an arbitrary number of extra data columns. These auxiliary attributes are automatically mapped to corresponding particle properties within OVITO.

To find out which properties are currently associated with the particles, you can open OVITO's :ref:`Data inspector <data_inspector>` panel, 
which is shown in the screenshot on the right. Alternatively, you can simply point the mouse cursor at some particle in the viewports to let OVITO display 
its property values in the status bar.

Assigning property values
-------------------------

OVITO provides a rich set of functions for modifying the properties of particles. These so-called *modifiers*
will be introduced in more detail in a later section of this manual. But to already give you a first idea of the principle:
The :ref:`particles.modifiers.assign_color` modifier function lets you assign a uniform color of your choice
to all currently selected particles. It does that by setting the ``Color`` property of the
particles to the given RGB value (if the ``Color`` property doesn't exist yet, it is automatically created). 
The subset of currently selected particles is determined by the ``Selection`` particle property: Particles whose ``Selection``
property has a non-zero value are part of the current selection set, while particles for which ``Selection=0`` are not selected.

Fittingly, OVITO provides a number of selection modifiers, which let you define a particle selection set by appropriately setting the values of the ``Selection`` property.
For example, the :ref:`Select type <particles.modifiers.select_particle_type>` modifier takes the ``Particle Type``
property of each particle to decide whether or not to select that particle. It allows you to select all atoms of a particular chemical type, for example,
and then perform some operation only on that subset of particles.

Another typical modifier is the :ref:`particles.modifiers.coordination_analysis` modifier.
It computes the number of neighbors of each particle within a given cutoff range and stores the computation results in a new particle property named ``Coordination``. 
Subsequently, you can refer to the values of this property, e.g., to select particles having a coordination number in a certain range
or to color particles based on their coordination number (see :ref:`particles.modifiers.color_coding` modifier).

Of course, it is possible to export the particle property values to an output file. OVITO supports a variety of output formats for that (see the 
:ref:`data export <usage.export>` section of this manual). For instance, the *XYZ* format is a simple table
format supporting an arbitrary set of output columns.
