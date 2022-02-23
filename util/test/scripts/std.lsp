; Standard Library

; Asserts the type of val against t, excepting err on failure
(set type_assert (fun (val t err) (
	(if (! (= (type t) "string")) (
		(except "Argument 1 to `type_assert' must be a string.")
	) ((nil)))

	(if (! (= (type err) "string")) (
		(except "Argument 2 to `type_assert' must be a string.")
	) ((nil)))

	(if (! (= (type val) t)) (
		(except err)
	) ((nil)))
)))

; Function to split a string into an array by a specified delimiter
(set split (fun (str delim) (
	(if (! (& (= (type str) "string") (= (type delim) "string"))) (
		(except "Arguments to `split' must be strings.")
	) ((nil)))

	(if (> (# delim) 1) (
		(except "Delimiter must be a single character")
	) ((nil)))

	(set r (array))

	(print delim)

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

; Returns the nth index of val in arr. Returns nil on failure.
(set find (fun (arr n val) (
	(type_assert arr "array"  "Argument 0 to `find' must be an array.")
	(type_assert val "number" "Argument 1 to `find' must be a number.")

	(set i 0)
	(set c 0)

	(while (< i (# arr)) (
		(if (= (at arr i) val) (
			(if (= c n) (
				(ret i)
			) ((nil)))

			(set c (+ c 1))
		) ((nil)))

		(set i (+ i 1))
	))

	(ret nil)
)))

; Format a string based on the values in an array.
;
; Example:
;     (format "Hello, {} {}!" (array ("wonderful" "world")))
;
; ...Will result in a string with the contents: "Hello, wonderful world!"
(set format (fun (fmt args) (
	(type_assert fmt "string" "Argument 0 to `format' must be a string.")
	(type_assert args "array" "Argument 1 to `format' must be an array.")

	(set i 0)
	(set n 0)
	(set r "")

	(while (< i (# fmt)) (
		(if (< (+ i 1) (# fmt)) (
			(if (& (= (at fmt i) "{") (= (at fmt (+ i 1)) "}")) (
				(set r (cat r (to_string (at args n))))

				(set i (+ i 1))

				(set n (+ n 1))
			) (
				(set r (cat r (at fmt i)))
			))
		) (
			(set r (cat r (at fmt i)))
		))

		(set i (+ i 1))
	))

	(ret r)
)))
