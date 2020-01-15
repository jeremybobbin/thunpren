#!/bin/sh
asserteq() {
	expected="$1"
	echo "running" 1>&2
	result="$(cat | ./hoc)"
	if [ "$expected" -ne "$result" -o "$?" -ne 0 ]; then
		echo "expected $expected, got $result" 1>&2
		return 1
	else
		echo passed 1>&2
		return 0
	fi
}

asserteq 7 << "EOF"
func plus2() {
	return $1 + 2
}
plus2(5)
EOF
