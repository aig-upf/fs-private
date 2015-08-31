
#include "external_base.hxx"

//! This is a default External class for those domains that do not need any custom external code.
class External : public ExternalBase {
public:
	External(const std::string& data_dir) : ExternalBase(data_dir) {}
};