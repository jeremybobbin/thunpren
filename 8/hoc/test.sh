#!/bin/sh
asserteq() {
	cmd="$1"
	expected="$2"
	result="$(eval "$cmd")"
	if [ "$expected" -ne "$result" -o "$?" -ne 0 ]; then
		echo "expected $expected, got $result" 1>&2
		return 1
	else
		echo passed 1>&2
		return 0
	fi
}

hocassert() 
{
	echo "running '$1'; expecting $2" 1>&2
	asserteq 'echo '"'$1'"' | ./hoc | tail -n1' "$2"
}

hocassert '5 + 5' 10
hocassert 'if (1) { print 0 } else { print 1 }' 0
hocassert 'if (10 <= 10) { print 1 }' 1
hocassert 'a = 2; a *= 2; a *= 2; print a' 8
hocassert 'a = 2; a *= 2 * 2 ; print a' 8
hocassert 'a = 1; b = 2; a || b = 3; b' 2
hocassert 'b = 0 && b = 1; b' 0
hocassert 'b = 0 && b = 1; b; 5' 5
hocassert 'for (i = 0; i <= 5; i += 1) { print i }' 5
