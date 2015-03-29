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

namespace detail { //_
    enum ComparisonOperation {
        Less,
        LessOrEqual,
        Equal,
        GreaterOrEqual,
        Greater
    };

    template <typename Member, typename Value>
    struct member_comparator {

        member_comparator(ComparisonOperation comparison, Member member, Value value)
            : m_comparator(comparison)
            , m_member(member)
            , m_value(value)
        {
        }

        const ComparisonOperation m_comparator;
        const Member m_member;
        const Value m_value;

        template <typename T>
        inline bool operator()(const T &item) const
        {
            return
                m_comparator == Less           ? (item.*m_member)() <  m_value :
                m_comparator == LessOrEqual    ? (item.*m_member)() <= m_value :
                m_comparator == Equal          ? (item.*m_member)() == m_value :
                m_comparator == GreaterOrEqual ? (item.*m_member)() >= m_value :
                m_comparator == Greater        ? (item.*m_member)() >  m_value :
                                                                     false;
        }

        template <typename T, typename V>
        inline bool operator()(const T &item, const V &value) const
        {
            return
                m_comparator == Less           ? (item.*m_member)() <  value :
                m_comparator == LessOrEqual    ? (item.*m_member)() <= value :
                m_comparator == Equal          ? (item.*m_member)() == value :
                m_comparator == GreaterOrEqual ? (item.*m_member)() >= value :
                m_comparator == Greater        ? (item.*m_member)() >  value :
                                                                     false;
        }


    };

    template <typename Member>
    struct member_matcher {
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
    };
} //^ namespace detail

namespace member_matcher {
    struct placeholder {} _;

    template <typename Member>
    detail::member_matcher<Member> member(Member m)
    {
        return detail::member_matcher<Member>(m);
    }
} // namespace member


} // namespace utils
} // namespace kamd

#endif // UTILS_MEMBER_MATCHER_H
