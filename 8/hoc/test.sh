#!/bin/sh
asserteq() {
	expected=$1
	result=$2
	if [ -z "$result" ] || [ "$expected" -ne "$result" ]; then
		echo "expected $expected, got $result" 1>&2
		return 1
	fi
	return 0
}

hocassert() {
	expected="$1"
	input=$(cat)
	printf "running:\n---\n%s\n---\n" "$input" 1>&2
	asserteq "$expected" "$(echo "$input" | ./hoc | tail -n1)"
	echo 1>&2
}

hocassert 7 << "EOF"
func plus2() {
	return $1 + 2
}
plus2(5)
EOF

hocassert 13 << "EOF"
func fib() {
	if ($1 <= 1) {
		return $1
	}
	return fib($1 - 1) + fib($1 - 2)
}
fib(7)
EOF

hocassert 13 << "EOF"
func fib(n) {
	if (n <= 1) {
		return n
	}
	return fib(n - 1) + fib(n - 2)
}
fib(7)
EOF

hocassert 100 << "EOF"
func add(a, b) {
	return a + b
}
add(49, 51)
EOF
