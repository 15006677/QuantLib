/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2004, 2005, 2006, 2007, 2008 StatPro Italia srl

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

#include <ql/settings.hpp>
#include <ql/utilities/dataparsers.hpp>

#ifdef QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER
#include "paralleltestrunner.hpp"
#else
#include <boost/test/included/unit_test.hpp>
#endif

/* Use BOOST_MSVC instead of _MSC_VER since some other vendors (Metrowerks,
   for example) also #define _MSC_VER
*/
#if !defined(BOOST_ALL_NO_LIB) && defined(BOOST_MSVC)
#  include <ql/auto_link.hpp>
#endif

#include "marketmodel.hpp"
#include "quantlibglobalfixture.hpp"
#include "riskneutraldensitycalculator.hpp"
#include "riskstats.hpp"
#include "rngtraits.hpp"
#include "rounding.hpp"
#include "sampledcurve.hpp"
#include "schedule.hpp"
#include "settings.hpp"
#include "shortratemodels.hpp"
#include "sofrfutures.hpp"
#include "solvers.hpp"
#include "spreadoption.hpp"
#include "squarerootclvmodel.hpp"
#include "stats.hpp"
#include "subperiodcoupons.hpp"
#include "svivolatility.hpp"
#include "swap.hpp"
#include "swapforwardmappings.hpp"
#include "swaption.hpp"
#include "swaptionvolatilitycube.hpp"
#include "swaptionvolatilitymatrix.hpp"
#include "swingoption.hpp"
#include "termstructures.hpp"
#include "timegrid.hpp"
#include "timeseries.hpp"
#include "tqreigendecomposition.hpp"
#include "tracing.hpp"
#include "transformedgrid.hpp"
#include "twoassetbarrieroption.hpp"
#include "twoassetcorrelationoption.hpp"
#include "ultimateforwardtermstructure.hpp"
#include "variancegamma.hpp"
#include "varianceoption.hpp"
#include "varianceswaps.hpp"
#include "volatilitymodels.hpp"
#include "vpp.hpp"
#include "xoshiro256starstar.hpp"
#include "zabr.hpp"
#include "zerocouponswap.hpp"

using namespace boost::unit_test_framework;

test_suite* init_unit_test_suite(int, char* []) {

    int argc = boost::unit_test::framework::master_test_suite().argc;
    char **argv = boost::unit_test::framework::master_test_suite().argv;
    SpeedLevel speed = speed_level(argc, argv);

    auto* test = BOOST_TEST_SUITE("QuantLib test suite");

    test->add(MarketModelTest::suite(speed));
    test->add(RiskStatisticsTest::suite());
    test->add(RngTraitsTest::suite());
    test->add(RoundingTest::suite());
    test->add(SampledCurveTest::suite());
    test->add(ScheduleTest::suite());
    test->add(SettingsTest::suite());
    test->add(ShortRateModelTest::suite(speed)); // fails with QL_USE_INDEXED_COUPON
    test->add(SofrFuturesTest::suite());
    test->add(Solver1DTest::suite());
    test->add(StatisticsTest::suite());
    test->add(SubPeriodsCouponTest::suite());
    test->add(SwapTest::suite());
    test->add(SwapForwardMappingsTest::suite());
    test->add(SwaptionTest::suite(speed));
    test->add(SwaptionVolatilityCubeTest::suite());
    test->add(SwaptionVolatilityMatrixTest::suite());
    test->add(TermStructureTest::suite());
    test->add(TimeGridTest::suite());
    test->add(TimeSeriesTest::suite());
    test->add(TqrEigenDecompositionTest::suite());
    test->add(TracingTest::suite());
    test->add(TransformedGridTest::suite());
    test->add(UltimateForwardTermStructureTest::suite());
    test->add(VarianceSwapTest::suite());
    test->add(VolatilityModelsTest::suite());
    test->add(Xoshiro256StarStarTest::suite());
    test->add(ZeroCouponSwapTest::suite());

    // tests for experimental classes
    test->add(RiskNeutralDensityCalculatorTest::experimental(speed));
    test->add(SpreadOptionTest::suite());
    test->add(SquareRootCLVModelTest::experimental());
    test->add(SviVolatilityTest::experimental());
    test->add(SwingOptionTest::suite(speed));
    test->add(TwoAssetBarrierOptionTest::suite());
    test->add(TwoAssetCorrelationOptionTest::suite());
    test->add(VarianceGammaTest::suite());
    test->add(VarianceOptionTest::suite());
    test->add(VPPTest::suite(speed));
    test->add(ZabrTest::suite(speed));

    return test;
}
