(set n 64)

(set y (- n 1))

(print "")

(while (>= y 0) (
	(set i 0)
	(while (< i y) (
		(put " ")
		(set i (+ i 1))
	))

	(set x 0)
	(while (< (+ x y) n) (
		(if (= (bit_and x y) 0) (
			(put "* ")
		) (
			(put "  ")
		))

		(set x (+ x 1))
	))

	(print "")

	(set y (- y 1))
))
