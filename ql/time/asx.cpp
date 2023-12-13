/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2007 StatPro Italia srl
 Copyright (C) 2004, 2005, 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni
 Copyright (C) 2015 Maddalena Zanzi

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

#include <ql/time/asx.hpp>
#include <ql/settings.hpp>
#include <ql/utilities/dataparsers.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/utility/string_view.hpp>
#include <string>
#include <cctype>

using boost::algorithm::to_upper_copy;
using std::string;

namespace QuantLib {

    namespace {
        const boost::string_view All_MONTH_CODES = "FGHJKMNQUVXZ";
    }

    bool ASX::isASXdate(const Date& date, bool mainCycle) {
        if (date.weekday()!=Friday)
            return false;

        Day d = date.dayOfMonth();
        if (d<8 || d>14)
            return false;

        if (!mainCycle) return true;

        switch (date.month()) {
          case March:
          case June:
          case September:
          case December:
            return true;
          default:
            return false;
        }
    }

    bool ASX::isASXcode(const std::string& in, bool mainCycle) {
        if (in.length() != 2)
            return false;

        // 2nd character of code needs to be digit
        if (!std::isdigit(in[1]))
            return false;

        // 1st character needs to represent the correct month
        const boost::string_view validMonthCodes = mainCycle ? "HMUZ" : All_MONTH_CODES;
        return validMonthCodes.contains(std::toupper(in[0]));
    }

    std::string ASX::code(const Date& date) {
        QL_REQUIRE(isASXdate(date, false),
                   date << " is not an ASX date");

        // month() is 1-based!
        const char monthCode = All_MONTH_CODES[date.month()-1];
        const char yearDigit = static_cast<char>(static_cast<int>('0') + (date.year() % 10));
        std::string code{monthCode, yearDigit};

        #ifdef QL_EXTRA_SAFETY_CHECKS
            QL_ENSURE(isASXcode(code, false),
                    "the result " << code <<
                    " is an invalid ASX code");
        #endif

        return code;
    }

    Date ASX::date(const std::string& asxCode,
                   const Date& refDate) {
        QL_REQUIRE(isASXcode(asxCode, false),
                   asxCode << " is not a valid ASX code");

        Date referenceDate = (refDate != Date() ?
                              refDate :
                              Date(Settings::instance().evaluationDate()));

        std::string code = to_upper_copy(asxCode);
        std::string ms = code.substr(0,1);
        QuantLib::Month m;
        if (ms=="F")      m = January;
        else if (ms=="G") m = February;
        else if (ms=="H") m = March;
        else if (ms=="J") m = April;
        else if (ms=="K") m = May;
        else if (ms=="M") m = June;
        else if (ms=="N") m = July;
        else if (ms=="Q") m = August;
        else if (ms=="U") m = September;
        else if (ms=="V") m = October;
        else if (ms=="X") m = November;
        else if (ms=="Z") m = December;
        else QL_FAIL("invalid ASX month letter");

        Year y = std::stoi(code.substr(1,1));
        /* year<1900 are not valid QuantLib years: to avoid a run-time
           exception few lines below we need to add 10 years right away */
        if (y==0 && referenceDate.year()<=1909) y+=10;
        Year referenceYear = (referenceDate.year() % 10);
        y += referenceDate.year() - referenceYear;
        Date result = ASX::nextDate(Date(1, m, y), false);
        if (result<referenceDate)
            return ASX::nextDate(Date(1, m, y+10), false);

        return result;
    }

    Date ASX::nextDate(const Date& date, bool mainCycle) {
        Date refDate = (date == Date() ?
                        Date(Settings::instance().evaluationDate()) :
                        date);
        Year y = refDate.year();
        QuantLib::Month m = refDate.month();

        Size offset = mainCycle ? 3 : 1;
        Size skipMonths = offset-(m%offset);
        if (skipMonths != offset || refDate.dayOfMonth() > 14) {
            skipMonths += Size(m);
            if (skipMonths<=12) {
                m = QuantLib::Month(skipMonths);
            } else {
                m = QuantLib::Month(skipMonths-12);
                y += 1;
            }
        }

        Date result = Date::nthWeekday(2, Friday, m, y);
        if (result<=refDate)
            result = nextDate(Date(15, m, y), mainCycle);
        return result;
    }

    Date ASX::nextDate(const std::string& ASXcode,
                       bool mainCycle,
                       const Date& referenceDate)  {
        Date asxDate = date(ASXcode, referenceDate);
        return nextDate(asxDate+1, mainCycle);
    }

    std::string ASX::nextCode(const Date& d,
                              bool mainCycle) {
        Date date = nextDate(d, mainCycle);
        return code(date);
    }

    std::string ASX::nextCode(const std::string& asxCode,
                              bool mainCycle,
                              const Date& referenceDate) {
        Date date = nextDate(asxCode, mainCycle, referenceDate);
        return code(date);
    }

}
