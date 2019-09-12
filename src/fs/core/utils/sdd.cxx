
#include <fs/core/utils/sdd.hxx>

#include <sdd/sddapi.hxx>
#include <boost/filesystem.hpp>   // includes all needed Boost.Filesystem declarations
#include <boost/algorithm/string/predicate.hpp>

#include <iostream>

namespace fsys = boost::filesystem;

//! Loads from disk all SDDs in the given directory (one per action schema)
void load_sdd_from_disk(const std::string& dir) {
    const std::string suffix(".vtree.sdd");
    fsys::path path(dir);
    if (!fsys::exists(path)) throw std::runtime_error("Non-existing base SDD directory");

    for (fsys::directory_entry& entry : fsys::directory_iterator(path)) {
        auto vtree_fname = entry.path().filename().string();
        if (boost::algorithm::ends_with(vtree_fname, suffix)) {
            const std::string schema_name = vtree_fname.substr(0, vtree_fname.size() - suffix.size());
            std::string mng_fname = schema_name + ".manager.sdd";
//            std::cout << "     " << vtree_fname << '\n';
//            std::cout << "          " << mng_fname << '\n';


            // set up vtree and manager
            Vtree* vtree = sdd_vtree_read((dir + vtree_fname).c_str());
            SddManager* manager = sdd_manager_new(vtree);

            printf("reading sdd from file ...\n");
            SddNode* alpha = sdd_read((dir + mng_fname).c_str(), manager);
            printf("  sdd size = %zu\n", sdd_size(alpha));
        }
    }


}