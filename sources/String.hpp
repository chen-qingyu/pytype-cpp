/**
 * @file String.hpp
 * @author 青羽 (chen_qingyu@qq.com, https://chen-qingyu.github.io/)
 * @brief String class.
 * @date 2023.01.08
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

#ifndef STRING_HPP
#define STRING_HPP

#include "utility.hpp"

#include "Integer.hpp"
#include "List.hpp"

namespace pyincpp
{

/// String class.
class String
{
private:
    // String.
    const std::string str_;

    // Used for FSM.
    enum state
    {
        S_BEGIN_BLANK = 1 << 0,        // begin blank character
        S_SIGN = 1 << 1,               // positive or negative sign
        S_INT_PART = 1 << 2,           // integer part
        S_DEC_POINT_HAS_LEFT = 1 << 3, // decimal point has left digit
        S_DEC_POINT_NOT_LEFT = 1 << 4, // decimal point doesn't have left digit
        S_DEC_PART = 1 << 5,           // decimal part
        S_EXP = 1 << 6,                // scientific notation identifier
        S_EXP_SIGN = 1 << 7,           // positive or negative sign of exponent part
        S_EXP_PART = 1 << 8,           // exponent part
        S_END_BLANK = 1 << 9,          // end blank character
        S_OTHER = 1 << 10,             // other
    };

    // Used for FSM.
    enum event
    {
        E_BLANK = 1 << 11,     // blank character: '\n', '\r', '\t', ' '
        E_SIGN = 1 << 12,      // positive or negative sign: '+', '-'
        E_NUMBER = 1 << 13,    // number: '[0-9a-zA-Z]'
        E_DEC_POINT = 1 << 14, // decimal point: '.'
        E_EXP = 1 << 15,       // scientific notation identifier: 'e', 'E'
        E_OTHER = 1 << 16,     // other
    };

    // Try to transform a character to an event.
    static event get_event(const char ch, const int base)
    {
        if (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t')
        {
            return E_BLANK;
        }
        else if (char_to_integer(ch, base) != -1)
        {
            return E_NUMBER;
        }
        else if (ch == '-' || ch == '+')
        {
            return E_SIGN;
        }
        else if (ch == '.')
        {
            return E_DEC_POINT;
        }
        else if (ch == 'e' || ch == 'E')
        {
            return E_EXP;
        }
        return E_OTHER;
    }

    // Try to transform a character to an integer based on 2-36 base.
    static int char_to_integer(char digit, int base) // 2 <= base <= 36
    {
        static const char* upper_digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        static const char* lower_digits = "0123456789abcdefghijklmnopqrstuvwxyz";
        for (int i = 0; i < base; ++i)
        {
            if (digit == upper_digits[i] || digit == lower_digits[i])
            {
                return i;
            }
        }
        return -1; // not an integer
    }

    // Format helper, see https://codereview.stackexchange.com/questions/269425/implementing-stdformat
    template <typename T>
    static void format_helper(std::ostringstream& oss, std::string_view& str, const T& value)
    {
        std::size_t open_bracket = str.find('{');
        std::size_t close_bracket = str.find('}', open_bracket + 1);
        if (open_bracket == std::string::npos || close_bracket == std::string::npos)
        {
            return;
        }
        oss << str.substr(0, open_bracket) << value;
        str = str.substr(close_bracket + 1);
    }

    // Create a string from std::string.
    String(std::string&& string)
        : str_(string)
    {
    }

public:
    /*
     * Constructor / Destructor
     */

    /// Default constructor. Construct an empty deque.
    String() = default;

    /// Create a string from null-terminated characters.
    String(const char* chars)
        : str_(chars)
    {
    }

    /// Copy constructor.
    String(const String& that) = default;

    /// Move constructor.
    String(String&& that)
        : str_(std::move(const_cast<std::string&>(that.str_)))
    {
    }

    /*
     * Comparison
     */

    /// Compare the string with another string.
    auto operator<=>(const String& that) const = default;

    /*
     * Assignment
     */

    /// Copy assignment operator.
    String& operator=(const String& that)
    {
        if (this != &that)
        {
            const_cast<std::string&>(str_) = that.str_;
        }

        return *this;
    }

    /// Move assignment operator.
    String& operator=(String&& that)
    {
        if (this != &that)
        {
            const_cast<std::string&>(str_) = std::move(const_cast<std::string&>(that.str_));
        }

        return *this;
    }

    /*
     * Iterator
     */

    /// Return an iterator to the first char of the string.
    auto begin() const
    {
        return str_.begin();
    }

    /// Return an iterator to the char following the last char of the string.
    auto end() const
    {
        return str_.end();
    }

    /*
     * Access
     */

    /// Return the const reference to element at the specified position in the string.
    /// Index can be negative, like Python's string: string[-1] gets the last element.
    const char& operator[](int index) const
    {
        internal::check_bounds(index, -size(), size());

        return str_[index >= 0 ? index : index + size()];
    }

    /*
     * Examination
     */

    /// Return the number of elements in the string.
    int size() const
    {
        return str_.size(); // no '\0'
    }

    /// Return true if the string contains no elements.
    bool is_empty() const
    {
        return str_.empty();
    }

    /// Return the index of the first occurrence of the specified pattern in the specified range [`start`, `stop`).
    /// Or -1 if the string does not contain the pattern (in the specified range).
    int find(const String& pattern, int start = 0, int stop = INT_MAX) const
    {
        if (start > size())
        {
            return -1;
        }

        stop = stop > str_.size() ? str_.size() : stop;
        std::string_view view(begin() + start, begin() + stop);
        auto pos = view.find(pattern.str_.data());

        return pos == std::string::npos ? -1 : start + int(pos);
    }

    /// Return `true` if the string contains the specified `pattern` in the specified range [`start`, `stop`).
    bool contains(const String& pattern, int start = 0, int stop = INT_MAX) const
    {
        return find(pattern, start, stop) != -1;
    }

    /// Count the total number of occurrences of the specified element in the string.
    int count(const char& element) const
    {
        return std::count(begin(), end(), element);
    }

    /// Convert the string to a double-precision floating-point decimal number.
    ///
    /// If the string is too big to be representable will return `HUGE_VAL`.
    /// If the string represents NaN will return `NAN`.
    /// If the string represents Infinity will return `(+-)INFINITY`.
    ///
    /// ### Example:
    /// ```
    /// String("233.33").to_decimal(); // 233.33
    /// String("123.456e-3").to_decimal(); // 0.123456
    /// String("1e+600").to_decimal(); // HUGE_VAL
    /// String("nan").to_decimal(); // NAN
    /// String("inf").to_decimal(); // INFINITY
    /// ```
    double to_decimal() const
    {
        // check infinity or nan
        static const char* pos_infs[12] = {"inf", "INF", "Inf", "+inf", "+INF", "+Inf", "infinity", "INFINITY", "Infinity", "+infinity", "+INFINITY", "+Infinity"};
        static const char* neg_infs[6] = {"-inf", "-INF", "-Inf", "-infinity", "-INFINITY", "-Infinity"};
        static const char* nans[9] = {"nan", "NaN", "NAN", "+nan", "+NaN", "+NAN", "-nan", "-NaN", "-NAN"};

        for (int i = 0; i < 12; ++i)
        {
            if (*this == pos_infs[i])
            {
                return INFINITY;
            }
        }
        for (int i = 0; i < 6; ++i)
        {
            if (*this == neg_infs[i])
            {
                return -INFINITY;
            }
        }
        for (int i = 0; i < 9; ++i)
        {
            if (*this == nans[i])
            {
                return NAN;
            }
        }

        // not infinity or nan

        double sign = 1; // default '+'
        double decimal_part = 0;
        int decimal_cnt = 0;
        double exp_sign = 1; // default '+'
        int exp_part = 0;

        // FSM
        state st = S_BEGIN_BLANK;
        for (int i = 0; i < str_.size(); ++i)
        {
            event ev = get_event(str_[i], 10);
            switch (st | ev)
            {
                case S_BEGIN_BLANK | E_BLANK:
                    st = S_BEGIN_BLANK;
                    break;

                case S_BEGIN_BLANK | E_SIGN:
                    sign = (str_[i] == '+') ? 1 : -1;
                    st = S_SIGN;
                    break;

                case S_BEGIN_BLANK | E_DEC_POINT:
                case S_SIGN | E_DEC_POINT:
                    st = S_DEC_POINT_NOT_LEFT;
                    break;

                case S_BEGIN_BLANK | E_NUMBER:
                case S_SIGN | E_NUMBER:
                case S_INT_PART | E_NUMBER:
                    decimal_part = decimal_part * 10 + char_to_integer(str_[i], 10);
                    st = S_INT_PART;
                    break;

                case S_INT_PART | E_DEC_POINT:
                    st = S_DEC_POINT_HAS_LEFT;
                    break;

                case S_DEC_POINT_NOT_LEFT | E_NUMBER:
                case S_DEC_PART | E_NUMBER:
                case S_DEC_POINT_HAS_LEFT | E_NUMBER:
                    decimal_part = decimal_part * 10 + char_to_integer(str_[i], 10);
                    decimal_cnt++;
                    st = S_DEC_PART;
                    break;

                case S_INT_PART | E_EXP:
                case S_DEC_POINT_HAS_LEFT | E_EXP:
                case S_DEC_PART | E_EXP:
                    st = S_EXP;
                    break;

                case S_EXP | E_SIGN:
                    exp_sign = (str_[i] == '+') ? 1 : -1;
                    st = S_EXP_SIGN;
                    break;

                case S_EXP | E_NUMBER:
                case S_EXP_SIGN | E_NUMBER:
                case S_EXP_PART | E_NUMBER:
                    exp_part = exp_part * 10 + char_to_integer(str_[i], 10);
                    st = S_EXP_PART;
                    break;

                case S_INT_PART | E_BLANK:
                case S_DEC_POINT_HAS_LEFT | E_BLANK:
                case S_DEC_PART | E_BLANK:
                case S_EXP_PART | E_BLANK:
                case S_END_BLANK | E_BLANK:
                    st = S_END_BLANK;
                    break;

                default:
                    st = S_OTHER;
                    i = str_.size(); // exit loop
                    break;
            }
        }
        if (st != S_INT_PART && st != S_DEC_POINT_HAS_LEFT && st != S_DEC_PART && st != S_EXP_PART && st != S_END_BLANK)
        {
            throw std::runtime_error("Error: Invalid literal for to_decimal().");
        }

        return sign * ((decimal_part / std::pow(10, decimal_cnt)) * std::pow(10, exp_sign * exp_part));
    }

    /// Convert the string to an integer object based on 2-36 `base`.
    ///
    /// Numeric character in 36 base: 0, 1, ..., 9, A(10), ..., F(15), G(16), ..., Y(34), Z(35).
    ///
    /// ### Example:
    /// ```
    /// String("233").to_integer(); // 233
    /// String("cafebabe").to_integer(16); // 3405691582
    /// String("z").to_integer(36); // 35
    /// ```
    Integer to_integer(int base = 10) const
    {
        // check base
        if (base < 2 || base > 36)
        {
            throw std::runtime_error("Error: Invalid base for to_integer().");
        }

        bool non_negative = true; // default '+'
        Integer integer_part;

        // FSM
        state st = S_BEGIN_BLANK;
        for (int i = 0; i < str_.size(); ++i)
        {
            event ev = get_event(str_[i], base);
            switch (st | ev)
            {
                case S_BEGIN_BLANK | E_BLANK:
                    st = S_BEGIN_BLANK;
                    break;

                case S_BEGIN_BLANK | E_SIGN:
                    non_negative = (str_[i] == '+') ? true : false;
                    st = S_SIGN;
                    break;

                case S_BEGIN_BLANK | E_NUMBER:
                case S_SIGN | E_NUMBER:
                case S_INT_PART | E_NUMBER:
                    integer_part = integer_part * base + char_to_integer(str_[i], base);
                    st = S_INT_PART;
                    break;

                case S_INT_PART | E_BLANK:
                case S_END_BLANK | E_BLANK:
                    st = S_END_BLANK;
                    break;

                default:
                    st = S_OTHER;
                    i = str_.size(); // exit loop
                    break;
            }
        }
        if (st != S_INT_PART && st != S_END_BLANK)
        {
            throw std::runtime_error("Error: Invalid literal for to_integer().");
        }

        return non_negative ? integer_part : -integer_part;
    }

    /// Return `true` if the string begins with the specified string, otherwise return `false`.
    bool starts_with(const String& str) const
    {
        return str_.starts_with(str.str_);
    }

    /// Return `true` if the string ends with the specified string, otherwise return `false`.
    bool ends_with(const String& str) const
    {
        return str_.ends_with(str.str_);
    }

    /*
     * Production
     */

    /// Shift the string to right `n` characters.
    String operator>>(int n) const
    {
        if (size() <= 1 || n == 0)
        {
            return *this;
        }

        if (n < 0)
        {
            return *this << -n;
        }

        return *this << size() - n;
    }

    /// Shift the string to left n characters.
    String operator<<(int n) const
    {
        if (size() <= 1 || n == 0)
        {
            return *this;
        }

        n %= size();

        if (n < 0)
        {
            return *this >> -n;
        }

        std::string buffer(size(), 0);
        std::rotate_copy(begin(), begin() + n, end(), buffer.begin());

        return buffer;
    }

    /// Rerurn the reversed string.
    String reverse() const
    {
        std::string buffer = str_;
        std::reverse(buffer.begin(), buffer.end());

        return buffer;
    }

    /// Return a copy of the string with all the characters converted to lowercase.
    String lower() const
    {
        std::string buffer = str_;
        std::for_each(buffer.begin(), buffer.end(), [](char& c)
                      { c = (c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c); });

        return buffer;
    }

    /// Return a copy of the string with all the characters converted to uppercase.
    String upper() const
    {
        std::string buffer = str_;
        std::for_each(buffer.begin(), buffer.end(), [](char& c)
                      { c = (c >= 'a' && c <= 'z' ? c - ('a' - 'A') : c); });

        return buffer;
    }

    /// Return a copy of the string and erase the contents of the string in the range [`start`, `stop`).
    String erase(int start, int stop) const
    {
        internal::check_bounds(start, 0, size() + 1);
        internal::check_bounds(stop, 0, size() + 1);

        std::string buffer = str_;
        buffer.erase(buffer.begin() + start, buffer.begin() + stop);

        return buffer;
    }

    /// Replace the string.
    String replace(const String& old_str, const String& new_str) const
    {
        std::string buffer;

        int this_start = 0;
        for (int patt_start = 0; (patt_start = find(old_str, this_start)) != -1; this_start = patt_start + old_str.size())
        {
            buffer += slice(this_start, patt_start).str_;
            buffer += new_str.str_;
        }
        if (this_start != size())
        {
            buffer += slice(this_start, size()).str_;
        }

        return buffer;
    }

    /// Remove leading and trailing characters (default is blank character) of the string.
    String strip(const signed char& ch = -1) const
    {
        std::string buffer = str_;

        int i = 0;
        while (i < buffer.size() && (ch == -1 ? buffer[i] <= 0x20 : buffer[i] == ch))
        {
            ++i;
        }
        buffer.erase(buffer.begin(), buffer.begin() + i);

        i = buffer.size() - 1;
        while (i >= 0 && (ch == -1 ? buffer[i] <= 0x20 : buffer[i] == ch))
        {
            --i;
        }
        buffer.erase(buffer.begin() + i + 1, buffer.end());

        return buffer;
    }

    /// Return slice of the string from `start` to `stop` with certain `step`.
    /// Index and step length can be negative.
    String slice(int start, int stop, int step = 1) const
    {
        if (step == 0)
        {
            throw std::runtime_error("Error: Slice step can not be zero.");
        }

        internal::check_bounds(start, -size(), size());
        internal::check_bounds(stop, -size() - 1, size() + 1);

        // convert
        start = start < 0 ? start + size() : start;
        stop = stop < 0 ? stop + size() : stop;

        // copy
        std::string buffer;
        for (int i = start; (step > 0) ? (i < stop) : (i > stop); i += step)
        {
            buffer += str_[i];
        }

        return buffer;
    }

    /// Generate a new string and append the specified `element` to the end of the string.
    String operator+(const char& element) const
    {
        return str_ + element;
    }

    /// Generate a new string and append the specified `string` to the end of the string.
    String operator+(const String& string) const
    {
        return str_ + string.str_;
    }

    /// Generate a new string and add the string to itself a certain number of `times`.
    String operator*(int times) const
    {
        if (times < 0)
        {
            throw std::runtime_error("Error: Times to repeat can not be less than zero.");
        }

        std::string buffer(size() * times, 0);
        for (int part = 0; part < times; part++)
        {
            std::copy(begin(), end(), buffer.begin() + size() * part);
        }

        return buffer;
    }

    /// Split the string with separator (default = " ").
    ///
    /// ### Example:
    /// ```
    /// String("one, two, three").split(", "); // ["one", "two", "three"]
    /// ```
    List<String> split(const String& sep = " ") const
    {
        if (sep.size() == 0)
        {
            throw std::runtime_error("Error: Empty separator.");
        }

        List<String> str_list;
        int this_start = 0;
        for (int patt_start = 0; (patt_start = find(sep, this_start)) != -1; this_start = patt_start + sep.size())
        {
            str_list += slice(this_start, patt_start);
        }
        if (this_start != size())
        {
            str_list += slice(this_start, size());
        }

        return str_list;
    }

    /// Return a string which is the concatenation of the strings in `str_list`.
    ///
    /// ### Example:
    /// ```
    /// String(".").join({"192", "168", "0", "1"}) // "192.168.0.1"
    /// ```
    String join(const List<String>& str_list) const
    {
        if (str_list.size() == 0)
        {
            return String();
        }

        if (str_list.size() == 1)
        {
            return str_list[0];
        }

        std::string buffer;
        for (int i = 0; i < str_list.size() - 1; i++)
        {
            buffer += str_list[i].str_;
            buffer += str_;
        }
        buffer += str_list[str_list.size() - 1].str_;
        return buffer;
    }

    /// Format `args` according to the format string, and return the result as a string.
    template <typename... Args>
    String format(const Args&... args) const
    {
        std::ostringstream oss;
        std::string_view str(str_);
        (format_helper(oss, str, args), ...);
        oss << str;
        return oss.str().c_str();
    }

    /*
     * Print / Input
     */

    /// Output the string to the specified output stream.
    friend std::ostream& operator<<(std::ostream& os, const String& string)
    {
        os << "\"";
        for (int i = 0; i < string.size(); i++)
        {
            os << string.str_[i];
        }
        os << "\"";

        return os;
    }

    /// Get a line of string from the specified input stream.
    friend std::istream& operator>>(std::istream& is, String& string)
    {
        return std::getline(is, const_cast<std::string&>(string.str_));
    }
};

} // namespace pyincpp

#endif // STRING_HPP
