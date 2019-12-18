#!/bin/sh
# prpages: compute number of pages that pr will print
awk '
FILENAME != prevfile {
	prevfile = FILENAME
	n += int((NR+55) / 56)
	NR = 0
}
END { print n }' $*
