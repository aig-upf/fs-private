import os
from string import Template

# some basic templates
_base = dict(
    pred_eval='state.getPredicate(${ext_idx})->at($point)',
    binding_size_check='assert(binding.size()==$arity);',
    binding_unwrap='ObjectIdx $varname = binding[$varidx];',
    symbol_extraction='auto ${ext_name} = state.getExtension(${ext_idx});',
    obj_id_extraction='symbols.get_object_id("${name}")',
    complex_effect_changeset='for (ObjectIdx __val__:${ext_name}->getAll(${point})) { ${pred_changeset} }',
    basic_conditional_effect='if ($op1 $op $op2) { ${effect} }',
    signature_empty_check='assert(signature.size() == 0);',
    signature_construction='signature.push_back(${state_var});',
    fact_init='Fact(_affectedVars[${var_idx}], ${value})',
    changeset_addition='changeset.add($fact);',
    satisfaction_check='return point[0] == ${value}; ${comment}',  # So far only arity-one checks
    action_name_init='const std::string $actionName::name_ = "${name}";',
    action_signature_init='const ActionSignature $actionName::signature_ = ${signature};',

    constraint_instantiation='new ${classname}(appRelevantVars[${i}], ${binding})',
    effect_instantiation='new ${classname}(effRelevantVars[${i}], effAffectedVars[${i}], binding)',

    satisfied_generic_header='isSatisfied(const ObjectIdxVector& relevant)',
    satisfied_unary_header='isSatisfied(ObjectIdx v1)',
    satisfied_binary_header='isSatisfied(ObjectIdx v1, ObjectIdx v2)',

    apply_generic_header='apply(const ObjectIdxVector& values) const { \n\t\tassert(applicable(values));',
    apply_zeroary_header='apply() const { \n\t\tassert(applicable());',
    apply_unary_header='apply(ObjectIdx v1) const { \n\t\tassert(applicable(v1));',
    apply_binary_header='apply(ObjectIdx v1, ObjectIdx v2) const { \n\t\tassert(applicable(v1, v2));',
)


class _Templates(object):
    def __init__(self, tpl_dir):
        self.tpl_dir = tpl_dir
        self.tpls = {k: Template(tpl) for k, tpl in _base.items()}

    # Returns a template object from memory or from file. If from file, caches the template for performance.
    def get(self, name):
        if name not in self.tpls:
            with open(self.tpl_dir + '/' + name + '.tpl', "r") as f:
                self.tpls[name] = Template(f.read())
        return self.tpls[name]

# We'll do away with one single instance:
currentDir = os.path.dirname(os.path.realpath(__file__))
tplDir = os.path.abspath(currentDir + '/tpls/')
tplManager = _Templates(tplDir)
