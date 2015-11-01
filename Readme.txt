This is the Readme file for hid-keyborad, an example of a USB keyborad device.


WHAT IS DEMONSTRATED?
=====================
This example demonstrates how HID class devices are implemented. The example
is kept as simple as possible, except the report descriptor which is taken
from a real-world keyborad.

It does NOT include a host side driver because all modern operating systems
include one. It does NOT implement USBRQ_HID_SET_REPORT and report-IDs. See
the "hid-data" example for this topic. It does NOT implement any special
features such as suspend mode etc.


PREREQUISITES
=============
Target hardware: You need an AVR based circuit based on one of the examples
(see the "circuits" directory at the top level of this package), e.g. the
metaboard (http://www.obdev.at/goto.php?t=metaboard).

AVR development environment: You need the gcc tool chain for the AVR, see
the Prerequisites section in the top level Readme file for how to obtain it.


BUILDING THE FIRMWARE
=====================
Change to the "firmware" directory and modify Makefile according to your
architecture (CPU clock, target device, fuse values) and ISP programmer. Then
edit usbconfig.h according to your pin assignments for D+ and D-. The default
settings are for the metaboard hardware.

Type "make hex" to build main.hex, then "make flash" to upload the firmware
to the device. Don't forget to run "make fuse" once to program the fuses. If
you use a prototyping board with boot loader, follow the instructions of the
boot loader instead.

Please note that the first "make hex" copies the driver from the top level
into the firmware directory. If you use a different build system than our
Makefile, you must copy the driver by hand.

RESPONSIBILITY
==============

YOU CAN USE THIS CODE FOR YOUR OWN RESPONSIBILITY

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

----------------------------------------------------------------------------
(c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH.
http://www.obdev.at/
