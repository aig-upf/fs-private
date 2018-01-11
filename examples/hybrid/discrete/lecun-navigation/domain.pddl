;; Robot navigation, confined within (x_min, x_max, y_min, y_max) required
;; to get into area defined by coordinates
;; (x^{G}_{min}, x^{G}_{max}, y^{G}_{min}, y^{G}_{max})
;;
;; Added action "change_facing_towards" and "move" actions to match the "Navigation Task" in
;;
;; Model-Based Planning in Discrete Action Spaces\
;; Michael Henaff, William F. Whitney and Yann LeCu
;; https://arxiv.org/pdf/1705.07177.pdf

(define (domain le_cun_navigation)

  (:types
            ;;angle - int
            direction - object
  )

  (:constants
    north south east west - direction
  )

  (:functions
    ;;(heading) - angle
    ;;(@cos ?x - angle ) - number
    ;;(@sin ?x - angle ) - number
    (x_max) - number
    (x_min) - number
    (y_max) - number
    (y_min) - number
    (x) - number
    (y) - number
    (dx ?d - direction) - number
    (dy ?d - direction) - number
    (facing) - direction
  )


  (:action change_facing_towards
    :parameters (?d - direction)
    :precondition ()
    :effect (and
                (assign (facing) ?d)
            )
  )

  (:action move_1_unit
    :parameters ()
    :precondition ()
    :effect (and
                  (assign (x) (max (x_min) (min (x_max) (+ (x) (dx (facing))))))
                  (assign (y) (max (y_min) (min (y_max) (+ (y) (dy (facing))))))
            )
  )

  (:action move_2_units
    :parameters ()
    :precondition ()
    :effect (and
                  (assign (x) (max (x_min) (min (x_max) (+ (x) (* 2.0 (dx (facing)))))))
                  (assign (y) (max (y_min) (min (y_max) (+ (y) (* 2.0 (dy (facing)))))))
            )
  )

  (:action move_3_units
    :parameters ()
    :precondition ()
    :effect (and
                (assign (x) (max (x_min) (min (x_max) (+ (x) (* 3.0 (dx (facing)))))))
                (assign (y) (max (y_min) (min (y_max) (+ (y) (* 3.0 (dy (facing)))))))
            )
  )

  (:action move_4_units
    :parameters ()
    :precondition ()
    :effect (and
                (assign (x) (max (x_min) (min (x_max) (+ (x) (* 4.0 (dx (facing)))))))
                (assign (y) (max (y_min) (min (y_max) (+ (y) (* 4.0 (dy (facing)))))))
            )
  )

  (:action move_5_units
    :parameters ()
    :precondition ()
    :effect (and
                (assign (x) (max (x_min) (min (x_max) (+ (x) (* 5.0 (dx (facing)))))))
                (assign (y) (max (y_min) (min (y_max) (+ (y) (* 5.0 (dy (facing)))))))
            )
  )


)
