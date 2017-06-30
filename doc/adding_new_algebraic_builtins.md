# Adding new Algebraic Built-in's Symbols to FS plus

#### Step 1

Add symbol of function to the rule *unaryBuiltIn*, *binaryOp* or *multiOp* as appropiate, in `python/parser/f_pddl_plus/FunctionalStrips.g4`

#### Step 2

Regenerate parser with the script `build_parser.py` in `python/parser/f_pddl_plus` with the following command:
```
$ python3 build_parser.py --grammar FunctionalStrips.g4 --deploy fstrips
```

#### Step 3

Register the new built-in for the parser backend. You need to add the function symbol with exactly the
same syntax as you did in Step 1 to the following scripts:

 1. to the list `built_in_functional_symbols` in the script `python/parser/pddl/functions.py`,
 2. to the set returned by the function `is_arithmetic_function` in `python/parser/fstrips.py`.

#### Step 4
TIP: Program very defensively and manage floating point exceptions and the like, speed is secondary.

 1. Implement sub-class of `ArithmeticTerm`. Existing sub-classes reside in the files `src/languages/fstrips/builtin.hxx`
 and `src/languages/fstrips/builtin.cxx`.
 2. Add forward declaration of the class implemented above in `src/languages/fstrips/language_fwd.hxx`
 3. Implement function `isXXXXTerm()` where XXXX is name of the term. See existing functions at the end of `builtin.hxx`.
 4. Add the symbol to the set of builtin terms to the set `ArithmeticTermFactory::_builtin_terms` in `src/languages/fstrips/builtin.cxx`.
 5. **If unary**, add the symbol to the set of builtin terms to the set `ArithmeticTermFactory::_unary_terms` in `src/languages/fstrips/builtin.cxx`.
 6. Register new term in the factory method `ArithmeticTermFactory::create` in `src/languages/fstrips/builtin.cxx`.
 7. Add entries to the Loki visitors in `src/languages/fstrips/operations`

#### Step 5 (Optional)

Expose the term to the Gecode interface by implementing a sub-class of `ComponentTranslator`. Existing
sub-classes reside in the files `src/constraint/gecodes/translators/component_translator.hxx` and
`src/constraint/gecodes/translators/component_translator.hxx`.

Register the term translator as part of the language in method `LogicalComponentRegistry::registerGecodeTranslators`
in `src/constraints/registry.cxx`.
