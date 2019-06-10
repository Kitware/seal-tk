==============
 Data Filters
==============

Data filters are subclasses of :code:`QSortFilterProxyModel`
that are used to present a subset of a data source model.
The use of Qt item models and standard filter classes
provides a single API for managing data
that is suitable for both textual and graphical representations.

Several built-in filter classes are provided.

Aggregator
----------

An aggregator is a proxy model which combines (compatible) data
from multiple data models.
Although any input model may be used,
it is most typical for the input models to be raw source models.

Scalar Data Filter
------------------

A scalar data filter filters a model
based on the numeric value of some field.
The value of the selected filter field
is compared against user-provided maximum and minimum bounds.

Most data items relate to time in some manner.
Since time is just another field,
a scalar data filter may be used as a temporal filter.

Filters can operate in two modes.
In "filter" mode, data which does not pass the filter is hidden.
In "shadow" mode, the filter continues to pass through all data items,
but modifies the presentation data for items that would not pass the filter
such that these items take on an altered appearance.

Classification Filter
---------------------

A classification filter is a special case of a scalar data filter.
Rather than operating on a single value,
a classification filter operates on a classifier,
which is a collection of confidence scores
associated with possible classification types.
Accordingly, a classification filter
also accepts a collection of thresholds
(one per classification type)
rather than a single pair of thresholds.
