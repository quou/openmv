; Function to split a string into an array by
; a specified deliminator

(set split (fun (str delim) (
	(if (! (& (= (type str) "string") (= (type delim) "string"))) (
		(except "Arguments to `split' must be strings.")
	) ((nil)))

	(if (> (# delim) 1) (
		(except "Deliminator must be a single character")
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

(print (split "Hey,there,this,is,a,string." ","))
