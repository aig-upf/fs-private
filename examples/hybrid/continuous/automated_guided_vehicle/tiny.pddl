(define (problem test_agv)
    (:domain automated_guided_vehicle)
    (:objects
        v0 - vehicle
        tiny - layout
    )

    (:init
        (= (track_layout) tiny)

        ;; initial state of the vehicle
        (= (x v0) 0.0)
        (= (y v0) 0.0)
        (= (u v0) 10.0)
        (= (theta v0) 0.0)
        (= (current_mode v0) stop)

        ;; constants
        (= (max_theta v0) 0.707)
        (= (min_theta v0) -0.707)
        (= (max_u v0) 10.0)
        (= (min_u v0) 0.0)

        (= (epsilon_1 v0) 0.5)
        (= (epsilon_2 v0) 2.0)
    )

    (:goal
        (and
            (<= 23.0 (x v0)) (<= (x v0) 27.0)
            (<= -1.0 (y v0)) (<= (y v0) 1.0)
            (= (current_mode v0) stop)
        )
    )

    (:metric
        minimize
            (@e v0)
    )

)
