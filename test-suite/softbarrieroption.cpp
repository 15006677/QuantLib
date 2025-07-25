/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 William Day

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

#include "utilities.hpp"
#include <ql/instruments/softbarrieroption.hpp>
#include <ql/pricingengines/analyticsoftbarrierengine.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

struct SoftBarrierOptionData {
    SoftBarrier::Type barrierType;  // (e.g., KnockOut/KnockIn)
    Option::Type type;              // Call/Put
    Real s;                         // Spot price 
    Real strike;                    // X 
    Real upperBarrier;              // U 
    Real lowerBarrier;              // L 
    Rate q;                         // Dividend yield
    Rate r;                         // Risk-free rate 
    Time t;                         // Time to maturity 
    Volatility v;                   // Volatility
    Real result;                    // Expected NPV 
    Real tol;                       // Tolerance 
};


BOOST_AUTO_TEST_CASE(SoftBarrierOptionTests) {

    BOOST_TEST_MESSAGE("Testing soft barrier option pricing against textbook values");
    
    SoftBarrierOptionData values[] = {
        /* The data below is from "The complete guide to option pricing formulas 2nd Ed",E.G. Haug p.166 

          Note: In the book, b represents the cost of carry (r-q)
        */
        
        // barrierType,       optionType,   S,   X,  U,  L,    q,   r,   T,  vol, result, tol
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 95, 0.05, 0.1, 0.5, 0.1, 3.8075, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 90, 0.05, 0.1, 0.5, 0.1, 4.0175, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 85, 0.05, 0.1, 0.5, 0.1, 4.0529, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 80, 0.05, 0.1, 0.5, 0.1, 4.0648, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 75, 0.05, 0.1, 0.5, 0.1, 4.0708, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 70, 0.05, 0.1, 0.5, 0.1, 4.0744, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 65, 0.05, 0.1, 0.5, 0.1, 4.0768, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 60, 0.05, 0.1, 0.5, 0.1, 4.0785, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 55, 0.05, 0.1, 0.5, 0.1, 4.0798, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 50, 0.05, 0.1, 0.5, 0.1, 4.0808, 1e-4 },

    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 95, 0.05, 0.1, 0.5, 0.2, 4.5263, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 90, 0.05, 0.1, 0.5, 0.2, 5.5615, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 85, 0.05, 0.1, 0.5, 0.2, 6.0394, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 80, 0.05, 0.1, 0.5, 0.2, 6.2594, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 75, 0.05, 0.1, 0.5, 0.2, 6.3740, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 70, 0.05, 0.1, 0.5, 0.2, 6.4429, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 65, 0.05, 0.1, 0.5, 0.2, 6.4889, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 60, 0.05, 0.1, 0.5, 0.2, 6.5217, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 55, 0.05, 0.1, 0.5, 0.2, 6.5463, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 50, 0.05, 0.1, 0.5, 0.2, 6.5654, 1e-4 },

    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 95, 0.05, 0.1, 0.5, 0.3, 4.7297, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 90, 0.05, 0.1, 0.5, 0.3, 6.2595, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 85, 0.05, 0.1, 0.5, 0.3, 7.2496, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 80, 0.05, 0.1, 0.5, 0.3, 7.8567, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 75, 0.05, 0.1, 0.5, 0.3, 8.2253, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 70, 0.05, 0.1, 0.5, 0.3, 8.4578, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 65, 0.05, 0.1, 0.5, 0.3, 8.6142, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 60, 0.05, 0.1, 0.5, 0.3, 8.7260, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 55, 0.05, 0.1, 0.5, 0.3, 8.8099, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 50, 0.05, 0.1, 0.5, 0.3, 8.8751, 1e-4 },

        
        // barrierType,       optionType,   S,   X,  U,  L,    q,   r,   T,  vol, result, tol
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 95, 0.05, 0.1, 1.0, 0.1, 5.4187, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 90, 0.05, 0.1, 1.0, 0.1, 6.0758, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 85, 0.05, 0.1, 1.0, 0.1, 6.2641, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 80, 0.05, 0.1, 1.0, 0.1, 6.3336, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 75, 0.05, 0.1, 1.0, 0.1, 6.3685, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 70, 0.05, 0.1, 1.0, 0.1, 6.3894, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 65, 0.05, 0.1, 1.0, 0.1, 6.4034, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 60, 0.05, 0.1, 1.0, 0.1, 6.4133, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 55, 0.05, 0.1, 1.0, 0.1, 6.4208, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 50, 0.05, 0.1, 1.0, 0.1, 6.4266, 1e-4 },

    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 95, 0.05, 0.1, 1.0, 0.2, 5.3614, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 90, 0.05, 0.1, 1.0, 0.2, 6.9776, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 85, 0.05, 0.1, 1.0, 0.2, 7.9662, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 80, 0.05, 0.1, 1.0, 0.2, 8.5432, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 75, 0.05, 0.1, 1.0, 0.2, 8.8822, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 70, 0.05, 0.1, 1.0, 0.2, 9.0931, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 65, 0.05, 0.1, 1.0, 0.2, 9.2343, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 60, 0.05, 0.1, 1.0, 0.2, 9.3353, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 55, 0.05, 0.1, 1.0, 0.2, 9.4110, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 50, 0.05, 0.1, 1.0, 0.2, 9.4698, 1e-4 },

    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 95, 0.05, 0.1, 1.0, 0.3, 5.2300, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 90, 0.05, 0.1, 1.0, 0.3, 7.2046, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 85, 0.05, 0.1, 1.0, 0.3, 8.7092, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 80, 0.05, 0.1, 1.0, 0.3, 9.8118, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 75, 0.05, 0.1, 1.0, 0.3, 10.5964, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 70, 0.05, 0.1, 1.0, 0.3, 11.1476, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 65, 0.05, 0.1, 1.0, 0.3, 11.5384, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 60, 0.05, 0.1, 1.0, 0.3, 11.8228, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 55, 0.05, 0.1, 1.0, 0.3, 12.0369, 1e-4 },
    { SoftBarrier::KnockOut, Option::Call, 100, 100, 95, 50, 0.05, 0.1, 1.0, 0.3, 12.2036, 1e-4 }
}
    };


    DayCounter dc = Actual360();  // Or Actual365Fixed() if preferred
    Date today = Date::todaysDate();
    
    ext::shared_ptr<SimpleQuote> spot(new SimpleQuote(0.0));
    ext::shared_ptr<SimpleQuote> qRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> qTS = flatRate(today, qRate, dc);
    ext::shared_ptr<SimpleQuote> rRate(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> rTS = flatRate(today, rRate, dc);
    ext::shared_ptr<SimpleQuote> vol(new SimpleQuote(0.0));
    ext::shared_ptr<BlackVolTermStructure> volTS = flatVol(today, vol, dc);

    for (auto& value : values) {
        // Update market data

        spot->setValue(value.s);
        qRate->setValue(value.q);
        rRate->setValue(value.r);
        vol->setValue(value.v);

        // Option construction
        Date exDate = today + timeToDays(value.t);
        ext::shared_ptr<Exercise> exercise(new EuropeanExercise(exDate));
        ext::shared_ptr<StrikedTypePayoff> payoff(
            new PlainVanillaPayoff(value.type, value.strike));

        // Process and pricing engine
        ext::shared_ptr<GeneralizedBlackScholesProcess> process(
            new GeneralizedBlackScholesProcess(
                Handle<Quote>(spot),
                Handle<YieldTermStructure>(qTS),
                Handle<YieldTermStructure>(rTS),
                Handle<BlackVolTermStructure>(volTS)));

        SoftBarrierOption option(value.barrierType, value.U, value.L, 
                               payoff, exercise);
        option.setPricingEngine(ext::make_shared<AnalyticSoftBarrierEngine>(process));

        // Verification
        Real calculated = option.NPV();
        Real error = std::fabs(calculated - value.result);
        if (error > value.tol) {
            REPORT_FAILURE("Soft barrier option value", 
                         value.barrierType, value.U, value.L,
                         payoff, exercise, value.spot, value.q, value.r, today,
                         value.vol, value.result, calculated, error, value.tol);
        }
    }
}
