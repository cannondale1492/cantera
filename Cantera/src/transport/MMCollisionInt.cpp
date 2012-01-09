/**
 *  @file MMCollisionInt.cpp
 */
// Copyright 2001  California Institute of Technology */

#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif

#include "MMCollisionInt.h"
#include "utilities.h"
#include "polyfit.h"
#include "xml.h"
#include "XML_Writer.h"

#include <cstdio>

using namespace std;

namespace Cantera {

    const int DeltaDegree = 6;

    double MMCollisionInt::delta[8] = {0.0, 0.25, 0.50, 0.75, 1.0, 
                                       1.5, 2.0, 2.5};

    doublereal quadInterp(doublereal x0, doublereal* x, doublereal* y) {
        doublereal dx21, dx32, dx31, dy32, dy21, a;
        dx21 = x[1] - x[0];
        dx32 = x[2] - x[1];
        dx31 = dx21 + dx32;
        dy32 = y[2] - y[1];
        dy21 = y[1] - y[0];
        a = (dx21*dy32 - dy21*dx32)/(dx21*dx31*dx32);
        return a*(x0 - x[0])*(x0 - x[1]) + (dy21/dx21)*(x0 - x[1]) + y[1];
    }


    double MMCollisionInt::tstar22[37] = 
    {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0,
     1.2, 1.4, 1.6, 1.8, 2.0, 2.5, 3.0, 3.5, 4.0,
     5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 12.0, 14.0, 16.0,
     18.0, 20.0, 25.0, 30.0, 35.0, 40.0, 50.0, 75.0, 100.0};


    double MMCollisionInt::omega22_table[37*8] = {
        4.1005, 4.266,  4.833,  5.742,  6.729,  8.624,  10.34,  11.89,
        3.2626, 3.305,  3.516,  3.914,  4.433,  5.57,   6.637,  7.618,
        2.8399, 2.836,  2.936,  3.168,  3.511,  4.329,  5.126,  5.874,
        2.531,  2.522,  2.586,  2.749,  3.004,  3.64,   4.282,  4.895,
        2.2837, 2.277,  2.329,  2.46,   2.665,  3.187,  3.727,  4.249,
        2.0838, 2.081,  2.13,   2.243,  2.417,  2.862,  3.329,  3.786,
        1.922,  1.924,  1.97,   2.072,  2.225,  2.614,  3.028,  3.435,
        1.7902, 1.795,  1.84,   1.934,  2.07,   2.417,  2.788,  3.156,
        1.6823, 1.689,  1.733,  1.82,   1.944,  2.258,  2.596,  2.933,
        1.5929, 1.601,  1.644,  1.725,  1.838,  2.124,  2.435,  2.746,
        1.4551, 1.465,  1.504,  1.574,  1.67,   1.913,  2.181,  2.451,
        1.3551, 1.365,  1.4,    1.461,  1.544,  1.754,  1.989,  2.228,
        1.28,   1.289,  1.321,  1.374,  1.447,  1.63,   1.838,  2.053,
        1.2219, 1.231,  1.259,  1.306,  1.37,   1.532,  1.718,  1.912,
        1.1757, 1.184,  1.209,  1.251,  1.307,  1.451,  1.618,  1.795,
        1.0933, 1.1,    1.119,  1.15,   1.193,  1.304,  1.435,  1.578,
        1.0388, 1.044,  1.059,  1.083,  1.117,  1.204,  1.31,   1.428,
        0.99963, 1.004, 1.016,  1.035,  1.062,  1.133,  1.22,   1.319,
        0.96988, 0.9732, 0.983,  0.9991, 1.021,  1.079,  1.153,  1.236,
        0.92676, 0.9291, 0.936,  0.9473, 0.9628, 1.005,  1.058,  1.121,
        0.89616, 0.8979, 0.903,  0.9114, 0.923,  0.9545, 0.9955, 1.044,
        0.87272, 0.8741, 0.878,  0.8845, 0.8935, 0.9181, 0.9505, 0.9893,
        0.85379, 0.8549, 0.858,  0.8632, 0.8703, 0.8901, 0.9164, 0.9482,
        0.83795, 0.8388, 0.8414, 0.8456, 0.8515, 0.8678, 0.8895, 0.916,
        0.82435, 0.8251, 0.8273, 0.8308, 0.8356, 0.8493, 0.8676, 0.8901,
        0.80184, 0.8024, 0.8039, 0.8065, 0.8101, 0.8201, 0.8337, 0.8504,
        0.78363, 0.784,  0.7852, 0.7872, 0.7899, 0.7976, 0.8081, 0.8212,
        0.76834, 0.7687, 0.7696, 0.7712, 0.7733, 0.7794, 0.7878, 0.7983,
        0.75518, 0.7554, 0.7562, 0.7575, 0.7592, 0.7642, 0.7711, 0.7797,
        0.74364, 0.7438, 0.7445, 0.7455, 0.747,  0.7512, 0.7569, 0.7642,
        0.71982, 0.72,   0.7204, 0.7211, 0.7221, 0.725,  0.7289, 0.7339,
        0.70097, 0.7011, 0.7014, 0.7019, 0.7026, 0.7047, 0.7076, 0.7112,
        0.68545, 0.6855, 0.6858, 0.6861, 0.6867, 0.6883, 0.6905, 0.6932,
        0.67232, 0.6724, 0.6726, 0.6728, 0.6733, 0.6743, 0.6762, 0.6784,
        0.65099, 0.651,  0.6512, 0.6513, 0.6516, 0.6524, 0.6534, 0.6546,
        0.61397, 0.6141, 0.6143, 0.6145, 0.6147, 0.6148, 0.6148, 0.6147,
        0.5887, 0.5889, 0.5894, 0.59,   0.5903, 0.5901, 0.5895, 0.5885 
    };
    
    //-----------------------------

    // changed upper limit to 500 from 1.0e10  dgg 5/21/04
    double MMCollisionInt::tstar[39] = {
        0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0,
        1.2, 1.4, 1.6, 1.8, 2.0, 2.5, 3.0, 3.5, 4.0,
        5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 12.0, 14.0, 16.0,
        18.0, 20.0, 25.0, 30.0, 35.0, 40.0, 50.0, 75.0, 100.0, 500.0};

    double MMCollisionInt::astar_table[39*8] = {
        1.0065, 1.0840, 1.0840, 1.0840, 1.0840, 1.0840, 1.0840, 1.0840,
        1.0231, 1.0660, 1.0380, 1.0400, 1.0430, 1.0500, 1.0520, 1.0510,
        1.0424, 1.0450, 1.0480, 1.0520, 1.0560, 1.0650, 1.0660, 1.0640,
        1.0719, 1.0670, 1.0600, 1.0550, 1.0580, 1.0680, 1.0710, 1.0710,
        1.0936, 1.0870, 1.0770, 1.0690, 1.0680, 1.0750, 1.0780, 1.0780,
        1.1053, 1.0980, 1.0880, 1.0800, 1.0780, 1.0820, 1.0840, 1.0840,
        1.1104, 1.1040, 1.0960, 1.0890, 1.0860, 1.0890, 1.0900, 1.0900,
        1.1114, 1.1070, 1.1000, 1.0950, 1.0930, 1.0950, 1.0960, 1.0950,
        1.1104, 1.1070, 1.1020, 1.0990, 1.0980, 1.1000, 1.1000, 1.0990,
        1.1086, 1.1060, 1.1020, 1.1010, 1.1010, 1.1050, 1.1050, 1.1040,
        1.1063, 1.1040, 1.1030, 1.1030, 1.1040, 1.1080, 1.1090, 1.1080,
        1.1020, 1.1020, 1.1030, 1.1050, 1.1070, 1.1120, 1.1150, 1.1150,
        1.0985, 1.0990, 1.1010, 1.1040, 1.1080, 1.1150, 1.1190, 1.1200,
        1.0960, 1.0960, 1.0990, 1.1030, 1.1080, 1.1160, 1.1210, 1.1240,
        1.0943, 1.0950, 1.0990, 1.1020, 1.1080, 1.1170, 1.1230, 1.1260,
        1.0934, 1.0940, 1.0970, 1.1020, 1.1070, 1.1160, 1.1230, 1.1280,
        1.0926, 1.0940, 1.0970, 1.0990, 1.1050, 1.1150, 1.1230, 1.1300,
        1.0934, 1.0950, 1.0970, 1.0990, 1.1040, 1.1130, 1.1220, 1.1290,
        1.0948, 1.0960, 1.0980, 1.1000, 1.1030, 1.1120, 1.1190, 1.1270,
        1.0965, 1.0970, 1.0990, 1.1010, 1.1040, 1.1100, 1.1180, 1.1260,
        1.0997, 1.1000, 1.1010, 1.1020, 1.1050, 1.1100, 1.1160, 1.1230,
        1.1025, 1.1030, 1.1040, 1.1050, 1.1060, 1.1100, 1.1150, 1.1210,
        1.1050, 1.1050, 1.1060, 1.1070, 1.1080, 1.1110, 1.1150, 1.1200,
        1.1072, 1.1070, 1.1080, 1.1080, 1.1090, 1.1120, 1.1150, 1.1190,
        1.1091, 1.1090, 1.1090, 1.1100, 1.1110, 1.1130, 1.1150, 1.1190,
        1.1107, 1.1110, 1.1110, 1.1110, 1.1120, 1.1140, 1.1160, 1.1190,
        1.1133, 1.1140, 1.1130, 1.1140, 1.1140, 1.1150, 1.1170, 1.1190,
        1.1154, 1.1150, 1.1160, 1.1160, 1.1160, 1.1170, 1.1180, 1.1200,
        1.1172, 1.1170, 1.1170, 1.1180, 1.1180, 1.1180, 1.1190, 1.1200,
        1.1186, 1.1190, 1.1190, 1.1190, 1.1190, 1.1190, 1.1200, 1.1210,
        1.1199, 1.1200, 1.1200, 1.1200, 1.1200, 1.1210, 1.1210, 1.1220,
        1.1223, 1.1220, 1.1220, 1.1220, 1.1220, 1.1230, 1.1230, 1.1240,
        1.1243, 1.1240, 1.1240, 1.1240, 1.1240, 1.1240, 1.1250, 1.1250,
        1.1259, 1.1260, 1.1260, 1.1260, 1.1260, 1.1260, 1.1260, 1.1260,
        1.1273, 1.1270, 1.1270, 1.1270, 1.1270, 1.1270, 1.1270, 1.1280,
        1.1297, 1.1300, 1.1300, 1.1300, 1.1300, 1.1300, 1.1300, 1.1290,
        1.1339, 1.1340, 1.1340, 1.1350, 1.1350, 1.1340, 1.1340, 1.1320,
        1.1364, 1.1370, 1.1370, 1.1380, 1.1390, 1.1380, 1.1370, 1.1350,
        1.14187, 1.14187, 1.14187, 1.14187, 1.14187, 1.14187, 1.14187, 
        1.14187 };
    
    double MMCollisionInt::bstar_table[39*8] = {
        1.1852, 1.2963, 1.2963, 1.2963, 1.2963, 1.2963,1.2963, 1.2963,
        1.1960,  1.216,  1.237,  1.269,  1.285,  1.290,  1.297,  1.294,
        1.2451,  1.257,  1.340,  1.389,  1.366,  1.327,  1.314,  1.278,
        1.2900,  1.294,  1.272,  1.258,  1.262,  1.282,  1.290,  1.299,
        1.2986,  1.291,  1.284,  1.278,  1.277,  1.288,  1.294,  1.297,
        1.2865,  1.281,  1.276,  1.272,  1.277,  1.286,  1.292,  1.298,
        1.2665,  1.264,  1.261,  1.263,  1.269,  1.284,  1.292,  1.298,
        1.2455,  1.244,  1.248,  1.255,  1.262,  1.278,  1.289,  1.296,
        1.2253,  1.225,  1.234,  1.240,  1.252,  1.271,  1.284,  1.295,
        1.2078,  1.210,  1.216,  1.227,  1.242,  1.264,  1.281,  1.292,
        1.1919,  1.192,  1.205,  1.216,  1.230,  1.256,  1.273,  1.287,
        1.1678,  1.172,  1.181,  1.195,  1.209,  1.237,  1.261,  1.277,
        1.1496,  1.155,  1.161,  1.174,  1.189,  1.221,  1.246,  1.266,
        1.1366,  1.141,  1.147,  1.159,  1.174,  1.202,  1.231,  1.256,
        1.1270,  1.130,  1.138,  1.148,  1.162,  1.191,  1.218,  1.242,
        1.1197,  1.122,  1.129,  1.140,  1.149,  1.178,  1.205,  1.231,
        1.1080,  1.110,  1.116,  1.122,  1.132,  1.154,  1.180,  1.205,
        1.1016,  1.103,  1.107,  1.112,  1.120,  1.138,  1.160,  1.183,
        1.0980,  1.099,  1.102,  1.106,  1.112,  1.127,  1.145,  1.165,
        1.0958,  1.097,  1.099,  1.102,  1.107,  1.119,  1.135,  1.153,
        1.0935,  1.094,  1.095,  1.097,  1.100,  1.109,  1.120,  1.134,
        1.0925,  1.092,  1.094,  1.095,  1.098,  1.104,  1.112,  1.122,
        1.0922,  1.092,  1.093,  1.094,  1.096,  1.100,  1.106,  1.115,
        1.0922,  1.092,  1.093,  1.093,  1.095,  1.098,  1.103,  1.110,
        1.0923,  1.092,  1.093,  1.093,  1.094,  1.097,  1.101,  1.106,
        1.0923,  1.092,  1.092,  1.093,  1.094,  1.096,  1.099,  1.103,
        1.0927,  1.093,  1.093,  1.093,  1.094,  1.095,  1.098,  1.101,
        1.0930,  1.093,  1.093,  1.093,  1.094,  1.094,  1.096,  1.099,
        1.0933,  1.094,  1.093,  1.094,  1.094,  1.095,  1.096,  1.098,
        1.0937,  1.093,  1.094,  1.094,  1.094,  1.094,  1.096,  1.097,
        1.0939,  1.094,  1.094,  1.094,  1.094,  1.095,  1.095,  1.097,
        1.0943,  1.094,  1.094,  1.094,  1.095,  1.095,  1.096,  1.096,
        1.0944,  1.095,  1.094,  1.094,  1.094,  1.095,  1.095,  1.096,
        1.0944,  1.094,  1.095,  1.094,  1.094,  1.095,  1.096,  1.096,
        1.0943,  1.095,  1.094,  1.094,  1.095,  1.095,  1.095,  1.095,
        1.0941,  1.094,  1.094,  1.094,  1.094,  1.094,  1.094,  1.096,
        1.0947,  1.095,  1.094,  1.094,  1.093,  1.093,  1.094,  1.095,
        1.0957,  1.095,  1.094,  1.093,  1.092,  1.093,  1.093,  1.094,
        1.10185, 1.10185, 1.10185, 1.10185, 1.10185, 1.10185, 1.10185, 
        1.10185};


    double MMCollisionInt::cstar_table[39*8] = {
        0.8889,  0.77778, 0.77778,0.77778,0.77778,0.77778,0.77778,0.77778,
        0.88575, 0.8988, 0.8378, 0.8029, 0.7876, 0.7805, 0.7799, 0.7801,
        0.87268, 0.8692,0.8647,0.8479,0.8237,0.7975,0.7881,0.7784,
        0.85182, 0.8525,0.8366,0.8198,0.8054,0.7903,0.7839,0.782,
        0.83542, 0.8362,0.8306,0.8196,0.8076,0.7918,0.7842,0.7806,
        0.82629, 0.8278,0.8252,0.8169,0.8074,0.7916,0.7838,0.7802,
        0.82299, 0.8249,0.823,0.8165,0.8072,0.7922,0.7839,0.7798,
        0.82357, 0.8257,0.8241,0.8178,0.8084,0.7927,0.7839,0.7794,
        0.82657, 0.828,0.8264,0.8199,0.8107,0.7939,0.7842,0.7796,
        0.8311,  0.8234,0.8295,0.8228,0.8136,0.796,0.7854,0.7798,
        0.8363,  0.8366,0.8342,0.8267,0.8168,0.7986,0.7864,0.7805,
        0.84762, 0.8474,0.8438,0.8358,0.825,0.8041,0.7904,0.7822,
        0.85846, 0.8583,0.853,0.8444,0.8336,0.8118,0.7957,0.7854,
        0.8684,  0.8674,0.8619,0.8531,0.8423,0.8186,0.8011,0.7898,
        0.87713, 0.8755,0.8709,0.8616,0.8504,0.8265,0.8072,0.7939,
        0.88479, 0.8831,0.8779,0.8695,0.8578,0.8338,0.8133,0.799,
        0.89972, 0.8986,0.8936,0.8846,0.8742,0.8504,0.8294,0.8125,
        0.91028, 0.9089,0.9043,0.8967,0.8869,0.8649,0.8438,0.8253,
        0.91793, 0.9166,0.9125,0.9058,0.897,0.8768,0.8557,0.8372,
        0.92371, 0.9226,0.9189,0.9128,0.905,0.8861,0.8664,0.8484,
        0.93135, 0.9304,0.9274,0.9226,0.9164,0.9006,0.8833,0.8662,
        0.93607, 0.9353,0.9329,0.9291,0.924,0.9109,0.8958,0.8802,
        0.93927, 0.9387,0.9366,0.9334,0.9292,0.9162,0.905,0.8911,
        0.94149, 0.9409,0.9393,0.9366,0.9331,0.9236,0.9122,0.8997,
        0.94306, 0.9426,0.9412,0.9388,0.9357,0.9276,0.9175,0.9065,
        0.94419, 0.9437,0.9425,0.9406,0.938,0.9308,0.9219,0.9119,
        0.94571, 0.9455,0.9445,0.943,0.9409,0.9353,0.9283,0.9201,
        0.94662, 0.9464,0.9456,0.9444,0.9428,0.9382,0.9325,0.9258,
        0.94723, 0.9471,0.9464,0.9455,0.9442,0.9405,0.9355,0.9298,
        0.94764, 0.9474,0.9469,0.9462,0.945,0.9418,0.9378,0.9328,
        0.9479,  0.9478,0.9474,0.9465,0.9457,0.943,0.9394,0.9352,
        0.94827, 0.9481,0.948,0.9472,0.9467,0.9447,0.9422,0.9391,
        0.94842, 0.9484,0.9481,0.9478,0.9472,0.9458,0.9437,0.9415,
        0.94852, 0.9484,0.9483,0.948,0.9475,0.9465,0.9449,0.943,
        0.94861, 0.9487,0.9484,0.9481,0.9479,0.9468,0.9455,0.943,
        0.94872, 0.9486,0.9486,0.9483,0.9482,0.9475,0.9464,0.9452,
        0.94881, 0.9488,0.9489,0.949,0.9487,0.9482,0.9476,0.9468,
        0.94863, 0.9487,0.9489,0.9491,0.9493,0.9491,0.9483,0.9476,
        0.94444, 0.94444,0.94444,0.94444,0.94444,0.94444,0.94444,0.94444};


    void MMCollisionInt::init(XML_Writer* xml,  
        doublereal tsmin, doublereal tsmax, int log_level) {
#ifdef DEBUG_MODE
        ostream& logfile = xml->output();
        m_xml = xml;
#else
        m_xml = 0;
#endif
        m_loglevel = log_level;
#ifdef DEBUG_MODE
        if (m_loglevel > 0) {
            m_xml->XML_comment(logfile, "Collision Integral Polynomial Fits");
	}
	char p[200];
#endif
        m_nmin = -1;
        m_nmax = -1;
 
        for (int n = 0; n < 37; n++) {
            if (tsmin > tstar[n+1]) m_nmin = n;
            if (tsmax > tstar[n+1]) m_nmax = n+1;
        }
        if (m_nmin < 0 || m_nmin >= 36 || m_nmax < 0 || m_nmax > 36) {
            m_nmin = 0;
            m_nmax = 36;
        }
#ifdef DEBUG_MODE
        if (m_loglevel > 0)  {
            m_xml->XML_item(logfile, "Tstar_min", tstar[m_nmin + 1]);
            m_xml->XML_item(logfile, "Tstar_max", tstar[m_nmax + 1]);
        }
#endif
        m_logTemp.resize(37);
        doublereal rmserr, e22 = 0.0, ea = 0.0, eb = 0.0, ec = 0.0;

#ifdef DEBUG_MODE
        if (m_loglevel > 0)  {
            m_xml->XML_open(logfile, "dstar_fits");
            m_xml->XML_comment(logfile, "Collision integral fits at each "
                "tabulated T* vs. delta*.\n"
                "These polynomial fits are used to interpolate between "
                "columns (delta*)\n in the Monchick and Mason tables."
                " They are only used for nonzero delta*.");
            if (log_level < 4) {
                m_xml->XML_comment(logfile, 
                    "polynomial coefficients not printed (log_level < 4)"); 
            }
        }
#endif

        string indent = "    ";
        for (int i = 0; i < 37; i++) 
        {
            m_logTemp[i] = log(tstar[i+1]);
            vector_fp c(DeltaDegree+1);

            rmserr = fitDelta(0, i, DeltaDegree, DATA_PTR(c)); 
#ifdef DEBUG_MODE
            if (log_level > 3) {
                sprintf(p, " Tstar=\"%12.6g\"", tstar[i+1]); 
                m_xml->XML_open(logfile, "dstar_fit", p);
                m_xml->XML_item(logfile, "Tstar", tstar[i+1]);
                m_xml->XML_writeVector(logfile, indent, "omega22", 
                    c.size(), DATA_PTR(c));
            }
#endif
            m_o22poly.push_back(c);
            if (rmserr > e22) e22 = rmserr;

            rmserr = fitDelta(1, i, DeltaDegree, DATA_PTR(c));
            m_apoly.push_back(c);
#ifdef DEBUG_MODE
            if (log_level > 3)
                m_xml->XML_writeVector(logfile, indent, "astar", 
                    c.size(), DATA_PTR(c));
#endif
            if (rmserr > ea) ea = rmserr;

            rmserr = fitDelta(2, i, DeltaDegree, DATA_PTR(c));
            m_bpoly.push_back(c);
#ifdef DEBUG_MODE
            if (log_level > 3)
                m_xml->XML_writeVector(logfile, indent, "bstar", 
                    c.size(), DATA_PTR(c));
#endif
            if (rmserr > eb) eb = rmserr;

            rmserr = fitDelta(3, i, DeltaDegree, DATA_PTR(c));
            m_cpoly.push_back(c);
#ifdef DEBUG_MODE
            if (log_level > 3) {
                m_xml->XML_writeVector(logfile, indent, "cstar", 
                    c.size(), DATA_PTR(c));
	    }
#endif
            if (rmserr > ec) ec = rmserr;

#ifdef DEBUG_MODE
            if (log_level > 3) {
                m_xml->XML_close(logfile, "dstar_fit");
	    }

            if (log_level > 0) {
                sprintf(p,
                    "max RMS errors in fits vs. delta*:\n"
                    "      omega_22 =     %12.6g \n"
                    "      A*       =     %12.6g \n"
                    "      B*       =     %12.6g \n"
                    "      C*       =     %12.6g \n", e22, ea, eb, ec);
                m_xml->XML_comment(logfile, p);
                m_xml->XML_close(logfile, "dstar_fits");
            }
#endif
        }
    }

    MMCollisionInt::~MMCollisionInt() {}



    doublereal MMCollisionInt::fitDelta(int table, int ntstar, 
        int degree, doublereal* c) {
        vector_fp w(8);
        doublereal* begin = 0;
        int ndeg=0;
        switch (table) {
        case 0:
            begin = omega22_table + 8*ntstar; break;
        case 1:
            begin = astar_table + 8*(ntstar + 1); break;
        case 2:
            begin = bstar_table + 8*(ntstar + 1); break;
        case 3:
            begin = cstar_table + 8*(ntstar + 1); break;
        default:
            return 0.0;
        }
        w[0] = -1.0;
        return polyfit(8, delta, begin, DATA_PTR(w), degree, ndeg, 0.0, c);
    }

    doublereal MMCollisionInt::omega22(double ts, double deltastar) {
            int i;
            for (i = 0; i < 37; i++) if (ts < tstar22[i]) break;
            int i1, i2;
            i1 = i - 1;
            if (i1 < 0) i1 = 0;
            i2 = i1+3;
            if (i2 > 36) {
                i2 = 36;
                i1 = i2 - 3;
            }
            vector_fp values(3);
            for (i = i1; i < i2; i++) {
                if (deltastar == 0.0) values[i-i1] = omega22_table[8*i];
                else values[i-i1] = poly5(deltastar, DATA_PTR(m_o22poly[i]));
            }
            return quadInterp(log(ts), DATA_PTR(m_logTemp) 
                + i1, DATA_PTR(values));
    }

    doublereal MMCollisionInt::astar(double ts, double deltastar) {
            int i;
            for (i = 0; i < 37; i++) if (ts < tstar22[i]) break;
            int i1, i2;
            i1 = i - 1;
            if (i1 < 0) i1 = 0;
            i2 = i1+3;
            if (i2 > 36) {
                i2 = 36;
                i1 = i2 - 3;
            }
            vector_fp values(3);
            for (i = i1; i < i2; i++) {
                if (deltastar == 0.0) values[i-i1] = astar_table[8*(i + 1)];
                else values[i-i1] = poly5(deltastar, DATA_PTR(m_apoly[i]));
            }
            return quadInterp(log(ts), DATA_PTR(m_logTemp) 
                + i1, DATA_PTR(values));
        }


    doublereal MMCollisionInt::bstar(double ts, double deltastar) {
            int i;
            for (i = 0; i < 37; i++) if (ts < tstar22[i]) break;
            int i1, i2;
            i1 = i - 1;
            if (i1 < 0) i1 = 0;
            i2 = i1+3;
            if (i2 > 36) {
                i2 = 36;
                i1 = i2 - 3;
            }
            vector_fp values(3);
            for (i = i1; i < i2; i++) {
                if (deltastar == 0.0) values[i-i1] = bstar_table[8*(i + 1)];
                else values[i-i1] = poly5(deltastar, DATA_PTR(m_bpoly[i]));
            }
            return quadInterp(log(ts), DATA_PTR(m_logTemp) + i1, 
                DATA_PTR(values));
        }


    doublereal MMCollisionInt::cstar(double ts, double deltastar) {
            int i;
            for (i = 0; i < 37; i++) if (ts < tstar22[i]) break;
            int i1, i2;
            i1 = i - 1;
            if (i1 < 0) i1 = 0;
            i2 = i1+3;
            if (i2 > 36) {
                i2 = 36;
                i1 = i2 - 3;
            }
            vector_fp values(3);
            for (i = i1; i < i2; i++) {
                if (deltastar == 0.0) values[i-i1] = cstar_table[8*(i + 1)];
                else values[i-i1] = poly5(deltastar, DATA_PTR(m_cpoly[i]));
            }
            return quadInterp(log(ts), DATA_PTR(m_logTemp) + i1, 
                DATA_PTR(values));        }


    void MMCollisionInt::fit_omega22(ostream& logfile, int degree, 
        doublereal deltastar, doublereal* o22) 
    {

        int i, n = m_nmax - m_nmin + 1;
        int ndeg=0;
        string indent = "    ";
        vector_fp values(n);
        doublereal rmserr;
        vector_fp w(n);
        doublereal* logT = DATA_PTR(m_logTemp) + m_nmin;
        for (i = 0; i < n; i++) {
            if (deltastar == 0.0) values[i] = omega22_table[8*(i + m_nmin)];
            else values[i] = poly5(deltastar, DATA_PTR(m_o22poly[i+m_nmin]));
        }
        w[0]= -1.0;
        rmserr = polyfit(n, logT, DATA_PTR(values), 
            DATA_PTR(w), degree, ndeg, 0.0, o22);
#ifdef DEBUG_MODE
        if (m_loglevel > 0 && rmserr > 0.01) {
            char p[100];
            sprintf(p, "Warning: RMS error = %12.6g in omega_22 fit"
		    "with delta* = %12.6g\n", rmserr, deltastar);
            m_xml->XML_comment(logfile, p);
        }
#endif
    }

    void MMCollisionInt::fit(ostream& logfile, int degree, 
        doublereal deltastar, doublereal* a, doublereal* b, doublereal* c) 
    {
        int i, n = m_nmax - m_nmin + 1;
        int ndeg=0;
        //char s[100];
        string indent = "    ";
        vector_fp values(n);
        doublereal rmserr;
        vector_fp w(n);
        doublereal* logT = DATA_PTR(m_logTemp) + m_nmin;
        for (i = 0; i < n; i++) {
            if (deltastar == 0.0) values[i] = astar_table[8*(i + m_nmin + 1)];
            else values[i] = poly5(deltastar, DATA_PTR(m_apoly[i+m_nmin]));
        }
        w[0]= -1.0;
        rmserr = polyfit(n, logT, DATA_PTR(values), 
            DATA_PTR(w), degree, ndeg, 0.0, a);
        
        for (i = 0; i < n; i++) {
            if (deltastar == 0.0) values[i] = bstar_table[8*(i + m_nmin + 1)];
            else values[i] = poly5(deltastar, DATA_PTR(m_bpoly[i+m_nmin]));
        }
        w[0]= -1.0;
        rmserr = polyfit(n, logT, DATA_PTR(values), 
            DATA_PTR(w), degree, ndeg, 0.0, b);

        for (i = 0; i < n; i++) {
            if (deltastar == 0.0) values[i] = cstar_table[8*(i + m_nmin + 1)];
            else values[i] = poly5(deltastar, DATA_PTR(m_cpoly[i+m_nmin]));
        }
        w[0]= -1.0;
        rmserr = polyfit(n, logT, DATA_PTR(values), 
            DATA_PTR(w), degree, ndeg, 0.0, c);
#ifdef DEBUG_MODE
        if (m_loglevel > 2) {
            char p[100];
            sprintf(p, " dstar=\"%12.6g\"", deltastar); 
            m_xml->XML_open(logfile, "tstar_fit", p);

            m_xml->XML_writeVector(logfile, indent, "astar", degree+1, a);
            if (rmserr > 0.01) {
                sprintf(p, "Warning: RMS error = %12.6g for A* fit", rmserr);
                m_xml->XML_comment(logfile, p);
            }

            m_xml->XML_writeVector(logfile, indent, "bstar", degree+1, b);
            if (rmserr > 0.01) { 
                sprintf(p, "Warning: RMS error = %12.6g for B* fit", rmserr);
                m_xml->XML_comment(logfile, p);
            }

            m_xml->XML_writeVector(logfile, indent, "cstar", degree+1, c);

            if (rmserr > 0.01) { 
                sprintf(p, "Warning: RMS error = %12.6g for C* fit", rmserr);
                m_xml->XML_comment(logfile, p);
            }
            m_xml->XML_close(logfile, "tstar_fit");
        }
#endif
    }

} // namespace




