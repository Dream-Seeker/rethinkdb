// Copyright 2010-2013 RethinkDB, all rights reserved.
#ifndef RDB_PROTOCOL_SCOPE_HPP_
#define RDB_PROTOCOL_SCOPE_HPP_

#include <deque>
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "errors.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

#include "containers/archive/boost_types.hpp"
#include "http/json.hpp"
#include "rdb_protocol/rdb_protocol_json.hpp"
#include "rpc/serialize_macros.hpp"

template <class T>
inline void guarantee_T(const T &) { }

template <>
inline void guarantee_T<boost::shared_ptr<scoped_cJSON_t> >(const boost::shared_ptr<scoped_cJSON_t> &j) {
    guarantee(j);
}

template <class T>
class variable_scope_t {
public:
    void put_in_scope(const std::string &name, const T &t);

    T get(const std::string &name) const;

    // Calling this only makes sense in the typechecker. All variables
    // are guranteed by the typechecker to be present at runtime.
    bool is_in_scope(const std::string &name) const;

    void push();

    void pop();

    // TODO(rntz): find a better way to do this.
    void dump(std::vector<std::string> *argnames, std::vector<T> *argvals) const;

    class new_scope_t {
    public:
        new_scope_t(variable_scope_t<T> *parent);
        new_scope_t(variable_scope_t<T> *parent, const std::string &name, const T &t);
        ~new_scope_t();
    private:
        variable_scope_t<T> *parent;
    };

    RDB_MAKE_ME_SERIALIZABLE_1(scopes);

private:
    std::list<std::map<std::string, T> > scopes;
};

/* an implicit_value_t allows for a specific implicit value to exist at certain
 * points in execution for example the argument to get attr is implicitly
 * defined to be the value of the row upon entering a filter,map etc.
 * implicit_value_t supports scopes for its values but does not allow looking
 * up values in any scope to the current one. */
template <class T>
class implicit_value_t {
public:
    implicit_value_t() : depth(0) {
        push();
    }

    void push() {
        scopes.push_front(boost::optional<T>());
        depth += 1;
    }

    void push(const T &t) {
        scopes.push_front(t);
        depth += 1;
    }

    void pop() {
        scopes.pop_front();
        depth -= 1;
    }

    class impliciter_t {
    public:
        explicit impliciter_t(implicit_value_t *_parent)
            : parent(_parent)
        {
            parent->push();
        }

        impliciter_t(implicit_value_t *_parent, const T& t)
            : parent(_parent)
        {
            parent->push(t);
        }

        ~impliciter_t() {
            parent->pop();
        }
    private:
        implicit_value_t *parent;
    };

    bool has_value() const {
        return scopes.front();
    }

    T get_value() const {
        return *scopes.front();
    }

    bool depth_is_legal() const {
        return depth == 2; //We push once at the beginning, so we want 2 here.
    }

    RDB_MAKE_ME_SERIALIZABLE_2(scopes, depth);
private:
    std::list<boost::optional<T> > scopes;
    int depth;
};

#endif  // RDB_PROTOCOL_SCOPE_HPP_
