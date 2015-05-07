;;;;;;;;;;;;
;;; longhaul domain
;;;;;;;;;;;;

(define (domain fn-long-haul)
	(:requirements :strips :typing :equality :adl)
	(:types
		customer location commodity truck - object
		amount demand - int	
	)
	
	(:predicates
		(compatible ?t - truck ?k - commodity)
	)

	(:functions
		;; fluents - these change
		(location ?t - truck) - location ;; location of truck t
		(outstanding ?c - customer ?k - commodity) - demand ;; outstanding demand of commodity k by customer c
		(delivered ?c - customer ?k - commodity) - demand ;; amount of commodity k delivered to customer c
		(available ?t - truck) - amount ;; available capacity on truck t
		(load ?t - truck ?k - commodity ?c - customer) - amount ;; amount of commodity k loaded on truck t for customer c
		(day_cost ) - int
		(dist ?l1 - location ?l2 - location) - int ;; distance in kms between two locations		

		;; static, never changes
		(site ?c - customer) - location ;; location of customer t
		(depot)	- location ;; location of the depot
		(op_cost ?t - truck) - int ;; operating costs of truck
		(demand ?c - customer ?k - commodity) - demand ;; initial demand of commodity k by customer ?c
	)
	
	(:action load_truck_1
		:parameters ( ?t - truck ?k - commodity ?c - customer )
		:precondition 	(and
					(= (location ?t) (depot))
					(> (available ?t) 0)
					(compatible ?t ?k)
					(> (outstanding ?c ?k) 0)
					(>= (available ?t) (outstanding ?c ?k))
				)
		:effect	(and
				;;MRJ: Not supported?
				;;(decrease (available ?t) (outstanding ?c ?k))
				;;(increase (load ?t ?k ?c) (outstanding ?c ?k))
				(assign (available ?t) (- (available ?t) (outstanding ?c ?k)))
				(assign (load ?t ?k ?c) (+ (load ?t ?k ?c) (outstanding ?c ?k)))
				(assign (outstanding ?c ?k) 0)
			)
	)
		
	(:action load_truck_2
		:parameters ( ?t - truck ?k - commodity ?c - customer )
		:precondition 	(and
					(= (location ?t) (depot))
					(> (available ?t) 0)
					(compatible ?t ?k)
					(> (outstanding ?c ?k) 0)
					(< (available ?t) (outstanding ?c ?k))
				)
		:effect	(and
				(assign (available ?t) 0)
				;;MRJ: Not supported?
				;;(decrease (outstanding ?c ?k) (available ?t))
				;;(increase (load ?t ?k ?c) (available ?t))
				(assign (outstanding ?c ?k) (- (outstanding ?c ?k) (available ?t)))
				(assign (load ?t ?k ?c) (+ (load ?t ?k ?c) (available ?t)))
			)
	)

	(:action load_truck_3
		:parameters ( ?t - truck ?k - commodity ?c - customer ?x - amount )
		:precondition 	(and
					(= (location ?t) (depot))
					(>= (available ?t) ?x)
					(compatible ?t ?k)
					(> (outstanding ?c ?k) ?x)
				)
		:effect	(and
				(decrease (available ?t) ?x)
				(increase (load ?t ?k ?c) ?x)
				(decrease (outstanding ?c ?k) ?x)
			)
	)

	(:action unload_truck
		:parameters 	(?t - truck ?k - commodity ?c - customer)
		:precondition 	(and
					(= (location ?t) (site ?c))
					(> (load ?t ?k ?c) 0)
					(<= (load ?t ?k ?c) (- (demand ?c ?k) (delivered ?c ?k)))
					(< (delivered ?c ?k) (demand ?c ?k))				
				)
		:effect		(and
					(assign (load ?t ?k ?c) 0)
					;; MRJ: Not supported?
					;;(increase (delivered ?c ?k) (load ?t ?k ?c))
					;;(increase (available ?t) (load ?t ?k ?c))
					(assign (delivered ?c ?k) (+ (delivered ?c ?k) (load ?t ?k ?c)))
					(assign (available ?t) (+ (available ?t) (load ?t ?k ?c)))
				)
	)

	(:action move_truck
		:parameters	(?t - truck ?l1 - location ?l2 - location)
		:precondition	(and
					(= (location ?t) ?l1)
					(not (= (location ?t) ?l2))
				)
		:effect		(and
					(assign (location ?t) ?l2)
					(assign (day_cost) (+(day_cost) (* (op_cost ?t) (dist ?l1 ?l2))))
				)
	)
)
