
//! This is a default External class for those domains that do not need any custom external code.
class External : public fs0::ExternalI {
public:
	External(const ProblemInfo& info, const std::string& data_dir) {}
};