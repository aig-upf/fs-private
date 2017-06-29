# Adding new Algebraic Built-in's Symbols to FS plus

#### Step 1

Add symbol of function to the rule *unaryBuiltIn* in `preprocessor/antlr4_parsers/FunctionalStrips.g4`

#### Step 2

Regenerate parser with the script `build_parser.py` in `preprocessor/antlr4_parsers` with the following command:
```
$ python3 build_parser.py --grammar FunctionalStrips.g4 --deploy fstrips
```

#### Step 3

Register the new built-in for the parser backend. You need to add the function symbol with exactly the
same syntax as you did in Step 1 to the following scripts:

 1. to the list `built_in_functional_symbols` in the script `preprocessor/anltr4_parsers/fstrips/functions.py`,
 2. to the list `BASE_SYMBOLS` in `preprocessor/compilation/parser.py`.

#### Step 4
TIP: Program very defensively and manage floating point exceptions and the like, speed is secondary.

 1. Implement sub-class of `ArithmeticTerm`. Existing sub-classes reside in the files `src/languages/fstrips/builtin.hxx`
 and `src/languages/fstrips/builtin.cxx`.
 2. Implement function `isXXXXTerm()` where XXXX is name of the term. See existing functions at the end of `builtin.hxx`.
 3. Add the symbol to the set of builtin terms to the set `ArithmeticTermFactory::_builtin_terms` in `src/languages/fstrips/builtin.cxx`.
 4. **If unary**, add the symbol to the set of builtin terms to the set `ArithmeticTermFactory::_unary_terms` in `src/languages/fstrips/builtin.cxx`.
 3. Register new term in the factory method `ArithmeticTermFactory::create` in `src/languages/fstrips/builtin.cxx`.

#### Step 5 (soon to be deprecated)

Add check to method `LinearTransform::checkForHigherOrderTerms` in `src/languages/fstrips/transformations.cxx` to avoid
the linearization algorithms to mess up any nested terms.

#### Step 6 (Optional)

Expose the term to the Gecode interface by implementing a sub-class of `ComponentTranslator`. Existing
sub-classes reside in the files `src/constraint/gecodes/translators/component_translator.hxx` and
`src/constraint/gecodes/translators/component_translator.hxx`.

Register the term translator as part of the language in method `LogicalComponentRegistry::registerGecodeTranslators`
in `src/constraints/registry.cxx`.
