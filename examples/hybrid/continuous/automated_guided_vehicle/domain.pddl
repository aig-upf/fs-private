;; Automated Guided Vehicle
;; Example from Lee and Seshia, Introduction to Embedded Systems

(define
    (domain automated_guided_vehicle)

    (:types
        vehicle steering acceleration layout - object
    )

    (:constants
        stop left straight right - steering
        maintain increasing decreasing - acceleration
    )

    (:functions
        (current_steering ?v - vehicle) - steering
        (current_acceleration ?v - vehicle) - acceleration

        (track_layout) - layout

        (x ?v - vehicle) - number ;; coordinates
        (y ?v - vehicle) - number
        (u ?v - vehicle) - number ;; linear velocity in m/sec
        (theta ?v - vehicle) - number ;; angular speed in rad/sec

        (@e ?v - vehicle ) - number ;; sensor
        (error ?v - vehicle ) - number

        (min_u ?v - vehicle) - number
        (max_u ?v - vehicle) - number
        (min_theta ?v - vehicle) - number
        (max_theta ?v - vehicle) - number
        (epsilon_1 ?v - vehicle) - number
        (epsilon_2 ?v - vehicle) - number
    )

    ;; speed constraints
    (:constraint speed_bounds
        :parameters (?v - vehicle)
        :condition (and
            (>= (max_u ?v) (u ?v))
            (<= (min_u ?v) (u ?v))
        )
    )

    (:constraint error_bounds
        :parameters (?v - vehicle)
        :condition (and
            (< (abs (@e ?v)) (epsilon_2 ?v))
        )
    )

    (:process increasing_acceleration
        :parameters (?v - vehicle)
        :precondition (= (current_acceleration ?v) increasing)
        :effect (and
            (increase (u ?v) (* #t 0.45))
        )
    )

    (:process decreasing_acceleration
        :parameters (?v - vehicle)
        :precondition (= (current_acceleration ?v) decreasing)
        :effect (and
            (decrease (u ?v) (* #t -0.45))
        )
    )

    (:process right_dynamics
        :parameters (?v - vehicle)
        :precondition (and
            (= (current_steering ?v) right)
        )
        :effect (and
            (increase (x ?v) (* #t (* (u ?v) (cos (theta ?v)))))
            (increase (y ?v) (* #t (* (u ?v) (sin (theta ?v)))))
            (increase (theta ?v) (* #t -3.1459))
        )
    )

    (:process straight_dynamics
        :parameters (?v - vehicle)
        :precondition (and
            (= (current_steering ?v) straight)
        )
        :effect (and
            (increase (x ?v) (* #t (* (u ?v) (cos (theta ?v)))))
            (increase (y ?v) (* #t (* (u ?v) (sin (theta ?v)))))
            (increase (theta ?v) (* #t 0.0))
        )
    )

    (:process left_dynamics
        :parameters (?v - vehicle)
        :precondition (and
            (= (current_steering ?v) left)
        )
        :effect (and
            (increase (x ?v) (* #t (* (u ?v) (cos (theta ?v)))))
            (increase (y ?v) (* #t (* (u ?v) (sin (theta ?v)))))
            (increase (theta ?v) (* #t 3.14159))
        )
    )

    (:process stop_dynamics
        :parameters (?v - vehicle)
        :precondition (and
            (= (current_steering ?v) stop)
        )
        :effect (and
            (increase (x ?v) (* #t 0.0))
            (increase (y ?v) (* #t 0.0))
            (increase (theta ?v) (* #t 0.0))
        )
    )

    (:event record_error
        :parameters (?v - vehicle)
        :precondition (and)
        :effect (assign (error ?v) (@e ?v))
    )

    (:action activate_acceleration
        :parameters (?v - vehicle ?m - acceleration )
        :precondition (and  (not (= (current_steering ?v) stop))
                            (not (= (current_acceleration ?v) ?m))
        )
        :effect (assign (current_acceleration ?v) ?m)
    )

    (:action activate_steering
        :parameters (?v - vehicle ?m - steering)
        :precondition (and
            (not (= (current_steering ?v) ?m))
        )
        :effect (assign (current_steering ?v) ?m)
    )



)
