; Simple standard library

; Function to split a string into an array by
; a specified delimiter
(set split (fun (str delim) (
	(if (! (& (= (type str) "string") (= (type delim) "string"))) (
		(except "Arguments to `split' must be strings.")
	) ((nil)))

	(if (> (# delim) 1) (
		(except "Delimiter must be a single character")
	) ((nil)))

	(set r (array))

	(set cur "")

	(set i 0)
	(while (< i (# str)) (
		(if (= (at str i) delim) (
			(seta r (# r) cur)
			(set cur "")
		) (
			(set cur (cat cur (at str i)))
		))

		(set i (+ i 1))
	))

	(seta r (# r) cur)

	(ret r)
)))

; Take a sub string of a larger string or array
(set sub (fun (str start end) (
	(if (& (! (= (type str) "string")) (! (= (type str) "array"))) (
		(except "Argument 0 to `sub' must be a string or array.")
	) ((nil)))

	(if (! (= (type start) "number")) (
		(except "Argument 1 to `sub' must be a number.")
	) ((nil)))

	(if (! (= (type end) "number")) (
		(except "Argument 2 to `sub' must be a number.")
	) ((nil)))

	(if (< end 0) (
		(set end (- (# str) (- (neg end) 1)))
	) ((nil)))

	(if (< start 0) (
		(set start 0)
	) ((nil)))

	(if (= (type str) "string") (
		(set r "")

		(set i start)
		(while (< i end) (
			(set r (cat r (at str i)))
			(set i (+ i 1))
		))

		(ret r)
	) (
		(set r (array))

		(set i start)
		(while (< i end) (
			(seta r (# r) (at str i))
			(set i (+ i 1))
		))

		(ret r)
	))
)))
