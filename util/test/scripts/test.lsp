@import "util/test/scripts/std.lsp"

(set other_fun nil)

(set some_fun (fun (x y) (
	(put "Hello, I'm a function! X: ")
	(put x)
	(put " Y: ")
	(print y)

	(print (other_fun 10 1))

	; Recursion is a thing!
	(if (> x 0) (
		(some_fun (- x 1) y)
	) ((nil)))
)))

(set other_fun (fun (a b) (
	(ret (+ a b))
)))

(set a 290)
(set b 30)

(print "")

(print a)
(print b)

(print (+ a b))

(some_fun 10 2)

(native_fun "Hello, world!")

(print (< 1 0))
(print (<= 1 5))
(print (> 1 0))
(print (>= 1 0))

(print (= 3 3))

(if (< 2 3) (
	(print "Hello, from an if block.")
	(print "Second line")
)(
	(print "Hello, from an else block.")
	(print "Second line")
))

(set i 0)
(while (< i 100) (
	(print i)

	(set i (+ i 1))
))

(print (memory_usage))

; Print Fibonacci numbers

(set a 0)
(set b 1)
(set c 0)

(set n 100)

(while (< c n) (
	(set c (+ a b))

	(set a b)
	(set b c)

	(print c)
))

(while false (
	(nil)
))

(set test (new TestPointer))
(print test)
(print_test_ptr test)

(set file_path "util/test/scripts/test.lsp")
(set test_file (fopen file_path "r"))

(if (fgood test_file) (
	(set data (fgets test_file))
	(while data (
		(put data)

		(set data (fgets test_file))
	))
	(fclose test_file)
) (
	(except (cat "Failed to read file: " file_path))
))

(print " ==")

(print (split "ssd, sddg, hj" ","))

(set some_array (array (22 4 (cat "Hello," "world!"))))
(print (at some_array 2))
(seta some_array 2 "a")
(rm some_array 1)
(print (at some_array 1))
(print some_array)

(seta some_array 300 "haha")

(print (# some_array))

(set thing "hello, world!")
(print (at thing 5)) ; Prints ,
