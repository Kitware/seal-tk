==============
 Data Sources
==============

A data source is a class which provides data.
Within the MVC paradigm, a data source fills the Model role.
An instance of a data source should describe a priori
the types of data which may be provided by that source,
however a source is abstractly capable
of providing any and all sorts of data.

Data (except for video data) is provided via a data model.
The data model implements the Qt item model (:code:`QAbstractItemModel`),
and may be cloned in order to allow a snapshot of the data
to be used by other threads.

Video
-----

A video source is one which provides imagery,
consisting of one or more "frames",
each of which has associated metadata.
Video sources index frames according to time value.

Each frame has one or more of the following:

 - Pixel data (required)
 - Frame time (required)
 - Frame number (required)
 - Frame name
 - Frame transform

A frame transform, if present, describes a mapping
from the frame image's space to a "scene" space.
Such a transform makes it possible to map data
(image pixels, detections, etc.) from several video sources
into a homogenous representation.

The video source may supply additional data which is not frame specific:

 - Video name
 - Temporal datum
 - Spatial datum

Datums are used to identify, without user intervention,
data sources which can reasonably be combined into a single view.

Unlike other data types,
video data is not provided as a Qt item model.
Instead, consumers must issue requests for data,
which the source typically services asynchronously.

Detections
----------

A detection source provides a collection of detections as a flat list.

A detection has one or more of the following:

 - Identifier
 - Temporal location
 - Spatial location
 - Object type classification
 - Descriptor

Tracks
------

A track source provides a collection of tracks,
which are linked sets of associated detections
(usually corresponding to a tracked object).
The data is organized into a tree,
with each parent node representing a track,
and each child node referencing a detection.

A track has one or more of the following:

 - Identifier
 - Track type classification
 - Collection of associated detections

Activities
----------

An activity source provides a collection of activities.
The data is organized into a tree,
with each parent node representing an activity,
and each child node referencing a track segment.

An activity has one or more of the following:

 - Identifier
 - Activity type classification
 - Textual description
 - Collection of associated track intervals
