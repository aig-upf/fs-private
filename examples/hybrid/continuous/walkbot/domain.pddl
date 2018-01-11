;; Continuous time formulation of this domain, see original in Lohr's Thesis
;;
;;@techreport{aldinger2016jumpbot,
;;  title={The Jumpbot domain for numeric planning},
;;  author={Aldinger, Johannes and L{\"o}hr, Johannes},
;;  year={2016},
;;  institution={Technical report 279, University of Freiburg}
;;}

(define
    (domain agile_robot_world_simple)

    (:types
        mode - object
    )

    (:constants
        accelerating decelerating steering_right steering_left jumping stepping - mode
    )

    (:functions
        (current_mode) - mode
        (x) - number
        (y) - number
        (v_x) - number
        (v_y) - number
        (min_x) - number
        (max_x) - number
        (min_y) - number
        (max_y) - number
        (k_acc) - number
        (k_dec) - number
        (k_steer) - number
    )

    ;; Surface bounds
    (:constraint surface_bounds
        :parameters ()
        :condition (and (>= (x) (min_x)) (<= (x) (max_x)) (>= (y) (min_y)) (<= (y) (max_y)) )
    )

    ;; robot position determined by velocities along each axis, this is always
    ;; active, equations
    ;; \dot{x} = v_x
    ;; \dot{y} = v_y
    (:process displacement
        :parameters ()
        :precondition ()
        :effect (and
                        (increase (x) (* #t (v_x)))
                        (increase (y) (* #t (v_y)))
                )
    )

    ;; stepping and jumping have the same dynamics, yet when jumping "state constraints"
    ;; are "turned off" by being trivially true. Dynamics are given as a "simple double
    ;; integrator" i.e. "velocities remain constant"

    (:process stepping_dynamics
    :parameters ()
    :precondition (= (current_mode) stepping)
    :effect (and
                    (increase (v_x) (* #t 0.0))
                    (increase (v_y) (* #t 0.0))
            )
    )

    ;;(:process jumping_dynamics
    ;;    :parameters ()
    ;;    :precondition (= (current_mode) jumping)
    ;;    :effect (and
    ;;                    (increase (v_x) (* #t 0.0))
    ;;                    (increase (v_y) (* #t 0.0))
    ;;            )
    ;;)

    ;; As per equation 3.21 in page 36
    (:process accelerating_dynamics
        :parameters ()
        :precondition (= (current_mode) accelerating)
        :effect (and
                    (increase (v_x) (* #t (* (k_acc) (v_x))))
                    (increase (v_y) (* #t (* (k_acc) (v_y))))
                )
    )

    ;; As per equation 3.22 in page 36
    (:process decelerating_dynamics
        :parameters ()
        :precondition (= (current_mode) decelerating)
        :effect (and
                    (decrease (v_x) (* #t (* (k_dec) (v_x))))
                    (decrease (v_y) (* #t (* (k_dec) (v_y))))
                )
    )

    ;; As per equation 3.23 in page 36
    (:process steering_right_dynamics
        :parameters ()
        :precondition (= (current_mode) steering_right)
        :effect (and
                    (increase (v_x) (* #t (* (k_steer) (v_y))))
                    (decrease (v_y) (* #t (* (k_steer) (v_x))))
                )
    )

    ;; As per equation 3.24 in page 36
    (:process steering_left_dynamics
        :parameters ()
        :precondition (= (current_mode) steering_left)
        :effect (and
                    (decrease (v_x) (* #t (* (k_steer) (v_y))))
                    (increase (v_y) (* #t (* (k_steer) (v_x))))

                )
    )

    ;; Instantaneous actions, follow from Table 3.1

    (:action step
        :parameters ()
        :precondition ()
        :effect (assign (current_mode) stepping)
    )

    (:action accelerate
        :parameters ()
        :precondition ()
        :effect (assign (current_mode) accelerating)
    )

    (:action decelerate
        :parameters ()
        :precondition ()
        :effect (assign (current_mode) decelerating)
    )

    ;;(:action jumping_1
    ;;    :parameters ()
    ;;    :precondition (> (v_x) 1.0)
    ;;    :effect (assign (current_mode) jumping)
    ;;)

    ;;(:action jumping_2
    ;;    :parameters ()
    ;;    :precondition (> (v_y) 1.0)
    ;;    :effect (assign (current_mode) jumping)
    ;;)

    (:action steer_right
        :parameters ()
        :precondition (not (= (current_mode) steering_left))
        :effect (assign (current_mode) steering_right)
    )

    (:action steer_left
        :parameters ()
        :precondition (not (= (current_mode) steering_right))
        :effect (assign (current_mode) steering_left)
    )


)
