/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef UTILS_MEMBER_MATCHER_H
#define UTILS_MEMBER_MATCHER_H

namespace kamd {
namespace utils {

namespace member_matcher {
    struct placeholder {} _;

    namespace detail { //_
        enum ComparisonOperation {
            Less,
            LessOrEqual,
            Equal,
            GreaterOrEqual,
            Greater
        };

        // Member comparison object
        // Call operator returns true if:
        //     collection item <comparison> specified item
        // where <comparison> can be <, >, ==, >=, <=
        template <typename Member, typename Value>
        struct member_comparator { //_

            member_comparator(ComparisonOperation comparison, Member member, Value value)
                : m_comparator(comparison)
                , m_member(member)
                , m_value(value)
            {
            }

            const ComparisonOperation m_comparator;
            const Member m_member;
            const Value m_value;

            // When passing only a item to compare with,
            // it means that we already have the value for comparison.
            // For example (member(M) > 5)(2)
            template <typename T>
            inline bool operator()(const T &collItem) const
            {
                return operator()(collItem, m_value);
            }

            // When passing the placeholder aka 'ignore' as a value,
            // it means that we already have the value for comparison.
            // For example (member(M) > 5)(collItem, _)
            template <typename T>
            inline bool operator()(const T &collItem,
                                   const placeholder &) const
            {
                return operator()(collItem, m_value);
            }

            // Like the previous one, but with reversed argument order
            template <typename T>
            inline bool operator()(const placeholder &,
                                   const T &collItem) const
            {
                return compare(m_value, (collItem.*m_member)());
            }

            // Comparing two values
            // For example (member(M) > _)(item, 5)
            template <typename T, typename V>
            inline bool operator()(const T &collItem, const V &value) const
            {
                // TODO: Make this work if the arguments are reversed,
                //       or even if both arhuments need to be checked
                //       for the specified member
                return compare((collItem.*m_member)(), value);
            }


        private:
            template <typename Left, typename Right>
            inline bool compare(const Left &left, const Right &right) const
            {
                return
                    m_comparator == Less           ? left <  right :
                    m_comparator == LessOrEqual    ? left <= right :
                    m_comparator == Equal          ? left == right :
                    m_comparator == GreaterOrEqual ? left >= right :
                    m_comparator == Greater        ? left >  right :
                                                     false;
            }

        }; //^

        // Chaining multiple comparators to achieve lexicographical
        // comparison of multiple members in order.
        // This would me so much nicer with variadic templates... f**ing MSVC.
        template <typename First, typename Second>
        struct member_comparator_chain {
            member_comparator_chain(First first, Second second)
                : first(first)
                , second(second)
            {
            }

            // Implement if needed...
            // template <typename T>
            // inline bool operator()(const T &item) const
            // {
            //     return first(item) || second(item);
            // }

            template <typename T, typename V>
            inline bool operator()(const T &item, const V &value) const
            {
                return first(item, value) ||
                       (!first(value, item) && second(item, value));
            }

            First first;
            Second second;
        };

        template <typename First, typename Second>
        inline member_comparator_chain<First, Second> operator&&(First first,
                                                                 Second second)
        {
            return member_comparator_chain<First, Second>(first, second);
        }

        // Provides syntax sugar for building member comparators
        template <typename Member>
        struct member_matcher { //_
            member_matcher(Member m)
                : m_member(m)
            {
            }

            #define IMPLEMENT_COMPARISON_OPERATOR(OPERATOR, NAME)                  \
                template <typename Value>                                          \
                inline member_comparator<Member, Value>                            \
                        operator OPERATOR (const Value &value) const               \
                {                                                                  \
                    return member_comparator<Member, Value>(NAME, m_member, value);\
                }

            IMPLEMENT_COMPARISON_OPERATOR(<  , Less)
            IMPLEMENT_COMPARISON_OPERATOR(<= , LessOrEqual)
            IMPLEMENT_COMPARISON_OPERATOR(== , Equal)
            IMPLEMENT_COMPARISON_OPERATOR(>= , GreaterOrEqual)
            IMPLEMENT_COMPARISON_OPERATOR(>  , Greater)

            #undef IMPLEMENT_COMPARISON_OPERATOR

            Member m_member;
        }; //^

    } //^ namespace detail

    template <typename Member>
    detail::member_matcher<Member> member(Member m)
    {
        return detail::member_matcher<Member>(m);
    }
} // namespace member


} // namespace utils
} // namespace kamd

#endif // UTILS_MEMBER_MATCHER_H
