/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003, 2004, 2005, 2008 StatPro Italia srl
 Copyright (C) 2004 Ferdinando Ametrano
 Copyright (C) 2004 Neil Firth

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

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/instruments/basketoption.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/pricingengines/basket/bjerksundstenslandspreadengine.hpp>
#include <ql/pricingengines/basket/fd2dblackscholesvanillaengine.hpp>
#include <ql/pricingengines/basket/fdndimblackscholesvanillaengine.hpp>
#include <ql/pricingengines/basket/kirkengine.hpp>
#include <ql/pricingengines/basket/mcamericanbasketengine.hpp>
#include <ql/pricingengines/basket/mceuropeanbasketengine.hpp>
#include <ql/pricingengines/basket/operatorsplittingspreadengine.hpp>
#include <ql/pricingengines/basket/singlefactorbsmbasketengine.hpp>
#include <ql/pricingengines/basket/stulzengine.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/stochasticprocessarray.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/termstructures/volatility/equityfx/hestonblackvolsurface.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/daycounters/yearfractiontodate.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/math/statistics/incrementalstatistics.hpp>
#include "../ql/pricingengines/basket/denglizhoubasketengine.hpp"

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(BasketOptionTests)

#undef REPORT_FAILURE_2
#define REPORT_FAILURE_2(greekName, basketType, payoff, exercise, \
                         s1, s2, q1, q2, r, today, v1, v2, rho, \
                         expected, calculated, error, tolerance) \
    BOOST_ERROR( \
        exerciseTypeToString(exercise) << " " \
        << payoff->optionType() << " option on " \
        << basketTypeToString(basketType) \
        << " with " << payoffTypeToString(payoff) << " payoff:\n" \
        << "1st underlying value: " << s1 << "\n" \
        << "2nd underlying value: " << s2 << "\n" \
        << "              strike: " << payoff->strike() << "\n" \
        << "  1st dividend yield: " << io::rate(q1) << "\n" \
        << "  2nd dividend yield: " << io::rate(q2) << "\n" \
        << "      risk-free rate: " << io::rate(r) << "\n" \
        << "      reference date: " << today << "\n" \
        << "            maturity: " << exercise->lastDate() << "\n" \
        << "1st asset volatility: " << io::volatility(v1) << "\n" \
        << "2nd asset volatility: " << io::volatility(v2) << "\n" \
        << "         correlation: " << rho << "\n\n" \
        << "    expected   " << greekName << ": " << expected << "\n" \
        << "    calculated " << greekName << ": " << calculated << "\n"\
        << "    error:            " << error << "\n" \
        << "    tolerance:        " << tolerance);

#undef REPORT_FAILURE_3
#define REPORT_FAILURE_3(greekName, basketType, payoff, exercise, \
                         s1, s2, s3, r, today, v1, v2, v3, rho, \
                         expected, calculated, error, tolerance) \
    BOOST_ERROR( \
        exerciseTypeToString(exercise) << " " \
        << payoff->optionType() << " option on " \
        << basketTypeToString(basketType) \
        << " with " << payoffTypeToString(payoff) << " payoff:\n" \
        << "1st underlying value: " << s1 << "\n" \
        << "2nd underlying value: " << s2 << "\n" \
        << "3rd underlying value: " << s3 << "\n" \
        << "              strike: " << payoff->strike() <<"\n" \
        << "      risk-free rate: " << io::rate(r) << "\n" \
        << "      reference date: " << today << "\n" \
        << "            maturity: " << exercise->lastDate() << "\n" \
        << "1st asset volatility: " << io::volatility(v1) << "\n" \
        << "2nd asset volatility: " << io::volatility(v2) << "\n" \
        << "3rd asset volatility: " << io::volatility(v3) << "\n" \
        << "         correlation: " << rho << "\n\n" \
        << "    expected   " << greekName << ": " << expected << "\n" \
        << "    calculated " << greekName << ": " << calculated << "\n"\
        << "    error:            " << error << "\n" \
        << "    tolerance:        " << tolerance);


enum BasketType { MinBasket, MaxBasket, SpreadBasket };

std::string basketTypeToString(BasketType basketType) {
    switch (basketType) {
      case MinBasket:
        return "MinBasket";
      case MaxBasket:
        return "MaxBasket";
      case SpreadBasket:
        return "Spread";
    }
    QL_FAIL("unknown basket option type");
}

ext::shared_ptr<BasketPayoff> basketTypeToPayoff(BasketType basketType,
                                                 const ext::shared_ptr<Payoff> &p) {
    switch (basketType) {
      case MinBasket:
        return ext::shared_ptr<BasketPayoff>(new MinBasketPayoff(p));
      case MaxBasket:
        return ext::shared_ptr<BasketPayoff>(new MaxBasketPayoff(p));
      case SpreadBasket:
        return ext::shared_ptr<BasketPayoff>(new SpreadBasketPayoff(p));
    }
    QL_FAIL("unknown basket option type");
}

struct BasketOptionOneData {
    Option::Type type;
    Real strike;
    Real s;        // spot
    Rate q;        // dividend
    Rate r;        // risk-free rate
    Time t;        // time to maturity
    Volatility v;  // volatility
    Real result;   // expected result
    Real tol;      // tolerance
};

struct BasketOptionTwoData {
    BasketType basketType;
    Option::Type type;
    Real strike;
    Real s1;
    Real s2;
    Rate q1;
    Rate q2;
    Rate r;
    Time t; // years
    Volatility v1;
    Volatility v2;
    Real rho;
    Real result;
    Real tol;
};

struct BasketOptionThreeData {
    BasketType basketType;
    Option::Type type;
    Real strike;
    Real s1;
    Real s2;
    Real s3;
    Rate r;
    Time t; // months
    Volatility v1;
    Volatility v2;
    Volatility v3;
    Real rho;
    Real euroValue;
    Real amValue;
};

BasketOptionOneData oneDataValues[] = {
    //        type, strike,   spot,    q,    r,    t,  vol,   value, tol
    { Option::Put, 100.00,  80.00,   0.0, 0.06,   0.5, 0.4,  21.6059, 1e-2 },
    { Option::Put, 100.00,  85.00,   0.0, 0.06,   0.5, 0.4,  18.0374, 1e-2 },
    { Option::Put, 100.00,  90.00,   0.0, 0.06,   0.5, 0.4,  14.9187, 1e-2 },
    { Option::Put, 100.00,  95.00,   0.0, 0.06,   0.5, 0.4,  12.2314, 1e-2 },
    { Option::Put, 100.00, 100.00,   0.0, 0.06,   0.5, 0.4,  9.9458, 1e-2 },
    { Option::Put, 100.00, 105.00,   0.0, 0.06,   0.5, 0.4,  8.0281, 1e-2 },
    { Option::Put, 100.00, 110.00,   0.0, 0.06,   0.5, 0.4,  6.4352, 1e-2 },
    { Option::Put, 100.00, 115.00,   0.0, 0.06,   0.5, 0.4,  5.1265, 1e-2 },
    { Option::Put, 100.00, 120.00,   0.0, 0.06,   0.5, 0.4,  4.0611, 1e-2 },

    // Longstaff Schwartz 1D example
    // use constant and three Laguerre polynomials
    // 100,000 paths and 50 timesteps per year
    { Option::Put, 40.00, 36.00,   0.0, 0.06,   1.0, 0.2,  4.478, 1e-2 },
    { Option::Put, 40.00, 36.00,   0.0, 0.06,   2.0, 0.2,  4.840, 1e-2 },
    { Option::Put, 40.00, 36.00,   0.0, 0.06,   1.0, 0.4,  7.101, 1e-2 },
    { Option::Put, 40.00, 36.00,   0.0, 0.06,   2.0, 0.4,  8.508, 1e-2 },

    { Option::Put, 40.00, 38.00,   0.0, 0.06,   1.0, 0.2,  3.250, 1e-2 },
    { Option::Put, 40.00, 38.00,   0.0, 0.06,   2.0, 0.2,  3.745, 1e-2 },
    { Option::Put, 40.00, 38.00,   0.0, 0.06,   1.0, 0.4,  6.148, 1e-2 },
    { Option::Put, 40.00, 38.00,   0.0, 0.06,   2.0, 0.4,  7.670, 1e-2 },

    { Option::Put, 40.00, 40.00,   0.0, 0.06,   1.0, 0.2,  2.314, 1e-2 },
    { Option::Put, 40.00, 40.00,   0.0, 0.06,   2.0, 0.2,  2.885, 1e-2 },
    { Option::Put, 40.00, 40.00,   0.0, 0.06,   1.0, 0.4,  5.312, 1e-2 },
    { Option::Put, 40.00, 40.00,   0.0, 0.06,   2.0, 0.4,  6.920, 1e-2 },

    { Option::Put, 40.00, 42.00,   0.0, 0.06,   1.0, 0.2,  1.617, 1e-2 },
    { Option::Put, 40.00, 42.00,   0.0, 0.06,   2.0, 0.2,  2.212, 1e-2 },
    { Option::Put, 40.00, 42.00,   0.0, 0.06,   1.0, 0.4,  4.582, 1e-2 },
    { Option::Put, 40.00, 42.00,   0.0, 0.06,   2.0, 0.4,  6.248, 1e-2 },

    { Option::Put, 40.00, 44.00,   0.0, 0.06,   1.0, 0.2,  1.110, 1e-2 },
    { Option::Put, 40.00, 44.00,   0.0, 0.06,   2.0, 0.2,  1.690, 1e-2 },
    { Option::Put, 40.00, 44.00,   0.0, 0.06,   1.0, 0.4,  3.948, 1e-2 },
    { Option::Put, 40.00, 44.00,   0.0, 0.06,   2.0, 0.4,  5.647, 1e-2 }
};


BOOST_AUTO_TEST_CASE(testEuroTwoValues) {

    BOOST_TEST_MESSAGE("Testing two-asset European basket options...");

    /*
        Data from:
        Excel spreadsheet www.maths.ox.ac.uk/~firth/computing/excel.shtml
        and
        "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 pag 56-58
        European two asset max basket options
    */
    BasketOptionTwoData values[] = {
        //      basketType,   optionType, strike,    s1,    s2,   q1,   q2,    r,    t,   v1,   v2,  rho, result, tol
        // data from http://www.maths.ox.ac.uk/~firth/computing/excel.shtml
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90, 10.898, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70,  8.483, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50,  6.844, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30,  5.531, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10,  4.413, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.70, 0.00,  4.981, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.30, 0.00,  4.159, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.10, 0.00,  2.597, 1.0e-3},
        {MinBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.50, 0.10, 0.50,  4.030, 1.0e-3},

        {MaxBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90, 17.565, 1.0e-3},
        {MaxBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70, 19.980, 1.0e-3},
        {MaxBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50, 21.619, 1.0e-3},
        {MaxBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 22.932, 1.0e-3},
        {MaxBasket, Option::Call,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10, 24.049, 1.1e-3},
        {MaxBasket, Option::Call,  100.0,  80.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 16.508, 1.0e-3},
        {MaxBasket, Option::Call,  100.0,  80.0,  80.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30,  8.049, 1.0e-3},
        {MaxBasket, Option::Call,  100.0,  80.0, 120.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 30.141, 1.0e-3},
        {MaxBasket, Option::Call,  100.0, 120.0, 120.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 42.889, 1.0e-3},

        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90, 11.369, 1.0e-3},
        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70, 12.856, 1.0e-3},
        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50, 13.890, 1.0e-3},
        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30, 14.741, 1.0e-3},
        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10, 15.485, 1.0e-3},

        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 0.50, 0.30, 0.30, 0.10, 11.893, 1.0e-3},
        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 0.25, 0.30, 0.30, 0.10,  8.881, 1.0e-3},
        {MinBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 2.00, 0.30, 0.30, 0.10, 19.268, 1.0e-3},

        {MaxBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.90,  7.339, 1.0e-3},
        {MaxBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.70,  5.853, 1.0e-3},
        {MaxBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.50,  4.818, 1.0e-3},
        {MaxBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.30,  3.967, 1.1e-3},
        {MaxBasket,  Option::Put,  100.0, 100.0, 100.0, 0.00, 0.00, 0.05, 1.00, 0.30, 0.30, 0.10,  3.223, 1.0e-3},

        //      basketType,   optionType, strike,    s1,    s2,   q1,   q2,    r,    t,   v1,   v2,  rho,  result, tol
        // data from "Option pricing formulas" VB code + spreadsheet
        {MinBasket, Option::Call,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63,  4.8177, 1.0e-4},
        {MaxBasket, Option::Call,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63, 11.6323, 1.0e-4},
        {MinBasket,  Option::Put,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63,  2.0376, 1.0e-4},
        {MaxBasket,  Option::Put,   98.0, 100.0, 105.0, 0.00, 0.00, 0.05, 0.50, 0.11, 0.16, 0.63,  0.5731, 1.0e-4},
        {MinBasket, Option::Call,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  2.9340, 1.0e-4},
        {MinBasket,  Option::Put,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  3.5224, 1.0e-4},
        // data from "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 pag 58
        {MaxBasket, Option::Call,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  8.0701, 1.0e-4},
        {MaxBasket,  Option::Put,   98.0, 100.0, 105.0, 0.06, 0.09, 0.05, 0.50, 0.11, 0.16, 0.63,  1.2181, 1.0e-4},

        /* "Option pricing formulas", E.G. Haug, McGraw-Hill 1998 pag 59-60
            Kirk approx. for a european spread option on two futures*/

        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.20, 0.20, -0.5, 4.7530, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.20, 0.20,  0.0, 3.7970, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.20, 0.20,  0.5, 2.5537, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.25, 0.20, -0.5, 5.4275, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.25, 0.20,  0.0, 4.3712, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.25, 0.20,  0.5, 3.0086, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.20, 0.25, -0.5, 5.4061, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.20, 0.25,  0.0, 4.3451, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.1, 0.20, 0.25,  0.5, 2.9723, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.20, 0.20, -0.5,10.7517, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.20, 0.20,  0.0, 8.7020, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.20, 0.20,  0.5, 6.0257, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.25, 0.20, -0.5,12.1941, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.25, 0.20,  0.0, 9.9340, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.25, 0.20,  0.5, 7.0067, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.20, 0.25, -0.5,12.1483, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.20, 0.25,  0.0, 9.8780, 1.0e-3},
        {SpreadBasket, Option::Call, 3.0,  122.0, 120.0, 0.0, 0.0, 0.10,  0.5, 0.20, 0.25,  0.5, 6.9284, 1.0e-3}
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot1(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> spot2(new SimpleQuote(0.0));

    ext::shared_ptr<SimpleQuote> qRate1(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS1 = flatRate(today, qRate1, dc);
    ext::shared_ptr<SimpleQuote> qRate2(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS2 = flatRate(today, qRate2, dc);

    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    ext::shared_ptr<SimpleQuote> vol1(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS1 = flatVol(today, vol1, dc);
    ext::shared_ptr<SimpleQuote> vol2(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS2 = flatVol(today, vol2, dc);

    const Real mcRelativeErrorTolerance = 0.01;
    const Real fdRelativeErrorTolerance = 0.01;

    for (auto& value : values) {

        ext::shared_ptr<PlainVanillaPayoff> payoff(
            new PlainVanillaPayoff(value.type, value.strike));

        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));

        spot1->setValue(value.s1);
        spot2->setValue(value.s2);
        qRate1->setValue(value.q1);
        qRate2->setValue(value.q2);
        rRate->setValue(value.r);
        vol1->setValue(value.v1);
        vol2->setValue(value.v2);


        ext::shared_ptr<PricingEngine> analyticEngine;
        ext::shared_ptr<GeneralizedBlackScholesProcess> p1, p2;
        switch (value.basketType) {
            case MaxBasket:
            case MinBasket:
                p1 = ext::shared_ptr<GeneralizedBlackScholesProcess>(new BlackScholesMertonProcess(
                    Handle<Quote>(spot1), Handle<YieldTermStructure>(qTS1),
                    Handle<YieldTermStructure>(rTS), Handle<BlackVolTermStructure>(volTS1)));
                p2 = ext::shared_ptr<GeneralizedBlackScholesProcess>(new BlackScholesMertonProcess(
                    Handle<Quote>(spot2), Handle<YieldTermStructure>(qTS2),
                    Handle<YieldTermStructure>(rTS), Handle<BlackVolTermStructure>(volTS2)));
                analyticEngine = ext::shared_ptr<PricingEngine>(new StulzEngine(p1, p2, value.rho));
                break;
            case SpreadBasket:
                p1 = ext::shared_ptr<GeneralizedBlackScholesProcess>(
                    new BlackProcess(Handle<Quote>(spot1), Handle<YieldTermStructure>(rTS),
                                     Handle<BlackVolTermStructure>(volTS1)));
                p2 = ext::shared_ptr<GeneralizedBlackScholesProcess>(
                    new BlackProcess(Handle<Quote>(spot2), Handle<YieldTermStructure>(rTS),
                                     Handle<BlackVolTermStructure>(volTS2)));

                analyticEngine = ext::shared_ptr<PricingEngine>(
                    new KirkEngine(ext::dynamic_pointer_cast<BlackProcess>(p1),
                                   ext::dynamic_pointer_cast<BlackProcess>(p2), value.rho));
                break;
            default:
                QL_FAIL("unknown basket type");
        }

        std::vector<ext::shared_ptr<StochasticProcess1D> > procs = { p1, p2 };

        Matrix correlationMatrix(2, 2, value.rho);
        for (Integer j=0; j < 2; j++) {
            correlationMatrix[j][j] = 1.0;
        }

        ext::shared_ptr<StochasticProcessArray> process(
                         new StochasticProcessArray(procs,correlationMatrix));

        ext::shared_ptr<PricingEngine> mcEngine =
            MakeMCEuropeanBasketEngine<PseudoRandom, Statistics>(process)
            .withStepsPerYear(1)
            .withSamples(10000)
            .withSeed(42);

        ext::shared_ptr<PricingEngine> fdEngine(
            new Fd2dBlackScholesVanillaEngine(p1, p2, value.rho, 50, 50, 15));

        BasketOption basketOption(basketTypeToPayoff(value.basketType, payoff), exercise);

        // analytic engine
        basketOption.setPricingEngine(analyticEngine);
        Real calculated = basketOption.NPV();
        Real expected = value.result;
        Real error = std::fabs(calculated-expected);
        if (error > value.tol) {
            REPORT_FAILURE_2("value", value.basketType, payoff, exercise, value.s1, value.s2,
                             value.q1, value.q2, value.r, today, value.v1, value.v2, value.rho,
                             value.result, calculated, error, value.tol);
        }

        // fd engine
        basketOption.setPricingEngine(fdEngine);
        calculated = basketOption.NPV();
        Real relError = relativeError(calculated, expected, expected);
        if (relError > mcRelativeErrorTolerance ) {
            REPORT_FAILURE_2("FD value", value.basketType, payoff, exercise, value.s1, value.s2,
                             value.q1, value.q2, value.r, today, value.v1, value.v2, value.rho,
                             value.result, calculated, relError, fdRelativeErrorTolerance);
        }

        // mc engine
        basketOption.setPricingEngine(mcEngine);
        calculated = basketOption.NPV();
        relError = relativeError(calculated, expected, value.s1);
        if (relError > mcRelativeErrorTolerance ) {
            REPORT_FAILURE_2("MC value", value.basketType, payoff, exercise, value.s1, value.s2,
                             value.q1, value.q2, value.r, today, value.v1, value.v2, value.rho,
                             value.result, calculated, relError, mcRelativeErrorTolerance);
        }
    }
}

BOOST_AUTO_TEST_CASE(testBarraquandThreeValues, *precondition(if_speed(Slow))) {

    BOOST_TEST_MESSAGE("Testing three-asset basket options "
                       "against Barraquand's values...");

    /*
        Data from:
        "Numerical Valuation of High Dimensional American Securities"
        Barraquand, J. and Martineau, D.
        Journal of Financial and Quantitative Analysis 1995 3(30) 383-405
    */
    BasketOptionThreeData  values[] = {
        // time in months is with 30 days to the month..
        // basketType, optionType,       strike,    s1,    s2,   s3,    r,    t,   v1,   v2,  v3,  rho, euro, american,
        // Table 2
        // not using 4 month case to speed up test
/*
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 8.59, 8.59},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 3.84, 3.84},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 0.89, 0.89},
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 12.55, 12.55},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 7.87, 7.87},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 4.26, 4.26},
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 15.29, 15.29},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 10.72, 10.72},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 6.96, 6.96},
*/
/*
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 7.78, 7.78},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 3.18, 3.18},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 0.82, 0.82},
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 10.97, 10.97},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 6.69, 6.69},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 3.70, 3.70},
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 13.23, 13.23},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 9.11, 9.11},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 5.98, 5.98},
*/
/*
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 6.53, 6.53},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 2.38, 2.38},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 0.74, 0.74},
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 8.51, 8.51},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 4.92, 4.92},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 2.97, 2.97},
        {MaxBasket, Option::Call,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 10.04, 10.04},
        {MaxBasket, Option::Call,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 6.64, 6.64},
        {MaxBasket, Option::Call,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 4.61, 4.61},
*/
        // Table 3

        {MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 0.00, 0.00},
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 0.13, 0.23},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.0, 2.26, 5.00},
        //{MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 0.01, 0.01},
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 0.25, 0.44},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.0, 1.55, 5.00},
        //{MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 0.03, 0.04},
        //{MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 0.31, 0.57},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.0, 1.41, 5.00},

/*
        {MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 0.00, 0.00},
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 0.38, 0.48},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 0.5, 3.00, 5.00},
        {MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 0.07, 0.09},
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 0.72, 0.93},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 0.5, 2.65, 5.00},
        {MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 0.17, 0.20},
*/
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 0.91, 1.19},
/*
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 0.5, 2.63, 5.00},

        {MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 0.01, 0.01},
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 0.84, 0.08},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 1.00, 0.20, 0.30, 0.50, 1.0, 4.18, 5.00},
        {MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 0.19, 0.19},
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 1.51, 1.56},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 4.00, 0.20, 0.30, 0.50, 1.0, 4.49, 5.00},
        {MaxBasket, Option::Put,  35.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 0.41, 0.42},
        {MaxBasket, Option::Put,  40.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 1.87, 1.96},
        {MaxBasket, Option::Put,  45.0,  40.0,  40.0, 40.0, 0.05, 7.00, 0.20, 0.30, 0.50, 1.0, 4.70, 5.20}
*/
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot1(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> spot2(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> spot3(new SimpleQuote(0.0));

    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);

    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    ext::shared_ptr<SimpleQuote> vol1(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS1 = flatVol(today, vol1, dc);
    ext::shared_ptr<SimpleQuote> vol2(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS2 = flatVol(today, vol2, dc);
    ext::shared_ptr<SimpleQuote> vol3(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS3 = flatVol(today, vol3, dc);

    for (auto& value : values) {

        ext::shared_ptr<PlainVanillaPayoff> payoff(
            new PlainVanillaPayoff(value.type, value.strike));

        Date exDate = today + Integer(value.t) * 30;
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
        ext::shared_ptr<Exercise> amExercise(new AmericanExercise(today,
                                                                    exDate));

        spot1->setValue(value.s1);
        spot2->setValue(value.s2);
        spot3->setValue(value.s3);
        rRate->setValue(value.r);
        vol1->setValue(value.v1);
        vol2->setValue(value.v2);
        vol3->setValue(value.v3);

        ext::shared_ptr<StochasticProcess1D> stochProcess1(new
            BlackScholesMertonProcess(Handle<Quote>(spot1),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS1)));

        ext::shared_ptr<StochasticProcess1D> stochProcess2(new
            BlackScholesMertonProcess(Handle<Quote>(spot2),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS2)));

        ext::shared_ptr<StochasticProcess1D> stochProcess3(new
            BlackScholesMertonProcess(Handle<Quote>(spot3),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS3)));

        std::vector<ext::shared_ptr<StochasticProcess1D> > procs
            = {stochProcess1, stochProcess2, stochProcess3 };

        Matrix correlation(3, 3, value.rho);
        for (Integer j=0; j < 3; j++) {
            correlation[j][j] = 1.0;
        }

        ext::shared_ptr<StochasticProcessArray> process(
                               new StochasticProcessArray(procs,correlation));

        // use a 3D sobol sequence...
        // Think long and hard before moving to more than 1 timestep....
        ext::shared_ptr<PricingEngine> mcQuasiEngine =
            MakeMCEuropeanBasketEngine<LowDiscrepancy>(process)
            .withStepsPerYear(1)
            .withSamples(8091)
            .withSeed(42);

        BasketOption euroBasketOption(basketTypeToPayoff(value.basketType, payoff), exercise);
        euroBasketOption.setPricingEngine(mcQuasiEngine);

        Real expected = value.euroValue;
        Real calculated = euroBasketOption.NPV();
        Real relError = relativeError(calculated, expected, value.s1);
        Real mcRelativeErrorTolerance = 0.01;
        if (relError > mcRelativeErrorTolerance ) {
            REPORT_FAILURE_3("MC Quasi value", value.basketType, payoff, exercise, value.s1,
                             value.s2, value.s3, value.r, today, value.v1, value.v2, value.v3,
                             value.rho, value.euroValue, calculated, relError,
                             mcRelativeErrorTolerance);
        }


        Size requiredSamples = 1000;
        Size timeSteps = 500;
        BigNatural seed = 1;
        ext::shared_ptr<PricingEngine> mcLSMCEngine =
            MakeMCAmericanBasketEngine<>(process)
            .withSteps(timeSteps)
            .withAntitheticVariate()
            .withSamples(requiredSamples)
            .withCalibrationSamples(requiredSamples/4)
            .withSeed(seed);

        BasketOption amBasketOption(basketTypeToPayoff(value.basketType, payoff), amExercise);
        amBasketOption.setPricingEngine(mcLSMCEngine);

        expected = value.amValue;
        calculated = amBasketOption.NPV();
        relError = relativeError(calculated, expected, value.s1);
        Real mcAmericanRelativeErrorTolerance = 0.01;
        if (relError > mcAmericanRelativeErrorTolerance) {
            REPORT_FAILURE_3("MC LSMC Value", value.basketType, payoff, exercise, value.s1,
                             value.s2, value.s3, value.r, today, value.v1, value.v2, value.v3,
                             value.rho, value.amValue, calculated, relError,
                             mcRelativeErrorTolerance);
        }
    }
}

BOOST_AUTO_TEST_CASE(testTavellaValues) {

    BOOST_TEST_MESSAGE("Testing three-asset American basket options "
                       "against Tavella's values...");

    /*
        Data from:
        "Quantitative Methods in Derivatives Pricing"
        Tavella, D. A.   -   Wiley (2002)
    */
    BasketOptionThreeData  values[] = {
        // time in months is with 30 days to the month..
        // basketType, optionType,       strike,    s1,    s2,   s3,    r,    t,   v1,   v2,  v3,  rho, euroValue, american Value,
        {MaxBasket, Option::Call,  100,    100,   100, 100,  0.05, 3.00, 0.20, 0.20, 0.20, 0.0, -999, 18.082}
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot1(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> spot2(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> spot3(new SimpleQuote(0.0));

    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.1));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);

    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.05));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    ext::shared_ptr<SimpleQuote> vol1(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS1 = flatVol(today, vol1, dc);
    ext::shared_ptr<SimpleQuote> vol2(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS2 = flatVol(today, vol2, dc);
    ext::shared_ptr<SimpleQuote> vol3(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS3 = flatVol(today, vol3, dc);

    Real mcRelativeErrorTolerance = 0.01;
    Size requiredSamples = 10000;
    Size timeSteps = 20;
    BigNatural seed = 0;


    ext::shared_ptr<PlainVanillaPayoff> payoff(new
        PlainVanillaPayoff(values[0].type, values[0].strike));

    Date exDate = today + timeToDays(values[0].t);
    ext::shared_ptr<Exercise> exercise(new AmericanExercise(today, exDate));

    spot1 ->setValue(values[0].s1);
    spot2 ->setValue(values[0].s2);
    spot3 ->setValue(values[0].s3);
    vol1  ->setValue(values[0].v1);
    vol2  ->setValue(values[0].v2);
    vol3  ->setValue(values[0].v3);

    ext::shared_ptr<StochasticProcess1D> stochProcess1(new
        BlackScholesMertonProcess(Handle<Quote>(spot1),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS1)));

    ext::shared_ptr<StochasticProcess1D> stochProcess2(new
        BlackScholesMertonProcess(Handle<Quote>(spot2),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS2)));

    ext::shared_ptr<StochasticProcess1D> stochProcess3(new
        BlackScholesMertonProcess(Handle<Quote>(spot3),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS3)));

    std::vector<ext::shared_ptr<StochasticProcess1D> > procs = {stochProcess1,
                                                                stochProcess2,
                                                                stochProcess3};

    Matrix correlation(3,3, 0.0);
    for (Integer j=0; j < 3; j++) {
        correlation[j][j] = 1.0;
    }
    correlation[1][0] = -0.25;
    correlation[0][1] = -0.25;
    correlation[2][0] = 0.25;
    correlation[0][2] = 0.25;
    correlation[2][1] = 0.3;
    correlation[1][2] = 0.3;

    ext::shared_ptr<StochasticProcessArray> process(
                               new StochasticProcessArray(procs,correlation));
    ext::shared_ptr<PricingEngine> mcLSMCEngine =
        MakeMCAmericanBasketEngine<>(process)
        .withSteps(timeSteps)
        .withAntitheticVariate()
        .withSamples(requiredSamples)
        .withCalibrationSamples(requiredSamples/4)
        .withSeed(seed);

    BasketOption basketOption(basketTypeToPayoff(values[0].basketType,
                                                 payoff),
                              exercise);
    basketOption.setPricingEngine(mcLSMCEngine);

    Real calculated = basketOption.NPV();
    Real expected = values[0].amValue;
    Real errorEstimate = basketOption.errorEstimate();
    Real relError = relativeError(calculated, expected, values[0].s1);
    if (relError > mcRelativeErrorTolerance ) {
        REPORT_FAILURE_3("MC LSMC Tavella value", values[0].basketType,
                         payoff, exercise, values[0].s1, values[0].s2,
                         values[0].s3, values[0].r, today, values[0].v1,
                         values[0].v2, values[0].v3, values[0].rho,
                         values[0].amValue, calculated, errorEstimate,
                         mcRelativeErrorTolerance);
    }
}

BOOST_AUTO_TEST_SUITE(BasketOptionAmericanTest, *precondition(if_speed(Fast)))

struct sliceOne     { static const int from{0}, to{5}; };
struct sliceTwo     { static const int from{5}, to{11}; };
struct sliceThree   { static const int from{11}, to{17}; };
struct sliceFour    { static const int from{17}, to{23}; };
struct sliceFive    { static const int from{23}, to{29}; };

using slices = boost::mpl::vector<sliceOne, sliceTwo, sliceThree, sliceFour, sliceFive>;

BOOST_AUTO_TEST_CASE_TEMPLATE(testOneDAmericanValues, T, slices) {
    const int from = T::from;
    const int to = T::to;

    BOOST_TEST_MESSAGE("Testing basket American options against 1-D case "
                       "from " << from << " to " << to - 1 << "...");

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot1(new SimpleQuote(0.0));

    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);

    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.05));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    ext::shared_ptr<SimpleQuote> vol1(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS1 = flatVol(today, vol1, dc);

    Size requiredSamples = 10000;
    Size timeSteps = 52;
    BigNatural seed = 0;

    ext::shared_ptr<StochasticProcess1D> stochProcess1(new
        BlackScholesMertonProcess(Handle<Quote>(spot1),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS1)));

    std::vector<ext::shared_ptr<StochasticProcess1D>> procs = {stochProcess1};

    Matrix correlation(1, 1, 1.0);

    ext::shared_ptr<StochasticProcessArray> process(
        new StochasticProcessArray(procs, correlation));

    ext::shared_ptr<PricingEngine> mcLSMCEngine =
        MakeMCAmericanBasketEngine<>(process)
            .withSteps(timeSteps)
            .withAntitheticVariate()
            .withSamples(requiredSamples)
            .withCalibrationSamples(requiredSamples / 4)
            .withSeed(seed);

    for (Size i = from; i < to; i++) {
        ext::shared_ptr<PlainVanillaPayoff> payoff(
            new PlainVanillaPayoff(oneDataValues[i].type, oneDataValues[i].strike));

        Date exDate = today + timeToDays(oneDataValues[i].t);
        ext::shared_ptr<Exercise> exercise(new AmericanExercise(today, exDate));

        spot1 ->setValue(oneDataValues[i].s);
        vol1  ->setValue(oneDataValues[i].v);
        rRate ->setValue(oneDataValues[i].r);
        qRate ->setValue(oneDataValues[i].q);

        BasketOption basketOption( // process,
            basketTypeToPayoff(MaxBasket, payoff), exercise);
        basketOption.setPricingEngine(mcLSMCEngine);

        Real calculated = basketOption.NPV();
        Real expected = oneDataValues[i].result;
        // Real errorEstimate = basketOption.errorEstimate();
        Real relError = relativeError(calculated, expected, oneDataValues[i].s);
        // Real error = std::fabs(calculated-expected);

        if (relError > oneDataValues[i].tol) {
            BOOST_FAIL("expected value: " << oneDataValues[i].result << "\n"
                                          << "calculated:     " << calculated);
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

/* This unit test is a regression test to check for a crash in
   monte carlo if the required sample is odd.  The crash occurred
   because the samples array size was off by one when antithetic
   paths were added.
*/

BOOST_AUTO_TEST_CASE(testOddSamples) {

    BOOST_TEST_MESSAGE("Testing antithetic engine using odd sample number...");

    Size requiredSamples = 10001; // The important line
    Size timeSteps = 53;
    BasketOptionOneData values[] = {
        //        type, strike,   spot,    q,    r,    t,  vol,   value, tol
        { Option::Put, 100.00,  80.00,   0.0, 0.06,   0.5, 0.4,  21.6059, 1e-2 }
    };

    DayCounter dc = Actual360();
    Date today = Date::todaysDate();

    ext::shared_ptr<SimpleQuote> spot1(new SimpleQuote(0.0));

    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);

    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.05));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);

    ext::shared_ptr<SimpleQuote> vol1(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS1 = flatVol(today, vol1, dc);



    BigNatural seed = 0;

    ext::shared_ptr<StochasticProcess1D> stochProcess1(new
        BlackScholesMertonProcess(Handle<Quote>(spot1),
                                  Handle<YieldTermStructure>(qTS),
                                  Handle<YieldTermStructure>(rTS),
                                  Handle<BlackVolTermStructure>(volTS1)));

    std::vector<ext::shared_ptr<StochasticProcess1D> > procs = {stochProcess1};

    Matrix correlation(1, 1, 1.0);

    ext::shared_ptr<StochasticProcessArray> process(
                               new StochasticProcessArray(procs,correlation));

    ext::shared_ptr<PricingEngine> mcLSMCEngine =
        MakeMCAmericanBasketEngine<>(process)
        .withSteps(timeSteps)
        .withAntitheticVariate()
        .withSamples(requiredSamples)
        .withCalibrationSamples(requiredSamples/4)
        .withSeed(seed);

    for (auto& value : values) {
        ext::shared_ptr<PlainVanillaPayoff> payoff(
            new PlainVanillaPayoff(value.type, value.strike));

        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new AmericanExercise(today,
                                                                  exDate));

        spot1->setValue(value.s);
        vol1->setValue(value.v);
        rRate->setValue(value.r);
        qRate->setValue(value.q);

        BasketOption basketOption(// process,
                                  basketTypeToPayoff(MaxBasket, payoff),
                                  exercise);
        basketOption.setPricingEngine(mcLSMCEngine);

        Real calculated = basketOption.NPV();
        Real expected = value.result;
        // Real errorEstimate = basketOption.errorEstimate();
        Real relError = relativeError(calculated, expected, value.s);
        // Real error = std::fabs(calculated-expected);

        if (relError > value.tol) {
            BOOST_FAIL("expected value: " << value.result << "\n"
                                          << "calculated:     " << calculated);
        }
    }
}

BOOST_AUTO_TEST_CASE(testLocalVolatilitySpreadOption, *precondition(if_speed(Fast))) {

    BOOST_TEST_MESSAGE("Testing 2D local-volatility spread-option pricing...");

    const DayCounter dc = Actual360();
    const Date today = Date(21, September, 2017);
    const Date maturity = today + Period(3, Months);

    const Handle<YieldTermStructure> riskFreeRate(flatRate(today, 0.07, dc));
    const Handle<YieldTermStructure> dividendYield(flatRate(today, 0.03, dc));

    const Handle<Quote> s1(ext::make_shared<SimpleQuote>(100));
    const Handle<Quote> s2(ext::make_shared<SimpleQuote>(110));

    const ext::shared_ptr<HestonModel> hm1(
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                riskFreeRate, dividendYield,
                s1, 0.09, 1.0, 0.06, 0.6, -0.75)));

    const ext::shared_ptr<HestonModel> hm2(
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                riskFreeRate, dividendYield,
                s2, 0.1, 2.0, 0.07, 0.8, 0.85)));

    const Handle<BlackVolTermStructure> vol1(
        ext::make_shared<HestonBlackVolSurface>(Handle<HestonModel>(hm1)));

    const Handle<BlackVolTermStructure> vol2(
        ext::make_shared<HestonBlackVolSurface>(Handle<HestonModel>(hm2)));

    BasketOption basketOption(
        basketTypeToPayoff(
            SpreadBasket,
            ext::make_shared<PlainVanillaPayoff>(
                    Option::Call, s2->value() - s1->value())),
        ext::make_shared<EuropeanExercise>(maturity));

    const Real rho = -0.6;

    const ext::shared_ptr<GeneralizedBlackScholesProcess> bs2(
        ext::make_shared<GeneralizedBlackScholesProcess>(
            s2, dividendYield, riskFreeRate, vol2));

    const ext::shared_ptr<GeneralizedBlackScholesProcess> bs1(
        ext::make_shared<GeneralizedBlackScholesProcess>(
            s1, dividendYield, riskFreeRate, vol1));

    basketOption.setPricingEngine(
        ext::make_shared<Fd2dBlackScholesVanillaEngine>(
                bs1, bs2, rho, 11, 11, 6, 0,
                FdmSchemeDesc::Hundsdorfer(), true, 0.25));

    const Real tolerance = 0.01;
    const Real expected = 2.561;
    const Real calculated = basketOption.NPV();

    if (std::fabs(expected - calculated) > tolerance) {
        BOOST_ERROR("Failed to reproduce expected local volatility price"
                    << "\n    calculated: " << calculated
                    << "\n    expected:   " << expected
                    << "\n    tolerance:  " << tolerance);
    }
}

BOOST_AUTO_TEST_CASE(test2DPDEGreeks) {

    BOOST_TEST_MESSAGE("Testing Greeks of two-dimensional PDE engine...");

    const Real s1 = 100;
    const Real s2 = 100;
    const Real r = 0.013;
    const Volatility v = 0.2;
    const Real rho = 0.5;
    const Real strike = s1-s2;
    const Size maturityInDays = 1095;

    const DayCounter dc = Actual365Fixed();
    const Date today = Date::todaysDate();
    const Date maturity = today + maturityInDays;

    const ext::shared_ptr<SimpleQuote> spot1(
        ext::make_shared<SimpleQuote>(s1));
    const ext::shared_ptr<SimpleQuote> spot2(
        ext::make_shared<SimpleQuote>(s2));

    const Handle<YieldTermStructure> rTS(flatRate(today, r, dc));
    const Handle<BlackVolTermStructure> vTS(flatVol(today, v, dc));

    const ext::shared_ptr<BlackProcess> p1(
        ext::make_shared<BlackProcess>(Handle<Quote>(spot1), rTS, vTS));

    const ext::shared_ptr<BlackProcess> p2(
        ext::make_shared<BlackProcess>(Handle<Quote>(spot2), rTS, vTS));

    BasketOption option(
        ext::make_shared<SpreadBasketPayoff>(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, strike)),
        ext::make_shared<EuropeanExercise>(maturity));

    option.setPricingEngine(
        ext::make_shared<Fd2dBlackScholesVanillaEngine>(p1, p2, rho));

    const Real calculatedDelta = option.delta();
    const Real calculatedGamma = option.gamma();

    option.setPricingEngine(ext::make_shared<KirkEngine>(p1, p2, rho));

    const Real eps = 1.0;
    const Real npv = option.NPV();

    spot1->setValue(s1 + eps);
    spot2->setValue(s2 + eps);
    const Real npvUp = option.NPV();

    spot1->setValue(s1 - eps);
    spot2->setValue(s2 - eps);
    const Real npvDown = option.NPV();

    const Real expectedDelta = (npvUp - npvDown)/(2*eps);
    const Real expectedGamma = (npvUp + npvDown - 2*npv)/(eps*eps);

    const Real tol = 0.0005;
    if (std::fabs(expectedDelta - calculatedDelta) > tol) {
        BOOST_FAIL("failed to reproduce delta with 2dim PDE"
                   << std::fixed << std::setprecision(8)
                   << "\n    calculated: " << calculatedDelta
                   << "\n    expected:   " << expectedDelta
                   << "\n    tolerance:  " << tol);
    }

    if (std::fabs(expectedGamma - calculatedGamma) > tol) {
        BOOST_FAIL("failed to reproduce delta with 2dim PDE"
                   << std::fixed << std::setprecision(8)
                   << "\n    calculated: " << calculatedGamma
                   << "\n    expected:   " << expectedGamma
                   << "\n    tolerance:  " << tol);
    }
}


BOOST_AUTO_TEST_CASE(testBjerksundStenslandSpreadEngine) {
    BOOST_TEST_MESSAGE("Testing Bjerksund-Stensland spread engine...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(1, March, 2024);
    const Date maturity = today + Period(12, Months);

    const ext::shared_ptr<EuropeanExercise> exercise
        = ext::make_shared<EuropeanExercise>(maturity);

    const Real rho = 0.75;
    const Real f1 = 100, f2 = 110;
    const Handle<YieldTermStructure> r
        = Handle<YieldTermStructure>(flatRate(today, 0.05, dc));
    const ext::shared_ptr<BlackProcess> p1 =
        ext::make_shared<BlackProcess>(
            Handle<Quote>(ext::make_shared<SimpleQuote>(f1)), r,
            Handle<BlackVolTermStructure>(flatVol(today, 0.25, dc))
    );
    const ext::shared_ptr<BlackProcess> p2 =
        ext::make_shared<BlackProcess>(
            Handle<Quote>(ext::make_shared<SimpleQuote>(f2)), r,
            Handle<BlackVolTermStructure>(flatVol(today, 0.35, dc))
    );

    const ext::shared_ptr<PricingEngine> engine
        = ext::make_shared<BjerksundStenslandSpreadEngine>(p1, p2, rho);

    const Real strike = 5;
    BasketOption callOption(ext::make_shared<SpreadBasketPayoff>(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, strike)),
        exercise);

    callOption.setPricingEngine(engine);
    const Real callNPV = callOption.NPV();

    // reference value was calculated with python packages PyFENG 0.2.6
    const Real expectedPutNPV = 17.850835947276213;

    BasketOption putOption(ext::make_shared<SpreadBasketPayoff>(
            ext::make_shared<PlainVanillaPayoff>(Option::Put, strike)),
        exercise);

    putOption.setPricingEngine(engine);
    const Real putNPV = putOption.NPV();

    const Real tol = QL_EPSILON*100;
    Real diff = std::abs(putNPV - expectedPutNPV);

    if (diff > tol) {
        BOOST_FAIL("failed to reproduce reference put price "
                "using the Bjerksund-Stensland spread engine."
                   << std::fixed << std::setprecision(8)
                   << "\n    calculated: " << putOption.NPV()
                   << "\n    expected  : " << expectedPutNPV
                   << "\n    diff      : " << diff
                   << "\n    tolerance : " << tol);
    }

    const DiscountFactor df = r->discount(maturity);
    const Real fwd = (callNPV - putNPV)/df;
    diff = std::abs(fwd - (f1 - f2 - strike));

    if (diff > tol) {
        BOOST_FAIL("failed to reproduce call-put parity "
                "using the Bjerksund-Stensland spread engine."
                   << std::fixed << std::setprecision(8)
                   << "\n    calculated fwd: " << fwd
                   << "\n    expected fwd  : " << f1 - f2 - strike
                   << "\n    diff          : " << diff
                   << "\n    tolerance     : " << tol);
    }
}

BOOST_AUTO_TEST_CASE(testOperatorSplittingSpreadEngine) {
    BOOST_TEST_MESSAGE("Testing Strang Operator Splitting spread engine...");

    // Example taken from
    // Chi-Fai Lo, Pricing Spread Options by the Operator Splitting Method,
    // https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2429696

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(1, March, 2025);
    const Date maturity = yearFractionToDate(dc, today, 1.0);

    const Handle<YieldTermStructure> r
        = Handle<YieldTermStructure>(flatRate(today, 0.05, dc));

    const DiscountFactor df = r->discount(maturity);
    const DiscountFactor dq1 = flatRate(today, 0.03, dc)->discount(maturity);
    const DiscountFactor dq2 = flatRate(today, 0.02, dc)->discount(maturity);
    const Real f1 = 110*dq1/df, f2 = 90*dq2/df;

    const ext::shared_ptr<BlackProcess> p1 =
        ext::make_shared<BlackProcess>(
            Handle<Quote>(ext::make_shared<SimpleQuote>(f1)), r,
            Handle<BlackVolTermStructure>(flatVol(today, 0.3, dc))
    );
    const ext::shared_ptr<BlackProcess> p2 =
        ext::make_shared<BlackProcess>(
            Handle<Quote>(ext::make_shared<SimpleQuote>(f2)), r,
            Handle<BlackVolTermStructure>(flatVol(today, 0.2, dc))
    );

    BasketOption option(
        ext::make_shared<SpreadBasketPayoff>(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, 20.0)),
        ext::make_shared<EuropeanExercise>(maturity));

    const Real testData[][2] = {
            {-0.9, 18.9323},
            {-0.7, 18.0092},
            {-0.5, 17.0325},
            {-0.4, 16.5211},
            {-0.3, 15.9925},
            {-0.2, 15.4449},
            {-0.1, 14.8762},
            { 0.0, 14.284},
            { 0.1, 13.6651},
            { 0.2, 13.016},
            { 0.3, 12.3319},
            { 0.4, 11.6067},
            { 0.5, 10.8323},
            { 0.7, 9.0863},
            { 0.9, 6.9148}
    };
    for (Size i = 0; i < LENGTH(testData); ++i) {
        const Real rho = testData[i][0];
        const Real expected = testData[i][1];

        const ext::shared_ptr<PricingEngine> osEngine
            = ext::make_shared<OperatorSplittingSpreadEngine>(p1, p2, rho);

        option.setPricingEngine(osEngine);

        const Real diff = std::abs(option.NPV() - expected);
        const Real tol = 0.0001;

        if (diff > tol) {
            BOOST_FAIL("failed to reproduce reference values "
                    "using the operator splitting spread engine."
                       << std::fixed << std::setprecision(5)
                       << "\n    calculated: " << option.NPV()
                       << "\n    expected  : " << expected
                       << "\n    diff          : " << diff
                       << "\n    tolerance     : " << tol);
        }
    }
}


BOOST_AUTO_TEST_CASE(testStrangSplittingSpreadEngineVsMathematica) {
    BOOST_TEST_MESSAGE("Testing Strang Operator Splitting spread engine"
        "vs Mathematica results...");

    // Example taken from
    // Chi-Fai Lo, Pricing Spread Options by the Operator Splitting Method,
    // https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2429696
    // Reference results have been calculated with a Mathematica script.

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(27, May, 2024);

    const auto rTS = Handle<YieldTermStructure>(flatRate(today, 0.05, dc));
    const auto volTS2 = Handle<BlackVolTermStructure>(flatVol(today, 0.2, dc));

    struct TestData {
        Real T, K, vol1, rho, kirkNPV, strang1, strang2;
    };

    const TestData testCases[] = {
         {5.0, 20, 0.1, 0.6, 15.39520956886349, 15.39641179190707, 15.41992212706643},
         {10., 20, 0.1, 0.6, 22.91537136258191, 22.89480115264337, 22.95919510928365},
         {20., 20, 0.1, 0.6, 33.69859018569740, 33.59697949481467, 33.73582501903848},
         {1.0, 20, 0.3, 0.6, 10.9751711157804 , 10.97662152028116, 10.97661321814579},
         {2.0, 20, 0.3, 0.6, 15.68896063758723, 15.69277461480688, 15.69275497617036},
         {3.0, 20, 0.3, 0.6, 19.33110275816226, 19.33760645637910, 19.33758123861756},
         {4.0, 20, 0.3, 0.6, 22.40185479100672, 22.41113452093983, 22.41111679131122},
         {5.0, 20, 0.3, 0.6, 25.09737848235137, 25.10937922536118, 25.10938819057636},
         {1.0, 10, 0.3, 0.6, 16.10447007803242, 16.10494344785443, 16.10494658134660},
         {1.0, 40, 0.3, 0.6, 4.657519189575983, 4.657079657030094, 4.656973008981588},
         {1.0, 60, 0.3, 0.6, 1.837359067901817, 1.831230481909945, 1.831241843743509},
         {1.0, 20, 0.5, 0.6, 18.79838447214884, 18.79674735337080, 18.79654551825391},
         {1.0, 20, 0.3,-0.9, 20.17112122874686, 20.14780367419582, 20.15151348149147},
         {1.0, 20, 0.3, 0.0, 15.38036208157481, 15.37697052349819, 15.37728179978961},
         {2.0, 20, 0.3,-0.5, 25.80847626931109, 25.77323435009942, 25.77810550213640}
     };

    const Real s1 = 110.0, s2 = 90.0;
    const Real tol = 100*QL_EPSILON;

    for (const auto& testCase: testCases) {
        const Real rho = testCase.rho;;
        const Real strike = testCase.K;
        const Date maturityDate = yearFractionToDate(dc, today, testCase.T);
        const auto volTS1 = Handle<BlackVolTermStructure>(flatVol(today, testCase.vol1, dc));

        BasketOption option(
            ext::make_shared<SpreadBasketPayoff>(
                ext::make_shared<PlainVanillaPayoff>(Option::Call, strike)),
            ext::make_shared<EuropeanExercise>(maturityDate));

        const DiscountFactor dr = rTS->discount(maturityDate);

        const Real F1 = s1/dr;
        const Real F2 = s2/dr;

        const ext::shared_ptr<BlackProcess> p1
            = ext::make_shared<BlackProcess>(
                Handle<Quote>(ext::make_shared<SimpleQuote>(F1)), rTS, volTS1
        );

        const ext::shared_ptr<BlackProcess> p2
            = ext::make_shared<BlackProcess>(
                Handle<Quote>(ext::make_shared<SimpleQuote>(F2)), rTS, volTS2
        );

        option.setPricingEngine(ext::make_shared<KirkEngine>(p1, p2, rho));

        const Real kirkCalculated = option.NPV();
        Real diff = std::abs(testCase.kirkNPV - kirkCalculated);
        if (diff > tol*testCase.kirkNPV) {
            BOOST_FAIL("failed to reproduce Kirk reference values "
                       << std::fixed << std::setprecision(16)
                       << "\n    calculated: " << kirkCalculated
                       << "\n    expected  : " << testCase.kirkNPV
                       << "\n    diff      : " << diff
                       << "\n    tolerance : " << tol);
        }

        option.setPricingEngine(
            ext::make_shared<OperatorSplittingSpreadEngine>(
                p1, p2, rho, OperatorSplittingSpreadEngine::First)
        );

        const Real strang1Calculated = option.NPV();
        diff = std::abs(testCase.strang1 - strang1Calculated);
        if (diff > tol*testCase.strang1) {
            BOOST_FAIL("failed to reproduce Operator Splitting reference values "
                       << std::fixed << std::setprecision(16)
                       << "\n    calculated: " << strang1Calculated
                       << "\n    expected  : " << testCase.strang1
                       << "\n    diff      : " << diff
                       << "\n    tolerance : " << tol);
        }

        option.setPricingEngine(
            ext::make_shared<OperatorSplittingSpreadEngine>(
                p1, p2, rho, OperatorSplittingSpreadEngine::Second)
        );

        const Real strang2Calculated = option.NPV();
        diff = std::abs(testCase.strang2 - strang2Calculated);
        if (diff > tol*testCase.strang2) {
            BOOST_FAIL("failed to reproduce Operator Splitting reference values "
                       << std::fixed << std::setprecision(16)
                       << "\n    calculated: " << strang2Calculated
                       << "\n    expected  : " << testCase.strang2
                       << "\n    diff      : " << diff
                       << "\n    tolerance : " << tol);
        }
    }
}


BOOST_AUTO_TEST_CASE(testPDEvsApproximations) {
    BOOST_TEST_MESSAGE("Testing two-dimensional PDE engine "
            "vs analytical approximations...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(5, February, 2024);
    const Date maturity = today + Period(6, Months);

    const ext::shared_ptr<SimpleQuote> s1 = ext::make_shared<SimpleQuote>(100);
    const ext::shared_ptr<SimpleQuote> s2 = ext::make_shared<SimpleQuote>(100);

    const ext::shared_ptr<SimpleQuote> r = ext::make_shared<SimpleQuote>(0.05);

    const ext::shared_ptr<SimpleQuote> v1 = ext::make_shared<SimpleQuote>(0.25);
    const ext::shared_ptr<SimpleQuote> v2 = ext::make_shared<SimpleQuote>(0.4);

    const ext::shared_ptr<BlackProcess> p1 =
        ext::make_shared<BlackProcess>(
            Handle<Quote>(s1),
            Handle<YieldTermStructure>(flatRate(today, r, dc)),
            Handle<BlackVolTermStructure>(flatVol(today, v1, dc))
    );
    const ext::shared_ptr<BlackProcess> p2 =
        ext::make_shared<BlackProcess>(
            Handle<Quote>(s2),
            Handle<YieldTermStructure>(flatRate(today, r, dc)),
            Handle<BlackVolTermStructure>(flatVol(today, v2, dc))
    );

    const Real strike = 5;

    IncrementalStatistics statKirk, statBS2014, statOs1, statOs2;

    for (Option::Type type: {Option::Call, Option::Put}) {
        BasketOption option(
            ext::make_shared<SpreadBasketPayoff>(
                ext::make_shared<PlainVanillaPayoff>(type, strike)),
            ext::make_shared<EuropeanExercise>(maturity));

        for (Real rho: {-0.75, 0.0, 0.9}) {
            const ext::shared_ptr<PricingEngine> kirkEngine
                = ext::make_shared<KirkEngine>(p1, p2, rho);

            const ext::shared_ptr<PricingEngine> bs2014Engine
                = ext::make_shared<BjerksundStenslandSpreadEngine>(p1, p2, rho);

            const ext::shared_ptr<PricingEngine> osEngine1
                = ext::make_shared<OperatorSplittingSpreadEngine>(
                    p1, p2, rho, OperatorSplittingSpreadEngine::First);

            const ext::shared_ptr<PricingEngine> osEngine2
                = ext::make_shared<OperatorSplittingSpreadEngine>(
                    p1, p2, rho, OperatorSplittingSpreadEngine::Second);

            const ext::shared_ptr<PricingEngine> fdEngine
                = ext::make_shared<Fd2dBlackScholesVanillaEngine>(
                    p1, p2, rho, 50, 50, 15);

            for (Real rate: {0.0, 0.05, 0.2}) {
                r->setValue(rate);
                for (Real spot: {75, 90, 100, 105, 175}) {
                    s2->setValue(spot);

                    option.setPricingEngine(fdEngine);
                    const Real fdNPV = option.NPV();

                    option.setPricingEngine(kirkEngine);
                    statKirk.add(option.NPV() - fdNPV);

                    option.setPricingEngine(bs2014Engine);
                    statBS2014.add(option.NPV() - fdNPV);

                    option.setPricingEngine(osEngine1);
                    statOs1.add(option.NPV() - fdNPV);

                    option.setPricingEngine(osEngine2);
                    statOs2.add(option.NPV() - fdNPV);
                }
            }
        }
    }


    if (statKirk.standardDeviation() > 0.03) {
        BOOST_FAIL("failed to reproduce PDE spread option prices with Kirk engine."
                   << std::fixed << std::setprecision(5)
                   << "\n    stdev     : " << statKirk.standardDeviation()
                   << "\n    tolerance : " << 0.03);
    }
    if (statBS2014.standardDeviation() > 0.02) {
        BOOST_FAIL("failed to reproduce PDE spread option prices"
                " with Bjerksund-Stensland engine."
                   << std::fixed << std::setprecision(5)
                   << "\n    stdev     : " << statBS2014.standardDeviation()
                   << "\n    tolerance : " << 0.02);
    }
    if (statOs1.standardDeviation() > 0.02) {
        BOOST_FAIL("failed to reproduce PDE spread option prices"
                " with Operator-Splitting engine (first order)."
                   << std::fixed << std::setprecision(5)
                   << "\n    stdev     : " << statOs1.standardDeviation()
                   << "\n    tolerance : " << 0.02);
    }

    if (statOs2.standardDeviation() > 0.02) {
        BOOST_FAIL("failed to reproduce PDE spread option prices"
                " with Operator-Splitting engine (second order)."
                   << std::fixed << std::setprecision(5)
                   << "\n    stdev     : " << statOs2.standardDeviation()
                   << "\n    tolerance : " << 0.02);
    }
}

BOOST_AUTO_TEST_CASE(testNdimPDEvs2dimPDE) {
    BOOST_TEST_MESSAGE("Testing n-dimensional PDE engine vs two dimensional engine...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(25, February, 2024);
    const Date maturity = today + Period(6, Months);

    const ext::shared_ptr<SimpleQuote> s1 = ext::make_shared<SimpleQuote>(110);
    const ext::shared_ptr<SimpleQuote> s2 = ext::make_shared<SimpleQuote>(100);

    const Handle<YieldTermStructure> rTS
        = Handle<YieldTermStructure>(flatRate(today, 0.1, dc));

    const ext::shared_ptr<SimpleQuote> v1 = ext::make_shared<SimpleQuote>(0.5);
    const ext::shared_ptr<SimpleQuote> v2 = ext::make_shared<SimpleQuote>(0.3);

    const ext::shared_ptr<BlackProcess> p1 =
        ext::make_shared<BlackProcess>(
            Handle<Quote>(s1), rTS,
            Handle<BlackVolTermStructure>(flatVol(today, v1, dc))
    );

    const Handle<YieldTermStructure> qTS
        = Handle<YieldTermStructure>(flatRate(today, 0.075, dc));

    const ext::shared_ptr<GeneralizedBlackScholesProcess> p2 =
        ext::make_shared<GeneralizedBlackScholesProcess>(
            Handle<Quote>(s2), qTS, rTS,
            Handle<BlackVolTermStructure>(flatVol(today, v2, dc))
    );

    const Real rho = 0.75;

    const ext::shared_ptr<PricingEngine> twoDimEngine
        = ext::make_shared<Fd2dBlackScholesVanillaEngine>(p1, p2, rho, 25, 25, 15);

    const ext::shared_ptr<PricingEngine> nDimEngine
        = ext::make_shared<FdndimBlackScholesVanillaEngine>(
            std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> >({p1, p2}),
            Matrix({{1, rho}, {rho, 1}}),
            std::vector<Size>({25, 25}), 15
        );

    const Real tol = 1e-4;
    for (const auto& exercise: std::vector<ext::shared_ptr<Exercise> >(
            { ext::make_shared<EuropeanExercise>(maturity),
              ext::make_shared<AmericanExercise>(today, maturity)})) {

        for (Option::Type type: {Option::Call, Option::Put}) {
            BasketOption option(
                ext::make_shared<SpreadBasketPayoff>(
                    ext::make_shared<PlainVanillaPayoff>(type, 5)
                ),
                exercise
            );

            option.setPricingEngine(twoDimEngine);
            const Real sb2dNPV = option.NPV();

            option.setPricingEngine(nDimEngine);
            const Real sbndNPV = option.NPV();

            if (std::abs(sb2dNPV - sbndNPV) > tol) {
                BOOST_FAIL("failed to reproduce spread option prices"
                    " with multidimensional PDE engine."
                       << std::fixed << std::setprecision(5)
                       << "\n    calculated: " << sbndNPV
                       << "\n    expected:   " << sb2dNPV
                       << "\n    diff:       " << std::abs(sb2dNPV - sbndNPV)
                       << "\n    tolerance : " << tol);

            }

            BasketOption avgOption(
                ext::make_shared<AverageBasketPayoff>(
                    ext::make_shared<PlainVanillaPayoff>(type, 200), Array({1.5, 0.5})
                ),
                exercise
            );
            avgOption.setPricingEngine(twoDimEngine);
            const Real avg2dNPV =  avgOption.NPV();

            avgOption.setPricingEngine(nDimEngine);
            const Real avgndNPV = avgOption.NPV();

            if (std::abs(avg2dNPV - avgndNPV) > tol) {
                BOOST_FAIL("failed to reproduce average option prices"
                    " with multidimensional PDE engine."
                       << std::fixed << std::setprecision(5)
                       << "\n    calculated: " << avgndNPV
                       << "\n    expected:   " << avg2dNPV
                       << "\n    diff:       " << std::abs(avg2dNPV - avgndNPV)
                       << "\n    tolerance : " << tol);
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testNdimPDEinDifferentDims) {
    BOOST_TEST_MESSAGE("Testing n-dimensional PDE engine in different dimensions...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(25, February, 2024);
    const Date maturity = today + Period(6, Months);

    const std::vector<Real> underlyings({100, 50, 75, 120});
    const std::vector<Real> volatilities({0.3, 0.2, 0.6, 0.4});

    Real strike = 5.0;

    const Handle<YieldTermStructure> rTS = Handle<YieldTermStructure>(
        flatRate(today, 0.05, dc));
    const ext::shared_ptr<Exercise> exercise
        = ext::make_shared<EuropeanExercise>(maturity);

    const std::vector<Real> expected = {7.38391, 9.84887, 19.96825, 30.87275};

    std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes;
    for (Size d=1; d <= 4; ++d) {
        processes.push_back(
            ext::make_shared<BlackScholesProcess>(
                Handle<Quote>(ext::make_shared<SimpleQuote>(underlyings[d-1])),
                rTS,
                Handle<BlackVolTermStructure>(flatVol(today, volatilities[d-1], dc))
            )
        );

        strike += underlyings[d-1];

        Matrix rho(d, d);
        for (Size i=0; i < d; ++i)
            for (Size j=0; j < d; ++j)
                rho(i, j) = std::exp(-0.5*std::abs(Real(i-j)));

        BasketOption option(
            ext::make_shared<AverageBasketPayoff>(
                ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
                Array(d, 1.0)
            ),
            exercise
        );

        option.setPricingEngine(
            ext::make_shared<FdndimBlackScholesVanillaEngine>(
                processes, rho, std::vector<Size>(d, 20), 7
            )
        );

        const Real calculated = option.NPV();
        const Real diff = std::abs(calculated - expected[d-1]);
        const Real tol = 0.047;

        if (diff > tol) {
            BOOST_FAIL("failed to reproduce precalculated " << d << "-dim option price"
                   << std::fixed << std::setprecision(5)
                   << "\n    calculated: " << calculated
                   << "\n    expected:   " << expected[d-1]
                   << "\n    diff:       " << diff
                   << "\n    tolerance : " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testDengLiZhouVsPDE) {
    BOOST_TEST_MESSAGE("Testing Deng-Li-Zhou basket engine vs PDE engine...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(25, March, 2024);
    const Date maturity = today + Period(6, Months);

    const std::vector<Real> underlyings({50, 11, 55, 200});
    const std::vector<Real> volatilities({0.2, 0.6, 0.4, 0.3});
    const std::vector<Real> q({0.075, 0.05, 0.08, 0.04});

    const Handle<YieldTermStructure> rTS = Handle<YieldTermStructure>(
        flatRate(today, 0.05, dc));
    const ext::shared_ptr<Exercise> exercise = ext::make_shared<EuropeanExercise>(maturity);

    std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes;
    for (Size d=0; d < 4; ++d)
        processes.push_back(
            ext::make_shared<BlackScholesMertonProcess>(
                Handle<Quote>(ext::make_shared<SimpleQuote>(underlyings[d])),
                Handle<YieldTermStructure>(flatRate(today, q[d], dc)), rTS,
                Handle<BlackVolTermStructure>(flatVol(today, volatilities[d], dc))
            )
        );

    Matrix rho(4, 4);
    for (Size i=0; i < 4; ++i)
        for (Size j=i; j < 4; ++j)
            rho[i][j] = rho[j][i] =
                std::exp(-0.5*std::abs(Real(i)-Real(j)) - ((i!=j) ? 0.02*(i+j): 0.0));

    const Real strike = 5.0;

    BasketOption option(
        ext::make_shared<AverageBasketPayoff>(
            ext::make_shared<PlainVanillaPayoff>(Option::Put, strike),
            Array({-1.0, -5.0, -2.0, 1.0})
        ),
        exercise
    );

    option.setPricingEngine(ext::make_shared<DengLiZhouBasketEngine>(processes, rho));
    const Real calculated = option.NPV();

    option.setPricingEngine(
        ext::make_shared<FdndimBlackScholesVanillaEngine>(
            processes, rho, std::vector<Size>(4, 15), 10
        )
    );
    const Real expected = option.NPV();
    const Real diff = std::abs(calculated - expected);
    const Real tol = 0.05;

    if (diff > tol) {
        BOOST_FAIL("failed to compare basket option prices"
               << std::fixed << std::setprecision(5)
               << "\n    Deng-Li-Zhou: " << calculated
               << "\n    PDE:          " << expected
               << "\n    diff:         " << diff
               << "\n    tolerance:    " << tol);
    }
}


BOOST_AUTO_TEST_CASE(testDengLiZhouWithNegativeStrike) {
    BOOST_TEST_MESSAGE("Testing Deng-Li-Zhou basket engine with negative strike...");

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(27, May, 2024);
    const Date maturity = today + Period(6, Months);

    const std::vector<Real> underlyings({220, 105, 45, 1e-12});
    const std::vector<Real> volatilities({0.4, 0.25, 0.3, 0.25});
    const std::vector<Real> q({0.04, 0.075, 0.05, 0.1});

    const Handle<YieldTermStructure> rTS = Handle<YieldTermStructure>(
        flatRate(today, 0.03, dc));
    const ext::shared_ptr<Exercise> exercise = ext::make_shared<EuropeanExercise>(maturity);

    std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes;
    for (Size d=0; d < 4; ++d)
        processes.push_back(
            ext::make_shared<BlackScholesMertonProcess>(
                Handle<Quote>(ext::make_shared<SimpleQuote>(underlyings[d])),
                Handle<YieldTermStructure>(flatRate(today, q[d], dc)), rTS,
                Handle<BlackVolTermStructure>(flatVol(today, volatilities[d], dc))
            )
        );

    Matrix rho(4, 4, 0.0);
    rho[0][1] = rho[1][0] = 0.8;
    rho[0][2] = rho[2][0] =-0.2;
    rho[1][2] = rho[2][1] = 0.3;
    rho[0][0] = rho[1][1] = rho[2][2] = rho[3][3] = 1.0;
    rho[1][3] = rho[3][1] = 0.3;

    const Real strike = -2.0;

    BasketOption option(
        ext::make_shared<AverageBasketPayoff>(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
            Array({0.5, -2.0, 2.0, -0.75})
        ),
        exercise
    );

    option.setPricingEngine(
        ext::make_shared<DengLiZhouBasketEngine>(processes, rho));

    const Real calculated = option.NPV();
    const Real expected = 3.34412;
    const Real tol = 1e-5;
    const Real diff = std::abs(calculated - expected);

    if (diff > tol)
            BOOST_FAIL("failed to reproduce Deng-Li-Zhou value with negative strike"
                   << std::fixed << std::setprecision(5)
                   << "\n    Deng-Li-Zhou: " << calculated
                   << "\n    Expected:     " << expected
                   << "\n    diff:         " << diff
                   << "\n    tolerance:    " << tol);
}

BOOST_AUTO_TEST_CASE(testRootOfSumExponentials) {
    BOOST_TEST_MESSAGE("Testing the root of a sum of exponentials...");

    BOOST_CHECK_THROW(SingleFactorBsmBasketEngine::rootSumExponentials(
        {2.0, 3.0, 4.0}, {0.2, 0.4, -0.1}, 0.0) , Error
    );
    BOOST_CHECK_THROW(SingleFactorBsmBasketEngine::rootSumExponentials(
        {2.0, -3.0, 4.0}, {0.2, -0.4, -0.1}, 0.0), Error
    );

    const Real x = SingleFactorBsmBasketEngine::rootSumExponentials(
        {2.0, -3.0, 4.0}, {0.2, -0.4, 0.1}, 3.1);

    std::cout << x << std::endl;

}


BOOST_AUTO_TEST_CASE(testSingleFactorBsmBasketEngine) {
    BOOST_TEST_MESSAGE(
        "Testing single factor BSM basket engine against reference results...");

    // Reference results are calculated with the python library PyFENG
    // https://github.com/PyFE/PyFENG

    const DayCounter dc = Actual365Fixed();
    const Date today = Date(3, July, 2024);
    const Date maturity = today + Period(18, Months);

    const std::vector<Real> underlyings({220, 45, 102, 75});
    const std::vector<Real> volatilities({0.3, 0.45, 0.3, 0.25});
    const std::vector<Real> q({0.05, 0.075, 0.02, 0.1});

    const Handle<YieldTermStructure> rTS = Handle<YieldTermStructure>(
        flatRate(today, 0.03, dc));

    std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes;
    for (Size d=0; d < 4; ++d)
        processes.push_back(
            ext::make_shared<BlackScholesMertonProcess>(
                Handle<Quote>(ext::make_shared<SimpleQuote>(underlyings[d])),
                Handle<YieldTermStructure>(flatRate(today, q[d], dc)), rTS,
                Handle<BlackVolTermStructure>(flatVol(today, volatilities[d], dc))
            )
        );

    const Real strike = 150;

    BasketOption option(
        ext::make_shared<AverageBasketPayoff>(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, strike),
            Array({0.5, 2.0, 1.0, 1.5})
        ),
        ext::make_shared<EuropeanExercise>(maturity)
    );

    option.setPricingEngine(
        ext::make_shared<SingleFactorBsmBasketEngine>(processes));

}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
