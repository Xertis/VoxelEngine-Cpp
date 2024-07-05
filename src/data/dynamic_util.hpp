#ifndef DATA_DYNAMIC_UTIL_HPP_
#define DATA_DYNAMIC_UTIL_HPP_

#include "dynamic.hpp"

#include <glm/glm.hpp>

namespace dynamic {
    template<int n>
    inline dynamic::List_sptr to_value(glm::vec<n, float> vec) {
        auto list = dynamic::create_list();
        for (size_t i = 0; i < n; i++) {
            list->put(vec[i]);
        }
        return list;
    }

    template<int n, int m>
    inline dynamic::List_sptr to_value(glm::mat<n, m, float> mat) {
        auto list = dynamic::create_list();
        for (size_t i = 0; i < n; i++) {
            for (size_t j = 0; j < m; j++) {
                list->put(mat[i][j]);
            }
        }
        return list;
    }

    template<int n>
    void get_vec(const dynamic::Map_sptr& root, const std::string& name, glm::vec<n, float>& vec) {
        if (const auto& list = root->list(name)) {
            for (size_t i = 0; i < n; i++) {
                vec[i] = list->num(i);
            }
        }
    }
}

#endif // DATA_DYNAMIC_UTIL_HPP_