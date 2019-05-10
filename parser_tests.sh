#!/bin/sh
VALID_CODE_SEGMENTS=(
    "a : int"
    "a : int = 10" 
    "function counter() -> int {}"
    "function counter(int : a) -> int {}"
    "function counter(int : a) -> int { someVar : double = 3.1415927 }"
    "function counter(int : a, double : b) -> int {}"
    "if [10 > 20] {}"
    "if [10 > 20] {} else {}"
    "if [10 > 20] {} elsif [1 > 2] {}"
    "if [10 > 20] {} elsif [1 > 2] {} else {}" 
    "loop [i : int = 0; i < 100; i++] {}"
    "loop until [a > b] {}"
    "sum = 10 + 20 + 40"
    "difference = 100 - 52"
    "product = 12 * 22"
    "division = 1 / 52"
    "mudulus = 168 % 24"
    "power = 10 ^ 3"
    "function returnString() -> int { return 10 }"
)

for i in "${VALID_CODE_SEGMENTS[@]}"; do
    echo "Parsing: $i"  
    exec echo $i | ./parser
    echo 
done