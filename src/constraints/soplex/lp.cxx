#include <constraints/soplex/lp.hxx>
#include <languages/fstrips/scopes.hxx>
#include <languages/fstrips/terms.hxx>
#include <lapkt/tools/logging.hxx>
#include <problem_info.hxx>

namespace fs0  { namespace spx {
    using namespace soplex;

    LinearProgram::LinearProgram()
        : _num_vars(0) {

    }

    LinearProgram::~LinearProgram() {

    }

    void
    LinearProgram::add_constraint( const fs::Formula* abstract_f ) {
        auto f = dynamic_cast<const fs::RelationalFormula*>( abstract_f );
        if ( f == nullptr ) return; // Not supported

        std::set<VariableIdx> scope_f;
    	fs::ScopeUtils::computeFullScope(f, scope_f);
        for ( VariableIdx x : scope_f ) {
            // create new LP var
            if ( _variables.find(x) != _variables.end() ) continue;
            _num_vars++;
            // bind fs variable to lp variable
            _fs_var_to_lp.insert( std::make_pair( x, _num_vars-1 ));
            _lp_var_to_fs.insert( std::make_pair( _num_vars-1, x));
        }
        _variables.insert( scope_f.begin(), scope_f.end() );
        // Check for supported relational operators

        if ( f->symbol() != fs::RelationalFormula::Symbol::GEQ
            && f->symbol() != fs::RelationalFormula::Symbol::LEQ
            && f->symbol() != fs::RelationalFormula::Symbol::LT
            && f->symbol() != fs::RelationalFormula::Symbol::GT) {
            throw std::runtime_error("spx::LinearProgram::add_constraint() : At the moment only '>=', '>', '<' and '<=' constraints are supported.");
        }

        auto lhs = dynamic_cast<const fs::StateVariable* >(f->lhs());
        auto rhs = dynamic_cast<const fs::Constant* >(f->rhs());

        if ( lhs == nullptr )
            throw std::runtime_error( "spx::LinearProgramm::add_constraint() : At the moment the left hand side of a constraint needs to be a state variable");
        if ( rhs == nullptr )
            throw std::runtime_error( "spx::LinearProgramm::add_constraint() : At the moment the right hand side of a constraint needs to be a constant");

        _constraints.push_back( f );
    }

    void
    LinearProgram::setup( ) {
        // MRJ: here comes the fun bit
        /* set the objective sense */
        _lp.setIntParam(SoPlex::OBJSENSE, SoPlex::OBJSENSE_MINIMIZE);

        /* we first add variables */
        DSVector dummycol(0);
        // MRJ: Not entirely sure yet what is the second argument about but the
        // reset are:
        // - p_obj: Constant multiplying variable in objective function
        // - p_vector: ???
        // - upper: upper bound on the domain of the variable
        // - lower: lower bound on the domain of the variable
        for ( int i = 0; i < num_vars(); i ++)
            _lp.addColReal(LPCol(1.0, dummycol, infinity, -infinity));

        // and now it is time for the constraints
        for ( auto f : _constraints ) {
            auto lhs = dynamic_cast<const fs::StateVariable* >(f->lhs());
            auto rhs = dynamic_cast<const fs::Constant* >(f->rhs());
            int x = _fs_var_to_lp[lhs->getValue()];
            object_id o_b = rhs->getValue();
            float b;
            if ( o_type(o_b) == type_id::int_t)
                b = (float)fs0::value<int>(o_b);
            else if ( o_type(o_b) == type_id::float_t )
                b = fs0::value<float>(o_b);
            else
                throw std::runtime_error("spx::LinearProgram::setup() : unsupported constant type (not float or integer)");

            DSVector row(4);
            row.add(x, 1.0); // coefficient for variable x
            // Constraints are of the form l <= a^t x <= u
            // so for instance x >= b becomes
            //   b <= x <= infinity
            // and x <= b becomes
            //  -infinity <= x <= b
            if ( f->symbol() ==  fs::RelationalFormula::Symbol::GEQ )
                _lp.addRowReal(LPRow(b, row, infinity));
            else if ( f->symbol() ==  fs::RelationalFormula::Symbol::GT ) {
                LPT_INFO("soplex", "Relaxing constraint: " << *f << " New b = " << 1.01f*b);
                _lp.addRowReal(LPRow( b * 0.99f, row, infinity));
            }
            else if ( f->symbol() ==  fs::RelationalFormula::Symbol::LEQ )
                _lp.addRowReal(LPRow(-infinity, row, b));
            else if ( f->symbol() ==  fs::RelationalFormula::Symbol::LT ) {
                LPT_INFO("soplex", "Relaxing constraint: " << *f << " New b = " << 0.99f*b);
                _lp.addRowReal(LPRow(-infinity, row, b*0.99f));
            }


        }
    }

    void
    LinearProgram::solve( ) {
        /* write LP in .lp format */
        _lp.writeFileReal("dump.lp", NULL, NULL, NULL);
        _solution.clear();

        /* solve LP */
        SPxSolver::Status stat;
        DVector prim(num_vars());
        stat = _lp.optimize();

        /* get solution */
        if( stat == SPxSolver::OPTIMAL )
        {
           _lp.getPrimalReal(prim);
           LPT_INFO( "soplex",  "LP solved to optimality.");
           LPT_INFO( "soplex",  "Objective value is " << _lp.objValueReal() << ".");

           const ProblemInfo& info = ProblemInfo::getInstance();
           for ( int x = 0; x < num_vars(); x++ ) {
               LPT_INFO("soplex", "\t" << info.getVariableName(_lp_var_to_fs[x]) << " = " << prim[x]);
               VariableIdx fs_x = _lp_var_to_fs[x];
               type_id x_type = info.sv_type(fs_x);
               object_id v = x_type == type_id::float_t ? make_object((float)prim[x]) : make_object( x_type, (int)prim[x]);
               _solution.push_back(Atom(fs_x,v));
           }
        }
    }

}}
