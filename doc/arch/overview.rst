==========
 Overview
==========

SEAL-TK is designed to provide a flexible framework
for creating GUI visualization and analysis applications
for computer vision tools and algorithms.
Its overall design is based on lessons learned
developing previous generations of such tools,
especially the ViViA_ suite of applications.

Particular design goals include:

  - The framework should support remove of a data source
    from the collection of active data sources.

  - The framework should not be limited to a single set
    of associated data sources.

  - The framework should support concurrent display of multiple data sets.

  - The framework should support multiple associated imagery (video) sources
    within a single data set.

SEAL-TK attempts to follow the MVF (model, view, filter) design paradigm.
Data sources are designed as independent data models,
which can be independently channeled to multiple views,
moderated by multiple controllers and/or filters.
Views and connecting code should support the removal of a data source.

Applications built on top of the underlying framework classes
may impose additional restrictions
on the interaction of models, views and controllers.
The objective is for the framework itself to support development
of applications with an arbitrary set of such restrictions,
or with no such restriction at all.

.. _ViViA: https://github.com/kitware/vivia
