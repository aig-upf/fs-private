(define (problem instance_1_1)
	
	(:domain fn-long-haul)

	(:objects
		t0 - truck
		c0 - customer
		k0 k1 - commodity
		l0 l1 l2 - location
	)

	(:init
		(= (location t0) l0)
		(compatible t0 k0) (compatible t0 k1)
		(= (available t0) 16)
		(= (op_cost t0) 25)
		(= (load t0 k0 c0) 0) (= (load t0 k1 c0) 0)

		(= (depot) l0)

		(= (site c0) l2)
		(= (demand c0 k0) 10) (= (outstanding c0 k0) 10) (= (delivered c0 k0) 0)
		(= (demand c0 k1) 15) (= (outstanding c0 k1) 15) (= (delivered c0 k1) 0)
		
		(= (day_cost) 0)

		(= (dist l0 l1) 5) (= (dist l0 l2) 10)
		(= (dist l1 l0) 5) (= (dist l1 l2) 5) 
		(= (dist l2 l1) 5) (= (dist l2 l0) 10)
	)
	
	(:goal
		(and
			(= (delivered c0 k0) (demand c0 k0))
			(= (delivered c0 k1) (demand c0 k1))
			(= (location t0) (depot))
		)
	)

	(:bounds
		(amount - int[1..16]) ;; between 1 and capacity of largest truck
		(demand - int[0..15]) ;; between 0 and largest demand of any commodity
	)
)
