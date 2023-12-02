/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "prices.hpp"
#include "utilities.hpp"
#include <ql/prices.hpp>
#include <ql/time/date.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {
    namespace tt = boost::test_tools;

    [[maybe_unused]] void testMidEquivalent() {
        BOOST_TEST_MESSAGE("Testing midEquivalent()...");

        BOOST_TEST(1.5 == midEquivalent(1, 2, 3, 4), tt::tolerance(1e-14));
        BOOST_TEST(1.5 == midEquivalent(1, 2, 0, 4), tt::tolerance(1e-14));
        BOOST_TEST(1.5 == midEquivalent(1, 2, 3, 0), tt::tolerance(1e-14));
        BOOST_TEST(1.5 == midEquivalent(1, 2, 0, 4), tt::tolerance(1e-14));

        BOOST_TEST(1 == midEquivalent(1, 0, 3, 4));
        BOOST_TEST(1 == midEquivalent(1, 0, 0, 4));
        BOOST_TEST(1 == midEquivalent(1, 0, 3, 0));
        BOOST_TEST(1 == midEquivalent(1, 0, 0, 4));

        BOOST_TEST(2 == midEquivalent(0, 2, 3, 4));
        BOOST_TEST(2 == midEquivalent(0, 2, 0, 4));
        BOOST_TEST(2 == midEquivalent(0, 2, 3, 0));
        BOOST_TEST(2 == midEquivalent(0, 2, 0, 4));

        BOOST_TEST(3 == midEquivalent(0, 0, 3, 4));
        BOOST_TEST(4 == midEquivalent(0, 0, 0, 4));
        BOOST_TEST(3 == midEquivalent(0, 0, 3, 0));
        BOOST_TEST(4 == midEquivalent(0, 0, 0, 4));

        BOOST_CHECK_THROW(midEquivalent(0, 0, 0, 0), QuantLib::Error);
    }

    [[maybe_unused]] void testMidSafe() {
        BOOST_TEST_MESSAGE("Testing midSafe()...");

        BOOST_TEST(1.5 == midSafe(1, 2), tt::tolerance(1e-14));

        BOOST_CHECK_THROW(midSafe(0, 0), QuantLib::Error);
        BOOST_CHECK_THROW(midSafe(1, 0), QuantLib::Error);
        BOOST_CHECK_THROW(midSafe(0, 2), QuantLib::Error);
    }

    [[maybe_unused]] void testIntervalPriceInspectors() {
        BOOST_TEST_MESSAGE("Testing IntervalPrice::<Inspectors>()...");

        const IntervalPrice p(1, 2, 3, 4);

        BOOST_TEST(1 == p.open());
        BOOST_TEST(1 == p.value(IntervalPrice::Open));

        BOOST_TEST(2 == p.close());
        BOOST_TEST(2 == p.value(IntervalPrice::Close));

        BOOST_TEST(3 == p.high());
        BOOST_TEST(3 == p.value(IntervalPrice::High));

        BOOST_TEST(4 == p.low());
        BOOST_TEST(4 == p.value(IntervalPrice::Low));
    }


    void testEquality(const IntervalPrice& lhs, const IntervalPrice& rhs) {
        using T = IntervalPrice::Type;
        for (const auto t : {T::Open, T::Close, T::High, T::Low})
            BOOST_TEST(lhs.value(t) == rhs.value(t));
    }

    [[maybe_unused]] void testIntervalPriceModifiers() {
        BOOST_TEST_MESSAGE("Testing IntervalPrice::<Modifiers>()...");

        IntervalPrice p(1, 2, 3, 4);

        p.setValue(11, IntervalPrice::Open);
        testEquality(p, IntervalPrice(11, 2, 3, 4));

        p.setValue(12, IntervalPrice::Close);
        testEquality(p, IntervalPrice(11, 12, 3, 4));

        p.setValue(13, IntervalPrice::High);
        testEquality(p, IntervalPrice(11, 12, 13, 4));

        p.setValue(14, IntervalPrice::Low);
        testEquality(p, IntervalPrice(11, 12, 13, 14));

        p.setValues(21, 22, 23, 24);
        testEquality(p, IntervalPrice(21, 22, 23, 24));
    }

    TimeSeries<IntervalPrice> createSeries() {
        std::vector<Date> d;
        d.emplace_back((Day)1, (Month)1, (Year)2001);
        d.emplace_back((Day)3, (Month)3, (Year)2003);
        d.emplace_back((Day)2, (Month)2, (Year)2002);

        const std::vector<Real> open{11, 13, 12};
        const std::vector<Real> close{21, 23, 22};
        const std::vector<Real> high{31, 33, 32};
        const std::vector<Real> low{41, 43, 42};

        return IntervalPrice::makeSeries(d, open, close, high, low);
    }

    [[maybe_unused]] void testIntervalPriceMakeSeries() {
        BOOST_TEST_MESSAGE("Testing IntervalPrice::makeSeries()...");

        const TimeSeries<IntervalPrice> priceSeries = createSeries();

        BOOST_TEST(3u == priceSeries.size());
        testEquality(priceSeries[{(Day)1, (Month)1, (Year)2001}], {11, 21, 31, 41});
        testEquality(priceSeries[{(Day)2, (Month)2, (Year)2002}], {12, 22, 32, 42});
        testEquality(priceSeries[{(Day)3, (Month)3, (Year)2003}], {13, 23, 33, 43});
    }
} // namespace

test_suite* priceTestSuite() {
    auto* suite = BOOST_TEST_SUITE("Prices tests");
    suite->add(QUANTLIB_TEST_CASE(&testMidEquivalent));
    suite->add(QUANTLIB_TEST_CASE(&testMidSafe));
    suite->add(QUANTLIB_TEST_CASE(&testIntervalPriceInspectors));
    suite->add(QUANTLIB_TEST_CASE(&testIntervalPriceModifiers));
    suite->add(QUANTLIB_TEST_CASE(&testIntervalPriceMakeSeries));
    return suite;
}
