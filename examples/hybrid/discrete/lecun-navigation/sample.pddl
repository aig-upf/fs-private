(define (problem sample)
    (:domain le_cun_navigation)
    (:init
        (= (facing) north)
        (= (x) 0.0)
        (= (y) 0.0)

        ;; constant values
        (= (x_max) 30.0)
        (= (x_min) -30.0)
        (= (y_max) 30.0)
        (= (y_min) -30.0)

        (= (dx north) 0.0) (= (dy north) 1.0)
        (= (dx south) 0.0) (= (dy south) -1.0)
        (= (dx east) 1.0) (= (dy east) 0.0)
        (= (dx west) -1.0) (= (dy west) 0.0)
    )

    (:goal
      (and
          (>= (x) 25.0) (<= (x) 25.0)
          (>= (y) -12.0) (<= (y) -12.0)
      )
    )

    (:constraints
      (and
            (>= (x) (x_min)) (<= (x) (x_max))
            (>= (y) (y_min)) (<= (y) (y_max))
      )
    )

)
