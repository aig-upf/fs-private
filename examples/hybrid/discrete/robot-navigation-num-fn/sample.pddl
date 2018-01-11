(define (problem sample)
    (:domain robot_navigation_num_fn)
    (:init
        (= (x) 0.0)
        (= (y) 0.0)

        ;; constant values
        (= (x_max) 10.0)
        (= (x_min) -10.0)
        (= (y_max) 10.0)
        (= (y_min) -10.0)

        (= (dx north) 0.0) (= (dy north) 1.5)
        (= (dx south) 0.0) (= (dy south) -1.5)
        (= (dx east) 1.5) (= (dy east) 0.0)
        (= (dx west) -1.5) (= (dy west) 0.0)
    )

    (:goal
      (and
          (>= (x) 3.0) (<= (x) 5.0)
          (>= (y) 3.0) (<= (y) 5.0)
      )
    )

    (:constraints
      (and
            (>= (x) (x_min)) (<= (x) (x_max))
            (>= (y) (y_min)) (<= (y) (y_max))
      )
    )

)
