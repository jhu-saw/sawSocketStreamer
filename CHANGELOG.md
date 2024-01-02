Change log
==========


1.2.0 (2024-01-02)
==================

* API changes:
  * In JSON configuration file, `data` has been replaced by `read-commands`
* Deprecated features:
  * `.rosintall` has been removed, migrated to `vcs`
* New features:
  * Added support for void and write events
  * Added void and write commands
  * Added snippet of code for C#/Unity
* Bug fixes:
  * Fixed CMake to use new cisst CMake macros, compiles with catkin and colcon

1.1.0 (2021-04-08)
==================

* API changes:
  * None
* Deprecated features:
  * None
* New features:
  * `.rosinstall` file for `wstool`
* Bug fixes:
  * Minor fix to compile with latest cisst libraries
  * Fixed README for CRTK naming conventions

1.0.0 (2019-04-09)
==================

* API changes:
  * First release!
* Deprecated features:
  * None
* New features:
  * Allows to stream data over UDP from most cisst/SAW read commands from a provided interface
* Bug fixes:
  * None
