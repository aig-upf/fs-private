# Integrating GeCode in FS0

This document purpose is to provide some insight to future readers about
what did involve the ingration of a fully-featured, mature Constraint Programming
framework on the FS0 planner.

## Preliminaries

### Source code repositories

Gecode 4.4.0 fork at Bitbucket:

https://bitbucket.org/miquelramirez/gecode-homebrew.git

## Problem description: 1st iteration

For each action $a$ and effect $e$

\[
e: C \rightarrow x^{k+1} = w
\]

we have the CSP $\gamma_{e} = ({\mathcal X}, {\mathcal C})$ where the variable set
${{\mathcal X}} is defined as follows:

$$
X = X_{a}^k \cup \{ x_{e}^{k+1} \}
$$

$X_{a}^k \subseteq  X$ is the projection of state variables $X$ over action
$a$ **[Question for Guillem: is this just to take the set of variables mentioned
on action preconditions, effect conditions and effect formulae?]** at RPG layer
$P^k$, and $x_{e}^{k+1}$ being the variable on the right hand side of the effect
formula, representing the (set of) value(s) of state variable $x_{e}$ in the
RPG layer $P^{k+1}$.

**NOTE: here I am deviating from ICAPS-15 notation, which I think is a bit
cryptic.**

The set of constraints ${\mathcal C}$ consists of the following constraints:

- The set of *domain constraints* $D^{k}(X)$ expressing the possible set of
values for state variables $X$ on the RPG layer $P^k$.
- The set of *precondition constraints* $Pre_a \land C$, where $Pre_a$ is
corresponds to the set of formulas in the precondition of $a$, and $C$ is the set
of formulas in the condition of the effect $e$ (possibly empty for *unconditional*
effects).
- The *effect constraint* $x^{k+1} = w$, where $w$ is a *linear constraint* over
variables $Y \subseteq X_{a}^k$.
- Last, we have a further *domain constraint* defined over $x_{e}^{k+1}$.

Note that a linear constraint has the following form
$$
\sum_{i} a_i x_i = c
$$
where $a_i$ and $c$ are *constants*. For instance, the PDDL 3.0 effect formula
```
(assign (f x ) 1)
```
becomes the linear constraint
$$
y^{k+1} = 1
$$
where $y = f(x)$, and the PDDL 3.0 effect
```
(assign (f x) (+ (f x) 1))
```
becomes the linear effect constraint
$$
y^{k+1} = y^k + 1
$$
which is equivalent to
$$
- y^{k} + y^{k+1} = 1
$$
This allows us to define $P^{k+1}$ as a set of domain constraints over state
variables $x^{k+1}$, where each constraint is given by
$$
x^{k+1} \in \bigcup_{e \in \delta(x)} D(x_{e}^{k+1})
$$
In words, the domain of state variable $x$ on RPG layer $k+1$ is the *union*
of the domains of variables $x_{e}^{k+1}$ in each CSP corresponding to an
effect $e$ where $x$ appears on the right hand side of $e$, **after** proving
its global consistency (or approximating it with some local consistency scheme).

### Discussion

#### The difference between focusing over the effects or the actions

The construction of RPG layers $P^k$ in FS0 differs from the above in several
important respects. First, a CSP $\gamma_{a}$ is defined for each action $a$,
rather than over  *effects* $e$. Second, the set of constraints only include
*precondition constraints* $Pre_a \land C$ as defined above. Third,
the set of variables considered is $X_w$, the variables appearing on the *right
hand side* of the effect, which is a subset of $X_{a}$ above.

Then, the set of values for state variable $y$ is determined in two steps:

- Enforce local consistency over $Pre_a \land C$, let $\sigma(X_w)$ be the Cartesian
Product of the sets of locally consistent values for variables $x \in X_w$,
- for each variable $y$ on the right side of and effect $D_a(y^{k+1})$
is defined as the set

$$
D_a(y^{k+1}) = \{ v_y = w(v_1, \ldots, v_n) \,|\, (v_1, \ldots, v_n) \in \sigma(X_w)\}
$$

where $v_y$ is the value that results of evaluating $w$ over each possible
interpretation of variables $X_w$.

The above is a quite direct and straightforward method, that probably is not
going to scale up if the arity of $X_w$ and the maximal domain size of vars in
$X_w$ become "interesting".

How does Miquel's 1st iteration behave when one has an action $a$ with effects

$$
e_1 : y = x + 5
$$

$$
e_2 : z = x + 1
$$

where we have the domain constraint $y \in [0,10]$? Would the $\gamma_{a}$ and
$\gamma_{e}$ CSPs leading to the same inferences? Which would result on tighter
domains for the next layer in the RPG?

It is easy to see that, after enforcing local consistency on $\gamma_{e_1}$,
the resulting domain for $x$ will not contain any value greater than $5$ yet,
doing the same on $\gamma_{e_2}$, will result in $x$ having a less tight domain,
which in turn would lead to a less tight domain for $z$.

Can the 1st iteration model be reformulated so it is defined over actions $a$?
Yes, the only difficulty that needs to be bridged is that posed by conditional
effects. Yet these can be handled via *reified constraints* (see 2nd Iteration).

#### Efficiency concerns and exploiting monotonicity in the domains $X^k$ variables

The initial e-mail version of this discussion kind of suggested a commitment to
perform global consistency. This is **not** the case at all. Both exact and
approximate methods are to be supported.

A possible optimization, which is already being used by FS0 **NOTE: Check with
Guillem**, consists in enforcing *local* consistency over the CSP $Pre(a)$, and
then take the resulting domains to become the initial domain of effect CSP's
$\gamma_{e}$. The problem with doing this is that **conditional effects cannot
be handled**.

Still work in progress in FS0 is to find a way to solve incrementally the Goal
CSPs $\gamma_{G}$ which are solved in each layer of the RPG. This follows from the
following observation.

If we are solving $\gamma_{G}$ in the layer $k+1$, the values in the domains of
the variables in $k$ are **known** to be locally consistent. So one thing that
is possible, in principle, would be to use some smart propagation scheme that
exploits this fact to avoid re-computation. The current inference algorithm in
FS0 is AC-3, which is easy to modify to exploit this property. It is not clear
that this is so easy to do as well for the inference algorithms for more
expressive constraints (global constraints etc.)

One possible way of handling this with GeCode would be to allow for the Goal CSP
$\gamma_{G}$ to be dynamic, in the sense that we can add bounds/domain constraints
to it at each layer. So that for layer $k$ the query is done against CSP
$\gamma^{k}_{G}$ which is like $\gamma_{G}$ but adding suitably defined constraints
capturing the local consistency check results performed in layer $k-1$. Gecode is
optimized for a similar use case that arises when doing backtracking search (i.e.
whenever a variable $x$ of CSP $\gamma$ is assigned, the process of propagation and
simplification results a **new** CSP $\gamma'$ which is built incrementally from
$\gamma$).

#### FS0 was somewhat stressed by binary constraints

Accounting for the long-hauling logistic problems resulted quite involved because
of effects and preconditions of the form
$$
y = f(x_1, x_2)
$$
This was solved by enforcing an additional precondition constraint of the form
$$
f(x_1, x_2) \in D^{P}(y)
$$
where $D^{P}$ is the domain of the variable as defined in the domain. The problem
is that doing this is not sufficient when enforcing just local consistency.

For example, let the effect be
$$
y = x_1 + x_2
$$
and $D^{P}$ is $[0,10]$. If we just enforce arc-consistency over the variables
in the preconditions, it is perfectly possible that we end up with domains like
$$
D(x_1) = \{ 0, 6 \}
$$
$$
D(x_2) = \{ 0, 8 \}
$$
When evaluating $y = x_1 + x_2$ as suggested above, even if we're enforcing that
precondition constraint, we will end up with a $y$ limited to a bogus value. This
would be solved by having that domain constraint $D^{P}(y)$ in the effect CSP.

## Problem description: 2nd Iteration

FS0 solves (or approximates) a number of CSPs for a variety of tasks when
computing heuristics. Each task requires a specific type of CSP:

- *Goal checking*, to check whether the goal $G$ is **necessarily** true
at some given layer $P^k$ (i.e. $G^v$ is true in some $v \in V^k$, the set
  of possible interpretations of variables $x^k \in P^k$).
- *Support checking*, to determine the set of possible interpretations of
variables $x^{k+1}$ given $V^k$, subject to satisfying action's precondition
(that include global state constraints), condition and effect constraints.

Computing $h^{\*}_{max}$ ($h^{C}_{max}$) involves to enforce global (local)
consistency over three types of CSPs, described below:

- *Goal* CSPs $\gamma_{G}^{k}$, that implement the query "is G necessarily true
at layer $k$",
- *Action* CSPs $\gamma_{a}^{k}$, that determine the possible values of affected
variables $X_{a}^{k+1}$,
- and *Effect* CSPs, $\gamma_{e}^{k}$, that determine the possible values of
variables affected by $e$, $X_{e}^{k+1}$.

We observe that support checking can be implemented either by Action or Effect
CSPs, resulting in two formulations with slightly different properties.

We next discuss each CSP in detail.

### Goal CSPs

Goal CSPs $\gamma_{G} = ({\mathcal X}, {\mathcal C})$ consists of variable set
matching that of state variables $X$, and constraints ${\mathcal C}$:

- *Domain constraints* over state variables $X$ of the general form
$$
x \in D_x
$$
where $D_x$ is a *set* specifying the set of possible values for $x$. Special
forms of these constraints may be *bounds constraints*
$$
x \geq lb,\,x \leq ub
$$
where $lb$ ($ub$) denotes the lower (upper) bound for variable $x$, or disjunctions
over such bounds constraints, such as
$$
\bigvee_{k} ( x \geq {lb}_{k} \land x \leq {ub}_{k})
$$
when the variable $x$ value is to be comprised by *at least one* of $k$ intervals
$[{lb}_k,{ub})k]$.
- *Global state constraints* $S_{global}$, if any such constraints over state variables
$X$ are provided in the problem description.
- *Goal constraints* $G$ defined over state variables $X$ as provided in the
problem description.

For the task of checking whether the goal $G$ is necessarily true at some given
$RPG$ layer $k$, we will use a parametric form of $\gamma_{G}$, namely
$\gamma^{k}_{G}$, defined as
$$
G \land S_{global} \bigwedge_{x \in X} D^{k}_{x}
$$

### Action CSPs

For each action $a = \langle Pre,\, Eff \rangle$ where $Pre$ is an arbitrary
set of constraints over state variables $X$ and $Eff$ is a set of pairs
$$
Eff = \{ e=\langle C_e, w_{e} \rangle \}
$$
where $C_e$ is an arbitrary set of constraints over state variables $X$ and
$w_e$ is the *effect expression* of the form
$$
y = f(X_{e})
$$
where $y$ is a state variable and $X_{e}\,\subseteq\,X$ is a subset of state
variables $X$.

Action CSPs $\gamma_{a} = ({\mathcal X}, {\mathcal C})$ consist of variable set
${\mathcal X} = Y_{a} \cup X_{a}$ where

- $Y_{a}\,\subseteq\,X$ is the set of state variables appearing on the left hand
side of effect expressions $w_e$, or **output variables**,
- the set of variables constrained by preconditions, global state constraints or
effect expression right hand side
$$
X_{a} = Vars(Pre) \cup Vars(S_{global}) \cup \bigcup_{e \in Eff} Vars(w_e)
$$
or **input variables**.

The set of constraints ${\mathcal C}$ comprises

- *Domain constraints* $D_{x}$ and $D_{y}$ over the input and output variables
of action $a$. These domain constraints can be of the same form as allowed for
Goal CSPs.
- *Precondition constraints* over input variables $X_{a}$,
- *Global state constraints*  $S_{global}$, note that if
$S_{global} \neq \emptyset$ then potentially $X_{a} = X$,
- the *Effect constraints* $C_e \rightarrow y=w_{e}$ for each $e \in Eff$.

For the task of determining what values of variables $Y_a$ are possible in a given
RPG layer $k+1$ we will use a parametric form $\gamma_{a}$, namely $\gamma^{k}_{a}$, defined as follows
$$
Pre\,\land\,S_{global}\,\land\,\bigwedge_{e \in Eff} C_e \rightarrow y=w_{e}\,
\land \bigwedge_{x \in X_a} D^{k}_{x} \land \bigwedge_{y \in Y_a} D_{y}
$$

### Effect CSPs

For each action $a  = \langle Pre,\, Eff \rangle$ and effect $e \in Eff$

\[
e: C \rightarrow y = w
\]

we have the CSP $\gamma_{a,e} = ({\mathcal X}, {\mathcal C})$ where the variable set
${\mathcal X} = Y_{a} \cup X_{a}$ and

- $Y_{e}\,\subseteq\,X$ is the set of state variables appearing on the left hand
side of effect expressions $y = w$, or **output variables**,
- the set of variables constrained by preconditions, global state constraints or
effect expression right hand side
$$
X_{a} = Vars(Pre) \cup Vars(S_{global}) \cup Vars(C) \cup Vars(w)
$$
or **input variables**.

The set of constraints ${\mathcal C}$ comprises

- *Domain constraints* $D_{x}$ and $D_{y}$ over the input and output variables
of action $a$. These domain constraints can be of the same form as allowed for
Goal CSPs.
- *Precondition constraints* $Pre\,\land\,C$ over input variables $X_{a}$,
- *Global state constraints*  $S_{global}$, note that if
$S_{global} \neq \emptyset$ then potentially $X_{a} = X$,
- the *Effect constraints* $y=w$.

For the task of determining what values of variables $Y_a$ are possible in a given
RPG layer $k+1$ we will use a parametric form $\gamma_{e}$, namely
$\gamma^{k}_{e}$, defined as follows
$$
Pre\,\land\,C\,\land\,S_{global}\,\land\, y=w_{e}\,
\land \bigwedge_{x \in X_a} D^{k}_{x} \land \bigwedge_{y \in Y_a} D_{y}
$$
