#!/bin/sh
asserteq() {
	cmd="$1"
	expected="$2"
	echo "running $cmd" 1>&2
	result="$(eval $cmd)"
	if [ "$expected" -ne "$result" -o "$?" -ne 0 ]; then
		echo "expected $expected, got $result" 1>&2
		return 1
	else
		echo passed 1>&2
		return 0
	fi
}

asserteq "echo '5 + 5' | ./hoc" 10
asserteq "echo 'if (1) { print 0 } else { print 1 }' | ./hoc" 0
asserteq "echo 'if (10 <= 10) { print 1 }' | ./hoc" 1
