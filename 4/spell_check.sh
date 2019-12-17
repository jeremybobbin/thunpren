#!/bin/sh

words=$(mktemp)
sort /usr/share/dict/words -o $words

mispelled_words() 
{
	tr -sc A-Za-z '\n' < $1 | sort -u | comm -23 - $words
}

for f in $*; do
	for word in `mispelled_words $f`; do
		echo "$f: $word"
		nl "$f" | fgrep "$word" 
	done
done
