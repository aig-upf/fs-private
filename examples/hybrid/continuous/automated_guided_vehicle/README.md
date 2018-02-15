# Supervisory Control for the Automated Guided Vehicle

This example illustrates the modelling capabilities of FS hybrid module. A
full description of the supervisory control system can be found on
Chapter 4, Section 2.3 of Lee & Seshia's ["Introduction to Embedded Systems:
A Cyber-Physical Approach"](http://leeseshia.org/releases/LeeSeshia_DigitalV2_2.pdf).
Two versions of the control system are provided:

 - ```domain_sc.pddl``` - this version of the domain includes the simple controller
 proposed by Lee & Seshia implemented by having events whose preconditions correspond
 with the guards of the FSM, and their effects setting the mode for the low-level
 control signals. The decisions available to the planner, at any given t, are whether
 to stop the vehicle or set it on a straight course.
 - ```domain.pddl``` - in this version of the domain the control policy given
 by Lee & Seshia has been removed, and instead the planner can decide whether to
 steer towards either left or right.

 FS will have a hard time producing complete plans for very long tracks. This is
 known and makes evident that computing off-line plans, i.e. switches between modes
 of control signals, is hardly going to be a feasible approach for delivering
 control systems that operate under the constraints posed by so-called large
 volume control systems on CPU time, memory available and robustness over very
 large horizons.

## Components of the example

 - ```data```, ```test```, ```tiny```, ```tiny_2_segments``` - layouts of three
 "test" tracks, each of the tracks is associated  with instance files.
 ```test.pddl```, ```tiny.pddl```, ```tiny_2_segments.pddl```.
 - ```domain.pddl``` and ```domain_sc.pddl``` - domain files with the specifications
 of events/reactions, global constraints and differential constraints.
 - ```external.hxx``` and ```external.cxx``` - ```C++``` implementation of the
 function $e(t)$, function $(@e ?v - vehicle)$, the distance to the track at time t.
