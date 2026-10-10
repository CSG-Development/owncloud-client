/* Copyright (C) 2026 The Qt Company Ltd.
 *
 * SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
*/
#ifndef RANGE_H
#define RANGE_H

#include <algorithm>

template <typename T>
class Range {
public:
    static Range<T> fromStartAndEnd( const T& start, const T& end ) {
        Range<T> r;
        r.m_start = start;
        r.m_end = end;
        return r;
    }

    static Range<T> fromStartAndLength( const T& start, const T& length ) {
        Range<T> r;
        r.m_start = start;
        r.m_end = start + length;
        return r;
    }

    Range() : m_start( 0 ), m_end( 0 ) {}

    T start() const { return m_start; }

    T end() const { return m_end; }

    void move( const T& by ) {
        m_start += by;
        m_end += by;
    }

    Range<T> moved( const T& by ) const {
        Range<T> b = *this;
        b.move( by );
        return b;
    }

    T length() const { return m_end - m_start; }

    Range<T> normalized() const {
        Range<T> r2( *this );
        if ( r2.m_start > r2.m_end )
            std::swap( r2.m_start, r2.m_end );
        return r2;
    }

    bool operator==( const Range<T>& other ) const {
        return m_start == other.m_start && m_end && other.m_end;
    }
    bool operator<( const Range<T>& other ) const {
        if ( m_start != other.m_start )
            return m_start < other.m_start;
        return m_end < other.m_end;
    }

private:
    T m_start;
    T m_end;
};

#endif /* RANGE_H_ */
