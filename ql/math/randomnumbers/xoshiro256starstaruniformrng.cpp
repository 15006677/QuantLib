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
#include <ql/math/randomnumbers/xoshiro256starstaruniformrng.hpp>

namespace QuantLib {

    Xoshiro256StarStarUniformRng::Xoshiro256StarStarUniformRng(unsigned long long seed)
    : Xoshiro256StarStarUniformRng(seed, seed, seed, seed) {}

    Xoshiro256StarStarUniformRng::Xoshiro256StarStarUniformRng(unsigned long long s0,
                                                               unsigned long long s1,
                                                               unsigned long long s2,
                                                               unsigned long long s3) {
        seedInitialization(s0, s1, s2, s3);

        // using a seed s0 == s1 == s2 == s3 needs some warm up
        for (int i = 0; i < 1000; ++i) {
            nextInt64();
        }
    }

    void Xoshiro256StarStarUniformRng::seedInitialization(unsigned long long s0,
                                                          unsigned long long s1,
                                                          unsigned long long s2,
                                                          unsigned long long s3) const {
        s0_ = s0;
        s1_ = s1;
        s2_ = s2;
        s3_ = s3;
        if (s0_ == 0 && s1_ == 0 && s2_ == 0 && s3_ == 0) {
            do {
                s0_ = SeedGenerator::instance().get();
                s1_ = SeedGenerator::instance().get();
                s2_ = SeedGenerator::instance().get();
                s3_ = SeedGenerator::instance().get();
            } while (s0_ == 0 || s1_ == 0 || s2_ == 0 || s3_ == 0);
        }
    }
}
