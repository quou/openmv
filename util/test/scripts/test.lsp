@import "std.lsp"

(print (split "Hel,lo,world!" ","))
(print (sub "Hello, world!" 0 10))

(print (find (array (30 5 2 45 5 312 5)) 2 5))

(print (format "Hello, {} {}!" (array ("wonderful" "world"))))

(collect_garbage)

(set test_fun (fun (x a b c d e f g h) (
	(set yy 10)
	(set xy 10)
	(set zy 10)
	(set wy 10)
	(set hy 10)
	(set dy 10)
	(set qy 10)
	(set ny 10)
	(print x)

	(if (< x 10) (
		(set x (test_fun (+ x 1) 1 2 3 4 5 6 7 8))
	) ((nil)))
)))

(test_fun 0 1 2 3 4 5 6 7 8)
