/**
 * @file Tuple.hpp
 * @author 青羽 (chen_qingyu@qq.com, https://chen-qingyu.github.io/)
 * @brief Tuple template class.
 * @date 2023.03.13
 *
 * @copyright Copyright (C) 2023
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef TUPLE_HPP
#define TUPLE_HPP

#include <ostream> // std::ostream

#include "utility.hpp"

namespace pytype
{

// Empty tuple template class.
template <typename... _>
class Tuple
{
public:
    // Check whether two tuples are equal.
    template <typename... Ts>
    constexpr bool operator==(const Tuple<Ts...>& that) const
    {
        return that.size() == 0;
    }

    // Check whether two tuples are not equal.
    template <typename... Ts>
    constexpr bool operator!=(const Tuple<Ts...>& that) const
    {
        return that.size() != 0;
    }

    // Empty tuple have no element.
    constexpr int size() const
    {
        return 0;
    }
};

/**
 * @brief Tuple template class.
 *
 * @tparam T the type of first element
 * @tparam Ts the type of rest elements
 */
template <typename T, typename... Ts>
class Tuple<T, Ts...> : public Tuple<Ts...>
{
private:
    // The value stored in current level.
    T value_;

public:
    /*
     * Constructor / Destructor
     */

    /**
     * @brief Construct a new tuple object.
     *
     * @param value the first element
     * @param values the rest elements
     */
    Tuple<T, Ts...>(const T& value, const Ts&... values)
        : Tuple<Ts...>(values...)
        , value_(value)
    {
    }

    /*
     * Comparison
     */

    /**
     * @brief Check whether two tuples are equal.
     *
     * @param that another tuple
     * @return true if two tuples are equal
     */
    bool operator==(const Tuple<T, Ts...>& that) const
    {
        return value_ == that.value_ ? (rest() == that.rest()) : false;
    }

    /**
     * @brief Check whether two tuples are not equal.
     *
     * @param that another tuple
     * @return true if two tuples are not equal
     */
    bool operator!=(const Tuple<T, Ts...>& that) const
    {
        return !(*this == that);
    }

    /*
     * Access
     */

    /**
     * @brief Get the i-th element of the tuple.
     *
     * @tparam N the index
     * @return the i-th element
     */
    template <int N>
    decltype(auto) get() const
    {
        static_assert(N < 1 + sizeof...(Ts));
        if constexpr (N == 0)
        {
            return value_;
        }
        else
        {
            return Tuple<Ts...>::template get<N - 1>();
        }
    }

    /*
     * Examination (will not change the object itself)
     */

    /**
     * @brief Return the number of elements in the tuple.
     *
     * @return the number of elements in the tuple
     */
    constexpr int size() const
    {
        return 1 + sizeof...(Ts);
    }

    /**
     * @brief Return the rest of the tuple.
     *
     * @return the rest of the tuple
     */
    const Tuple<Ts...>& rest() const
    {
        return *((Tuple<Ts...>*)this);
    }
};

/*
 * Non-member functions
 */

/**
 * @brief Creates a tuple object, deducing the target type from the types of arguments.
 *
 * @tparam Ts the type of elements in the tuple
 * @param values zero or more arguments to construct the tuple from
 * @return a Tuple object containing the given values
 */
template <typename... Ts>
decltype(auto) make_tuple(Ts&&... values)
{
    return Tuple<Ts...>(values...);
}

// Underlying implementation
namespace impl
{

template <typename... Ts>
void print(std::ostream& os, const Tuple<Ts...>& tuple)
{
    if constexpr (sizeof...(Ts) > 0)
    {
        os << tuple.template get<0>() << (tuple.size() == 1 ? "" : ", ");
        print(os, tuple.rest());
    }
}

} // namespace impl

/**
 * @brief Output tuple data to the specified output stream.
 *
 * @tparam Ts the type of elements in the tuple, must be printable
 * @param os an output stream
 * @param tuple the tuple to be printed to the output stream
 * @return self reference of the output stream
 */
template <typename... Ts>
std::ostream& operator<<(std::ostream& os, const Tuple<Ts...>& tuple)
{
    os << "(";
    impl::print(os, tuple);
    os << ")";

    return os;
}

} // namespace pytype

#endif // TUPLE_HPP
