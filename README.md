# godot-cacheserv
Cache server module for godot engine

This module was made as part of the Google Summer of Code 2019 program. The purpose of this module is to allow Godot engine to shift from using OS based caching for any IO to its own caching, relying only on fast unbuffered IO on the OS side. This allows for ease of use within costrained environments such as game consoles and other limited hardware.

[Here][1] is a list of all the commits I've made during the project time period.

I've also made a [design document][2] that attempts to explain how everything works.

[1]: https://github.com/WarpspeedSCP/godot/commits?author=WarpspeedSCP
[2]: https://docs.google.com/document/d/1u5pnouYPkF44VpupJ3J_TUTM_RS5JVG2fOLJKAT9QU4
