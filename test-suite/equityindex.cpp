/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 Copyright (C) 2023 Marcin Rybacki

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

#include "equityindex.hpp"
#include "utilities.hpp"
#include <ql/indexes/equityindex.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/currencies/europe.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace equityindex_test {

    struct CommonVars {

        Date today;
        Calendar calendar;
        DayCounter dayCount;
        Currency currency;

        ext::shared_ptr<EquityIndex> equityIndex;
        RelinkableHandle<YieldTermStructure> interestHandle;
        RelinkableHandle<YieldTermStructure> dividendHandle;

        // cleanup
        SavedSettings backup;
        // utilities

        CommonVars() {
            calendar = TARGET();
            dayCount = Actual365Fixed();
            currency = EURCurrency();

            equityIndex = ext::shared_ptr<EquityIndex>(
                new EquityIndex("eqIndex", currency, calendar, interestHandle, dividendHandle));

            equityIndex->addFixing(Date(10, February, 2021), 9450.0);
            equityIndex->addFixing(Date(31, January, 2023), 8690.0);

            today = calendar.adjust(Date(31, January, 2023));
            Settings::instance().evaluationDate() = today;

            interestHandle.linkTo(flatRate(today, 0.03, dayCount));
            dividendHandle.linkTo(flatRate(today, 0.01, dayCount));
        }
    };
}

void EquityIndexTest::testTodaysFixingForecast() {
    BOOST_TEST_MESSAGE("Testing todays fixing forecast...");

    using namespace equityindex_test;

    CommonVars vars;
    const Real tolerance = 1.0e-8;

    Real forecastedFixing = vars.equityIndex->fixing(vars.today, true);
    Real pastFixing = vars.equityIndex->pastFixing(vars.today);

    if ((std::fabs(pastFixing - forecastedFixing) > tolerance))
        BOOST_ERROR("forecasted fixing should be equal to past fixing\n"
                    << "    past fixing:    " << pastFixing << "\n"
                    << "    forecasted fixing:    " << forecastedFixing << "\n");

}

test_suite* EquityIndexTest::suite() {
    auto* suite = BOOST_TEST_SUITE("Equity index tests");

    suite->add(QUANTLIB_TEST_CASE(&EquityIndexTest::testTodaysFixingForecast));

    return suite;
}
