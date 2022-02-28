# Copyright 2016 Emilie Gillet.
# 
# Author: Emilie Gillet (emilie.o.gillet@gmail.com)
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# See http://creativecommons.org/licenses/MIT/ for more information.

# System specifications
F_CRYSTAL      = 8000000L
F_CPU          = 72000000L
SYSCLOCK       = SYSCLK_FREQ_72MHz
FAMILY         = f37x
# USB            = enabled

APPLICATION_LARGE = TRUE
BOOTLOADER        = ArpOn_bootloader

# Prefered upload command
UPLOAD_COMMAND  = upload_combo_jtag_erase_first

# Packages to build
TARGET         = ArpOn
PACKAGES       = ArpOn \
		ArpOn/drivers \
		ArpOn/dsp \
		ArpOn/dsp/fx \
		ArpOn/dsp/drum_modelling \
		ArpOn/dsp/engine \
		ArpOn/dsp/physical_modelling \
		ArpOn/dsp/speech \
		stmlib/dsp \
		stmlib/utils \
		stmlib/system
RESOURCES      = ArpOn/resources

TOOLCHAIN_PATH ?= /usr/local/arm-4.8.3/

include stmlib/makefile.inc

# Rule for building the firmware update file
wav:  $(TARGET_BIN)
	python stm_audio_bootloader/qpsk/encoder.py \
		-t stm32f3 -k -s 48000 -b 12000 -c 6000 -p 256 \
		$(TARGET_BIN)
