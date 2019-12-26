#!/bin/sh
time find /etc -type f 2>/dev/null | xargs cat 2>/dev/null | $* >/dev/null
