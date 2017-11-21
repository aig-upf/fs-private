#pragma once


namespace fs0 {
    class State;
}

namespace fs0 {
    //! Reward class
    //!
    //! Base classs for all reward signals
    class Reward {
    public:
        Reward() = default;
        virtual ~Reward() = default;

        //! Reward::evaluate()
        //! evaluates reward function on the given State
        virtual float   evaluate( const State& s ) const = 0;
        virtual float   terminal( const State& s ) const { return 0.0f;};

    protected:

    };
} /*fs0 */
