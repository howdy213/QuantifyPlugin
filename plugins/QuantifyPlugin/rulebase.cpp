#include "rulebase.h"

///
/// \brief setVariable
/// \param rec
/// \param var
/// \param delta
/// \return
///
Record setVariable(const Record &rec, const QString &var, double delta) {
    Record r = rec;
    if (var == "t1")
        r.t1 += delta;
    else if (var == "t2")
        r.t2 += delta;
    else if (var == "t3")
        r.t3 += delta;
    else if (var == "t") {
        r.t1 += delta;
        r.t2 += delta;
        r.t3 += delta;
    } else if (var == "s1")
        r.s1 += delta;
    else if (var == "s2")
        r.s2 += delta;
    else if (var == "s3")
        r.s3 += delta;
    else if (var == "s") {
        r.s1 += delta;
        r.s2 += delta;
        r.s3 += delta;
    }
    return r;
}
///
/// \brief getVariable
/// \param rec
/// \param var
/// \return
///
double getVariable(const Record &rec, const QString &var) {
    if (var == "t1")
        return rec.t1;
    if (var == "t2")
        return rec.t2;
    if (var == "t3")
        return rec.t3;
    if (var == "s1")
        return rec.s1;
    if (var == "s2")
        return rec.s2;
    if (var == "s3")
        return rec.s3;
    return 0.0;
}
