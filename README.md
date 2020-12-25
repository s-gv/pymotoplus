# pymotoplus

Python interface to Yaskawa MotoPlus API. An ad hoc wire protocol over UDP is used to communicate
with the Yaskawa controller (YRC1000, YRC1000micro etc.).

The implementation is incomplete, but many common operations such as movement and impedance control
are available.

Load `output/pymotoplus.out` on the teach pendant and connect the PC to the Yaskawa controller via ethernet.
See `sample.py` or the `main()` function in `pymotoplus.py` for examples of how to use this library.

## Requirements
- Python 3.6

