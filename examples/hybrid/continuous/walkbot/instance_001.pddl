(define (problem lohr_random_instance_001 )
  (:domain agile_robot_world_simple)
  (:objects

  )

  (:init
    ;; Constant values

    ;; Values delimiting the extent of the surface the robot moves on. These
    ;; have been deduced from observing Fig. 3.6 (page 38)
    (= (min_x) 0.0)
    (= (max_x) 10.0)
    (= (min_y) 0.0)
    (= (max_y) 10.0)

    ;; Values determining acceleration, deceleration and steering rates
    (= (k_acc) 0.5)
    (= (k_dec) 2.0)
    (= (k_steer) 1.0)

    ;; Initial coordinates of the robot, see last paragraph in page 37
    (= (x) 2.0) (= (y) 8.0)

    ;; Initial velocities, taken from Table 7.1
    (= (v_x) -1.0140) (= (v_y) 0.2664)

    (= (current_mode) stepping)
  )

  (:goal
    (and
        ;; Goal state is x=8, y=2, v_x=0, v_y=0, accuracies for each
        ;; goal atom are e(x)=0.25, e(y)=0.25, e(v_x) = 0.2, e(v_y)=0.2
        ;; (see first paragraph page 38)
        (>= (x) 7.75) (<= (x) 8.25)
        (>= (y) 1.75) (<= (y) 2.25)
        (>= (v_x) -0.2) (<= (v_x) 0.2)
        (>= (v_y) -0.2) (<= (v_y) 0.2)
	)
  )



)

