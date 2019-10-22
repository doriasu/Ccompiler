#!/bin/bash
try() {
  expected="$1"
  input="$2"
  ./9cc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

try 0 "0;"
try 42 "42;"
try 21 "5+20-4;"
try 41 " 12 + 34 - 5;"
try 47 '5+6*7;'
try 15 '5*(9-6);'
try 4 '(3+5)/2;'
try 10 '-10+20;'
try 2 '+2;'
try 0 '0==1;'
try 1 '42==42;'
try 1 '0!=1;'
try 0 '42!=42;'

try 1 '0<1;'
try 0 '1<1;'
try 0 '2<1;'
try 1 '0<=1;'
try 1 '1<=1;'
try 0 '2<=1;'

try 1 '1>0;'
try 0 '1>1;'
try 0 '1>2;'
try 1 '1>=0;'
try 1 '1>=1;'
try 0 '1>=2;'
try 3 'a=1;b=2;a+b;'
try 8 'a=8;a;'
try 6 'foo=3;bar=3;foo+bar;'

try 3 'a=3;return a;'
try 8 'a=3;b=5;return a+b;'
try 3 'a=1;b=1;if(a==b)return 1+2;return 4;'
try 4 'a=2;b=3;if(a==b)return 3;return 4;'
echo OK
