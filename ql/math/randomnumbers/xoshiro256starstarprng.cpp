/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2023 Ralf Konrad Eckel

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

#include <ql/math/randomnumbers/seedgenerator.hpp>
#include <ql/math/randomnumbers/xoshiro256starstarprng.hpp>

namespace QuantLib {

    Xoshiro256StarStar::Xoshiro256StarStar(unsigned long seed)
    : Xoshiro256StarStar(seed, seed, seed, seed) {}

    Xoshiro256StarStar::Xoshiro256StarStar(unsigned long s0,
                                           unsigned long s1,
                                           unsigned long s2,
                                           unsigned long s3) {
        seedInitialization(s0, s1, s2, s3);

        // using a seed needs some warm up
        for (int i = 0; i < 1000; ++i) {
            nextInt64();
        }
    }

    void Xoshiro256StarStar::seedInitialization(unsigned long s0,
                                                unsigned long s1,
                                                unsigned long s2,
                                                unsigned long s3) {
        if (s0 == 0 && s1 == 0 && s2 == 0 && s3 == 0) {
            s[0] = SeedGenerator::instance().get();
            s[1] = SeedGenerator::instance().get();
            s[2] = SeedGenerator::instance().get();
            s[3] = SeedGenerator::instance().get();
        } else {
            s[0] = s0;
            s[1] = s1;
            s[2] = s2;
            s[3] = s3;
        }
    }
}
