
        else if(classname == "${classname}") {
			ScopedConstraint::vcptr constraints = {
				${constraint_list}
			};
			ScopedEffect::vcptr effects = {
				${effect_list}
			};
			pointer = new ${classname}(binding, constraints, effects);
        }