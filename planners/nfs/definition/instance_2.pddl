(define (problem instance_2)
  (:domain fn-counters)
  (:objects
    c0 c1 - counter
  )

  (:init
    (= (value c0) 0)
	(= (value c1) 0)
  )

  (:goal (and 
    (< (value c0) (value c1))
  ))

  
  

  (:bounds (val - int[0..4]))
)
