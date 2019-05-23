

#include <fs/core/fstrips/grounding.hxx>

namespace fs0::fstrips {

std::string Grounding::compute_state_variable_name(const symbol_id& symbol, const std::vector<object_id>& point) {
    const auto& sname = language_->get_symbol_name(symbol);
    if (point.empty()) return sname;
    std::string spoint;
    for (unsigned i = 0, m = point.size(); i < m; ++i) {
        spoint += language_->get_object_name(point[i]);
        if (i < m-1) spoint += ", ";
    }
    return sname + "(" + spoint + ")";
}

VariableIdx Grounding::add_state_variable(const symbol_id& symbol, const std::vector<object_id>& point) {
    SymbolInfo symbol_info = language_->symbolinfo(symbol);
    const auto& signature = symbol_info.signature();

    unsigned var_id = variableNames.size();
    variableNames.push_back(compute_state_variable_name(symbol, point));
    variableIds.insert(std::make_pair(variableNames.back(), var_id));

    // We store the FS type for faster retrieval. For predicates, it is the one corresponding to "bool"
    TypeIdx fstype;
    if (symbol_info.type()==symbol_t::Predicate) {
        fstype = language_->get_fs_type_id("bool");
    } else {
        assert(!signature.empty());
        fstype = signature.back();
    }
    variableTypes.push_back(fstype);
    _sv_types.push_back(language_->get_type_id(fstype));

    variableDataToId.insert(std::make_pair(std::make_pair(symbol, point), var_id));
    variableIdToData.emplace_back(symbol, point);
    return var_id;
}

std::ostream& Grounding::print(std::ostream& os) const {
    os << "Grounding object:\n\tState Variables:" << std::endl;
    for (unsigned i = 0; i < variableNames.size(); ++i) {
        os << "\t\t" << i << ": " << variableNames[i] << std::endl;
    }
    return os;
}

} // namespaces
