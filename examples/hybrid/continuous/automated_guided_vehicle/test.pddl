(define (problem test_agv)
    (:domain automated_guided_vehicle)
    (:objects
        v0 - vehicle
        test - layout
    )

    (:init
        (= (track_layout) test)

        ;; initial state of the vehicle
        (= (x v0) 0.0)
        (= (y v0) 0.0)
        (= (u v0) 4.4704)
        (= (theta v0) 0.0)
        (= (current_steering v0) stop)
        (= (current_acceleration v0) maintain)

        ;; constants
        (= (max_theta v0) 0.707)
        (= (min_theta v0) -0.707)
        (= (max_u v0) 4.4704)
        (= (min_u v0) 0.0)

        (= (epsilon_1 v0) 0.01)
        (= (epsilon_2 v0) 0.02)

        (= (error v0) 0.0)
    )

    (:goal
        (and
            (<= -2.0 (x v0)) (<= (x v0) 2.0)
            (<= 98.0 (y v0)) (<= (y v0) 100.0)
            (= (current_steering v0) stop)
        )
    )

    (:metric
        minimize
            (:terminal (sqrt (+ (^ (- (x v0) 0.0) 2.0) (^ (- (y v0) 99.0) 2.0) )) )
            (:stage (/ (abs (@e v0)) (epsilon_1 v0)))
    )

)
