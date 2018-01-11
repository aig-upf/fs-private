;; Robot navigation, confined within (x_min, x_max, y_min, y_max) required
;; to get into area defined by coordinates
;; (x^{G}_{min}, x^{G}_{max}, y^{G}_{min}, y^{G}_{max})

(define (domain robot_navigation_num_fn)

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
  )

  (:action move
    :parameters (?d - direction)
    :precondition ()
    :effect (and
                  (assign (x) (+ (x) (dx ?d)) )
                  (assign (y) (+ (y) (dy ?d)) )
            )
  )

)
