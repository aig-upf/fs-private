#pragma once

#include <heuristics/novelty/features.hxx>

namespace fs0 {

    class EllipticalMapping2D : public Feature {
    public:

        EllipticalMapping2D() = delete;
        EllipticalMapping2D( const std::vector<VariableIdx>& S, std::shared_ptr<State> x0, std::shared_ptr<State> xG );

        virtual ~EllipticalMapping2D();
        virtual lapkt::novelty::NoveltyFeature<State>* clone() const override { return new EllipticalMapping2D(*this); }
        FSFeatureValueT evaluate( const State& s ) const override;

        const State& start() const { return *_start; }
        const State& goal() const { return *_goal; }
        virtual std::ostream& print(std::ostream& os) const;

        static void make_goal_relative_features( std::vector< lapkt::novelty::NoveltyFeature<State>* >& features );

    protected:
        std::shared_ptr<State>      _start;
        std::shared_ptr<State>      _goal;
    };
}
