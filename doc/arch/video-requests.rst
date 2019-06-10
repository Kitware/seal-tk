================
 Video Requests
================

SEAL-TK uses a request-response mechanism
for obtaining video data from video :doc:`data-sources`.
This mechanism is designed to permit a thread safe,
many-to-many mapping between video sources and consumers.

In order to issue video requests,
each consumer must have a :code:`VideoRequestor`.
The requestor is a :code:`QObject` managed by a shared pointer,
which is owned by both the consumer and any issued requests.
The use of a shared pointer ensures that the video source,
which may process requests in a separate thread,
is able to safely send a response,
even if the consumer no longer exists.

.. note::
  In order to service multiple consumers,
  the video source must be able to issue a reply to a specific consumer,
  which cannot be accomplished by means of a signal emitted by the source.
  In order to route responses to the correct consumer,
  an intermediate broker object is required.
  Additionally, the source must be able
  to directly invoke a method on this broker,
  which requires that the source is able
  to obtain a "safe" pointer to the broker
  ("safe" meaning "guaranteed to remain valid during use"),
  which can only be accomplished via shared-ownership.

A :code:`VideoRequest` consists of
a shared pointer to the requestor which is issuing the request,
a temporal specification (consisting of a time point and seek mode),
and a request identifier.
For details, refer to :code:`VideoRequest`'s API documentation.
