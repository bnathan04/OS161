#!/bin/csh
# testloop.csh - runs through asst1 - stoplight test 50 times

set j = 0
while ($j < 50)
	os161-tester 3 btree
	@ j++
end	
