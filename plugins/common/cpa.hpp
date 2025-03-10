#ifndef CPA_HPP
#define CPA_HPP

#include "typesmoment.hpp"
#include <QtGlobal>
#include <omp.h>

namespace SICAK {

    template <class T, class U, class V>
    //void UniFoCpaAddTraces(Moments2DContext<T>& c, const PowerTraces<U>& pt, const PowerPredictions<V>& pp) {
    void UniFoCpaAddTraces(Moments2DContext<T>& c, const U* tracesBuffer, const V* predictsBuffer, size_t noOfTraces, size_t noOfCandidates, size_t samplesPerTrace) {

        if(c.p1MOrder() != 1 || c.p1CSOrder() != 2 || c.p12ACSOrder() != 1 || c.p1MOrder() != c.p2MOrder() || c.p1CSOrder() != c.p2CSOrder())
            qCritical("Not a valid first-order univariate CPA context!");

        if (c.p1Width() != samplesPerTrace)
            qCritical("Incompatible context: Numbers of samples per trace don't match.");

        if (c.p2Width() != noOfCandidates)
            qCritical("Incompatible context: Numbers of key candidates don't match.");

        //if (pt.noOfTraces() != pp.noOfTraces())
            //throw RuntimeException("Number of power traces doesn't match the number of power predictions.");

        //const long long noOfTraces = pt.noOfTraces();
        //const long long samplesPerTrace = pt.samplesPerTrace();
        //const long long noOfCandidates = pp.noOfCandidates();

        for (size_t trace = 0; trace < noOfTraces; trace++) {

            #pragma omp parallel for
            for (size_t candidate = 0; candidate < noOfCandidates; candidate++) {

                T p_trace = static_cast<T>(c.p1Card()) / (static_cast<T>(c.p1Card()) + 1.0f);
                //T p_pp = pp(candidate, trace);
                T p_pp = *(predictsBuffer + trace*noOfCandidates + candidate);
                T p_predsAvg = c.p2M(1)(candidate);
                T p_optAlpha = p_trace * (p_pp - p_predsAvg);
                T * p_predsTracesCSum = &( c.p12ACS(1)(0, candidate) );
                //const U * p_pt = &(pt(0, trace));
                const U * p_pt = tracesBuffer + trace*samplesPerTrace;
                T * p_tracesAvg = &( c.p1M(1)(0) );

                for (size_t sample = 0; sample < samplesPerTrace; sample++) {

                    *(p_predsTracesCSum++) += p_optAlpha * (static_cast<T>(*(p_pt++)) - *(p_tracesAvg++));

                }

            }

            for (size_t candidate = 0; candidate < noOfCandidates; candidate++) {
                T temp = 0;
                //temp = (static_cast<T>(pp(candidate, trace)) - c.p2M(1)(candidate));
                temp = (static_cast<T>(*(predictsBuffer + trace*noOfCandidates + candidate)) - c.p2M(1)(candidate));
                c.p2M(1)(candidate) += (temp / static_cast<T>((c.p1Card() + 1)));
                //c.p2CS(2)(candidate) += temp * (static_cast<T>(pp(candidate, trace)) - c.p2M(1)(candidate));
                c.p2CS(2)(candidate) += temp * (static_cast<T>(*(predictsBuffer + trace*noOfCandidates + candidate)) - c.p2M(1)(candidate));
            }

            for (size_t sample = 0; sample < samplesPerTrace; sample++) {
                T temp = 0;
                //temp = (static_cast<T>(pt(sample, trace)) - c.p1M(1)(sample));
                temp = (static_cast<T>(*(tracesBuffer + trace*samplesPerTrace + sample)) - c.p1M(1)(sample));
                c.p1M(1)(sample) += (temp / static_cast<T>((c.p1Card() + 1)));
                //c.p1CS(2)(sample) += temp * (static_cast<T>(pt(sample, trace)) - c.p1M(1)(sample));
                c.p1CS(2)(sample) += temp * (static_cast<T>(*(tracesBuffer + trace*samplesPerTrace + sample)) - c.p1M(1)(sample));
            }

            c.p1Card() = c.p1Card() + 1;

        }

        c.p2Card() = c.p1Card();

    }

    template <class T>
    void UniFoCpaComputeCorrelationMatrix(const Moments2DContext<T> & c, Matrix<T> & correlations){

        if(c.p1MOrder() != 1 || c.p1CSOrder() != 2 || c.p12ACSOrder() != 1 || c.p1MOrder() != c.p2MOrder() || c.p1CSOrder() != c.p2CSOrder() || c.p1Card() != c.p2Card())
            qCritical("Not a valid first-order univariate CPA context!");

        size_t samplesPerTrace = c.p1Width();
        size_t noOfCandidates = c.p2Width();

        correlations.init(samplesPerTrace, noOfCandidates);
        Vector<T> sqrtTracesCS2(samplesPerTrace);
        Vector<T> sqrtPredsCS2(noOfCandidates);


        for (size_t sample = 0; sample < samplesPerTrace; sample++) {
            sqrtTracesCS2(sample) = sqrt(c.p1CS(2)(sample));
        }

        for (size_t candidate = 0; candidate < noOfCandidates; candidate++) {
            sqrtPredsCS2(candidate) = sqrt(c.p2CS(2)(candidate));
        }

        for (size_t candidate = 0; candidate < noOfCandidates; candidate++) {

            for (size_t sample = 0; sample < samplesPerTrace; sample++) {

                if (sqrtTracesCS2(sample) == 0 || sqrtPredsCS2(candidate) == 0) qCritical("Division by zero");

                correlations(sample, candidate) = c.p12ACS(1)(sample, candidate) / (sqrtTracesCS2(sample) * sqrtPredsCS2(candidate));

            }

        }

    }

    template <class T, class U, class V>
    //void UniHoCpaAddTraces(Moments2DContext<T>& c, const PowerTraces<U>& pt, const PowerPredictions<V>& pp, size_t attackOrder) {
    void UniHoCpaAddTraces(Moments2DContext<T>& c, const U* tracesBuffer, const V* predictsBuffer, size_t noOfTraces, size_t noOfCandidates, size_t samplesPerTrace, size_t attackOrder) {

        if(c.p1MOrder() != 1 || c.p1CSOrder() != (2 * attackOrder) || c.p2CSOrder() != 2 || c.p12ACSOrder() != attackOrder || c.p1MOrder() != c.p2MOrder())
            qCritical("Not a valid higher-order univariate CPA context!", attackOrder);

        //if (c.p1Width() != pt.samplesPerTrace())
        if (c.p1Width() != samplesPerTrace)
            qCritical("Incompatible context: Numbers of samples per trace don't match.");

        //if (c.p2Width() != pp.noOfCandidates())
        if (c.p2Width() != noOfCandidates)
            qCritical("Incompatible context: Numbers of key candidates don't match.");

        //if (pt.noOfTraces() != pp.noOfTraces())
        //    throw RuntimeException("Number of power traces doesn't match the number of power predictions.");

        if(attackOrder < 1)
            qCritical("Invalid order of the attack.");

        //const long long noOfTraces = pt.noOfTraces();
        //const long long samplesPerTrace = pt.samplesPerTrace();
        //const long long noOfCandidates = pp.noOfCandidates();

        // precomputed values
        Matrix<T> deltaT(samplesPerTrace, 2 * attackOrder);
        Matrix<T> deltaL(noOfCandidates, 1);
        Matrix<T> minusDivN(2 * attackOrder, 1);
        Matrix<T> nCr(2*attackOrder + 1, 2 * attackOrder + 1, 0);


        // precompute combination numbers
        for(size_t n = 0; n <= 2*attackOrder; n++){
            nCr(n, 0) = 1;
            for(size_t r = 1; r <= n; r++){
                nCr(n, r) = (nCr(n, r-1) * (n - r + 1)) / r;
            }
        }


        // Add every power trace
        for (size_t trace = 0; trace < noOfTraces; trace++) {

            T n = c.p1Card() + 1.0; // n = cardinality of the merged set
            T divN = 1.0 / n;

            {
                //  precompute deltaT
                T * p_deltaT = &(deltaT(0, 0));
                //const U * p_pt = &(pt(0, trace));
                const U * p_pt = tracesBuffer + trace*samplesPerTrace;
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

                // also precompute deltaL
                T * p_deltaL = &(deltaL(0, 0));
                //const V * p_pp = &(pp(0, trace));
                const V * p_pp = predictsBuffer + trace*noOfCandidates;
                const T * p_predsAvg = &( c.p2M(1)(0) );
                for (size_t candidate = 0; candidate < noOfCandidates; candidate++) {

                    (*p_deltaL++) = static_cast<T>(*p_pp++) - (*p_predsAvg++);

                }

                // precompute (-1*divN)^d
                minusDivN(0,0) = (-1.0) * divN;
                for (size_t order = 1; order < 2 * attackOrder; order++){

                    minusDivN(order, 0) = minusDivN(order-1, 0) * minusDivN(0,0);

                }

            }

            // update ACSs
            for(size_t deg = attackOrder; deg >= 1; deg--){

                const T p_beta = ( ( std::pow(-1.0, deg+1) * static_cast<T>(n-1) + std::pow(n-1, deg+1) ) / std::pow(n, deg+1) );

                #pragma omp parallel for
                for (size_t candidate = 0; candidate < noOfCandidates; candidate++) {

                    T * p_acs = &(c.p12ACS(deg)(0, candidate));
                    const T * p_deltaT = &( deltaT(0, deg - 1) );
                    const T * p_cs = (deg >= 2) ? &(c.p1CS(deg)(0)) : nullptr; // be sure not to dereference, unless deg is >= 2 !!!
                    const T p_alpha = p_beta * deltaL(candidate, 0);
                    const T p_gamma = ( (-1.0) * ( deltaL(candidate, 0) * divN ) );

                    for (size_t sample = 0; sample < samplesPerTrace; sample++) {

                        (*p_acs) += (p_alpha * (*p_deltaT++));
                        (*p_acs) += (deg >= 2) ? (p_gamma * (*p_cs++)) : 0;

                        p_acs++;

                    }

                    for(size_t p = 1; p <= deg - 1; p++){

                        p_acs = &(c.p12ACS(deg)(0, candidate));
                        const T * p_deltaTPow = &( deltaT(0, p - 1) );
                        const T * p_lessAcs = &(c.p12ACS(deg - p)(0, candidate));
                        const T * p_lessCs = (deg-p >= 2) ? &(c.p1CS(deg-p)(0)) : nullptr;
                        const T p_delta = minusDivN(p - 1, 0) * nCr(deg, p);

                        for (size_t sample = 0; sample < samplesPerTrace; sample++) {

                            T sumTerm = (*p_lessAcs++);
                            sumTerm += (deg-p >= 2) ? ( p_gamma * (*p_lessCs++) ) : 0;

                            sumTerm *= p_delta * (*p_deltaTPow++);

                            (*p_acs++) += sumTerm;

                        }

                    }

                }

            }

            // update traces CSs
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

            // update predictions CSs
            for (size_t candidate = 0; candidate < noOfCandidates; candidate++) {

                //T deltaL2 = static_cast<T>(pp(candidate, trace)) - c.p2M(1)(candidate);
                T deltaL2 = static_cast<T>(*(predictsBuffer + trace*noOfCandidates + candidate)) - c.p2M(1)(candidate);

                c.p2CS(2)(candidate) += ((deltaL2 * deltaL2) * (n-1.0)) * divN;

            }

            // update Ms
            for (size_t sample = 0; sample < samplesPerTrace; sample++) {

                //T deltaT2 = static_cast<T>(pt(sample, trace)) - c.p1M(1)(sample);
                T deltaT2 = static_cast<T>(*(tracesBuffer + trace*samplesPerTrace + sample)) - c.p1M(1)(sample);

                c.p1M(1)(sample) += deltaT2 * divN;

            }

            for (size_t candidate = 0; candidate < noOfCandidates; candidate++) {

                //T deltaL2 = static_cast<T>(pp(candidate, trace)) - c.p2M(1)(candidate);
                T deltaL2 = static_cast<T>(*(predictsBuffer + trace*noOfCandidates + candidate)) - c.p2M(1)(candidate);

                c.p2M(1)(candidate) += deltaL2 * divN;

            }

            // update Card
            c.p1Card() = c.p1Card() + 1;

        }

        c.p2Card() = c.p1Card();

    }

    template <class T>
    void UniHoCpaComputeCorrelationMatrix(const Moments2DContext<T> & c, Matrix<T> & correlations, size_t attackOrder){

        if(c.p1MOrder() != 1 || c.p1CSOrder() < attackOrder * 2 || c.p2CSOrder() != 2 || c.p12ACSOrder() < attackOrder || c.p1MOrder() != c.p2MOrder() || c.p1Card() != c.p2Card())
            qCritical("Not a valid higher-order univariate CPA context!", attackOrder);

        if(attackOrder < 1)
            qCritical("Invalid order of the attack.", attackOrder);

        size_t samplesPerTrace = c.p1Width();
        size_t noOfCandidates = c.p2Width();

        T n = c.p1Card();

        //if(n == 0) throw RuntimeException("Empty context.");
        if(n == 0) return;

        correlations.init(samplesPerTrace, noOfCandidates);

        T divN = 1.0 / n;

        Vector<T> sqrtTracesCS(samplesPerTrace);
        Vector<T> sqrtPredsCS(noOfCandidates);

        // predictions variance
        for (size_t candidate = 0; candidate < noOfCandidates; candidate++) {
            sqrtPredsCS(candidate) = std::sqrt(divN * c.p2CS(2)(candidate));
        }

        // traces variance
        if(attackOrder == 1){

            for (size_t sample = 0; sample < samplesPerTrace; sample++) {
                sqrtTracesCS(sample) = std::sqrt(divN * c.p1CS(2)(sample));
            }

        } else { // higher

            for (size_t sample = 0; sample < samplesPerTrace; sample++) {
                sqrtTracesCS(sample) = std::sqrt( c.p1CS(attackOrder*2)(sample) - ( std::pow(c.p1CS(attackOrder)(sample), 2) * divN) );
            }

        }

        for (size_t candidate = 0; candidate < noOfCandidates; candidate++) {

            for (size_t sample = 0; sample < samplesPerTrace; sample++) {

                if (sqrtTracesCS(sample) == 0 || sqrtPredsCS(candidate) == 0) qCritical("Division by zero");

                correlations(sample, candidate) = (divN * c.p12ACS(attackOrder)(sample, candidate)) / (sqrtTracesCS(sample) * sqrtPredsCS(candidate));

            }

        }

    }

}

#endif // CPA_HPP
