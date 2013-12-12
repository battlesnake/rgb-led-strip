#!/usr/bin/python3

timeslice = 20
steps = 20

pins = [2,1,3]

# Imports for PiFace, timing and user interface
from time import sleep
import pifacedigitalio as pfdio
import signal
import sys
import curses

# Ctrl+C handler, performs clean-up before terminating
def signal_handler(signal, frame):
	terminate()

# Register Ctrl+C handler
signal.signal(signal.SIGINT, signal_handler)

# Clean shutdown of the driver (clears PiFace ports)
def terminate():
	global pins
	for pin in pins:
		pfd.output_pins[pin].turn_off()
	sys.exit(0)
	
def percent(num):
	return (('%.1f' % (num * 100.0 / steps)) + "%  ")

def degrees(num):
	return (('%.1f' % (num * 360.0 / steps)) + "°  ")

# PiFace interface
pfdio.init()
pfd = pfdio.PiFaceDigital()

def main(scr):
	global timeslice, steps, pins
	hue = 0
	sat = 0
	val = 0
	scr.nodelay(1)
	# Main loop
	while True:
		# Driver state
		scr.addstr(2, 2, "Hue: " + degrees(hue))
		scr.addstr(4, 2, "Sat: " + percent(sat))
		scr.addstr(6, 2, "Val: " + percent(val))
		# Ugly input handling (Python lacks switch/select/case)
		c = None
		while 1:
			c = scr.getch()
			if c == curses.KEY_LEFT:
				hue = max(0, hue - 1)
			elif c == curses.KEY_RIGHT:
				hue = min(steps, hue + 1)
			elif c == curses.KEY_DOWN:
				val = max(0, val - 1)
			elif c == curses.KEY_UP:
				val = min(steps, val + 1)
			elif c == curses.KEY_NPAGE:
				sat = max(0, sat - 1)
			elif c == curses.KEY_PPAGE:
				sat = min(steps, sat + 1)
			# Exit
			elif c == ord('q'):
				return
			# No key to handle
			if c == curses.ERR:
				break
		# HSV -> RGB
		H = hue * 1.0 / steps
		S = sat * 1.0 / steps
		V = val * 1.0 / steps
		C = V * S
		X = C * (1 - abs(((H * 6) % 2) - 1))
		m = V - C
		pythonsucks = {
			0: [C,X,0],
			1: [X,C,0],
			2: [0,C,X],
			3: [0,X,C],
			4: [X,0,C],
			5: [C,0,X]
		}
		k = int(H * 6) % 6
		rgb = [int((x + m) * timeslice) for x in pythonsucks[k]]
		scr.addstr(30, 2, "RGB: {0}, {1}, {2}    ".format(rgb[0], rgb[1], rgb[2]))
		# Run timeslices
		for idx, pin in enumerate(pins):
			if rgb[idx] > 0:
				pfd.output_pins[pin].turn_on()
		t1 = 0
		for slices in range(0, 10):
			remaining = [t for t in rgb if t > t1]
			if not remaining:
				break
			t2 = min(remaining)
			if (t2 == timeslice):
				break
			sleep((t2 - t1) / 1000)
			for idx in [idx for idx, t in enumerate(rgb) if t == t2]:
				pfd.output_pins[pins[idx]].turn_off()
			t1 = t2

# Start the interface
curses.wrapper(main)

# Clean exit
terminate()
