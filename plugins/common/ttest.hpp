#ifndef TTEST_HPP
#define TTEST_HPP

#include "typesmoment.hpp"
#include <QtGlobal>

namespace SICAK {

    template <class T, class U>
    void UniHoTTestAddTraces(Moments2DContext<T>& c, const U* buffer, size_t samplesPerTrace, size_t noOfTraces, size_t attackOrder) {
    //void UniHoTTestAddTraces(Moments2DContext<T>& c, const PowerTraces<U>& randTraces, size_t attackOrder) {

        if(c.p1MOrder() != 1
            || c.p1CSOrder() != 2 * attackOrder
            || c.p12ACSOrder() != 0
            )
            qFatal("Not a valid higher-order univariate t-test context!");

        if (c.p1Width() != samplesPerTrace)
            qFatal("Numbers of samples don't match.");

        if(attackOrder < 1)
            qFatal("Invalid order of the t-test.", attackOrder);

        //const long long samplesPerTrace = randTraces.samplesPerTrace();
        //const long long noOfRandTraces = randTraces.noOfTraces();

        Matrix<T> deltaT(samplesPerTrace, 2 * attackOrder);
        Matrix<T> minusDivN(2 * attackOrder, 1);
        Matrix<T> nCr(2*attackOrder + 1, 2 * attackOrder + 1, 0);

        // precompute combination numbers
        for(size_t n = 0; n <= 2*attackOrder; n++){
            nCr(n, 0) = 1;
            for(size_t r = 1; r <= n; r++){
                nCr(n, r) = (nCr(n, r-1) * (n - r + 1)) / r;
            }
        }

        // random traces
        for (size_t trace = 0; trace < noOfTraces; trace++) {

            T n = c.p1Card() + 1.0; // n = cardinality of the merged set
            T divN = 1.0 / n;

            {
                //  precompute deltaT
                T * p_deltaT = &(deltaT(0, 0));
                //const U * p_pt = &(randTraces(0, trace));
                const U * p_pt = buffer + (trace * samplesPerTrace);
                const T * p_tracesAvg = &( c.p1M(1)(0) );
                for (size_t sample = 0; sample < samplesPerTrace; sample++) {

                    (*p_deltaT++) = static_cast<T>((*p_pt++)) - (*p_tracesAvg++);

                }

                // and all its necessary powers
                for (size_t order = 1; order < 2 * attackOrder; order++){

                    const T * p_firstDeltaT = &(deltaT(0, 0));
                    const T * p_lastDeltaT = &(deltaT(0, order-1));
                    p_deltaT = &(deltaT(0, order));

                    for (size_t sample = 0; sample < samplesPerTrace; sample++) {
                        (*p_deltaT++) = (*p_lastDeltaT++) * (*p_firstDeltaT++);
                    }

                }

                // precompute (-1*divN)^d
                minusDivN(0,0) = (-1.0) * divN;
                for (size_t order = 1; order < 2 * attackOrder; order++){

                    minusDivN(order, 0) = minusDivN(order-1, 0) * minusDivN(0,0);

                }

            }

            for(size_t deg = 2 * attackOrder; deg >= 2; deg--){

                const T p_alpha = ( (n > 1) ? (1.0 - std::pow( (-1.0)/(n-1.0), deg-1 ) ) : 0 );
                const T p_beta = p_alpha * std::pow( ((n-1.0) * divN), deg);
                const T * p_deltaT = &( deltaT(0, deg - 1) );
                T * p_p1CSdeg = &(c.p1CS(deg)(0));

                for (size_t sample = 0; sample < samplesPerTrace; sample++) {

                    (*p_p1CSdeg++) += p_beta * (*p_deltaT++);

                }

                for(size_t p = 1; p <= deg - 2; p++){

                    const T * p_p1CSless = &( c.p1CS(deg-p)(0) );
                    const T p_delta = minusDivN(p - 1, 0) * nCr(deg, p);
                    const T * p_deltaTPow = &( deltaT(0, p - 1) );
                    p_p1CSdeg = &(c.p1CS(deg)(0));

                    for (size_t sample = 0; sample < samplesPerTrace; sample++) {

                        (*p_p1CSdeg++) += *(p_p1CSless++) * p_delta * (*p_deltaTPow++);

                    }

                }

            }

            for (size_t sample = 0; sample < samplesPerTrace; sample++) {

                c.p1M(1)(sample) += deltaT(sample, 0) * divN;

            }

            c.p1Card() = c.p1Card() + 1;

        }

    }

    template <class T>
    void UniHoTTestComputeTValsDegs(const Moments2DContext<T> & c1, const Moments2DContext<T> & c2, Matrix<T> & tValsDegs, size_t attackOrder){

        if(c1.p1MOrder() != 1
            || c1.p1CSOrder() < attackOrder * 2
            || c1.p12ACSOrder() != 0
            || c1.p1MOrder() != c2.p1MOrder()
            || c1.p1CSOrder() != c2.p1CSOrder()
            || c1.p1Width() != c2.p1Width()
            ){
            qFatal("Not a valid higher-order univariate t-test context!");
            return;
        }

        size_t samplesPerTrace = c1.p1Width();

        tValsDegs.init(samplesPerTrace, 2);

        T randomCardinality = c1.p1Card();
        T constCardinality = c2.p1Card();

        for(size_t sample = 0; sample < samplesPerTrace; sample++){

            T meanDelta = 0;
            T randomVariance = 1;
            T constVariance = 1;

            if(attackOrder == 1){

                T randomMean = c1.p1M(1)(sample);
                T constMean  = c2.p1M(1)(sample);
                meanDelta = constMean - randomMean;

                randomVariance = c1.p1CS(2)(sample) / randomCardinality;
                constVariance  = c2.p1CS(2)(sample) / constCardinality;

            } else if(attackOrder == 2){

                T randomMean = c1.p1CS(2)(sample) / randomCardinality;
                T constMean  = c2.p1CS(2)(sample) / constCardinality;
                meanDelta = constMean - randomMean;

                randomVariance = (c1.p1CS(4)(sample) / randomCardinality) - std::pow( c1.p1CS(2)(sample) / randomCardinality , 2 );
                constVariance  = (c2.p1CS(4)(sample) / constCardinality)  - std::pow( c2.p1CS(2)(sample) / constCardinality  , 2 );


            } else { // > 2

                T randomMean = (c1.p1CS(attackOrder)(sample) / randomCardinality) / std::pow( std::sqrt( c1.p1CS(2)(sample) / randomCardinality ) , attackOrder);
                T constMean  = (c2.p1CS(attackOrder)(sample) / constCardinality)  / std::pow( std::sqrt( c2.p1CS(2)(sample) / constCardinality  ) , attackOrder);
                meanDelta = constMean - randomMean;

                randomVariance = ( (c1.p1CS(attackOrder*2)(sample) / randomCardinality) - std::pow( c1.p1CS(attackOrder)(sample) / randomCardinality , 2) )
                                 / std::pow( c1.p1CS(2)(sample) / randomCardinality , attackOrder);
                constVariance  = ( (c2.p1CS(attackOrder*2)(sample) / constCardinality)  - std::pow( c2.p1CS(attackOrder)(sample) / constCardinality  , 2) )
                                / std::pow( c2.p1CS(2)(sample) / constCardinality  , attackOrder);

            }


            // t-values
            tValsDegs(sample, 0) = (meanDelta) / std::sqrt((constVariance / constCardinality) + (randomVariance / randomCardinality));

            T num = ( constVariance / constCardinality ) + ( randomVariance / randomCardinality );
            num = num * num;

            T den1 = ( constVariance / constCardinality );
            den1 = den1 * den1;
            den1 = den1 / ( constCardinality - 1.0);

            T den2 = ( randomVariance / randomCardinality );
            den2 = den2 * den2;
            den2 = den2 / ( randomCardinality - 1.0);

            // degrees of freedom
            tValsDegs(sample, 1) = num / (den1 + den2);

        }

    }

}

#endif // TTEST_HPP
