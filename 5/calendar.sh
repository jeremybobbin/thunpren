#!/bin/sh

awk '
BEGIN {
	x = "Jan 31 Feb 28 Mar 31 Apr 30 May 31 Jun 30 " \
	    "Jul 31 Aug 31 Sep 30 Oct 31 Nov 30 Dec 31 Jan 31"
	y = "Sun Mon Tue Wed Thu Fri Sat Sun"

	split(x, data)
	split(y, weekdays)


	for (i = 1; i < 24; i += 2) {
		days[data[i]] = data[i+1]
		nextmon[data[i]] = data[i+2]
	}

	for (i = 1; i <= 7; i += 1) {
		nextday[weekdays[i]] = weekdays[i+1]
	}
	

	split("'"`date`"'", date)

	year = date[4]
	weekday1 = date[1]
	weekday2 = nextday[weekday1]
	day1 = date[2]
	day2 = day1 + 1
	mon1 = date[3]
	mon2 = mon1

	if (year % 4 == 0) # leap year
		days["Feb"] = 29

	if (day1 >= days[mon1]) {
		day2 = 1
		mon2 = nextmon[mon1]
	}

}
$1 == mon1 && $2 == day1 || $1 == mon2 && $2 == day2 \
	|| $1 == weekday1 || $1 == weekday2
' $*
