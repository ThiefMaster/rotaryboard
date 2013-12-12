RotaryBoard
===========

RotaryBoard is a small piece of AVR-powered hardware that contains a bunch of rotary encoders with
LEDs and buttons. All events are sent via the UART to another device, usually a PC, which then acts
on them and controls the LEDs on the board.

Its purpose is to control various volumes on my PC using hardware buttons since that is usually
much nicer and faster than doing it using a GUI on the PC.


Schematics
----------

Schematics and a PCB layout (160x100 stripboard) can be found in the schematics folder. The
schematics has been created using *Eagle 6.5* and the layout was made with *Sprint Layout 6.0*.
While free viewers are available for both formats, I've included images of both the `schematics`_
and the `PCB layout`_ for those who are just curious and (like myself) don't feel like installing
additional software.

The rotary encoders I used are `DPL12SV2424A25K3`_. They have 24 steps and detents, a push button
and two LEDs (e.g. red/green).


License
-------

Copyright © 2013 Adrian Mönnich (adrian@planetcoding.net). Released under the MIT License, see
`LICENSE`_ for details.

.. _schematics: https://raw.github.com/ThiefMaster/rotaryboard/master/schematics/rotaryboard.png
.. _PCB layout: https://raw.github.com/ThiefMaster/rotaryboard/master/schematics/rotaryboard-layout.png
.. _DPL12SV2424A25K3: http://octopart.com/dpl12sv2424a25k3-te+connectivity-8061992
.. _LICENSE: https://github.com/ThiefMaster/rotaryboard/blob/master/LICENSE

