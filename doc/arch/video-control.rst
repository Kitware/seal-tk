==================
 Video Controller
==================

A video controller is used to provide playback and seeking
on one or more video sources.
A controller is especially useful for synchronizing the display
of multiple video source.
In addition, a controller provides a signal-based mechanism
for representations to receive video,
which is implemented by creating an internal distributor object
for each source registered with the controller.
The distributor implements the required :code:`VideoRequestor` interface,
converting the invoked method into an emitted signal.

In addition to synchronization,
a video controller maintains a map of all frame times
across all video sources that have been added to the controller.
