#!/bin/bash
cat <<EOF | gcc -xc -c -o tmp2.o -
int ret() { return 3; }
int rett() { return 5; }
int rethiki(int a,int b){return a+b;}
EOF
try() {
  expected="$1"
  input="$2"
  ./9cc "$input" > tmp.s
  gcc -static -o tmp tmp.s tmp2.o
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
try 2 'if(2-1)return 2;else return 3;'
try 3 'if(0)return 5;else return 3;'
try 3 'a=1;b=1;if(a==b)return 1+2;return 4;'
try 5 'a=0;while(a<5)a=a+1;return a;'
try 55 'i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j;'
try 3 'for (;;) return 3; return 5;'
try 2 'if(0)return 5;return 2;'
try 3 'a=1;b=3;if(0){a=a+4;return a;}return b;'
try 5 'a=1;for(;a<5;a=a+1){b=2;c=3;d=4;}return a;'
try 2 'a=1;while(a<10){a=a+1;return a;}'
try 3 '{1; {2;} return 3;}'
try 10 'i=0; while(i<10) i=i+1; return i;'
try 55 'i=0; j=0; while(i<=10) {j=i+j; i=i+1;} return j;'
try 55 'i=0; j=0; for (i=0; i<=10; i=i+1) j=i+j; return j;'
try 3 'return ret();'
try 5 'return rett();'
try 11 'return rethiki(3,5)+ret();'
echo OK
