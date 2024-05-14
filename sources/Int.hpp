//! @file Int.hpp
//! @author Qingyu Chen (chen_qingyu@qq.com, https://chen-qingyu.github.io/)
//! @brief Int class.
//! @date 2023.01.21
//!
//! @copyright Copyright (C) 2023 - 2024
//!
//! This program is free software: you can redistribute it and/or modify
//! it under the terms of the GNU General Public License as published by
//! the Free Software Foundation, either version 3 of the License, or
//! (at your option) any later version.
//!
//! This program is distributed in the hope that it will be useful,
//! but WITHOUT ANY WARRANTY; without even the implied warranty of
//! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//! GNU General Public License for more details.
//!
//! You should have received a copy of the GNU General Public License
//! along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef INT_HPP
#define INT_HPP

#include "utility.hpp"

namespace pyincpp
{

/// Int provides support for big integer arithmetic.
class Int
{
private:
    // List of digits, represent absolute value of the integer.
    // Base 10, little endian.
    // Example: `12345000`
    // ```
    // digit: 0 0 0 5 4 3 2 1
    // index: 0 1 2 3 4 5 6 7
    // ```
    std::vector<signed char> digits_;

    // Sign of integer, 1 is positive, -1 is negative, and 0 is zero.
    signed char sign_ = 0; // need init value

    // Remove leading zeros elegantly.
    Int& remove_leading_zeros()
    {
        auto it = digits_.rbegin();
        while (it != digits_.rend() && *it == 0)
        {
            ++it;
        }
        digits_.erase(it.base(), digits_.end()); // note: rbegin().base() == end()

        return *this;
    }

    // Add `n` leading zeros elegantly.
    Int& add_leading_zeros(int n)
    {
        digits_.resize(digits_.size() + n, 0);

        return *this;
    }

    // Test whether the characters represent an integer.
    static bool is_integer(const char* chars, int len)
    {
        if (len == 0 || (len == 1 && (chars[0] == '+' || chars[0] == '-')))
        {
            return false;
        }

        for (int i = (chars[0] == '+' || chars[0] == '-'); i < len; i++)
        {
            // surprisingly, this is faster than `!std::isdigit(chars[i])`
            // my guess is that the conversion of char to int takes time
            if (chars[i] < '0' || chars[i] > '9')
            {
                return false;
            }
        }

        return true;
    }

    // Increment the absolute value by 1 quickly.
    // Require this != 0
    void abs_inc()
    {
        // add a leading zero for carry
        digits_.push_back(0);

        int i = 0;
        while (digits_[i] == 9)
        {
            ++i;
        }
        ++digits_[i];
        while (i != 0)
        {
            digits_[--i] = 0;
        }

        remove_leading_zeros();

        // keep sign unchanged
    }

    // Decrement the absolute value by 1 quickly.
    // Require this != 0
    void abs_dec()
    {
        int i = 0;
        while (digits_[i] == 0)
        {
            ++i;
        }
        --digits_[i];
        while (i != 0)
        {
            digits_[--i] = 9;
        }

        remove_leading_zeros();

        // if result is zero, set sign to 0
        sign_ = digits_.empty() ? 0 : sign_;
    }

public:
    /*
     * Constructor / Destructor
     */

    /// Construct a new zero integer object.
    Int() = default;

    /// Construct a new integer object based on the given null-terminated characters.
    Int(const char* chars)
    {
        const int len = std::strlen(chars);
        if (!is_integer(chars, len))
        {
            throw std::runtime_error("Error: Wrong integer literal.");
        }

        sign_ = (chars[0] == '-' ? -1 : 1);
        const int s = (chars[0] == '-' || chars[0] == '+'); // skip symbol
        const int digit_len = len - s;

        // this is faster than `reserve` and `push_back`, cause `push_back` is slower than `[]`
        digits_.resize(digit_len);
        for (int i = 0; i != digit_len; i++)
        {
            digits_[i] = chars[len - 1 - i] - '0';
        }

        remove_leading_zeros();

        // if result is zero, set sign to 0
        sign_ = digits_.empty() ? 0 : sign_;
    }

    /// Construct a new integer object based on the given int.
    Int(int integer)
    {
        if (integer == 0)
        {
            return;
        }

        // integer != 0
        sign_ = (integer > 0 ? 1 : -1);
        integer = std::abs(integer);
        while (integer > 0)
        {
            digits_.push_back(integer % 10);
            integer /= 10;
        }
    }

    /// Copy constructor.
    Int(const Int& that) = default;

    /// Move constructor.
    Int(Int&& that)
        : digits_(std::move(that.digits_))
        , sign_(std::move(that.sign_))
    {
        that.sign_ = 0;
    }

    /*
     * Comparison
     */

    /// Determine whether this integer is equal to another integer.
    constexpr bool operator==(const Int& that) const
    {
        return sign_ == that.sign_ && digits_ == that.digits_;
    }

    /// Compare the integer with another integer.
    constexpr auto operator<=>(const Int& that) const
    {
        if (sign_ != that.sign_)
        {
            if (sign_ == 1) // this is +, that is - or 0
            {
                return 1; // gt
            }
            else if (sign_ == -1) // this is -, that is + or 0
            {
                return -1; // lt
            }
            else // this is 0, that is + or -
            {
                return that.sign_ == 1 ? -1 : 1;
            }
        }

        // the sign of two integers is the same

        if (digits_.size() != that.digits_.size())
        {
            if (sign_ == 1)
            {
                return digits_.size() > that.digits_.size() ? 1 : -1;
            }
            else
            {
                return digits_.size() > that.digits_.size() ? -1 : 1;
            }
        }

        for (int i = digits_.size() - 1; i >= 0; i--) // i = -1 if is zero, ok
        {
            if (digits_[i] != that.digits_[i])
            {
                if (sign_ == 1)
                {
                    return digits_[i] > that.digits_[i] ? 1 : -1;
                }
                else
                {
                    return digits_[i] > that.digits_[i] ? -1 : 1;
                }
            }
        }

        return 0; // eq
    }

    /*
     * Assignment
     */

    /// Copy assignment operator.
    Int& operator=(const Int& that) = default;

    /// Move assignment operator.
    Int& operator=(Int&& that)
    {
        digits_ = std::move(that.digits_);
        sign_ = std::move(that.sign_);

        that.sign_ = 0;

        return *this;
    }

    /*
     * Examination
     */

    /// Return the number of digits in the integer (based 10).
    constexpr int digits() const
    {
        return digits_.size();
    }

    /// Determine whether the integer is zero quickly.
    constexpr bool is_zero() const
    {
        return sign_ == 0;
    }

    /// Determine whether the integer is positive quickly.
    constexpr bool is_positive() const
    {
        return sign_ == 1;
    }

    /// Determine whether the integer is negative quickly.
    constexpr bool is_negative() const
    {
        return sign_ == -1;
    }

    /// Determine whether the integer is even quickly.
    constexpr bool is_even() const
    {
        return is_zero() ? true : (digits_[0] & 1) == 0;
    }

    /// Determine whether the integer is odd quickly.
    constexpr bool is_odd() const
    {
        return is_zero() ? false : (digits_[0] & 1) == 1;
    }

    /*
     * Manipulation
     */

    /// Return this += `rhs`.
    Int& operator+=(const Int& rhs)
    {
        return *this = *this + rhs;
    }

    /// Return this -= `rhs`.
    Int& operator-=(const Int& rhs)
    {
        return *this = *this - rhs;
    }

    /// Return this *= `rhs`.
    Int& operator*=(const Int& rhs)
    {
        return *this = *this * rhs;
    }

    /// Return this /= `rhs` (not zero).
    Int& operator/=(const Int& rhs)
    {
        return *this = *this / rhs;
    }

    /// Return this %= `rhs` (not zero).
    Int& operator%=(const Int& rhs)
    {
        return *this = *this % rhs;
    }

    /// Increment the value by 1 quickly.
    Int& operator++()
    {
        if (sign_ == 0)
        {
            sign_ = 1;
            digits_.push_back(1);
        }
        else
        {
            (sign_ == 1) ? abs_inc() : abs_dec();
        }

        return *this;
    }

    /// Decrement the value by 1 quickly.
    Int& operator--()
    {
        if (sign_ == 0)
        {
            sign_ = -1;
            digits_.push_back(1);
        }
        else
        {
            (sign_ == 1) ? abs_dec() : abs_inc();
        }

        return *this;
    }

    /*
     * Production
     */

    /// Return the copy of this.
    Int operator+() const
    {
        return *this;
    }

    /// Return the opposite value of this.
    Int operator-() const
    {
        Int num = *this;
        num.sign_ = -num.sign_;
        return num;
    }

    /// Return the absolute value of this.
    Int abs() const
    {
        return sign_ == -1 ? -*this : *this;
    }

    /// Return this + `rhs`.
    Int operator+(const Int& rhs) const
    {
        // if one of the operands is zero, just return another one
        if (sign_ == 0 || rhs.sign_ == 0)
        {
            return sign_ == 0 ? rhs : *this;
        }

        // if the operands are of opposite signs, perform subtraction
        if (sign_ == 1 && rhs.sign_ == -1)
        {
            return *this - (-rhs);
        }
        else if (sign_ == -1 && rhs.sign_ == 1)
        {
            return rhs - (-*this);
        }

        // the sign of two integers is the same and not zero

        // prepare variables
        int size = std::max(digits_.size(), rhs.digits_.size()) + 1;

        Int num1 = *this;
        num1.add_leading_zeros(size - 1 - num1.digits_.size());

        Int num2 = rhs;
        num2.add_leading_zeros(size - 1 - num2.digits_.size());

        Int result;
        result.sign_ = sign_; // the signs are same
        result.add_leading_zeros(size);

        // simulate the vertical calculation
        const auto& a = num1.digits_;
        const auto& b = num2.digits_;
        auto& c = result.digits_;
        for (int i = 0; i < size - 1; i++)
        {
            c[i] += a[i] + b[i];
            c[i + 1] = c[i] / 10;
            c[i] %= 10;
        }

        // remove leading zeros and return result
        return result.remove_leading_zeros();
    }

    /// Return this - `rhs`.
    Int operator-(const Int& rhs) const
    {
        // if one of the operands is zero
        if (sign_ == 0 || rhs.sign_ == 0)
        {
            return sign_ == 0 ? -rhs : *this;
        }

        // if the operands are of opposite signs, perform addition
        if (sign_ != rhs.sign_)
        {
            return *this + (-rhs);
        }

        // the sign of two integers is the same and not zero

        // prepare variables
        int size = std::max(digits_.size(), rhs.digits_.size());

        Int num1 = *this;
        num1.add_leading_zeros(size - num1.digits_.size());

        Int num2 = rhs;
        num2.add_leading_zeros(size - num2.digits_.size());

        Int result;
        result.sign_ = sign_;                       // the signs are same
        if (sign_ == 1 ? num1 < num2 : num1 > num2) // let num1.abs() >= num2.abs()
        {
            std::swap(num1, num2);
            result.sign_ = -result.sign_;
        }
        result.add_leading_zeros(size);

        // simulate the vertical calculation, assert a >= b
        auto& a = num1.digits_;
        const auto& b = num2.digits_;
        auto& c = result.digits_;
        for (int i = 0; i < size; i++)
        {
            if (a[i] < b[i]) // carry
            {
                a[i + 1]--;
                a[i] += 10;
            }
            c[i] = a[i] - b[i];
        }

        // remove leading zeros
        result.remove_leading_zeros();

        // if result is zero, set sign to 0
        result.sign_ = result.digits_.empty() ? 0 : result.sign_;

        // return result
        return result;
    }

    /// Return this * `rhs`.
    Int operator*(const Int& rhs) const
    {
        // if one of the operands is zero, just return zero
        if (sign_ == 0 || rhs.sign_ == 0)
        {
            return 0;
        }

        // the sign of two integers is not zero

        // prepare variables
        int size = digits_.size() + rhs.digits_.size();

        Int result;
        result.sign_ = (sign_ == rhs.sign_ ? 1 : -1); // the sign is depends on the sign of operands
        result.add_leading_zeros(size);

        // simulate the vertical calculation
        const auto& a = digits_;
        const auto& b = rhs.digits_;
        auto& c = result.digits_;
        for (int i = 0; i < a.size(); i++)
        {
            for (int j = 0; j < b.size(); j++)
            {
                c[i + j] += a[i] * b[j];
                c[i + j + 1] += c[i + j] / 10;
                c[i + j] %= 10;
            }
        }

        // remove leading zeros and return
        return result.remove_leading_zeros();
    }

    /// Return this / `rhs` (not zero).
    Int operator/(const Int& rhs) const
    {
        // if rhs is zero, throw an exception
        if (rhs.sign_ == 0)
        {
            throw std::runtime_error("Error: Divide by zero.");
        }

        // if this.abs() < rhs.abs(), just return 0
        if (digits() < rhs.digits())
        {
            return 0;
        }

        // the sign of two integers is not zero

        // prepare variables
        int size = digits_.size() - rhs.digits_.size() + 1;

        Int num1 = abs();

        Int tmp;       // intermediate variable for rhs * 10^i
        tmp.sign_ = 1; // positive

        // tmp = rhs * 10^(size), not size-1, since the for loop will erase first, so tmp is rhs * 10^(size-1) at first
        tmp.digits_ = std::vector<signed char>(size, 0);
        tmp.digits_.insert(tmp.digits_.end(), rhs.digits_.begin(), rhs.digits_.end());

        Int result;
        result.sign_ = (sign_ == rhs.sign_ ? 1 : -1); // the sign is depends on the sign of operands
        result.add_leading_zeros(size);

        // calculation
        for (int i = size - 1; i >= 0; i--)
        {
            // tmp = rhs * 10^i in O(1), I'm a fxxking genius
            // after testing, found that use vector is very faster than use deque `tmp.digits_.pop_front();`
            // my guess is that deque's various operations take longer than vector's
            tmp.digits_.erase(tmp.digits_.begin());

            while (num1 >= tmp) // <= 9 loops, so O(1)
            {
                result.digits_[i]++;
                num1 -= tmp;
            }
        }

        // remove leading zeros
        result.remove_leading_zeros();

        // if result is zero, set sign to 0
        result.sign_ = result.digits_.empty() ? 0 : result.sign_;

        // return result
        return result;
    }

    /// Return this % `rhs` (not zero).
    Int operator%(const Int& rhs) const
    {
        // if rhs is zero, throw an exception
        if (rhs.sign_ == 0)
        {
            throw std::runtime_error("Error: Divide by zero.");
        }

        // if this.abs() < rhs.abs(), just return this
        if (digits() < rhs.digits())
        {
            return *this;
        }

        // the sign of two integers is not zero

        // prepare variables
        int size = digits_.size() - rhs.digits_.size() + 1;

        Int result = abs();

        Int tmp;       // intermediate variable for rhs * 10^i
        tmp.sign_ = 1; // positive

        // tmp = rhs * 10^(size), not size-1, since the for loop will erase first, so tmp is rhs * 10^(size-1) at first
        tmp.digits_ = std::vector<signed char>(size, 0);
        tmp.digits_.insert(tmp.digits_.end(), rhs.digits_.begin(), rhs.digits_.end());

        // calculation
        for (int i = size - 1; i >= 0; i--)
        {
            // tmp = rhs * 10^i in O(1), I'm a fxxking genius
            // after testing, found that use vector is very faster than use deque `tmp.digits_.pop_front();`
            // my guess is that deque's various operations take longer than vector's
            tmp.digits_.erase(tmp.digits_.begin());

            while (result >= tmp) // <= 9 loops, so O(1)
            {
                result -= tmp;
            }
        }

        // remove leading zeros
        result.remove_leading_zeros();

        // if result is zero, set sign to 0, else to this's
        result.sign_ = result.digits_.empty() ? 0 : sign_;

        // return result
        return result;
    }

    /// Return the factorial of this.
    Int factorial() const
    {
        if (sign_ == -1)
        {
            throw std::runtime_error("Error: Negative integer have no factorial.");
        }

        Int result = 1; // 0! == 1

        for (Int i = *this; i.is_positive(); i.abs_dec()) // fast judgement, fast decrement
        {
            result *= i;
        }

        return result;
    }

    /// Calculate the next prime that greater than this.
    Int next_prime() const
    {
        if (*this < 2)
        {
            return 2;
        }

        Int prime = *this; // >= 2

        // if prime is even, let it odd and < this, because prime > 2 is odd and while prime += 2
        if (prime.is_even())
        {
            prime.abs_dec();
        }

        // prime >= 1
        while (true)
        {
            // faster than prime += 2
            prime.abs_inc();
            prime.abs_inc();

            bool is_prime = true;
            for (int i = 2; i * i <= prime; i++)
            {
                if (prime % i == 0)
                {
                    is_prime = false;
                    break; // prime is not prime, continue while loop
                }
            }
            if (is_prime)
            {
                break; // prime is prime, return
            }
        }

        return prime;
    }

    /// Convert the integer object to some integer of type T.
    /// @tparam T an integer type : int, long, and any custom type that support basic arithmetic operations.
    template <typename T>
    T to_integer() const
    {
        T result = 0;
        for (int i = digits_.size() - 1; i >= 0; i--) // i = -1 if is zero, ok
        {
            result = result * 10 + digits_[i];
        }
        return result * sign_;
    }

    /*
     * Static
     */

    /// Return the square root of `integer`.
    static Int sqrt(const Int& integer)
    {
        if (integer.sign_ == -1)
        {
            throw std::runtime_error("Error: Cannot compute square root of a negative integer.");
        }

        if (integer.is_zero())
        {
            return 0;
        }
        else if (integer < 4)
        {
            return 1;
        }
        else if (integer < 9)
        {
            return 2;
        }
        else if (integer < 16) // can not be omitted
        {
            return 3;
        }

        // using Newton's method

        // as far as possible to reduce the number of iterations
        // cur_sqrt = 10^(digits/2 - 1) in O(1)
        Int cur_sqrt;
        cur_sqrt.digits_ = std::vector<signed char>(integer.digits() / 2 - 1, 0); // integer.digits() >= 2
        cur_sqrt.digits_.push_back(1);
        cur_sqrt.sign_ = 1;

        Int pre_sqrt = -1;

        while (cur_sqrt != pre_sqrt)
        {
            pre_sqrt = cur_sqrt;
            cur_sqrt = (cur_sqrt + integer / cur_sqrt) / 2;
        }

        return cur_sqrt;
    }

    /// Return `(base**exp) % mod` (`mod` default = 0 means does not perform module).
    static Int pow(const Int& base, const Int& exp, const Int& mod = 0)
    {
        // check if base.abs() is 1
        // if base.abs() is 1, only when base is negative and exp is odd return -1, otherwise return 1
        if (base.digits() == 1 && base.digits_[0] == 1)
        {
            return base.sign_ == -1 && exp.is_odd() ? -1 : 1;
        }

        // then, check if exp is negative
        if (exp.is_negative())
        {
            if (base.is_zero())
            {
                throw std::runtime_error("Error: Math domain error.");
            }

            return 0;
        }

        // fast power algorithm

        Int num = base;
        Int n = exp;
        Int result = 1; // this**0 == 1

        while (!n.is_zero())
        {
            if (n.is_odd())
            {
                result = (mod.is_zero() ? result * num : (result * num) % mod);
            }
            num = (mod.is_zero() ? num * num : (num * num) % mod);
            n /= 2; // integer divide
        }

        return result;
    }

    /// Return the logarithm of `integer` based on `base`.
    static Int log(const Int& integer, const Int& base)
    {
        if (integer.sign_ <= 0 || base < 2)
        {
            throw std::runtime_error("Error: Math domain error.");
        }

        if (base == 10)
        {
            return integer.digits() - 1;
        }

        Int result;
        Int value = integer / base;
        while (!value.is_zero())
        {
            ++result;
            value /= base;
        }

        return result;
    }

    /// Calculate the greatest common divisor of two integers.
    static Int gcd(const Int& int1, const Int& int2)
    {
        // using Euclidean algorithm

        Int a = int1;
        Int b = int2;

        while (!b.is_zero()) // a, b = b, a % b until b == 0
        {
            auto t = b;
            b = a % b;
            a = t;
        }

        return a; // a is GCD
    }

    /// Calculate the least common multiple of two integers.
    static Int lcm(const Int& int1, const Int& int2)
    {
        if (int1.is_zero() || int2.is_zero())
        {
            return 0;
        }

        return (int1 * int2) / gcd(int1, int2); // LCM = (int1 * int2) / GCD
    }

    /// Return a non-negative random integer (with a specific number of `digits`).
    ///
    /// Using hardware device to generate true random integer if possible.
    ///
    /// If `digits` is not specified, it will generate a random integer with digits in [0, 4300],
    /// since the default limit of Python's int is 4300 digits.
    static Int random(int digits = -1)
    {
        if (digits < -1)
        {
            throw std::runtime_error("Error: `digits` must be a non-negative integer or default = -1.");
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> digit(0, 9);

        // the default limit of Python's int is 4300 digits as provided in `sys.int_info.default_max_str_digits`
        // see: https://docs.python.org/3/library/stdtypes.html#integer-string-conversion-length-limitation
        std::uniform_int_distribution<int> digits_limit(0, 4300);

        Int randint;
        randint.digits_.resize(digits == -1 ? digits_limit(gen) : digits); // may be 0
        randint.sign_ = randint.digits_.empty() ? 0 : 1;

        for (auto& d : randint.digits_)
        {
            d = digit(gen);
        }

        // reset most significant digit if is 0
        if (!randint.digits_.empty() && randint.digits_.back() == 0)
        {
            std::uniform_int_distribution<int> most_digit(1, 9);
            randint.digits_.back() = most_digit(gen);
        }

        return randint;
    }

    /*
     * Print / Input
     */

    /// Output the integer to the specified output stream.
    friend std::ostream& operator<<(std::ostream& os, const Int& integer)
    {
        if (integer.sign_ == 0)
        {
            return os << '0';
        }

        if (integer.sign_ == -1)
        {
            os << '-';
        }

        std::for_each(integer.digits_.rbegin(), integer.digits_.rend(), [&](const auto& d)
                      { os << char(d + '0'); });

        return os;
    }

    /// Get an integer from the specified input stream.
    friend std::istream& operator>>(std::istream& is, Int& integer)
    {
        std::string str;
        is >> str;
        integer = str.c_str();

        return is;
    }

    friend struct std::hash<pyincpp::Int>;
};

} // namespace pyincpp

template <>
struct std::hash<pyincpp::Int> // explicit specialization
{
    std::size_t operator()(const pyincpp::Int& integer) const
    {
        std::size_t value = std::hash<signed char>{}(integer.sign_);

        for (const auto& d : integer.digits_)
        {
            value ^= std::hash<signed char>{}(d) << 1;
        }

        return value;
    }
};

#endif // INT_HPP
